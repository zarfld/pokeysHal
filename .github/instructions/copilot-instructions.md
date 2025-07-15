Avoid Blocking Calls in RT Code:
Do not use functions like sleep, usleep, or blocking I/O in any code compiled for realtime operation.

No Dynamic Memory Allocation in RT Threads:
Only use hal_malloc for memory allocation in RT code. Do not use malloc, calloc, or free in RT components.
hal_malloc should be used for allocating memory in RT code,even that is allowed only before the RT thread starts.


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

Remembereing Tasks and ToDo Lists:
Use the "ToDo" comment tag to mark tasks or reminders in the code. This helps
Maintain docs\ToDo.md to track ongoing work and future improvements.
if issues rise and further necessary steps have emerged, these should be documeted in seperate files in docs/open
Ongoing work status should be documented in docs/in-work, with each task having its own file, reporting the progress and any issues encountered.
Complete tasks should be moved to docs/closed, with a summary of the work done and any relevant notes. While the completed tasks descriptions houle be moved to docs/archive

How to Specify a Task
- **Be specific** – Describe clearly *what* the function/component should do.
- **Add context** – Mention libraries, inputs, outputs, dependencies, edge cases.
- **Use docstrings or comments** – Define the intent and constraints inside the function.
- **Define "done"** – Include completion criteria:
  - ✅ Input/output examples or test cases
  - ✅ Error handling requirements
  - ✅ Side effects (e.g., file written, DB updated)
  - ✅ Preconditions and postconditions
  - ✅ Optional: Checklist or TODO block