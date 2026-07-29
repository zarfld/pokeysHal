## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F006}} (REQ-F-PEV2-006: Bitmapped Field Decomposition)
- Traces to: #{{NF003}} (REQ-NF-PEV2-003: Correct Enumeration Usage)

## Context

PoKeysLib uses bitmapped fields for axis configuration (`AxesConfig[]`,
`AxesSwitchConfig[]`, `SignalConfig[]`, bitmapped status fields like `LimitStatusN`,
`HomeStatus`, `MiscInputStatus`).  LinuxCNC HAL uses individual bit-typed HAL pins.  A
decision is needed on how to map between these two representations.

## Decision

For every bitmapped field that contains axis or signal status information:

1. Expose the **raw bitmap value** as a u32 HAL pin for diagnostic access.
2. Expose **each individual bit** as a separate HAL bit pin, using the PoKeysLib
   enumeration constant name to derive the bit index.
3. For active-low or inverted signals, also expose an `*-not` pin that is the logical
   inverse of the primary pin.

**Implementation pattern**:

```c
/* Example for MiscInputStatus */
*pev2->pin_MiscInputStatus = device->PEv2.MiscInputStatus;  /* raw u32 */
for (int i = 0; i < 8; i++) {
    int val = (device->PEv2.MiscInputStatus >> i) & 1;
    *pev2->pin_digin_Misc_in[i]     = val;
    *pev2->pin_digin_Misc_in_not[i] = !val;
}
```

**Enumeration constants to use**:

| Field | Enumeration |
|-------|-------------|
| `AxesConfig[]` | `ePK_PEv2_AxisConfig` |
| `AxesSwitchConfig[]` | `ePK_PEv2_AxisSwitchOptions` (alias `ePK_PulseEngineV2_AxisSwitchOptions`) |
| `SignalConfig[]` | `ePK_PEv2_AxisSignalOptions` |
| Status fields | Bit index `N` corresponds to axis `N` |

## Alternatives Considered

### Alternative 1 — Expose only the raw bitmap

**Pros**: Minimal code.
**Cons**: Forces LinuxCNC HAL configurations to use bit-manipulation logic (not supported
in standard HAL).  **Rejected**.

### Alternative 2 — Expose only individual bit pins

**Pros**: Cleanest HAL interface.
**Cons**: Loses ability to quickly inspect full axis state at once for debugging.
**Rejected** — both raw and individual pins are provided.

## Consequences

**Positive**

- Individual bit pins are directly usable in HAL signal connections.
- Inverted (`*-not`) pins avoid needing a `not` HAL component in configurations.
- Raw bitmap pins allow efficient diagnostic viewing.

**Negative**

- Doubles the number of HAL pins for bitmapped fields.
- `export_pev2_pins()` function becomes longer.

## Traceability

- **Traces to**: #{{F006}}, #{{NF003}}
- **Implemented by**: `PoKeysLibPulseEngine_v2Async.c`
- **Verified by**: (TEST issues — add when created)
