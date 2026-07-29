## Software Design Description (IEEE 1016-2009)

**Architecture Component**: #{{ARC}} (ARC-C-PEV2: PulseEngine v2 HAL Interface)
**Architecture Decisions**: #{{ADR1}}, #{{ADR2}}, #{{ADR3}}, #{{ADR4}}, #{{ADR5}}
**Requirements**: #{{F001}}, #{{F002}}, #{{F003}}, #{{F004}}, #{{F005}}, #{{F006}},
#{{F007}}, #{{F008}}, #{{NF001}}

---

## 1. Logical Design View

### 1.1 Data Types (`PoKeysLibHal.h`)

#### `sPoKeysHalPEv2Axis` (per-axis HAL pin/param storage)

```c
typedef struct {
    /* Input pins (HAL_IN → device) */
    hal_s32_t   *pin_ReferencePositionSpeed;
    hal_s32_t   *pin_PositionSetup;
    hal_bit_t   *pin_digout_AxisEnable_out;
    hal_bit_t   *pin_digout_AxisEnabled_out;
    hal_bit_t   *pin_digout_LimitOverride_out;

    /* Output pins (device → HAL_OUT) */
    hal_s32_t   *pin_CurrentPosition;
    hal_u32_t   *pin_AxesState;
    hal_u32_t   *pin_AxesConfig;
    hal_u32_t   *pin_AxesSwitchConfig;
    hal_s32_t   *pin_SoftLimitMaximum;
    hal_s32_t   *pin_SoftLimitMinimum;
    hal_u32_t   *pin_HomingSpeed;
    hal_u32_t   *pin_HomingReturnSpeed;
    hal_u32_t   *pin_HomingAlgorithm;
    hal_s32_t   *pin_HomeOffsets;
    hal_s32_t   *pin_ProbePosition;
    hal_s32_t   *pin_ProbeMaxPosition;
    hal_float_t *pin_MaxSpeed;
    hal_float_t *pin_MaxAcceleration;
    hal_s32_t   *pin_MPGjogMultiplier;
    hal_u32_t   *pin_MPGjogEncoder;
    hal_u32_t   *pin_MPGjogDivider;
    hal_bit_t   *pin_digin_LimitN_in;
    hal_bit_t   *pin_digin_LimitN_in_not;
    hal_bit_t   *pin_digin_LimitN_DedicatedInput;
    hal_bit_t   *pin_digin_LimitP_in;
    hal_bit_t   *pin_digin_LimitP_in_not;
    hal_bit_t   *pin_digin_LimitP_DedicatedInput;
    hal_bit_t   *pin_digin_Home_in;
    hal_bit_t   *pin_digin_Home_in_not;
    hal_bit_t   *pin_digin_Home_DedicatedInput;
    hal_bit_t   *pin_digin_Error_in;
    hal_bit_t   *pin_digin_Error_in_not;
    hal_bit_t   *pin_digin_Probe_in;
    hal_bit_t   *pin_digin_Probe_in_not;
    hal_bit_t   *pin_digin_SoftLimit_in;
    hal_bit_t   *pin_digin_AxisEnabled_in;
    hal_u32_t   *pin_BacklashWidth;
    hal_s32_t   *pin_BacklashRegister;
    hal_float_t *pin_BacklashAcceleration;
    hal_s32_t   *pin_HomeBackOffDistance;

    /* Parameters (storage in struct, address passed to hal_param_*_newf) */
    hal_u32_t   param_digin_LimitN_Pin;
    hal_u32_t   param_digin_LimitN_Filter;
    hal_bit_t   param_digin_LimitN_invert;
    hal_u32_t   param_digin_LimitP_Pin;
    hal_u32_t   param_digin_LimitP_Filter;
    hal_bit_t   param_digin_LimitP_invert;
    hal_u32_t   param_digout_AxisEnable_Pin;
    hal_bit_t   param_digout_AxisEnable_invert;
    hal_u32_t   param_digin_Home_Pin;
    hal_u32_t   param_digin_Home_Filter;
    hal_bit_t   param_digin_Home_invert;
    hal_float_t param_digin_SoftLimit_PosMin;
    hal_float_t param_digin_SoftLimit_PosMax;
    hal_float_t param_digin_Home_Offset;
} sPoKeysHalPEv2Axis;
```

Implements: #{{ADR5}} (per-axis sub-struct)

### 1.2 Static Storage (`PoKeysLibPulseEngine_v2Async.c`)

```c
static sPoKeysHalPEv2 *pev2_hal = NULL;  /* set during export_pev2_pins() */
```

No dynamic allocation; `sPoKeysHalPEv2` is allocated from HAL shared memory via
`hal_malloc()` during `export_pev2_pins()`.

---

## 2. Algorithms

### 2.1 `export_pev2_pins()` — HAL Pin and Parameter Export

**Precondition**: `comp_id` is valid (returned by `hal_init()`), `device` is non-NULL.

```
export_pev2_pins(prefix, comp_id, device):
    pev2 ← hal_malloc(sizeof(sPoKeysHalPEv2))
    if pev2 == NULL: return -ENOMEM

    /* Export global output pins */
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_nrOfAxes,        comp_id, "%s.PEv2.nrOfAxes", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_maxPulseFrequency,comp_id, "%s.PEv2.maxPulseFrequency", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_bufferDepth,      comp_id, "%s.PEv2.bufferDepth", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_slotTiming,       comp_id, "%s.PEv2.slotTiming", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_PulseEngineEnabled, comp_id, "%s.PEv2.PulseEngineEnabled", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_PulseGeneratorType, comp_id, "%s.PEv2.PulseGeneratorType", prefix)
    hal_pin_bit_newf(HAL_OUT, &pev2->pin_PG_swap_stepdir,  comp_id, "%s.PEv2.PG_swap_stepdir", prefix)
    hal_pin_bit_newf(HAL_OUT, &pev2->pin_PG_extended_io,   comp_id, "%s.PEv2.PG_extended_io", prefix)
    hal_pin_bit_newf(HAL_OUT, &pev2->pin_ChargePumpEnabled, comp_id, "%s.PEv2.ChargePumpEnabled", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_PulseEngineActivated, comp_id, "%s.PEv2.PulseEngineActivated", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_PulseEngineState, comp_id, "%s.PEv2.PulseEngineState", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_MiscInputStatus,  comp_id, "%s.PEv2.MiscInputStatus", prefix)
    for i in 0..7:
        hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Misc_in[i],     comp_id, "%s.PEv2.digin.Misc-%d.in", prefix, i)
        hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Misc_in_not[i], comp_id, "%s.PEv2.digin.Misc-%d.in-not", prefix, i)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_LimitOverride,    comp_id, "%s.PEv2.LimitOverride", prefix)
    hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Emergency_in,     comp_id, "%s.PEv2.digin.Emergency.in", prefix)
    hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Emergency_in_not, comp_id, "%s.PEv2.digin.Emergency.in-not", prefix)
    hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Probed_in,  comp_id, "%s.PEv2.digin.Probed.in", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_AxisEnabledMask,  comp_id, "%s.PEv2.AxisEnabledMask", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_AxisEnabledStatesMask, comp_id, "%s.PEv2.AxisEnabledStatesMask", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_ExternalRelayOutputs, comp_id, "%s.PEv2.ExternalRelayOutputs", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_ExternalOCOutputs, comp_id, "%s.PEv2.ExternalOCOutputs", prefix)
    for i in 0..3:
        hal_pin_bit_newf(HAL_IN, &pev2->pin_digout_ExternalRelay_out[i], comp_id, "%s.PEv2.digout.ExternalRelay-%d.out", prefix, i)
        hal_pin_bit_newf(HAL_IN, &pev2->pin_digout_ExternalOC_out[i],    comp_id, "%s.PEv2.digout.ExternalOC-%d.out", prefix, i)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_ProbeStatus,      comp_id, "%s.PEv2.ProbeStatus", prefix)
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_BacklashCompensationEnabled, comp_id, "%s.PEv2.BacklashCompensationEnabled", prefix)

    /* Export global input pins */
    hal_pin_bit_newf(HAL_IN, &pev2->pin_digout_Emergency_out, comp_id, "%s.PEv2.digout.Emergency.out", prefix)
    hal_pin_u32_newf(HAL_IN, &pev2->pin_LimitOverrideSetup, comp_id, "%s.PEv2.LimitOverrideSetup", prefix)
    hal_pin_u32_newf(HAL_IN, &pev2->pin_HomingStartMaskSetup, comp_id, "%s.PEv2.HomingStartMaskSetup", prefix)
    hal_pin_u32_newf(HAL_IN, &pev2->pin_ProbeStartMaskSetup, comp_id, "%s.PEv2.ProbeStartMaskSetup", prefix)
    hal_pin_float_newf(HAL_IN, &pev2->pin_ProbeSpeed,       comp_id, "%s.PEv2.ProbeSpeed", prefix)

    /* Export global parameters */
    hal_param_u32_newf(HAL_RW, &pev2->param_digin_Emergency_Pin,   comp_id, "%s.PEv2.digin.Emergency.Pin", prefix)
    hal_param_bit_newf(HAL_RW, &pev2->param_digin_Emergency_invert, comp_id, "%s.PEv2.digin.Emergency.invert", prefix)
    hal_param_u32_newf(HAL_RW, &pev2->param_digout_Emergency_Pin,  comp_id, "%s.PEv2.digout.Emergency.Pin", prefix)
    hal_param_u32_newf(HAL_RW, &pev2->param_digin_Probe_Pin,       comp_id, "%s.PEv2.digin.Probe.Pin", prefix)
    hal_param_bit_newf(HAL_RW, &pev2->param_digin_Probe_invert,    comp_id, "%s.PEv2.digin.Probe.invert", prefix)

    /* Determine axis count */
    n_axes ← device->PEv2.info.nrOfAxes
    if n_axes == 0:
        rtapi_print_msg(RTAPI_MSG_WARN, "PEv2: nrOfAxes=0, defaulting to 8")
        n_axes ← 8

    /* Export per-axis pins and parameters */
    for ax in 0..(n_axes-1):
        export_pev2_axis_pins(pev2, comp_id, prefix, ax)

    device->PEv2.pHal = pev2
    return 0
```

### 2.2 `pev2_read_device_to_hal()` — Device Status to HAL Outputs

```
pev2_read_device_to_hal(pev2, dev):
    pe ← &dev->PEv2
    *pev2->pin_nrOfAxes        ← pe->info.nrOfAxes
    *pev2->pin_PulseEngineEnabled  ← pe->PulseEngineEnabled
    *pev2->pin_PulseEngineState    ← pe->PulseEngineState
    *pev2->pin_PulseEngineActivated ← pe->PulseEngineActivated
    *pev2->pin_ChargePumpEnabled   ← (pe->ChargePumpEnabled != 0)
    *pev2->pin_MiscInputStatus     ← pe->MiscInputStatus
    for i in 0..7:
        bit ← (pe->MiscInputStatus >> i) & 1
        *pev2->pin_digin_Misc_in[i]     ← bit
        *pev2->pin_digin_Misc_in_not[i] ← !bit
    *pev2->pin_LimitOverride ← pe->LimitOverride
    emg ← pe->EmergencyInputStatus != 0
    *pev2->pin_digin_Emergency_in     ← emg
    *pev2->pin_digin_Emergency_in_not ← !emg
    *pev2->pin_ExternalRelayOutputs ← pe->ExternalRelayOutputs
    *pev2->pin_ExternalOCOutputs    ← pe->ExternalOCOutputs
    *pev2->pin_ProbeStatus ← pe->ProbeStatus
    *pev2->pin_digin_Probed_in ← (pe->ProbeStatus != 0)
    *pev2->pin_AxisEnabledMask       ← pe->AxisEnabledMask
    *pev2->pin_AxisEnabledStatesMask ← pe->AxisEnabledStatesMask
    *pev2->pin_BacklashCompensationEnabled ← pe->BacklashCompensationEnabled

    for ax in 0..(nAxes-1):
        axis ← &pev2->axis[ax]
        *axis->pin_CurrentPosition ← pe->CurrentPosition[ax]
        *axis->pin_AxesState       ← pe->AxesState[ax]
        *axis->pin_AxesConfig      ← pe->AxesConfig[ax]
        *axis->pin_AxesSwitchConfig ← pe->AxesSwitchConfig[ax]
        *axis->pin_SoftLimitMinimum ← pe->SoftLimitMinimum[ax]
        *axis->pin_SoftLimitMaximum ← pe->SoftLimitMaximum[ax]
        *axis->pin_MaxSpeed        ← pe->MaxSpeed[ax]
        *axis->pin_MaxAcceleration ← pe->MaxAcceleration[ax]
        *axis->pin_BacklashWidth   ← pe->BacklashWidth[ax]
        *axis->pin_BacklashRegister ← pe->BacklashRegister[ax]
        *axis->pin_BacklashAcceleration ← pe->BacklashAcceleration[ax]

        /* Bitmapped status extraction (uses enumeration bit positions) */
        *axis->pin_digin_LimitN_in  ← (pe->LimitStatusN >> ax) & 1
        *axis->pin_digin_LimitN_in_not ← !((pe->LimitStatusN >> ax) & 1)
        *axis->pin_digin_LimitP_in  ← (pe->LimitStatusP >> ax) & 1
        *axis->pin_digin_LimitP_in_not ← !((pe->LimitStatusP >> ax) & 1)
        *axis->pin_digin_Home_in    ← (pe->HomeStatus >> ax) & 1
        *axis->pin_digin_Home_in_not ← !((pe->HomeStatus >> ax) & 1)
        *axis->pin_digin_Error_in   ← (pe->ErrorInputStatus >> ax) & 1
        *axis->pin_digin_Error_in_not ← !((pe->ErrorInputStatus >> ax) & 1)
        *axis->pin_digin_LimitN_DedicatedInput ← (pe->DedicatedLimitNInputs >> ax) & 1
        *axis->pin_digin_LimitP_DedicatedInput ← (pe->DedicatedLimitPInputs >> ax) & 1
        *axis->pin_digin_Home_DedicatedInput   ← (pe->DedicatedHomeInputs >> ax) & 1
        *axis->pin_digin_SoftLimit_in  ← (pe->SoftLimitStatus >> ax) & 1
        *axis->pin_digin_AxisEnabled_in ← (pe->AxisEnabledStatesMask >> ax) & 1
```

**Complexity**: O(nAxes), worst case O(8)

### 2.3 `pev2_write_hal_to_device()` — HAL Inputs to Device Struct

```
pev2_write_hal_to_device(pev2, dev):
    pe ← &dev->PEv2
    pe->EmergencyOutput    ← *pev2->pin_digout_Emergency_out ? 1 : 0
    pe->LimitOverride      ← *pev2->pin_LimitOverrideSetup
    pe->HomingStartMaskSetup ← *pev2->pin_HomingStartMaskSetup
    pe->ProbeStartMaskSetup  ← *pev2->pin_ProbeStartMaskSetup
    pe->ProbeSpeed         ← (float)*pev2->pin_ProbeSpeed

    /* Assemble ExternalRelay bitmask from individual pins */
    relay_mask ← 0
    oc_mask    ← 0
    for i in 0..3:
        if *pev2->pin_digout_ExternalRelay_out[i]: relay_mask |= (1 << i)
        if *pev2->pin_digout_ExternalOC_out[i]:    oc_mask    |= (1 << i)
    pe->ExternalRelayOutputs ← relay_mask
    pe->ExternalOCOutputs    ← oc_mask

    for ax in 0..(nAxes-1):
        axis ← &pev2->axis[ax]
        pe->ReferencePositionSpeed[ax] ← *axis->pin_ReferencePositionSpeed
        pe->PositionSetup[ax]          ← *axis->pin_PositionSetup
```

---

## 3. Interaction View — Servo Cycle Sequence

```
[RT Servo Thread — 1 kHz]
      |
      |-- pev2_write_hal_to_device(pev2, dev)   [HAL inputs → device struct]
      |
      |-- PK_PEv2_ExternalOutputsSetAsync(dev)  [send relay/OC outputs]
      |-- PK_PEv2_PulseEngineMovePVAsync(dev)   [send motion command if active]
      |
      |-- PK_PEv2_StatusGetAsync(dev)           [request status (async, no block)]
      |
      |-- [Later: receive parsed by PK_PEv2_StatusParse callback]
      |
      |-- pev2_read_device_to_hal(pev2, dev)    [device struct → HAL outputs]
      |
      |<-- return
```

---

## 4. Implementation View — File Mapping

| File | Responsibility |
|------|----------------|
| `PoKeysLibHal.h` | `sPoKeysHalPEv2`, `sPoKeysHalPEv2Axis` struct definitions |
| `PoKeysLibPulseEngine_v2Async.c` | `export_pev2_pins()`, `pev2_read_device_to_hal()`, `pev2_write_hal_to_device()`, all `PK_PEv2_*Async()` functions |
| `PoKeysLibAsync.h` | `register_async_task()` declaration |
| `experimental/pokeys_async.c` | Calls `export_pev2_pins()` in `EXTRA_SETUP()` |

---

## 5. Design Rationale

All design choices trace directly to the ADRs:

- Pin naming `pokeys.[DevID].PEv2.*` → #{{ADR1}}
- Axis-conditional pin creation from `nrOfAxes` → #{{ADR2}}
- Raw bitmap + individual bit pins + `*-not` pins → #{{ADR3}}
- Separated `pev2_read_device_to_hal()` / `pev2_write_hal_to_device()` functions → #{{ADR4}}
- `sPoKeysHalPEv2` / `sPoKeysHalPEv2Axis` struct organisation → #{{ADR5}}

## Traceability

- **Architecture Component**: #{{ARC}}
- **Implements**: #{{F001}}, #{{F002}}, #{{F003}}, #{{F004}}, #{{F005}}, #{{F006}},
  #{{F007}}, #{{F008}}, #{{NF001}}
- **Based on ADRs**: #{{ADR1}}, #{{ADR2}}, #{{ADR3}}, #{{ADR4}}, #{{ADR5}}
- **Implemented by**: `PoKeysLibPulseEngine_v2Async.c`, `PoKeysLibHal.h`
- **Verified by**: (TEST issues — add when created)
