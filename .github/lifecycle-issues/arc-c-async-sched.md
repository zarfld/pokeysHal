## Architecture Component

**Status**: Specified

**Based on ADRs**:
- #{{ADR1}} (ADR-SCHED-001: Four-Level Priority Classification)
- #{{ADR2}} (ADR-SCHED-002: Load Threshold Table)
- #{{ADR3}} (ADR-SCHED-003: Prime-Number Stagger)
- #{{ADR4}} (ADR-SCHED-004: Static Dirty-Flag)
- #{{ADR5}} (ADR-SCHED-005: Machine-On Lock)

## Component Purpose

**Responsibility**: Provide a priority-aware, load-sensitive periodic task dispatcher for
all PoKeys async subsystem functions.  The scheduler owns the firing cadence of 14
registered tasks, ensuring real-time position feedback is never delayed regardless of device
load, interface congestion, or machine state.

**Scope**:

- Task registration with priority and prime-number-staggered initial offset
- Single-task-per-call dispatching (most-overdue eligible task)
- Load-based throttling (four priority thresholds)
- Machine-on suppression of LOW-priority tasks
- System load monitoring via protocol command 0x05

**Out of Scope**:

- Async mailbox / request-response protocol (handled by `PoKeysLibAsync.c` mailbox layer)
- HAL pin registration (handled by `PoKeysLib**Async.c` subsystem files)
- RT thread scheduling (handled by LinuxCNC RTAPI / PREEMPT_RT)

## Interfaces

### C API (declared in `PoKeysLibAsync.h`)

```c
/* Register a periodic task */
int register_async_task(async_func_t func, sPoKeysDevice *dev,
                        double freq_hz, const char *name,
                        task_priority_t priority);

/* Fire the single most-overdue eligible task */
int async_dispatcher(void);

/* Enable / disable a named task */
void async_task_set_active(const char *name, int active);

/* Return number of registered tasks */
size_t async_task_count(void);

/* Machine-on state control */
void scheduler_set_machine_on(int machine_on);

/* System load accessors */
uint8_t scheduler_get_system_load(void);
void    scheduler_update_system_load(uint8_t cpu_load);
```

### Key Data Structure

```c
typedef struct {
    async_func_t    func;
    sPoKeysDevice  *dev;
    int64_t         interval_ns;
    int64_t         next_call_time;
    const char     *name;
    int             active;
    task_priority_t priority;
} periodic_async_task_t;
```

## Dependencies

**Internal**:

- `PoKeysLibAsync.h` / `PoKeysLibAsync.c` — mailbox, `rtapi_get_time()`
- `PoKeysLib**Async.c` subsystem files — call `register_async_task()`
- `experimental/pokeys_async.c` — drives `async_dispatcher()` each RT cycle

**External**:

- LinuxCNC RTAPI (`rtapi_get_time`, `rtapi_print_msg`)

## Technology Stack

- Language: C99
- Build: `make -f Makefile.noqmake` / `halcompile`
- No dynamic allocation; no OS threading primitives

## Traceability

- **Based on**: #{{ADR1}}, #{{ADR2}}, #{{ADR3}}, #{{ADR4}}, #{{ADR5}}
- **Traces to**: #{{F001}}, #{{F002}}, #{{F003}}, #{{F004}}, #{{F005}}, #{{F006}}, #{{NF001}}, #{{NF002}}, #{{NF003}}
- **Implemented by**: `PoKeysLibAsync.c` (core scheduler), `PoKeysLibIOAsync.c` (dirty-flag), `experimental/pokeys_async.c` (wiring)
- **Verified by**: (TEST issues — add when created)
