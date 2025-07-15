Avoid Blocking Calls in RT Code:
Do not use functions like sleep, usleep, or blocking I/O in any code compiled for realtime operation.

No Dynamic Memory Allocation in RT Threads:
Only use hal_malloc for memory allocation in RT code. Do not use malloc, calloc, or free in RT components.

No Mutexes or Semaphores in RT Code:
Avoid using locking primitives in RT code. Use lock-free or atomic operations if synchronization is needed.

Use Async APIs for Device Communication:
Prefer async versions of library functions (e.g., PoKeysLibUARTAsync.c) in RT code to avoid blocking.

Conditional Compilation for RT:
Use #ifdef RTAPI to separate RT-specific code from userspace code.

Deterministic Execution:
Avoid unpredictable control flow, excessive branching, or unbounded loops in RT code.

RT-safe Logging:
Use rtapi_print_msg for logging in RT code, and avoid standard I/O functions.

Build System:
Ensure RT components are built with the correct flags (-DRTAPI, etc.) and linked against LinuxCNC RT libraries.

Protocol specifications:
"PoKeys - protocol specification.pdf" shoukld be used as information source for implementing communication protocols in RT code.

UserSpace Reference code:
submodule pokeyslib should be used as a reference for implementing user-space communication protocols. Feature parity with the RT code is required, extending the user-space code with additional features is allowed.

Interrupt Safety:
Avoid using global/static variables in RT code unless they are protected or designed for concurrent access. Prefer local variables or RT-safe data structures.

Time Measurement:
Use RT-safe timing functions (e.g., those provided by RTAPI) for measuring time intervals in RT code. Avoid standard library timing functions.

Error Handling:
Ensure all error paths in RT code are deterministic and do not result in blocking, waiting, or unpredictable behavior.

Code Documentation:
Document any RT-specific constraints or assumptions in code comments, especially around timing, concurrency, or hardware access.

Testing:
When possible, provide or update tests to verify RT code does not introduce blocking, excessive latency, or nondeterministic behavior.