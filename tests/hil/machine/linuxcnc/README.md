# tests/hil/machine/linuxcnc — Machine Integration Tests

## Scope

These tests verify the full `DM542_XXYZ_mill` LinuxCNC machine configuration,
including homing, simulated switches, kinematics, and the complete HAL/INI stack.
They use the same physical PoKeys57E loopback fixture but load the full machine
configuration, placing them in the `tests/hil/` tree so the HIL safety instruction
(`applyTo: tests/hil/**`) applies.

Integration tests intentionally test the combination of:
driver behavior + HAL naming + INI parameter propagation + homing algorithm +
`wcomp` window comparators + joint mappings + machine-on/e-stop routing.

A failure here can come from any of those layers. Use primitive HIL tests
(`tests/hil/basic/`) first to isolate driver-level defects before running
machine integration tests.

## What belongs here

- Full INI/HAL stack load verification (`DM542_XXYZ_mill`)
- `Pokeys57E_SimPins.hal` with `wcomp` position-derived simulated switches
- `pokeys_homing.hal` homing algorithm wiring
- PEv2 axis switch configuration end-to-end
- Spindle DAC scaling path
- LinuxCNC joint wiring (`joint.N.motor-pos-cmd`, `joint.N.motor-pos-fb`)
- Machine-on / e-stop / probe signal routing

## What does NOT belong here

Primitive driver tests (pin mapping, raw digital I/O, async mailbox, HAL export)
belong in `tests/hil/basic/` where failures are diagnosable against a single subsystem.

## Directory layout

```
tests/integration/linuxcnc/
  README.md                  — this file
  DM542_XXYZ_mill/           — full INI/HAL configuration under test
    Pokeys57E_DM542_XXYZ_mill.ini
    Pokeys_DM542_XXYZ_mill.hal
    pokeys_homing.hal
    Pokeys57E_SimPins.hal
  pytest/                    — integration test cases
```

## Status

Not yet implemented. Prerequisite: primitive HIL tests (HIL-000 through HIL-010)
must pass on the physical fixture before integration tests are written.
