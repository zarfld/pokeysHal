# PoKeys Protocol Compliance Summary (Realtime Code)

This document crosschecks the main protocol features from "PoKeys - protocol specification.pdf" against the current realtime code implementation in `experimental/` and related async libraries.

## Methodology
- Protocol commands and features are extracted from the PDF and matched to RT code functions.
- Compliance, missing features, and issues are noted.
- Follows repository instructions for ToDo/task tracking and documentation.

---

## Protocol Features & RT Implementation

| Protocol Command/Feature         | RT Code Function(s)                | Compliance Status | Notes/Issues |
|----------------------------------|------------------------------------|-------------------|--------------|
| Device Enumeration               | PK_EnumerateUSBDevices, PK_ConnectToDevice, PK_ConnectToDeviceWSerial | ✅ Implemented     | Async, non-blocking |
| Digital IO (Get/Set)             | PK_DigitalIOGet, PK_DigitalIOSet   | ✅ Implemented     | Uses async API |
| Analog IO (Get/Set)              | PK_AnalogIOGet, PK_AnalogIOSet     | ✅ Implemented     | Uses async API |
| RTC (Get/Set)                    | PK_RTCGetAsync, PK_RTCGet          | ✅ Implemented     | Async preferred |
| UART Communication               | PK_UARTConfigureAsync, PK_UARTWriteAsync, PK_UARTReadAsync | ✅ Implemented     | Async only in RT |
| Timeout/Retry                    | PK_TimeoutAndRetryCheck, PK_ReceiveAndDispatch | ✅ Implemented     | RT-safe logic |
| Matrix Keyboard/LED              | PK_MatrixKBGet, PK_MatrixLEDSet    | ✅ Implemented     | Async APIs used |
| Pulse Engine                     | PK_PulseEngineGet, PK_PulseEngineSet | ✅ Implemented     | Async APIs used |
| CAN Communication                | PK_CANGet, PK_CANSet               | ✅ Implemented     | Async APIs used |
| Failsafe                         | PK_FailsafeGet, PK_FailsafeSet     | ✅ Implemented     | Async APIs used |
| 1-Wire, I2C, SPI                 | PK_1WireGet, PK_I2CGet, PK_SPIGet  | ✅ Implemented     | Async APIs used |
| Device Disconnect                | PK_DisconnectDevice                | ✅ Implemented     | RT-safe |

---

## Missing Features / Issues
- No blocking calls found in RT code.
- All memory allocation in RT code uses `hal_malloc` and only before thread start.
- No mutexes/semaphores in RT code.
- All error paths are deterministic.
- Logging uses `rtapi_print_msg`.
- Feature parity with user-space code is maintained.

---

## ToDo
- [ ] Review new protocol features in future PDF versions and update RT code as needed.
- [ ] Add more automated tests for edge cases and error handling in RT code.
- [ ] Document any protocol changes or implementation issues in `docs/open/` and track progress in `docs/in-work/`.

---

## Completion Criteria
- All protocol commands/features are implemented in RT code using async APIs.
- No blocking, nondeterministic, or unsafe operations in RT code.
- Documentation and ToDo tracking are up to date.

---

_Last reviewed: 2025-07-15_
