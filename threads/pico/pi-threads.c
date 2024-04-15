#include <stdio.h>
#include "mem.h"
#include "time-utils.h"
#include "pi.h"
#include "pi-threads.h"

#include "semphr.h"

struct pi_threadS {
    TaskHandle_t task;
};

struct pi_mutexS {
    SemaphoreHandle_t m;
};

typedef struct wait_listS {
    SemaphoreHandle_t wait;
    struct wait_listS *next;
} wait_list_t;

struct pi_condS {
    pi_mutex_t *lock;
    wait_list_t *wait_list;
    wait_list_t **wait_list_tail;
};

static void
rtos_sleep(unsigned ms)
{
    //vTaskDelay(pdMS_TO_TICKS(ms));
    vTaskDelay(ms);
}

void pi_threads_init(void)
{
    pi_init_no_reboot();
    pico_set_sleep_fn(rtos_sleep);
}

void pi_threads_start_and_wait()
{
    vTaskStartScheduler();
}
int
create_task(void (*main)(void *), void *arg, TaskHandle_t *task)
{
    return xTaskCreate(main, "pi-thread", 2*1024, arg, 1, task);
}

pi_thread_t *
pi_thread_create(void (*main)(void *), void *arg)
{
    pi_thread_t *t = fatal_malloc(sizeof(*t));

    if (! create_task(main, arg, &t->task)) {
	free(t);
	return NULL;
    }
    return t;
}

void pi_thread_create_anonymous(void (*main)(void *), void *arg)
{
    create_task(main, arg, NULL);
}

pi_mutex_t *pi_mutex_new()
{
    pi_mutex_t *m = fatal_malloc(sizeof(*m));
    m->m = xSemaphoreCreateMutex();
    if (! m->m) {
	free(m);
	return NULL;
    }
    return m;
}

static void pi_thread_wait()
{
}

static void pi_thread_resume(pi_thread_t *t)
{
}

void pi_mutex_lock(pi_mutex_t *m)
{
    xSemaphoreTake(m->m, portMAX_DELAY);
}

int pi_mutex_trylock(pi_mutex_t *m)
{
    if (xSemaphoreTake(m->m, 0)) return 0;
    return -1;
}

void pi_mutex_unlock(pi_mutex_t *m)
{
    xSemaphoreGive(m->m);
}

void pi_mutex_destroy(pi_mutex_t *m)
{
    vSemaphoreDelete(m->m);
    free(m);
}

pi_cond_t *pi_cond_new()
{
    pthread_condattr_t attr;
    pi_cond_t *c = fatal_malloc(sizeof(*c));
    c->lock = pi_mutex_new();
    c->wait_list = NULL;
    c->wait_list_tail = &c->wait_list;
    return c;
}

static wait_list_t *
add_to_wait_list(pi_cond_t *c)
{
    wait_list_t *l;

    pi_mutex_lock(c->lock);
    *c->wait_list_tail = l = fatal_malloc(sizeof(*l));
    l->next = NULL;
    l->wait = xSemaphoreCreateBinary();
    pi_mutex_unlock(c->lock);

    return l;
}

static TickType_t abstime_to_ticks(const struct timespec *abstime)
{
    struct timespec now;
    nano_gettime(&now);
    int ms = nano_elapsed_ms(&now, abstime);
    return pdMS_TO_TICKS(ms);
}

int pi_cond_timedwait(pi_cond_t *c, pi_mutex_t *m, const struct timespec *abstime)
{
    wait_list_t *l = add_to_wait_list(c);
    pi_mutex_unlock(m);
    int ret = xSemaphoreTake(l->wait, abstime_to_ticks(abstime));
    if (ret) vSemaphoreDelete(l->wait);
    pi_mutex_lock(m);

    if (ret) return 0;
    else return -1;
}

void pi_cond_wait(pi_cond_t *c, pi_mutex_t *m)
{
    wait_list_t *l = add_to_wait_list(c);
    pi_mutex_unlock(m);
    xSemaphoreTake(l->wait, portMAX_DELAY);
    vSemaphoreDelete(l->wait);
    pi_mutex_lock(m);
}

void pi_cond_signal(pi_cond_t *c)
{
    pi_mutex_lock(c->lock);
    if (c->wait_list) {
	wait_list_t *w = c->wait_list;
	c->wait_list = w->next;
	if (! c->wait_list) c->wait_list_tail = &c->wait_list;
	xSemaphoreGive(w->wait);
    }
    pi_mutex_unlock(c->lock);
}

void pi_cond_destroy(pi_cond_t *c)
{
    pi_mutex_destroy(c->lock);
    while (c->wait_list) {
	wait_list_t *w = c->wait_list;
	c->wait_list = w->next;
	
	free(w);
    }
    free(c);
}

void
pi_threads_dump_state()
{
    char *state;
    state = malloc(10*1000);
    vTaskList(state);
    printf("Task Name     State    Prio    Stack    #\n");
    printf("------------- -----    ----    -----   ---\n%s", state);
}
