## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F001}} (REQ-F-PEV2-001: Global PEv2 HAL Pin Export)
- Traces to: #{{F002}} (REQ-F-PEV2-002: Per-Axis PEv2 HAL Pin Export)
- Traces to: #{{F007}} (REQ-F-PEV2-007: HAL Parameter Export)

## Context

The PEv2 HAL component has both global (device-wide) and per-axis pins and parameters.
A data structure is needed to hold all HAL pin pointers without requiring global arrays.
The architecture must align with the existing `sPoKeysHalPEv2` struct pattern already
present in `PoKeysLibHal.h`.

## Decision

Extend the existing `sPoKeysHalPEv2` struct in `PoKeysLibHal.h` to include all pin
and parameter pointers required by the PEv2 HAL interface specification.

The struct **shall** be organized in two sections:
1. **Global section**: one pointer per global pin/parameter
2. **Per-axis section**: an array of sub-structs `sPoKeysHalPEv2Axis[8]` where each
   sub-struct holds pointers for one axis

```c
typedef struct {
    /* Global HAL pins */
    hal_u32_t  *pin_nrOfAxes;
    hal_u32_t  *pin_maxPulseFrequency;
    hal_u32_t  *pin_bufferDepth;
    hal_u32_t  *pin_slotTiming;
    hal_u32_t  *pin_PulseEngineEnabled;
    /* ... all other global pins ... */

    /* Per-axis HAL pins */
    sPoKeysHalPEv2Axis axis[8];
} sPoKeysHalPEv2;

typedef struct {
    hal_s32_t  *pin_CurrentPosition;
    hal_s32_t  *pin_ReferencePositionSpeed;
    hal_bit_t  *pin_digin_LimitN_in;
    hal_bit_t  *pin_digin_LimitN_in_not;
    /* ... all other per-axis pins ... */

    /* Per-axis HAL parameters */
    hal_u32_t   param_digin_LimitN_Pin;
    hal_u32_t   param_digin_LimitP_Pin;
    /* ... all other per-axis params ... */
} sPoKeysHalPEv2Axis;
```

HAL parameters are stored by value in the struct (not pointers) since `hal_param_*_newf()`
takes a pointer to the storage location.

## Alternatives Considered

### Alternative 1 — Flat arrays indexed by axis number

**Pros**: Less struct nesting.
**Cons**: Harder to add per-axis pins; no type safety grouping.  **Rejected**.

### Alternative 2 — Separate global and axis structs as top-level pointers

**Pros**: Independent allocation.
**Cons**: More complex lifetime management; not needed given static allocation.
**Rejected**.

## Consequences

**Positive**

- All PEv2 HAL state is encapsulated in a single struct passed to helper functions.
- `sPoKeysHalPEv2Axis` clearly groups per-axis concerns.
- Easy to add new per-axis pins by extending `sPoKeysHalPEv2Axis`.

**Negative**

- `sPoKeysHalPEv2` struct becomes large (~200+ pointers).
- Requires `PoKeysLibHal.h` changes and recompilation of all dependent files.

## Traceability

- **Traces to**: #{{F001}}, #{{F002}}, #{{F007}}
- **Implemented by**: `PoKeysLibHal.h` (`sPoKeysHalPEv2`, `sPoKeysHalPEv2Axis`)
- **Verified by**: (TEST issues — add when created)
