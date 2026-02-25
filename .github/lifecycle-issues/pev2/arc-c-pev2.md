## Architecture Component

**Status**: Specified

**Based on ADRs**:
- #{{ADR1}} (ADR-PEV2-001: HAL Pin Naming Convention)
- #{{ADR2}} (ADR-PEV2-002: Axis-Conditional Pin Creation)
- #{{ADR3}} (ADR-PEV2-003: Bitmapped-to-HAL-Pin Decomposition)
- #{{ADR4}} (ADR-PEV2-004: Read/Write Phase Separation)
- #{{ADR5}} (ADR-PEV2-005: Global vs Per-Axis Struct Organisation)

## Component Purpose

**Responsibility**: Provide the complete PulseEngine v2 (PEv2) HAL interface between
PoKeysLib and LinuxCNC HAL.  This component owns HAL pin and parameter export for all
PEv2 signals, the read/write data flow between device struct fields and HAL pins each
servo cycle, and scheduling of the relevant async PoKeysLib functions.

**Scope**:

- Export all global PEv2 HAL pins and parameters (`pokeys.[DevID].PEv2.*`)
- Export per-axis HAL pins and parameters (`pokeys.[DevID].PEv2.[N].*`) for each enabled axis
- Update HAL output pins from `device->PEv2.*` fields after each status receive
- Update `device->PEv2.*` fields from HAL input pins before each command send
- Decompose bitmapped fields into individual HAL bit pins using PoKeysLib enumerations
- Schedule `PK_PEv2_StatusGetAsync()`, `PK_PEv2_Status2GetAsync()`,
  `PK_PEv2_AdditionalParametersGetAsync()` as periodic async tasks
- Trigger `PK_PEv2_ExternalOutputsSetAsync()` and motion commands as needed

**Out of Scope**:

- PoKeysLib mailbox/async infrastructure (in `PoKeysLibAsync.c`)
- HAL pin export for non-PEv2 subsystems (IO, encoders, CAN, etc.)
- LinuxCNC motion planning (handled by LinuxCNC core)

## Interfaces

### C API (declared in `PoKeysLibAsync.h` and implemented in `PoKeysLibPulseEngine_v2Async.c`)

```c
/**
 * Export all PEv2 HAL pins and parameters.
 * Called once from EXTRA_SETUP() in experimental/pokeys_async.c.
 *
 * @param prefix   HAL component name prefix (e.g., "pokeys")
 * @param comp_id  HAL component ID returned by hal_init()
 * @param device   Pointer to the PoKeys device struct
 * @return 0 on success, negative HAL error code on failure
 */
int export_pev2_pins(const char *prefix, long comp_id, sPoKeysDevice *device);

/**
 * Register periodic PEv2 async tasks with the scheduler.
 * Called once from EXTRA_SETUP() after export_pev2_pins().
 *
 * @param device  Pointer to the PoKeys device struct
 * @return 0 on success, negative error code on failure
 */
int register_pev2_tasks(sPoKeysDevice *device);

/**
 * Update HAL output pins from device->PEv2.* fields.
 * Called after PK_PEv2_StatusGetAsync response is parsed.
 */
static void pev2_read_device_to_hal(sPoKeysHalPEv2 *pev2, sPoKeysDevice *dev);

/**
 * Update device->PEv2.* fields from HAL input pins.
 * Called before PK_PEv2_*SetAsync / motion commands.
 */
static void pev2_write_hal_to_device(sPoKeysHalPEv2 *pev2, sPoKeysDevice *dev);
```

### Data Structures (in `PoKeysLibHal.h`)

```c
typedef struct {
    hal_s32_t  *pin_CurrentPosition;
    hal_s32_t  *pin_ReferencePositionSpeed;
    hal_u32_t  *pin_AxesState;
    hal_u32_t  *pin_AxesConfig;
    hal_u32_t  *pin_AxesSwitchConfig;
    hal_bit_t  *pin_digin_LimitN_in;
    hal_bit_t  *pin_digin_LimitN_in_not;
    hal_bit_t  *pin_digin_LimitN_DedicatedInput;
    hal_bit_t  *pin_digin_LimitP_in;
    hal_bit_t  *pin_digin_LimitP_in_not;
    hal_bit_t  *pin_digin_LimitP_DedicatedInput;
    hal_bit_t  *pin_digin_Home_in;
    hal_bit_t  *pin_digin_Home_in_not;
    hal_bit_t  *pin_digin_Home_DedicatedInput;
    hal_bit_t  *pin_digin_Error_in;
    hal_bit_t  *pin_digin_Error_in_not;
    hal_bit_t  *pin_digin_Probe_in;
    hal_bit_t  *pin_digin_Probe_in_not;
    hal_bit_t  *pin_digin_SoftLimit_in;
    hal_bit_t  *pin_digin_AxisEnabled_in;
    hal_bit_t  *pin_digout_AxisEnable_out;
    hal_bit_t  *pin_digout_AxisEnabled_out;
    hal_bit_t  *pin_digout_LimitOverride_out;
    hal_s32_t  *pin_PositionSetup;
    hal_s32_t  *pin_SoftLimitMaximum;
    hal_s32_t  *pin_SoftLimitMinimum;
    hal_u32_t  *pin_HomingSpeed;
    hal_u32_t  *pin_HomingReturnSpeed;
    hal_u32_t  *pin_HomingAlgorithm;
    hal_s32_t  *pin_HomeOffsets;
    hal_s32_t  *pin_ProbePosition;
    hal_s32_t  *pin_ProbeMaxPosition;
    hal_float_t *pin_MaxSpeed;
    hal_float_t *pin_MaxAcceleration;
    hal_s32_t  *pin_MPGjogMultiplier;
    hal_u32_t  *pin_MPGjogEncoder;
    hal_u32_t  *pin_MPGjogDivider;
    hal_u32_t  *pin_BacklashWidth;
    hal_s32_t  *pin_BacklashRegister;
    hal_float_t *pin_BacklashAcceleration;
    hal_s32_t  *pin_HomeBackOffDistance;
    /* Parameters (stored by value, addr passed to hal_param_*_newf) */
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

## Dependencies

**Internal**:

- `PoKeysLibAsync.h` / `PoKeysLibAsync.c` — `CreateRequestAsync()`, `SendRequestAsync()`,
  `register_async_task()`
- `PoKeysLibHal.h` — `sPoKeysHalPEv2`, `sPoKeysHalPEv2Axis` struct definitions
- `PoKeysLib.h` — `sPoKeysDevice`, `sPoKeysPEv2`, enumeration constants
- `experimental/pokeys_async.c` — calls `export_pev2_pins()` and `register_pev2_tasks()`

**External**:

- LinuxCNC HAL (`hal_pin_*_newf()`, `hal_param_*_newf()`, `hal_init()`)
- PoKeys device firmware (UDP protocol commands `0xB2`, `0xD3`, etc.)

## Technology Stack

- Language: C99
- Build: `make -f Makefile.noqmake` / `halcompile`
- No dynamic allocation; no blocking socket calls in RT path

## Traceability

- **Based on**: #{{ADR1}}, #{{ADR2}}, #{{ADR3}}, #{{ADR4}}, #{{ADR5}}
- **Traces to**: #{{F001}}, #{{F002}}, #{{F003}}, #{{F004}}, #{{F005}}, #{{F006}},
  #{{F007}}, #{{F008}}, #{{NF001}}, #{{NF002}}, #{{NF003}}
- **Implemented by**: `PoKeysLibPulseEngine_v2Async.c`,
  `PoKeysLibHal.h` (`sPoKeysHalPEv2`, `sPoKeysHalPEv2Axis`)
- **Verified by**: (TEST issues — add when created)
