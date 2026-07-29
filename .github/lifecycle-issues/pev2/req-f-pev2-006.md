## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

Bitmapped PEv2 fields **shall** be decomposed into individual HAL pins using the correct
PoKeysLib enumerations.  The HAL component **shall not** expose raw bitmaps as the only
interface; individual bit-level pins **shall** be provided.

### Required decompositions

#### `AxesConfig[N]` → per-axis config flags (using `ePK_PEv2_AxisConfig`)

| Bit | Constant | HAL Parameter or Pin |
|-----|----------|---------------------|
| `(1<<0)` | `PK_AC_ENABLED` | `PEv2.[N].AxesConfig` bit 0 |
| `(1<<1)` | `PK_AC_INVERTED` | axis direction inverted |
| `(1<<2)` | `PK_AC_INTERNAL_PLANNER` | internal planner enabled |
| `(1<<3)` | `PK_AC_POSITION_MODE` | position mode |
| `(1<<4)` | `PK_AC_INVERTED_HOME` | home direction inverted |
| `(1<<5)` | `PK_AC_SOFT_LIMIT_ENABLED` | soft limits enabled |
| `(1<<6)` | `PK_AC_FAST_OUTPUT` | fast output mode |
| `(1<<7)` | `PK_AC_ENABLED_MASKED` | output enable pin masking |

#### `AxesSwitchConfig[N]` → per-axis switch flags (using `ePK_PEv2_AxisSwitchOptions`)

| Bit | Constant | HAL Signal |
|-----|----------|------------|
| `(1<<0)` | `PK_ASO_SWITCH_LIMIT_N` | limit- switch connected |
| `(1<<1)` | `PK_ASO_SWITCH_LIMIT_P` | limit+ switch connected |
| `(1<<2)` | `PK_ASO_SWITCH_HOME` | home switch connected |
| `(1<<3)` | `PK_ASO_SWITCH_COMBINED_LN_H` | home shared with limit- |
| `(1<<4)` | `PK_ASO_SWITCH_COMBINED_LP_H` | home shared with limit+ |
| `(1<<5)` | `PK_ASO_SWITCH_INVERT_LIMIT_N` | invert limit- polarity |
| `(1<<6)` | `PK_ASO_SWITCH_INVERT_LIMIT_P` | invert limit+ polarity |
| `(1<<7)` | `PK_ASO_SWITCH_INVERT_HOME` | invert home polarity |

#### `SignalConfig[N]` → signal inversion flags (using `ePK_PEv2_AxisSignalOptions`)

| Bit | Constant | HAL Signal |
|-----|----------|------------|
| `(1<<0)` | `PK_ASO_INVERT_STEP` | invert step signal |
| `(1<<1)` | `PK_ASO_INVERT_DIRECTION` | invert direction signal |

The raw bitmapped values (`AxesConfig`, `AxesSwitchConfig`) **shall** remain available as
u32 HAL pins for diagnostic purposes.

## Acceptance Criteria

### Scenario 1 — AxesSwitchConfig bit extraction

**Given** `device->PEv2.AxesSwitchConfig[0]` is `0x07` (limit-, limit+, and home bits set)
**When** `pev2_read_device_to_hal()` is called
**Then** `PEv2.0.digin.LimitN.in` = TRUE, `PEv2.0.digin.LimitP.in` = TRUE,
`PEv2.0.digin.Home.in` = TRUE

### Scenario 2 — Correct enumeration constant values

**Given** the PoKeysLib header `PoKeysLib.h`
**When** the code is compiled
**Then** `PK_ASO_SWITCH_LIMIT_N` = 1, `PK_ASO_SWITCH_LIMIT_P` = 2,
`PK_ASO_SWITCH_HOME` = 4 (no compile errors or warnings)

### Scenario 3 — Raw bitmap pin still available

**Given** `device->PEv2.AxesSwitchConfig[1]` is `0xAB`
**When** `pev2_read_device_to_hal()` is called
**Then** `pokeys.0.PEv2.1.AxesSwitchConfig` reads `0xAB` (raw value preserved)

## Verification Method

Test (automated) + Code inspection

## Priority

P1 – High

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c`, uses `PoKeysLib.h` enumerations
- **Verified by**: (TEST issues — add when created)
