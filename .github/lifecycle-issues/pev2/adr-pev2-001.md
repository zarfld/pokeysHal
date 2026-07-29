## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F001}} (REQ-F-PEV2-001: Global PEv2 HAL Pin Export)
- Traces to: #{{F002}} (REQ-F-PEV2-002: Per-Axis PEv2 HAL Pin Export)
- Traces to: #{{NF002}} (REQ-NF-PEV2-002: Interface Completeness)

## Context

The PEv2 HAL interface requires a consistent and predictable pin naming scheme so that
LinuxCNC users and integrators can rely on the same names across PoKeys devices.  The
existing `pokeys.comp` file uses the pattern `pokeys.[DevID].PEv2.*` for global pins and
`pokeys.[DevID].PEv2.[AxisID].*` for per-axis pins.

A decision is needed on whether to follow this convention exactly or introduce a new
scheme.

## Decision

Adopt the pin naming convention from the HAL Interface: PEv2 issue specification exactly:

- Global pins: `pokeys.[DevID].PEv2.<pin-name>`
- Per-axis pins: `pokeys.[DevID].PEv2.[N].<pin-name>` where N is 0-based axis index
- Digital input signals: `...digin.<Signal>.in` and `...digin.<Signal>.in-not`
- Digital output signals: `...digout.<Signal>.out`
- HAL parameters: same path but created with `hal_param_*_newf()` instead of `hal_pin_*_newf()`

The naming **shall** match the specification in the issue to ensure `pokeys.comp`
compatibility.

## Alternatives Considered

### Alternative 1 — LinuxCNC joint.N.* naming

**Pros**: Directly maps to LinuxCNC joint interface.
**Cons**: Loses the PoKeys-specific grouping; breaks compatibility with `pokeys.comp`
users.  **Rejected**.

### Alternative 2 — Flat naming without hierarchy

**Pros**: Simpler.
**Cons**: Harder to read; no grouping of related signals.  **Rejected**.

## Consequences

**Positive**

- Direct drop-in replacement for `pokeys.comp` configurations.
- Users familiar with the specification can predict pin names without reading code.
- Consistent with other PoKeys HAL subsystems (IO, encoders).

**Negative**

- Long pin names require longer HAL config lines.
- Must maintain strict naming discipline across all `hal_pin_*_newf()` calls.

## Traceability

- **Traces to**: #{{F001}}, #{{F002}}, #{{NF002}}
- **Implemented by**: `PoKeysLibPulseEngine_v2Async.c` (`export_pev2_pins()`)
- **Verified by**: (TEST issues — add when created)
