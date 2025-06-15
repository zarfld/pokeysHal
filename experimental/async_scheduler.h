#ifndef ASYNC_SCHEDULER_H
#define ASYNC_SCHEDULER_H

#include "PoKeysLibHal.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ASYNC_TASKS 16

typedef int (*async_func_t)(sPoKeysDevice *dev);

typedef struct {
    async_func_t func;
    sPoKeysDevice *dev;
    int64_t interval_ns;
    int64_t next_call_time;
    const char *name;
    int active;
} periodic_async_task_t;

int register_async_task(async_func_t func, sPoKeysDevice *dev, double freq_hz, const char *name);
void async_dispatcher(void);
void async_task_set_active(const char *name, int active);
size_t async_task_count(void);

#ifdef __cplusplus
}
#endif

#endif // ASYNC_SCHEDULER_H
