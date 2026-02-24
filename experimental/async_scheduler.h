/* async_scheduler.h — backward-compatibility shim
 *
 * The async scheduler has been migrated into the PoKeysLibAsync infrastructure
 * layer (PoKeysLibAsync.h / PoKeysLibAsync.c) so that any PoKeysLib**Async.c
 * subsystem file can call register_async_task() by including PoKeysLibAsync.h.
 *
 * This file is kept only so that existing code that #includes it directly
 * continues to compile without modification.
 */
#ifndef ASYNC_SCHEDULER_H
#define ASYNC_SCHEDULER_H

#include "PoKeysLibAsync.h"

#endif /* ASYNC_SCHEDULER_H */
