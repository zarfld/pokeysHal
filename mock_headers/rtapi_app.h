#ifndef __RTAPI_APP_H
#define __RTAPI_APP_H

#include "rtapi.h"

/* Mock RTAPI app header for LinuxCNC */

#define RTAPI_APP_EARLY 0
#define RTAPI_APP_MAIN 1
#define RTAPI_APP_EXIT 2

typedef int (*rtapi_app_module_init_fn_t)(void);
typedef void (*rtapi_app_module_exit_fn_t)(void);

#endif
