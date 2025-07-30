# PoKeys Minimal LinuxCNC Configuration

**Created:** 2025-07-30  
**Purpose:** Test PoKeys HAL component integration with LinuxCNC  
**Target:** 3-axis CNC machine with PoKeys device via Ethernet  

## Configuration Overview

This is a minimal LinuxCNC configuration designed to test the PoKeys HAL component integration. It provides:

- **3-axis control** (X, Y, Z) using PEv2 (PulseEngine v2)
- **Basic I/O** through PoKeys digital inputs/outputs
- **Spindle control** via PWM output
- **Emergency stop** and safety systems
- **Status indicators** using relay outputs

## Pin Mappings

### Digital Inputs
- **Pin 0**: Machine enable (inverted - LOW = enabled)
- **Pin 1**: Emergency stop (inverted - LOW = enabled)
- **Pin 2-4**: Currently reading TRUE (connected inputs)

### PEv2 System
- **Axis 0**: X-axis control via `pokeys-async.0.PEv2.0.*`
- **Axis 1**: Y-axis control via `pokeys-async.0.PEv2.1.*`
- **Axis 2**: Z-axis control via `pokeys-async.0.PEv2.2.*`

### Digital Outputs (PEv2 Relays)
- **Relay 0**: Coolant flood
- **Relay 1**: Coolant mist  
- **Relay 2**: Machine ready indicator
- **Relay 3**: Program running indicator

### Analog Outputs
- **PWM 0**: Spindle speed control (0-5V, 0-1800 RPM)

## Files Description

- **pokeys_minimal.ini**: Main configuration file with machine parameters
- **pokeys_minimal.hal**: Main HAL file with component loading and connections
- **custom_postgui.hal**: Post-GUI HAL connections (minimal for testing)
- **shutdown.hal**: Safe shutdown procedures
- **pokeys_minimal.var**: G-code variables file

## Usage

### Starting the Configuration
```bash
cd /home/cnc/Documents/LinuxCnc_PokeysLibComp/pokeysHal/test_configs/pokeys_minimal
linuxcnc pokeys_minimal.ini
```

### Testing Sequence
1. **Load Configuration**: Start LinuxCNC with this configuration
2. **Check Status**: Verify PoKeys device connection in HAL
3. **Enable Machine**: Ensure digital input 0 is LOW (machine enabled)
4. **Test Emergency Stop**: Toggle digital input 1 to test E-stop
5. **Home Axes**: Test homing sequence (if limit switches connected)
6. **Test Movement**: Try small jog movements
7. **Test I/O**: Check spindle control and coolant outputs

## Safety Notes

- **Emergency Stop**: Ensure E-stop is properly connected to digital input 1
- **Machine Enable**: Digital input 0 acts as master enable (inverted)
- **Limits**: All limit switches use inverted logic (LOW = triggered)
- **Shutdown**: All outputs are safely disabled on shutdown

## Troubleshooting

### Common Issues
1. **Component not loading**: Check that pokeys_async.so is installed
2. **Device not found**: Verify PoKeys device is powered and on network
3. **No feedback**: Check PEv2 configuration and encoder connections
4. **E-stop active**: Verify digital input 1 is HIGH for normal operation

### Debugging Commands
```bash
# Check HAL pins and connections
halcmd show pin | grep pokeys
halcmd show sig
halcmd show param | grep pokeys

# Monitor pin states
halmeter pokeys-async.0.digin.0.in
halmeter pokeys-async.0.PEv2.0.joint-pos-fb
```

## Future Enhancements

- Add encoder feedback configuration
- Implement proper home switch sequences
- Add probe input configuration
- Configure additional I/O as needed
- Add tool changer support
- Implement advanced PEv2 features

---

*This configuration provides a foundation for testing and can be expanded based on specific machine requirements.*
