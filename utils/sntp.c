#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "net.h"
#include "pi-threads.h"
#include "wifi.h"
#include "sntp.h"

static int n_complete = 0;
static pi_mutex_t *m;
static pi_cond_t *c;

static void
run(void *arg)
{
    const char *host = (const char *) arg;
    struct timespec now;

    if (net_sntp_time(host, &now) < 0) {
	perror("net_sntp_time");
    } else {
        printf("%48s: %06lu.%06lu\n", host ? host : "<default>", (unsigned long) now.tv_sec, (unsigned long) now.tv_nsec);
    }
    pi_mutex_lock(m);
    n_complete++;
    pi_cond_signal(c);
    pi_mutex_unlock(m);
}

static struct {
    int argc;
    char **argv;
} args;

#include "time-utils.h"

static void
threads_main(int argc, char **argv)
{
    wifi_init(CYW43_HOST_NAME);
    wifi_wait_for_connection();

    m = pi_mutex_new();
    c = pi_cond_new();

    pi_thread_create("sntp", run, NULL);
    for (int i = 1; i < args.argc; i++) pi_thread_create("sntp", run, args.argv[i]);

    pi_mutex_lock(m);
    while (n_complete < args.argc) {
	pi_cond_wait(c, m);
    }
    pi_mutex_unlock(m);
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
