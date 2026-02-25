## Non-Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

**Quality Attribute Category**: Performance (latency, throughput, jitter)

## Requirement Description

The scheduler **shall** distribute task firing times such that at most one task fires per
`async_dispatcher()` invocation.  The peak USB/network packet rate **shall** not exceed
1 packet per dispatcher call.  The prime-number stagger mechanism (REQ-F-SCHED-003) **shall**
prevent all tasks coinciding at startup.

## Measurable Criteria

| Metric                                                    | Target                 | Measurement Method             |
|-----------------------------------------------------------|------------------------|--------------------------------|
| Max packets per `async_dispatcher()` call                 | 1                      | Code inspection                |
| Peak packet burst at component startup                    | ≤ 1 packet/ms          | Wireshark / USB analyser       |
| Steady-state TX rate at device load 100 %                 | 0 NORMAL + 0 LOW pkt/s | Load-test bench + Wireshark    |
| Following-error count under maximum interface load        | < 5 per minute         | LinuxCNC `halscope`            |

## Verification Method

Test (manual) + Analysis

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibAsync.c` (`async_dispatcher()` fires at most one task per call)
- **Verified by**: (TEST issues — add when created)
