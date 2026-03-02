# Convert PoKeysLib Subsystem to RT-Capable Async HAL Implementation

## Description

This skill guides the conversion of synchronous (blocking) PoKeysLib subsystem implementations to asynchronous, real-time capable HAL implementations for LinuxCNC. The conversion splits blocking I/O operations into non-blocking request-sending functions and response-parsing callbacks, ensuring the RT thread never blocks.

## When to Use This Skill

Use this skill when you need to:
- Convert an existing `PoKeysLib<Subsystem>.c` file to async operation
- Add a new subsystem with real-time constraints to the PoKeys HAL driver
- Implement LinuxCNC HAL pin exports for a PoKeys subsystem
- Verify protocol compliance for PoKeys command implementations

## Prerequisites

### Required Knowledge
- C programming (C99 standard)
- LinuxCNC HAL architecture and pin concepts
- Real-time programming constraints (no blocking, no dynamic allocation)
- PoKeys USB/Ethernet device protocol

### Required Files
- Original synchronous implementation: `PoKeysLib<Subsystem>.c`
- Protocol specification: `PoKeys - protocol specification.pdf`
- Header files: `PoKeysLibHal.h`, `PoKeysLibAsync.h`

### Required Tools
- Markitdown MCP server (for reading protocol specification PDF)
- LinuxCNC development environment
- HAL compiler (`halcompile`)

## Inputs

| Input | Type | Description | Required |
|-------|------|-------------|----------|
| `subsystem_name` | string | Name of subsystem (e.g., "Encoders", "IO", "PEv2") | Yes |
| `source_file` | path | Path to original `PoKeysLib<Subsystem>.c` file | Yes |
| `protocol_spec` | path | Path to `PoKeys - protocol specification.pdf` | Yes |
| `device_structure` | object | Pointer to `sPoKeysDevice` structure in `PoKeysLibHal.h` | Yes |

## Outputs

| Output | Type | Description |
|--------|------|-------------|
| `async_implementation` | file | New `PoKeysLib<Subsystem>Async.c` file |
| `hal_export_function` | function | `export_<subsystem>_pins()` function for HAL integration |
| `async_request_functions` | functions | Non-blocking request functions (suffix: `Async`) |
| `parse_callbacks` | functions | Response parsing callbacks (suffix: `Parse`) |
| `protocol_constants` | definitions | Command codes and bit masks in `PoKeysLibAsync.h` |

## Step-by-Step Instructions

### Step 1: Verify Protocol Specification

Before writing any code, verify the subsystem's protocol specification:

```markdown
1. Use Markitdown MCP server to read the protocol PDF:
   - Tool: `mcp_microsoft_mar_convert_to_markdown`
   - File: `PoKeys - protocol specification.pdf`

2. Document for your subsystem:
   - Command codes (main and subcommands)
   - Request structure (parameter bytes, payload bytes)
   - Response structure (byte layout, data types)
   - Bitfield definitions (options, status flags)
   - Error codes and handling

3. Add missing definitions to `PoKeysLibAsync.h`:
   - Command codes in `pokeys_command_t` enum
   - Subcommands in subsystem-specific enums
   - Bit mask constants (#define)
   - Response offset constants
```

**Example:**
```c
// Add to PoKeysLibAsync.h
typedef enum {
    PK_CMD_ENCODER_SETTINGS_GET = 0x16,
    PK_CMD_ENCODER_KEYMAP_A_GET = 0x17,
    // ... more commands
} pokeys_command_t;

#define ENCODER_OPT_ENABLE (1 << 0)
#define ENCODER_OPT_X4_SAMPLING (1 << 1)
```

### Step 2: Create Async Implementation File

Create `PoKeysLib<Subsystem>Async.c`:

```c
#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"

// Your async implementation here
```

### Step 3: Implement HAL Pin Export Function

Every subsystem MUST export HAL pins using this pattern:

```c
int export_<subsystem>_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
{
    int r = 0;
    
    // For each pin/parameter:
    // 1. Log creation attempt
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.<pin-name>\n", 
                    __FILE__, __FUNCTION__, prefix);
    
    // 2. Create the pin
    r = hal_pin_<type>_newf(HAL_<direction>, &(target_pointer), comp_id,
                            "%s.<pin-name>", prefix);
    
    // 3. Check for errors
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.<pin-name> failed\n",
                        __FILE__, __FUNCTION__, prefix);
        return r;
    }
    
    return 0;
}
```

**Pin Naming Conventions:**
- Digital input: `%s.<subsystem>.digin.<name>.in`
- Digital output: `%s.<subsystem>.digout.<name>.out`
- Indexed channels: `%s.<subsystem>.<ch>.<name>`

### Step 4: Convert Blocking Functions to Async

For each function in the original implementation:

**Original (Blocking):**
```c
int32_t PK_SubsystemActionGet(sPoKeysDevice* device)
{
    CreateRequest(device->request, 0xC4, 0, 0, 0, 0);
    if (SendRequest(device) == PK_OK) {  // BLOCKS HERE
        // Parse response inline
        device->field = device->response[8];
    }
    return PK_OK;
}
```

**Converted (Async):**
```c
// Request function - queues async request
int PK_SubsystemActionGetAsync(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;
    
    // Use named constant from pokeys_command_t
    CreateAndSendRequestAsync(device, PK_CMD_SUBSYSTEM_ACTION_GET, 
                             NULL, 0, NULL, 0, PK_SubsystemActionParse);
    return PK_OK;
}

// Parse callback - extracts response data
int PK_SubsystemActionParse(sPoKeysDevice* device, const uint8_t* response)
{
    if (device == NULL || response == NULL) return PK_ERR_TRANSFER;
    
    // Use named constants for byte offsets and bit masks
    device->field = response[PK_RESPONSE_PAYLOAD_START];
    
    return PK_OK;
}
```

### Step 5: Handle Multi-Step Operations

For operations requiring multiple sequential requests:

```c
typedef enum {
    PK_SUBSYSTEM_STEP_NONE,
    PK_SUBSYSTEM_STEP_GET_CONFIG,
    PK_SUBSYSTEM_STEP_GET_STATUS,
    PK_SUBSYSTEM_STEP_COMPLETE
} PKSubsystemStep;

typedef struct {
    PKSubsystemStep step;
    uint8_t request_id;
} PKSubsystemAsyncContext;
```

### Step 6: Add Function Declarations to Header

Add to `PoKeysLibAsync.h`:

```c
// Subsystem HAL pin export
int export_<subsystem>_pins(const char *prefix, long comp_id, sPoKeysDevice *device);

// Async functions
int PK_<Subsystem><Action>Async(sPoKeysDevice* device);

// Parse callbacks
int PK_<Subsystem><Action>Parse(sPoKeysDevice* device, const uint8_t* response);
```

### Step 7: Testing

Test in this order:

1. **Userspace HAL component** (non-RT testing):
   ```bash
   halrun <<EOF
   loadusr -W pokeys_async
   show pin
   show param
   exit
   EOF
   ```

2. **Real-time HAL component**:
   ```bash
   halrun <<EOF
   loadrt threads name1=test-thread period1=1000000
   loadrt pokeys_async
   addf pokeys-async.0 test-thread
   start
   show pin
   exit
   EOF
   ```

3. **Timing verification** (oscilloscope + GPIO instrumentation)

## Real-Time Constraints

**CRITICAL - Always Follow These Rules:**

### Allowed in RT Thread
✅ Stack allocation (local variables)  
✅ Direct memory access to pre-allocated structures  
✅ Simple arithmetic and bitwise operations  
✅ Conditional logic with bounded execution  
✅ `rtapi_print_msg()` for debugging (sparingly)  

### Forbidden in RT Thread
❌ Blocking calls (`SendRequest()`, `sleep()`, `usleep()`)  
❌ Dynamic memory allocation (`malloc`, `free`)  
❌ Unbounded loops (use fixed iteration counts)  
❌ Floating-point in critical paths  
❌ File operations  
❌ Network waiting  
❌ Mutex locks that might block  

## Quality Checklist

Before submitting your async implementation:

- [ ] No magic numbers (all protocol values defined in `PoKeysLibAsync.h`)
- [ ] Command codes use `pokeys_command_t` enum
- [ ] Bit masks use named constants
- [ ] HAL pins follow naming conventions
- [ ] Every pin export has error checking
- [ ] Parse callbacks are fast (<50µs soft RT, <5µs hard RT)
- [ ] No blocking calls anywhere
- [ ] No dynamic allocation
- [ ] Protocol verified against specification
- [ ] Comments reference protocol spec sections
- [ ] Tested in both userspace and RT modes

## Examples

### Complete Example: Encoder Subsystem

See the full example in [tasks](./tasks) document, sections:
- Step 2: HAL Pin Export (lines 48-280)
- Step 2.5: Protocol Verification (lines 282-478)
- Step 3: Async Conversion (lines 480-610)

### Quick Reference: Function Naming

| Original | Async Request | Parse Callback |
|----------|---------------|----------------|
| `PK_EncoderConfigurationGet()` | `PK_EncoderConfigurationGetAsync()` | `PK_EncoderOptionsParse()`<br>`PK_FastEncodersOptionsParse()` |
| `PK_EncoderValuesGet()` | `PK_EncoderValuesGetAsync()` | `PK_EncoderValuesGetAsync_ProcessPage0()` |

## References

- **Detailed Guide**: [tasks](./tasks) - Complete step-by-step conversion guide
- **Protocol Specification**: `PoKeys - protocol specification.pdf`
- **Architecture Rules**: `.github/instructions/pokeyshal-architecture.instructions.md`
- **HAL Documentation**: LinuxCNC HAL documentation

## Common Pitfalls

1. **Using magic numbers instead of named constants**
   - ❌ `CreateAndSendRequestAsync(device, 0xC4, ...)`
   - ✅ `CreateAndSendRequestAsync(device, PK_CMD_ENCODER_SETTINGS_GET, ...)`

2. **Missing error handling in pin export**
   - ❌ `hal_pin_u32_newf(...); // No error check`
   - ✅ `r = hal_pin_u32_newf(...); if (r != 0) return r;`

3. **Blocking in parse callbacks**
   - ❌ `SendRequest()` in parse callback (BLOCKS!)
   - ✅ Only extract data, no I/O operations

4. **Inconsistent pin naming**
   - ❌ `pokeys.encoder_3_count`
   - ✅ `pokeys.encoder.3.count`

5. **Not verifying protocol specification**
   - ❌ Guessing byte offsets and bit masks
   - ✅ Reading protocol PDF and documenting findings

## Support

For questions or issues:
1. Review the [tasks](./tasks) document
2. Check the protocol specification
3. Examine existing async implementations (e.g., `PoKeysLibEncodersAsync.c`)
4. Verify against architecture rules in `.github/instructions/`

## Version History

- **v1.0** (2026-03-02) - Initial skill creation with protocol verification and HAL pin export patterns
