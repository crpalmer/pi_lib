#include <stdio.h>
#include "pi.h"
#include "mem.h"
#include "consoles.h"
#include "freertos-heap.h"
#include "pi-threads.h"
#include "set-consoles-lock.h"
#include "time-utils.h"

static void rtos_sleep(unsigned ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

const char *get_task_name() {
   return pcTaskGetName(NULL);
}

static void init_with_threads(void *main_as_vp) {
    mem_set_get_task_name(get_task_name);
    pi_init_no_reboot();
    file_init();
    char *argv = fatal_strdup("pico");
    pi_threads_main_t main = (pi_threads_main_t) main_as_vp;
    main(1, &argv);
    vTaskDelete(NULL);
}
    
#define STACK_SIZE 1024

void pi_init_with_threads(pi_threads_main_t main, int argc, char **argv) {
    malloc_lock_init();
    xTaskCreate(init_with_threads, "main", STACK_SIZE, (void *) main, 1, NULL);
    pico_set_sleep_fn(rtos_sleep);
    set_consoles_lock();
    vTaskStartScheduler();
}

PiThread::PiThread(const char *name) : name(name) {
}

PiThread *PiThread::start(int priority) {
    xTaskCreate(PiThread::thread_entry, name ? name : "pi-thread", STACK_SIZE, this, priority, NULL);
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

bool PiMutex::trylock() {
    return xSemaphoreTake(m, 0);
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

static const char *task_state_to_str(eTaskState state) {
    switch (state) {
    case eReady: return "rdy";
    case eRunning: return "run";
    case eBlocked: return "blkd";
    case eSuspended: return "susp";
    case eDeleted: return "del";
    case eInvalid: return "invl";
    }
    assert(0);
    return NULL;
}

#include <malloc.h>

static uint32_t getTotalHeap(void) {
   extern char __StackLimit, __bss_end__;

   return &__StackLimit  - &__bss_end__;
}

size_t pi_threads_get_free_ram(void) {
   struct mallinfo m = mallinfo();

   return getTotalHeap() - m.uordblks;
}

void
pi_threads_dump_state() {
    int n_tasks = uxTaskGetNumberOfTasks() + 2;	// just in case somehow 2 get created between calls!
    TaskStatus_t *status = (TaskStatus_t *) fatal_malloc(sizeof(*status) * n_tasks);
    unsigned long total_run_time;
    n_tasks = uxTaskGetSystemState(status, n_tasks, &total_run_time);

    consoles_printf(" #  Task Name        State Prio Stack\n");
    consoles_printf("--- ---------------- ----- ---- -----\n");
    for (int i = 0; i < n_tasks; i++) {
	consoles_printf("%3d %-16s %-5s %4d %4ld\n", status[i].xTaskNumber, status[i].pcTaskName, task_state_to_str(status[i].eCurrentState), status[i].uxCurrentPriority, status[i].usStackHighWaterMark);
    }
    fatal_free(status);
}

void
pi_thread_asserted(const char *expr, const char *filename, int line)
{
    printf("ASSERTION FAILED: %s @ %s : %d\n", expr, filename, line);
    fflush(stderr);
    pi_reboot_bootloader();
}
