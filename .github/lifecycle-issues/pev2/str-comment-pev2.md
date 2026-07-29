## Lifecycle Issues Created

This stakeholder requirement has been refined into the following lifecycle issues:

### Functional Requirements

- #{{F001}} — REQ-F-PEV2-001: Global PEv2 HAL Pin Export (all `PEv2.*` status/config pins)
- #{{F002}} — REQ-F-PEV2-002: Per-Axis PEv2 HAL Pin Export (`PEv2.[N].*` pins for each axis)
- #{{F003}} — REQ-F-PEV2-003: Conditional Pin Creation — only for required/enabled axes
- #{{F004}} — REQ-F-PEV2-004: Input Pin Update — HAL input pins written to PoKeysLib pointers
- #{{F005}} — REQ-F-PEV2-005: Output Pin Update — PoKeysLib pointers written to HAL output pins
- #{{F006}} — REQ-F-PEV2-006: Bitmapped Field Decomposition (AxesConfig, AxesSwitchConfig, etc.)
- #{{F007}} — REQ-F-PEV2-007: HAL Parameter Export (Pin assignment, invert flags, filter values)
- #{{F008}} — REQ-F-PEV2-008: PoKeysLib Function Call Sequence (async variants, correct order)

### Non-Functional Requirements

- #{{NF001}} — REQ-NF-PEV2-001: Real-Time Safety (no blocking calls in servo thread)
- #{{NF002}} — REQ-NF-PEV2-002: Interface Completeness (100% of specified pins present)
- #{{NF003}} — REQ-NF-PEV2-003: Correct Enumeration Usage (no magic numbers in bitmask ops)

### Architecture Decision Records

- #{{ADR1}} — ADR-PEV2-001: HAL Pin Naming Convention (`pokeys.[DevID].PEv2.*`)
- #{{ADR2}} — ADR-PEV2-002: Axis-Conditional Pin Creation (based on `nrOfAxes`)
- #{{ADR3}} — ADR-PEV2-003: Bitmapped-to-HAL-Pin Decomposition Strategy
- #{{ADR4}} — ADR-PEV2-004: Read/Write Phase Separation (`pev2_read_device_to_hal` / `pev2_write_hal_to_device`)
- #{{ADR5}} — ADR-PEV2-005: Global vs Per-Axis HAL Struct Organisation

### Architecture Component

- #{{ARC}} — ARC-C-PEV2: PulseEngine v2 HAL Interface Component

### Software Design Description

- #{{SDD}} — SDD-PEV2: Software Design Description for PEv2 HAL Interface

## Status

**STR Status**: Refined → child lifecycle issues created.

All lifecycle issues must pass through the following status progression:
`Specified → Analyzed → Traceable → Approved → Implemented → Verified`

This STR issue can be closed once all child REQ issues reach **Verified** status and the
corresponding PR is merged.
