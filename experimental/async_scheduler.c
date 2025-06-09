// async_scheduler.c
#include "async_scheduler.h"
#include "rtapi.h"
#include "rtapi_string.h"
#include <string.h>

static periodic_async_task_t async_tasks[MAX_ASYNC_TASKS];
static size_t task_count = 0;

int register_async_task(async_func_t func, sPoKeysDevice *dev, double freq_hz, const char *name) {
    if (task_count >= MAX_ASYNC_TASKS || freq_hz <= 0.0) return -1;

    int64_t now = rtapi_get_time();
    int64_t interval = (int64_t)(1e9 / freq_hz);

    async_tasks[task_count++] = (periodic_async_task_t){
        .func = func,
        .dev = dev,
        .interval_ns = interval,
        .next_call_time = now + interval,
        .name = name,
        .active = 1
    };
    return 0;
}

void async_dispatcher(void) {
    int64_t now = rtapi_get_time();
    size_t selected_index = SIZE_MAX;
    int64_t earliest_due = INT64_MAX;

    for (size_t i = 0; i < task_count; ++i) {
        if (!async_tasks[i].active) continue;
        if (async_tasks[i].next_call_time <= now && async_tasks[i].next_call_time < earliest_due) {
            selected_index = i;
            earliest_due = async_tasks[i].next_call_time;
        }
    }

    if (selected_index != SIZE_MAX) {
        periodic_async_task_t *t = &async_tasks[selected_index];
        int ret = t->func(t->dev);
        t->next_call_time = now + t->interval_ns;

        if (ret) {
            rtapi_print_msg(RTAPI_MSG_ERR,
                "Async call %s FAILED (ret=%d)\n", t->name, ret);
        }
    }
}

void async_task_set_active(const char *name, int active) {
    for (size_t i = 0; i < task_count; ++i) {
        if (strcmp(async_tasks[i].name, name) == 0) {
            async_tasks[i].active = (active != 0);
            return;
        }
    }
}

size_t async_task_count(void) {
    return task_count;
}
