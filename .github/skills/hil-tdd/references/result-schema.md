# HIL Test Result and Status Schema

Single authority for HIL status vocabulary. Referenced by `hil-tdd.instructions.md`
and `hil-tdd/SKILL.md`. Do not duplicate this table elsewhere.

---

## Status terms

| Term | Meaning |
|---|---|
| `HIL-observed` | Exploratory physical observation recorded as oracle evidence. Not a test result. |
| `HIL-test-executed` | A named HIL test ran to completion, regardless of outcome. |
| `HIL-verified` | A named HIL test passed on the named fixture at the stated revision. |
| `RT-validated` | Behavior was tested in the applicable LinuxCNC real-time environment. Do not redefine; the repository engineering contract owns this term. |
| `Timing-validated` | Timing was measured against a documented threshold with an identified method. |

Do not use the legacy status label. Use `HIL-test-executed` (ran) or
`HIL-verified` (passed) instead.

---

## Test result outcomes

| Outcome | Condition |
|---|---|
| SKIPPED | `POKEYS_HIL` not set, or non-HIL runner |
| ERROR | HIL requested but device absent; device identity or revision mismatch; continuity/self-test fails; `runnable: false`; fixture is still `draft`; expected device ID is unresolved or mismatched for a required run |
| FAIL | Hardware behavior differs from expected |
| PASS | Test executed successfully |

A required HIL job that produces zero test results must not be reported as green.

---

## Prohibited claims

- Do not claim `HIL-verified` from a mock, simulator, or userspace-only run.
- Do not claim `RT-validated` from a userspace test.
- Do not claim `Timing-validated` without a measured baseline and documented tolerance.
- Do not claim any status from code review or static analysis.
