#include <stdio.h>
#include "mem.h"
#include "consoles.h"
#include "time-utils.h"
#include "pi.h"
#include "pi-threads.h"
#include "set-consoles-lock.h"

static void
rtos_sleep(unsigned ms) {
    //vTaskDelay(pdMS_TO_TICKS(ms));
    vTaskDelay(ms);
}

static void init_with_threads(void *main_as_vp) {
    pi_init_no_reboot();
    file_init();
    ms_sleep(2000);

    char *argv = fatal_strdup("pico");
    pi_threads_main_t main = (pi_threads_main_t) main_as_vp;
    main(1, &argv);

    vTaskDelete(NULL);
}
    
#define STACK_SIZE 2048

void pi_init_with_threads(pi_threads_main_t main, int argc, char **argv) {
    xTaskCreate(init_with_threads, "main", STACK_SIZE, (void *) main, 1, NULL);
    pico_set_sleep_fn(rtos_sleep);
    set_consoles_lock();
    vTaskStartScheduler();
}

PiThread::PiThread(const char *name) : name(name) {
}

PiThread *PiThread::start() {
    xTaskCreate(PiThread::thread_entry, name ? name : "pi-thread", STACK_SIZE, this, 1, NULL);
    return this;
}

PiThread::~PiThread() {
}

PiMutex::PiMutex() {
    m = xSemaphoreCreateMutex();
}

PiMutex::~PiMutex() {
    vSemaphoreDelete(m);
}

void PiMutex::lock() {
    xSemaphoreTake(m, portMAX_DELAY);
}

int PiMutex::trylock() {
    if (xSemaphoreTake(m, 0)) return 0;
    return -1;
}

void PiMutex::unlock() {
    xSemaphoreGive(m);
}

PiCond::PiCond() {
    lock = new PiMutex();
}

PiCond::~PiCond() {
    broadcast();
    delete lock;
}

static TickType_t abstime_to_ticks(const struct timespec *abstime) {
    struct timespec now;
    nano_gettime(&now);
    int ms = nano_elapsed_ms(&now, abstime);
    return pdMS_TO_TICKS(ms);
}

SemaphoreHandle_t PiCond::add_to_wait_list() {
    SemaphoreHandle_t waiting = xSemaphoreCreateBinary();

    lock->lock();
    wait_list.push_back(waiting);
    lock->unlock();

    return waiting;
}

int PiCond::timedwait(PiMutex *m, const struct timespec *abstime) {
    SemaphoreHandle_t waiting = add_to_wait_list();

    m->unlock();

    int ret = xSemaphoreTake(waiting, abstime_to_ticks(abstime));
    vSemaphoreDelete(waiting);

    m->lock();

    if (ret) return 0;
    else return -1;
}

void PiCond::wait(PiMutex *m) {
    SemaphoreHandle_t waiting = add_to_wait_list();

    m->unlock();

    xSemaphoreTake(waiting, portMAX_DELAY);
    vSemaphoreDelete(waiting);

    m->lock();
}

void PiCond::signal() {
    lock->lock();

    if (! wait_list.empty()) {
	SemaphoreHandle_t waiting = wait_list.front();
	wait_list.pop_front();
	xSemaphoreGive(waiting);
    }

    lock->unlock();
}

void PiCond::broadcast() {
    lock->lock();

    while (! wait_list.empty()) {
	SemaphoreHandle_t waiting = wait_list.front();
	wait_list.pop_front();
	xSemaphoreGive(waiting);
    }

    lock->unlock();
}

#include <malloc.h>

static uint32_t getTotalHeap(void) {
   extern char __StackLimit, __bss_end__;

   return &__StackLimit  - &__bss_end__;
}

static uint32_t getFreeHeap(void) {
   struct mallinfo m = mallinfo();

   return getTotalHeap() - m.uordblks;
}

char *
pi_threads_get_state()
{
    char *state;
    state = (char *) fatal_malloc(10*1048);
    sprintf(state, "Free heap memory: %d\n", (int) getFreeHeap());
    vTaskList(&state[strlen(state)]);
    return state;
}

void
pi_threads_dump_state()
{
    char *state = pi_threads_get_state();

    printf("Task Name     State    Prio    Stack    #\n");
    printf("------------- -----    ----    -----   ---\n%s", state);
    free(state);
}

void
pi_thread_asserted(const char *expr, const char *filename, int line)
{
    printf("ASSERTION FAILED: %s @ %s : %d\n", expr, filename, line);
    fflush(stderr);
    pi_reboot_bootloader();
}
