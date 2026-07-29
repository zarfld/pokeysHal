## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F004}} (REQ-F-PEV2-004: Input Pin to PoKeysLib Pointer Update)
- Traces to: #{{F005}} (REQ-F-PEV2-005: PoKeysLib Pointer to Output Pin Update)
- Traces to: #{{F008}} (REQ-F-PEV2-008: PoKeysLib Function Call Sequence)
- Traces to: #{{NF001}} (REQ-NF-PEV2-001: Real-Time Safety)

## Context

Two distinct data flows exist in the PEv2 HAL interface:
1. **Device → HAL**: Device status retrieved from PoKeysLib is pushed to HAL output pins.
2. **HAL → Device**: HAL input pin values are written to PoKeysLib struct fields before
   sending commands.

A clear architectural separation is needed to ensure these flows are correctly ordered
within the servo cycle and that no RT violations occur.

## Decision

Separate the two flows into two dedicated functions:

```c
/**
 * Called AFTER PK_PEv2_StatusGetAsync response is parsed.
 * Reads device->PEv2.* and writes HAL output pins.
 */
static void pev2_read_device_to_hal(sPoKeysHalPEv2 *pev2, sPoKeysDevice *dev);

/**
 * Called BEFORE PK_PEv2_*SetAsync / PK_PEv2_*MoveAsync calls.
 * Reads HAL input pins and writes device->PEv2.*.
 */
static void pev2_write_hal_to_device(sPoKeysHalPEv2 *pev2, sPoKeysDevice *dev);
```

The servo-thread call order is:

```
1. pev2_write_hal_to_device()          /* apply HAL inputs to device struct */
2. PK_PEv2_*SetAsync() / MoveAsync()   /* send updated values to device */
3. PK_PEv2_StatusGetAsync()            /* request new status */
4. [async response processed later]
5. pev2_read_device_to_hal()           /* update HAL outputs from latest status */
```

## Alternatives Considered

### Alternative 1 — Single combined update function

**Pros**: Fewer function calls.
**Cons**: Interleaved read/write logic is error-prone and harder to reason about in RT
context.  **Rejected**.

### Alternative 2 — Inline updates alongside each PoKeysLib call

**Pros**: Co-located logic.
**Cons**: RT unsafe if PoKeysLib blocking variants are accidentally called; harder to
audit for RT violations.  **Rejected**.

## Consequences

**Positive**

- Clear separation of read and write phases.
- Easy to audit each function for RT violations independently.
- Correct ordering guaranteed: inputs applied before sends, outputs updated after receives.

**Negative**

- Two traversals of the axis array per servo cycle (one for read, one for write).

## Traceability

- **Traces to**: #{{F004}}, #{{F005}}, #{{F008}}, #{{NF001}}
- **Implemented by**: `PoKeysLibPulseEngine_v2Async.c`
- **Verified by**: (TEST issues — add when created)
