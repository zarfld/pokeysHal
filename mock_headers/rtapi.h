#ifndef __RTAPI_H
#define __RTAPI_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* Mock RTAPI header for LinuxCNC */
typedef int rtapi_task_t;
typedef int rtapi_u32_t;
typedef int rtapi_s32_t;

#define RTAPI_TASK_PERIOD 50000000
#define RTAPI_PRIORITY_DEFAULT 0

typedef void (*rtapi_task_fn_t)(void *);

int rtapi_init(const char *name);
int rtapi_exit(int task);
int rtapi_task_new(rtapi_task_fn_t func, void *arg, int prio, int comp_id, unsigned long period, int uses_fp);
int rtapi_task_delete(int task_id);
int rtapi_task_start(int task_id);
int rtapi_task_stop(int task_id);
int rtapi_task_resume(int task_id);
int rtapi_task_pause(int task_id);

void rtapi_print(const char *fmt, ...);
void rtapi_print_msg(int level, const char *fmt, ...);
int rtapi_snprintf(char *str, size_t size, const char *fmt, ...);

enum rtapi_msg_level {
    RTAPI_MSG_NONE,
    RTAPI_MSG_ERR,
    RTAPI_MSG_WARN,
    RTAPI_MSG_INFO,
    RTAPI_MSG_DBG
};

unsigned long rtapi_get_time(void);
void rtapi_delay(unsigned long ns);

#endif
