# HAL Pin/Parameter Naming Convention Analysis for pokeys_async

## Overview
This document analyzes the HAL pin and parameter naming conventions from the existing LinuxCNC PoKeys userspace components to establish consistent naming patterns for the new `pokeys_async` component.

## Analysis Summary
After examining 7 userspace components from the LinuxCNC_PokeysLibComp repository, clear naming patterns have been identified that should be maintained for compatibility.

## Component Analysis Results

### 1. PulseEngine Component (PokeysCompPulsEngine_base.c)
**Naming Pattern:** `%s.PEv2.%01d.function-name`
- `%s.PEv2.%01d.joint-pos-cmd` - Position command input
- `%s.PEv2.%01d.joint-vel-cmd` - Velocity command input  
- `%s.PEv2.%01d.joint-pos-fb` - Position feedback output
- `%s.PEv2.%01d.digin.Home.in` - Digital input pins
- `%s.PEv2.%01d.digin.LimitN.in` - Limit switch inputs
- `%s.PEv2.deb.out` - Debug output

### 2. Encoder Component (PoKeysCompEncoders.c)
**Naming Pattern:** `%s.encoder.%01d.function-name`
- `%s.encoder.%01d.count` - Encoder count output (s32)
- `%s.encoder.%01d.position` - Encoder position output (float)
- `%s.encoder.%01d.velocity` - Encoder velocity output (float)
- `%s.encoder.%01d.reset` - Encoder reset input (bit)
- `%s.encoder.%01d.index-enable` - Index enable input (bit)
- `%s.encoder.%01d.scale` - Scale parameter (float, rw)
- `%s.encoder.deb.out` - Debug output

### 3. Main Component (pokeys.c)
**Naming Pattern:** `%s.subsystem.function-name`
- `%s.enum-usb-dev` - USB device enumeration
- `%s.enum-fusb-dev` - Fast USB device enumeration
- `%s.enum-udp-dev` - UDP device enumeration
- `%s.deb.out` - Debug output
- `%s.err` - Error status
- `%s.connected` - Connection status
- `%s.connected.usb` - USB connection status
- `%s.connected.udp` - UDP connection status
- `%s.alive` - Alive status
- `%s.machine-is-on` - Machine status input
- `%s.info.PinCount` - Device info pins
- `%s.rtc.sec` - Real-time clock pins

### 4. IO Component (PoKeysCompIO.c)
**Naming Pattern:** `%s.io-type.%01d.function-name`
- `%s.adcout.%01d.value` - Analog output value input (float)
- `%s.adcout.%01d.enable` - Analog output enable input (bit)
- `%s.adcout.%01d.deb.out` - Debug output (u32)
- `%s.adcout.%01d.offset` - Offset parameter (float, rw)
- `%s.adcout.%01d.scale` - Scale parameter (float, rw)
- `%s.adcin.%01d.value` - Analog input value output (float)
- `%s.adcin.%01d.value-raw` - Raw analog input (float)
- `%s.digin.%01d.in` - Digital input (bit)
- `%s.digin.%01d.in-not` - Inverted digital input (bit)
- `%s.digout.%01d.out` - Digital output (bit)
- `%s.counter.%01d.value` - Counter value (u32)

### 5. PoNET Component (PoKeysCompPoNet.c)
**Naming Pattern:** `%s.kbd48CNC.%01d.function-name`
- `%s.kbd48CNC.%01d.Button.%01d` - Button input (48 buttons)
- `%s.kbd48CNC.%01d.Led.%01d.out` - LED output (48 LEDs)
- `%s.kbd48CNC.%01d.LedPWM.%01d` - LED PWM control
- `%s.kbd48CNC.%01d.Brightness` - Overall brightness control

### 6. PoExtBus Component (PoKeysCompPoExtBus.c)
**Naming Pattern:** `%s.PoExtBus.%01d.function-name`
- Device-specific naming for extended bus devices

### 7. Parameter Naming Conventions

#### Standard Parameter Types:
- **Read/Write Parameters:** `HAL_RW` for user-configurable values
- **Read-Only Parameters:** `HAL_RO` for status/info values
- **Common Parameters:**
  - `.scale` - Scaling factors (float, rw)
  - `.offset` - Offset values (float, rw) 
  - `.invert` - Inversion flags (bit, rw)
  - `.enable` - Enable flags (bit, rw)

## Recommended Naming Standard for pokeys_async

### Base Component Name
- Component name: **`pokeys_async`** (as specified by user)
- Prefix format: `pokeys_async.N` where N is the instance number

### Pin Naming Convention
```
pokeys_async.N.subsystem.channel.function-name
```

### Subsystem Categories

#### 1. System Status & Control
```
pokeys_async.N.connected          - Connection status (bit, out)
pokeys_async.N.alive              - Component alive status (bit, out)  
pokeys_async.N.err                - Error status (bit, out)
pokeys_async.N.machine-is-on      - Machine enable input (bit, in)
pokeys_async.N.deb.out            - Debug output (s32, out)
```

#### 2. Digital I/O
```
pokeys_async.N.digin.MM.in         - Digital input pin MM (bit, out)
pokeys_async.N.digin.MM.in-not     - Inverted digital input MM (bit, out)
pokeys_async.N.digout.MM.out       - Digital output pin MM (bit, in)
```

#### 3. Analog I/O  
```
pokeys_async.N.adcin.MM.value      - Analog input MM scaled value (float, out)
pokeys_async.N.adcin.MM.value-raw  - Analog input MM raw value (float, out)
pokeys_async.N.adcout.MM.value     - Analog output MM value (float, in)
pokeys_async.N.adcout.MM.enable    - Analog output MM enable (bit, in)
```

#### 4. Encoders
```
pokeys_async.N.encoder.MM.count    - Encoder MM count (s32, out)
pokeys_async.N.encoder.MM.position - Encoder MM position (float, out)
pokeys_async.N.encoder.MM.velocity - Encoder MM velocity (float, out)
pokeys_async.N.encoder.MM.reset    - Encoder MM reset (bit, in)
pokeys_async.N.encoder.MM.index-enable - Encoder MM index enable (bit, in)
```

#### 5. Pulse Engine v2
```
pokeys_async.N.PEv2.MM.joint-pos-cmd  - Joint MM position command (float, in)
pokeys_async.N.PEv2.MM.joint-vel-cmd  - Joint MM velocity command (float, in)
pokeys_async.N.PEv2.MM.joint-pos-fb   - Joint MM position feedback (float, out)
pokeys_async.N.PEv2.MM.digin.Home.in  - Joint MM home input (bit, out)
pokeys_async.N.PEv2.MM.digin.LimitN.in - Joint MM negative limit (bit, out)
pokeys_async.N.PEv2.MM.digin.LimitP.in - Joint MM positive limit (bit, out)
```

#### 6. PoNET kbd48CNC
```
pokeys_async.N.kbd48CNC.MM.Button.NN    - Device MM Button NN (bit, out)
pokeys_async.N.kbd48CNC.MM.Led.NN.out   - Device MM LED NN (bit, in)
pokeys_async.N.kbd48CNC.MM.LedPWM.NN    - Device MM LED NN PWM (u32, in)
pokeys_async.N.kbd48CNC.MM.Brightness   - Device MM brightness (u32, in)
```

#### 7. Counters
```
pokeys_async.N.counter.MM.value     - Counter MM value (u32, out)
```

### Parameter Naming Convention

#### System Parameters  
```
pokeys_async.N.devSerial           - Device serial number (u32, rw)
pokeys_async.N.info.DeviceTypeID   - Device type ID (u32, ro)
pokeys_async.N.info.SerialNumber   - Device serial number (u32, ro)
```

#### I/O Parameters
```
pokeys_async.N.digin.MM.invert      - Digital input MM invert (bit, rw)
pokeys_async.N.digout.MM.invert     - Digital output MM invert (bit, rw)
pokeys_async.N.adcin.MM.scale       - Analog input MM scale (float, rw)
pokeys_async.N.adcin.MM.offset      - Analog input MM offset (float, rw)
pokeys_async.N.adcout.MM.scale      - Analog output MM scale (float, rw)
pokeys_async.N.adcout.MM.offset     - Analog output MM offset (float, rw)
pokeys_async.N.adcout.MM.high-limit - Analog output MM high limit (float, rw)
pokeys_async.N.adcout.MM.low-limit  - Analog output MM low limit (float, rw)
```

#### Encoder Parameters
```
pokeys_async.N.encoder.MM.scale     - Encoder MM scale factor (float, rw)
pokeys_async.N.encoder.MM.enable    - Encoder MM enable (bit, rw)
pokeys_async.N.encoder.MM.4x-sampling - Encoder MM 4x sampling (bit, rw)
pokeys_async.N.encoder.MM.2x-sampling - Encoder MM 2x sampling (bit, rw)
```

## Implementation Guidelines

### 1. Formatting Rules
- Use lowercase for all pin/parameter names
- Use hyphens (-) for multi-word names, not underscores
- Use dots (.) as hierarchical separators
- Use %01d for single-digit zero-padded numbers
- Use %02d for double-digit zero-padded numbers when needed

### 2. Data Types
- **bit:** Boolean values (0/1)
- **s32:** Signed 32-bit integers  
- **u32:** Unsigned 32-bit integers
- **float:** Floating-point values

### 3. Direction Convention
- **HAL_IN:** Input to the component (from LinuxCNC to hardware)
- **HAL_OUT:** Output from the component (from hardware to LinuxCNC)
- **HAL_RW:** Read/write parameters (user configurable)
- **HAL_RO:** Read-only parameters (status/info)

### 4. Debug Pins
- Always include `.deb.out` pins for major subsystems
- Use s32 type for debug outputs
- Increment debug values to track execution flow

### 5. Compatibility Notes
- Pin names should be compatible with existing HAL configurations
- Parameter names should match existing userspace component patterns
- Maintain consistent numbering schemes (0-based indexing)

## Conclusion

This naming convention analysis provides a comprehensive framework for implementing HAL pins and parameters in the `pokeys_async` component while maintaining compatibility with existing LinuxCNC PoKeys installations. The patterns identified ensure consistency, readability, and ease of use for integrators configuring PoKeys devices in LinuxCNC systems.

The next step is to implement these naming conventions in the `pokeys_async` component HAL interface layer, ensuring all pin and parameter exports follow these established patterns.
