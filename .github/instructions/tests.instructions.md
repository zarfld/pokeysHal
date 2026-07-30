---
description: "PoKeysHal C and RT test requirements: traceability, protocol coverage, and verification disclosure."
applyTo: "**/tests/**,**/test_*.c,**/*_test.c,test_compile.sh"
---

# PoKeysHal Test Requirements

## Traceability

- Every test file that verifies a specific behavior must reference the applicable requirement issue: `/* Verifies: #N (REQ-F-xxx) */`
- Bug-fix commits must include a regression test where one is mechanically feasible.
- A test must be able to fail for the relevant reason. A test that cannot fail is not a behavioral test.
- Compilation alone is not behavioral verification. Do not represent a compile-only check as a passing test.
- Do not change a test to match incorrect implementation behavior to obtain a green result.

## Protocol and Async Tests

Tests for async subsystem commands must cover:

- **Valid response**: correct command byte, correct request ID, well-formed payload → expected HAL pin state after dispatch.
- **Malformed response**: truncated, zero-length, or corrupted 64-byte packet → no crash, no stale data written.
- **Command-ID mismatch**: response command byte does not match the sent command → packet silently discarded.
- **Request-ID mismatch**: response request ID does not match the pending mailbox entry → packet silently discarded.
- **Payload boundary**: minimum and maximum valid field values; value one beyond range → rejection or saturation.
- **Timeout exhaustion**: all retries expire without response → mailbox entry released, error state set, no blocking.
- **Retry path**: first attempt times out, retry succeeds → correct result accepted.
- **Mailbox lifecycle**: entry allocated before send, released after parse or final timeout; no double-free.
- **HAL state**: HAL output pin value before dispatch (initial/stale) and after successful dispatch (updated).
- **Disconnected device**: send attempted with no device present → non-blocking return, no crash.

## RT vs Userspace

- Tests that execute only in userspace must be explicitly documented as such: `/* Userspace only: RT path not testable without hardware */`
- Tests that require hardware must be marked: `/* Requires PoKeys device: skipped in CI */`
- Do not claim RT-validated behavior from a userspace test alone.

## Timing

- Do not assert specific timing values (µs) without a measured baseline from instrumented hardware.
- Timing compliance requires explicit measurement evidence, not code inspection.
