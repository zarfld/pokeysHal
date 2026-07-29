## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F003}} (REQ-F-PEV2-003: Conditional Pin Creation for Enabled Axes)
- Traces to: #{{NF002}} (REQ-NF-PEV2-002: Interface Completeness)

## Context

The PoKeys PEv2 hardware supports 1–8 axes depending on device model and firmware.
Creating HAL pins for all 8 axes on a 2-axis device wastes HAL namespace and can confuse
integrators.  A strategy is needed for deciding which per-axis pins to create.

## Decision

Use `device->PEv2.info.nrOfAxes` as the upper bound for per-axis pin creation.  Create
per-axis HAL pin groups for indices `0` to `nrOfAxes - 1` only.

The value of `nrOfAxes` is populated by `PK_PEv2_StatusGetAsync()` during the initial
device connection handshake in `EXTRA_SETUP()`.

```c
for (int ax = 0; ax < (int)device->PEv2.info.nrOfAxes; ax++) {
    /* create per-axis pins for ax */
}
```

If `nrOfAxes` is 0 at pin-export time (device not yet responding), create pins for all
8 axes with a warning, to avoid blocking startup.

## Alternatives Considered

### Alternative 1 — Always create all 8 axis pin groups

**Pros**: Simple; no conditional logic.
**Cons**: Pollutes HAL namespace; confusing for 2–4 axis machines.  **Rejected** as
primary strategy (kept as fallback for `nrOfAxes == 0`).

### Alternative 2 — Use `AxesConfig[N] & PK_AC_ENABLED` to filter

**Pros**: Finer granularity.
**Cons**: Config may not be available at pin export time; axis can be enabled/disabled
at runtime while pins cannot be created/destroyed dynamically.
**Rejected** — `nrOfAxes` is the correct hardware-level count.

## Consequences

**Positive**

- Clean HAL namespace matches the actual hardware capability.
- Integrators can immediately see how many axes are supported.

**Negative**

- If device responds after pin export with a different `nrOfAxes`, pins cannot be
  retroactively created (requires restart).

## Traceability

- **Traces to**: #{{F003}}, #{{NF002}}
- **Implemented by**: `PoKeysLibPulseEngine_v2Async.c` (`export_pev2_pins()`)
- **Verified by**: (TEST issues — add when created)
