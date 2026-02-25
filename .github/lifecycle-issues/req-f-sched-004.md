## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

## Requirement Description

The `PK_DigitalIOSetAsync()` function **shall** maintain a static 7-byte cache of the last
transmitted digital output pattern (pin values + mask).  A new 64-byte USB/network packet
**shall** be transmitted **only** when the computed output pattern differs from the cached
copy.  When the pattern is unchanged, the function **shall** return `PK_OK` immediately
without issuing any network request.

On the very first invocation (process/module start) the function **shall** always transmit,
regardless of output values, to initialise the device to a known state.

## Acceptance Criteria

### Scenario 1 — No transmission on unchanged outputs

**Given** `PK_DigitalIOSetAsync()` was called once with output pattern P
**When** it is called again with identical HAL pin values (same pattern P)
**Then** no USB/network packet is sent
**And** the return value is `PK_OK`

### Scenario 2 — Transmission on changed output

**Given** the last transmitted pattern had pin 5 = 0
**When** pin 5 is set to 1 and `PK_DigitalIOSetAsync()` is called
**Then** a new 64-byte packet is transmitted with the updated pattern

### Scenario 3 — Unconditional first transmission

**Given** `PK_DigitalIOSetAsync()` has never been called in this process lifetime
**When** it is called for the first time (all pins = 0)
**Then** a packet is transmitted even though the output is all-zeros

## Verification Method

Test (automated)

## Priority

P1 – High

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibIOAsync.c` (`PK_DigitalIOSetAsync()` dirty-flag block)
- **Verified by**: (TEST issues — add when created)
