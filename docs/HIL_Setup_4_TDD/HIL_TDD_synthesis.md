# HIL/TDD Guidance Synthesis

**Status**: Approved for documentation/skeleton scope
**Date**: 2026-07-30
**Input documents**: 6 thought documents in `docs/HIL_Setup_4_TDD/`
**Produces**: authoritative input for `tdd-hil-infrastructure-audit.prompt.md`

**Approval metadata**: this synthesis was accepted as the basis for the initial HIL-TDD infrastructure skeleton in PR #139. No hardware runs were executed in this phase; evidence is limited to fixture definitions, documented constraints, and review-approved guidance.

**Conflict and resolution**: earlier drafts conflated primitive HIL with machine integration and used ambiguous status terms. The active guidance now separates those scopes, centralizes status vocabulary in the HIL result schema, and keeps the fixture draft until HW-0 through HW-5 are resolved.

---

## 1. Source Inventory

| File | Type | Unique contribution | Notes |
|---|---|---|---|
| `HIL_TDD_guidance_suggestion1.md` | Proposal | 4-layer HIL architecture; `hil-tdd` skill; `hil-tests.instructions.md` name | Superseded on instruction name |
| `HIL_moduletests_thoughts.md` | **Exact duplicate** of suggestion1 | None | Delete; adds no independent evidence |
| `PoKeys57E pin modes.md` | Evidence + proposals | Pin capability table, OC16-CNC reservation, PWM/ADC constraints, motor-enable conflict | Unique; keep as fixture reference |
| `TDD Guidance for PokeysHal1.md` | Proposal | HIL-as-oracle model; `HIL-observed` status; observation ID format | Unique; merge into skill |
| `TDD Guidance for PokeysHal2.md` | Proposal | Hard primitive-HIL / machine-integration separation; test-layer table | Unique; merge into skill |
| `TDD_HILguidance_thoughts.md` | Proposal + draft | Full instruction draft; CI workflow YAML; complete test sequence | Primary draft source |

`HIL_moduletests_thoughts.md` is a content duplicate of `HIL_TDD_guidance_suggestion1.md`. Count once.

---

## 2. Accepted Decisions

### D-01 — Instruction file name: `hil-tdd.instructions.md`

Use `hil-tdd.instructions.md`. Consistent with skill name `hil-tdd` and naming
conventions of other skills (`convert-to-hal-rtapi`). Supersedes `hil-tests`
proposed by suggestion1/moduletests.

### D-02 — `wcomp` and `DM542_XXYZ_mill` are integration scope

Primitive HIL (`tests/hil/basic/`) must not load `wcomp`, kinematics, `motmod`,
or the `DM542_XXYZ_mill` stack. These belong in `tests/hil/machine/linuxcnc/`.

A failure in primitive HIL must point to one driver subsystem. Loading the full
machine stack makes failures ambiguous across driver, HAL naming, INI, homing,
and wiring.

### D-03 — Emergency loopback excluded from v1

The emergency loopback (Pin 33 → Pin 52) is excluded until HW-1 and HW-2 are
resolved. `Pokeys57E_SimPins.hal` comments say Pin 52; the INI says
`PEv2_EmergencyInputPin=54`. These are conflicting provenance, not verified facts.

### D-04 — No dedicated HIL agent at this time

`TDDDriver` and `TestingSpecialist` receive routing hooks only. A dedicated
`HILTestOperator` agent is justified only when the fixture gains SSH access to
a dedicated host, power cycling, bench reservation, or instrument access.

### D-05 — Directory split

```
tests/hil/basic/              — primitive driver tests; no machine stack
tests/hil/machine/linuxcnc/   — full DM542 config, wcomp, homing
```

`applyTo: "tests/hil/**"` in the instruction covers both.

### D-06 — `HIL-observed` status category

Add `HIL-observed` as a category between `Assumed` and `Unit-tested`:

| Status | Meaning |
|---|---|
| `HIL-observed` | Real board behavior was observed and recorded as oracle evidence |
| `HIL-test-executed` | Named HIL test ran, regardless of outcome |
| `HIL-verified` | Named HIL test passed on the named fixture revision |
| `Timing-validated` | Timing measured against a documented threshold |

Do not redefine `RT-validated`; the repository engineering contract owns that term.
Remove the legacy status label — its distinction from `HIL-verified` is unclear.

### D-07 — `applyTo` scope: `tests/hil/**` only

Do not expand to `*.hal`, `*.ini`, or `.github/workflows/**`. That would load HIL
safety rules when editing any HAL file, recreating the context-inflation problem
corrected in PR #135.

### D-08 — Fixture is a draft until hardware questions are resolved

The fixture YAML must carry `fixture_status: draft` and `runnable: false` until
all verification fields are confirmed. Per-loopback safe-state must be defined per
channel, not as a global `false`. The value `false` ≠ electrically inactive for
inverted or active-low outputs.

---

## 3. Claim Matrix (summary)

| ID | Claim | Classification | Decision |
|---|---|---|---|
| C-01 | Physical pin N = HAL index N−1 | Verified (SimPins.hal) | Encode as fixture invariant |
| C-02 | HIL does not replace unit/protocol tests | Supported | Adopt as rule |
| C-03 | DM542 config is integration scope | Proposal | Accept (D-02) |
| C-04 | wcomp is Layer 3 / integration | Proposal | Accept (D-02) |
| C-05 | Emergency wired Pin 33 → Pin 52 | Provenance | Excluded (D-03) |
| C-06 | INI `PEv2_EmergencyInputPin=54` | Provenance | Excluded (D-03) |
| C-07/08 | Instruction file name | Conflict | Resolved: `hil-tdd` (D-01) |
| C-09 | No dedicated HIL agent | Supported | Adopt (D-04) |
| C-10 | AxisEnableOutputPins_N=1 conflicts FastEncoder1A | Proposal | Deferred (HW-3) |
| C-11 | PWM→ADC needs RC filter | Supported | Documented constraint |
| C-12 | `HIL-observed` status category | Proposal | Accept (D-06) |
| C-13 | `applyTo: tests/hil/**` only | Proposal | Accept (D-07) |
| C-14 | Agent routing hooks only | Supported | Adopt |
| C-15 | First PR: documentation and skeleton only | Supported | Adopt |
| C-16 | Loopback pairs from SimPins.hal | Provenance | Unconfirmed; mark as such |
| C-17 | Pins 9, 11, 51 reserved for OC16-CNC | Supported | Encode in YAML |
| C-18 | PWM pins 17–22, ADC pins 41–47 | Supported | Encode in fixture MD |
| C-19/20 | Separate hil/basic and hil/machine dirs | Proposal | Accept (D-05) |

---

## 4. Unresolved Hardware Questions

All hardware questions block at least one capability or loopback entry in the
fixture. They cannot be resolved from documents alone.

| ID | Question | Blocks |
|---|---|---|
| HW-0 | Actual device identity and firmware version | Device identity fields in YAML |
| HW-1 | Emergency wiring: Pin 52 or Pin 54? | Emergency loopback |
| HW-2 | `PEv2_EmergencyInputPin`: physical pin or API index? | Emergency loopback |
| HW-3 | `PEv2_AxisEnableOutputPins_N=1` vs FastEncoder1A on pin 1 | PE/encoder tests |
| HW-4 | Physical continuity of declared loopback pairs | All loopbacks (currently provenance only) |
| HW-5 | Polarity and pull-up configuration of loopback endpoints | Safe-state definitions |

---

## 5. Canonical Document Architecture

| Source document | Disposition |
|---|---|
| `HIL_TDD_guidance_suggestion1.md` | Archive — superseded by this synthesis |
| `HIL_moduletests_thoughts.md` | Delete — exact content duplicate of suggestion1 |
| `PoKeys57E pin modes.md` | Retain content → `tests/hil/setups/pokeys57e-loopback-v1.md` |
| `TDD Guidance for PokeysHal1.md` | Oracle model merged into `hil-tdd/SKILL.md` → archive |
| `TDD Guidance for PokeysHal2.md` | Layer separation merged into `hil-tdd/SKILL.md` → archive |
| `TDD_HILguidance_thoughts.md` | Primary draft source for instruction and skill → archive |

Target: six guidance documents → two live artifacts (instruction + skill) + fixture
YAML/MD pair + oracle log. No overlapping active guidance documents.

---

## 6. Inputs Required for Infrastructure Audit

Before running `tdd-hil-infrastructure-audit.prompt.md`, confirm:

1. This synthesis document is committed and accessible at
   `docs/HIL_Setup_4_TDD/HIL_TDD_synthesis.md`.
2. Whether `apply-tdd-infrastructure-plan.prompt.md` should be invoked after the
   audit or whether the audit output becomes a GitHub issue independently.
3. The current LinuxCNC version and kernel RT type on the test machine.
4. Whether a self-hosted GitHub Actions runner exists for the fixture host.
5. Whether the target HAL component is `pokeys_async` (the async rewrite) or
   the legacy `pokeys` userspace component — the machine HAL loads `pokeys`,
   but the build targets `pokeys_async`.
