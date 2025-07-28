# Pulse Engine v2 Commands

This document lists the exported helper functions for operating the motion controller in **PoKeysLibPulseEngine_v2.c**. Each routine issues a specific subcommand under `PK_CMD_PULSE_ENGINE_V2`.


## Command reference

### PK_PEv2_StatusGet
* **Command ID**: `PEV2_CMD_GET_STATUS (0x00)`
* **Request fields**: `param1` acts as a handshake value.
* **Response fields**: updates `sPoKeysPEv2` with engine status bits, axis states, current positions and engine info.

### PK_PEv2_Status2Get
* **Command ID**: `PEV2_CMD_GET_STATUS2 (0x08)`
* **Request fields**: none.
* **Response fields**: bytes 8‒10 report numbers of dedicated limit and home inputs.

### PK_PEv2_PulseEngineSetup
* **Command ID**: `PEV2_CMD_SETUP (0x01)`
* **Request payload**: bytes 8‒13 contain engine enable flag, charge pump flag, generator type, buffer size, emergency switch polarity and output‑enable mask.

### PK_PEv2_AdditionalParametersGet
* **Command ID**: `PEV2_CMD_CONFIGURE_MISC (0x06)` with `param3=1`.
* **Response fields**: byte 8 stores the emergency input pin number.

### PK_PEv2_AdditionalParametersSet
* **Command ID**: `PEV2_CMD_CONFIGURE_MISC (0x06)` with `param1=1`.
* **Request fields**: byte 8 carries the emergency input pin.

### PK_PEv2_AxisConfigurationGet
* **Command ID**: `PEV2_CMD_GET_AXIS_CONFIGURATION (0x10)`
* **Request fields**: `param1` selects the axis (0‑7).
* **Response fields**: returns the axis configuration block: limits pins, homing speeds, soft limits, filter settings, step/dir options and more, filling the corresponding members in `sPoKeysPEv2`.

### PK_PEv2_AxisConfigurationSet
* **Command ID**: `PEV2_CMD_SET_AXIS_CONFIGURATION (0x11)`
* **Request payload**: same layout as the block returned by `PK_PEv2_AxisConfigurationGet`.

### PK_PEv2_PositionSet
* **Command ID**: `PEV2_CMD_SET_AXIS_POSITION (0x03)`
* **Request fields**: `param2` is a bit mask of affected axes; bytes 8‒39 contain the new positions for all axes.

### PK_PEv2_PulseEngineStateSet
* **Command ID**: `PEV2_CMD_SET_STATE (0x02)`
* **Request fields**: param1 is the desired state, param2 controls limit override and param3 holds the axis enable mask.

### PK_PEv2_PulseEngineMove
* **Command ID**: `PEV2_CMD_MOVE (0x20)`
* **Request payload**: reference position or speed for each axis (8×32‑bit values).

### PK_PEv2_PulseEngineMovePV
* **Command ID**: `PEV2_CMD_MOVE_PV (0x25)`
* **Request payload**: same reference positions plus 16‑bit scaled velocity ratios in bytes 40‒55.

### PK_PEv2_ExternalOutputsGet
* **Command ID**: `PEV2_CMD_SET_OUTPUTS (0x04)` with `param3=1`.
* **Response fields**: byte 3 holds relay outputs, byte 4 holds open‑collector outputs.

### PK_PEv2_ExternalOutputsSet
* **Command ID**: `PEV2_CMD_SET_OUTPUTS (0x04)`
* **Request fields**: param1 is relay outputs and param2 is open‑collector outputs.

### PK_PEv2_BufferFill
* **Command ID**: `PEV2_CMD_FILL_BUFFER_8BIT (0xFF)`
* **Request fields**: param1 is the number of entries to copy, param2 carries the enable mask; payload bytes 8‒63 contain up to 56 bytes of motion data.
* **Response fields**: byte 2 reports the number of accepted entries and the engine status is refreshed.

### PK_PEv2_BufferFill_16
* **Command ID**: `0xFE` (16‑bit buffer variant)
* **Request/response**: identical to `PK_PEv2_BufferFill` but for 16‑bit motion words.

### PK_PEv2_BufferFillLarge
* **Command ID**: `PK_CMD_MULTIPART_PACKET (0xB0)` with subcommand `0xFF`.
* **Request fields**: param2 is entry count, param3 is the enable mask; payload sends 448 bytes of motion buffer using multipart transfer.

### PK_PEv2_BufferFillLarge_16
* **Command ID**: `PK_CMD_MULTIPART_PACKET (0xB0)` with subcommand `0xFE`.
* **Request/response**: same as `PK_PEv2_BufferFillLarge` but for 16‑bit entries.

### PK_PEv2_BufferClear
* **Command ID**: `PEV2_CMD_CLEAR_BUFFER (0xF0)`
* **Request fields**: none.

### PK_PEv2_PulseEngineReboot
* **Command ID**: `PEV2_CMD_REBOOT (0x05)`
* **Request fields**: none.

### PK_PEv2_HomingStart
* **Command ID**: `PEV2_CMD_START_HOMING (0x21)`
* **Request fields**: param1 contains the axis bit mask; bytes 8‒39 hold home offsets for each axis.

### PK_PEv2_HomingFinish
* **Command ID**: `PEV2_CMD_FINISH_HOMING (0x22)`
* **Request fields**: param1 is the new engine state (typically `STOPPED`), param2 is set to `1`.

### PK_PEv2_ProbingStart
* **Command ID**: `PEV2_CMD_START_PROBING (0x23)`
* **Request fields**: param1 selects axes; bytes 8‒39 contain max probe positions, byte 40 holds probe speed (float), bytes 44‑45 define probe input and polarity.

### PK_PEv2_ProbingHybridStart
* **Command ID**: `PEV2_CMD_START_PROBING (0x23)` with `param2=1`.
* **Request fields**: bytes 44‑45 define probe input and polarity.

### PK_PEv2_ProbingFinish
* **Command ID**: `PEV2_CMD_FINISH_PROBING (0x24)`
* **Response fields**: bytes 8‒39 return the detected probe position and byte 40 contains probe status bits.

### PK_PEv2_ProbingFinishSimple
* **Command ID**: `PEV2_CMD_FINISH_PROBING (0x24)` with `param1=1`.
* **Response fields**: same as `PK_PEv2_ProbingFinish` but leaves the engine state untouched.

### PK_PEv2_ThreadingPrepareForTrigger
* **Command ID**: `PEV2_CMD_PREPARE_TRIGGER (0x30)`

### PK_PEv2_ThreadingForceTriggerReady
* **Command ID**: `PEV2_CMD_FORCE_TRIGGER_READY (0x31)`

### PK_PEv2_ThreadingTrigger
* **Command ID**: `PEV2_CMD_ARM_TRIGGER (0x32)`

### PK_PEv2_ThreadingRelease
* **Command ID**: `PEV2_CMD_RELEASE_TRIGGER (0x33)`

### PK_PEv2_ThreadingCancel
* **Command ID**: `PEV2_CMD_CANCEL_THREADING (0x34)`

### PK_PEv2_ThreadingStatusGet
* **Command ID**: `PEV2_CMD_GET_THREADING_STATUS (0x35)`
* **Response fields**: returns trigger state flags and spindle diagnostics such as speed estimate, position error and debug values.

### PK_PEv2_ThreadingSetup
* **Command ID**: `PEV2_CMD_SET_THREADING_PARAMS (0x36)`
* **Request fields**: bytes 8, 12‒20 carry sensor mode, ticks per revolution, target RPM and filter gains. Byte 20 holds the ignored‑axis mask.

### PK_PEv2_BacklashCompensationSettings_Get
* **Command ID**: `PEV2_CMD_GET_BACKLASH_SETTINGS (0x40)`
* **Response fields**: arrays for backlash width, acceleration and current register plus global enable and max speed bytes.

### PK_PEv2_BacklashCompensationSettings_Set
* **Command ID**: `PEV2_CMD_SET_BACKLASH_SETTINGS (0x41)`
* **Request fields**: same structure as returned by the getter.

### PK_PEv2_SyncedPWMSetup
* **Command ID**: `PEV2_CMD_SETUP_SYNCED_PWM (0x0A)`
* **Request fields**: `enabled`, source axis and destination PWM channel.

### PK_PEv2_SyncOutputsSetup
* **Command ID**: `PEV2_CMD_SETUP_SYNCED_DIGITAL (0x0B)`
* **Request fields**: axis ID and eight‑byte mapping table.

### PK_PoStep_ConfigurationGet
* **Command ID**: `PEV2_CMD_SETUP_DRIVER_COMM (0x50)`
* **Response fields**: driver type, I²C address and update flags for each axis plus the global communication enable flag.

### PK_PoStep_ConfigurationSet
* **Command ID**: `PEV2_CMD_SETUP_DRIVER_COMM (0x50)` with `param1=0x10`.
* **Request payload**: same fields as returned by `PK_PoStep_ConfigurationGet`.

### PK_PoStep_StatusGet
* **Command ID**: `PEV2_CMD_GET_DRIVER_STATUS (0x51)`
* **Response fields**: voltage, temperature and status bytes for each PoStep driver.

### PK_PoStep_DriverConfigurationGet
* **Command IDs**: `PEV2_CMD_DRIVER_CURRENT_PARAMS (0x52)` and `PEV2_CMD_DRIVER_MODE_PARAMS (0x53)`
* **Response fields**: per‑axis current limits, driver mode, microstep setting and temperature limit.

### PK_PoStep_DriverConfigurationSet
* **Command IDs**: `PEV2_CMD_DRIVER_CURRENT_PARAMS (0x52)` and `PEV2_CMD_DRIVER_MODE_PARAMS (0x53)` with `param1=0x10`.
* **Request payload**: same structure as returned by `PK_PoStep_DriverConfigurationGet`.

### PK_PEv2_InternalDriversConfigurationGet
* **Command ID**: `PEV2_CMD_GET_INTERNAL_DRIVERS (0x18)`
* **Response fields**: step mode and current settings for the four internal drivers.

### PK_PEv2_InternalDriversConfigurationSet
* **Command ID**: `PEV2_CMD_SET_INTERNAL_DRIVERS (0x19)`
* **Request payload**: same fields as returned by the getter.

