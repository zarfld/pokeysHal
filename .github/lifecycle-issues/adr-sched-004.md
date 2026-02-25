## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F004}} (REQ-F-SCHED-004: Dirty-Flag Filtering for Digital Output Writes)
- Traces to: #{{NF002}} (REQ-NF-SCHED-002: Interface Load Distribution)

## Context

`PK_DigitalIOSetAsync()` is registered at 200 Hz.  In a typical CNC workflow, digital
outputs change at most a few times per second.  Without filtering, 200 identical 64-byte
packets are sent to the device per second for each completely static output state, wasting
~12 800 bytes/s of interface bandwidth.

## Decision

Use two `static uint8_t[7]` arrays inside `PK_DigitalIOSetAsync()` to cache the last
transmitted `dio[]` (pin values) and `mask[]` (prevent-update flags):

```c
static uint8_t last_dio_out[7]    = {0};
static uint8_t last_mask_out[7]   = {0};
static int     dio_out_initialized = 0;

/* ... build dio[] and mask[] from HAL pins ... */

if (dio_out_initialized &&
    memcmp(dio,  last_dio_out,  7) == 0 &&
    memcmp(mask, last_mask_out, 7) == 0) {
    return PK_OK;   /* nothing changed */
}
memcpy(last_dio_out, dio, 7);
memcpy(last_mask_out, mask, 7);
dio_out_initialized = 1;
/* ... send packet ... */
```

`static` storage means the cache survives across calls without dynamic allocation,
satisfying RT safety.

## Alternatives Considered

### Alternative — Dirty flag inside HAL component struct

**Pros**: Explicit ownership, multi-device capable.
**Cons**: Requires thread-safe access to HAL struct from async task context; more invasive.
Deferred — acceptable for single-device usage.

### Alternative — Compare 57 individual HAL pin bits

**Pros**: Finer granularity.
**Cons**: 57 comparisons vs. 2 × 7-byte `memcmp` — slower and error-prone.  Rejected.

## Consequences

**Positive**: Eliminates up to 200 redundant packets/s; `memcmp` is fast; RT-safe static
storage.

**Negative**: Single static cache assumes one device per process; multi-device support would
require refactoring.

## Traceability

- **Traces to**: #{{F004}}, #{{NF002}}
- **Implemented by**: `PoKeysLibIOAsync.c` (`PK_DigitalIOSetAsync()`)
- **Verified by**: (TEST issues — add when created)
