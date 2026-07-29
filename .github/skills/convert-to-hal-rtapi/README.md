# ConvertToHalRtapi Skill

Convert synchronous PoKeysLib subsystem implementations to asynchronous, real-time capable HAL implementations for LinuxCNC.

## Quick Start

**Goal**: Convert a blocking PoKeys subsystem (e.g., `PoKeysLibEncoders.c`) to a non-blocking async implementation (`PoKeysLibEncodersAsync.c`) suitable for hard real-time LinuxCNC operation.

**Key Concept**: Split blocking I/O into:
1. **Request function** (`PK_*Async()`) - Queues requests without blocking
2. **Parse callback** (`PK_*Parse()`) - Extracts response data when available

## Files in This Skill

### [skill.md](./skill.md)
**Main skill definition** - Use this file to understand:
- What this skill does and when to use it
- Required inputs and expected outputs
- Step-by-step implementation instructions
- Real-time constraints and quality checklist

### [tasks](./tasks)
**Detailed conversion guide** - Reference this for:
- Complete examples with before/after code
- HAL pin export patterns with error handling
- Protocol specification verification workflow
- Named constant definitions and usage
- Testing procedures

## Usage

### With GitHub Copilot Agents

Simply reference this skill in your prompt:

```
@workspace Using the ConvertToHalRtapi skill, convert PoKeysLibIO.c to async operation
```

### Manual Usage

1. Read [skill.md](./skill.md) for overview and steps
2. Follow detailed patterns in [tasks](./tasks)
3. Use provided examples as templates
4. Verify against protocol specification
5. Test in userspace, then RT mode

## What You'll Create

Starting with a synchronous subsystem like this:

```c
int32_t PK_SubsystemGet(sPoKeysDevice* device)
{
    CreateRequest(device->request, 0xC4, 0, 0, 0, 0);
    if (SendRequest(device) == PK_OK) {  // BLOCKS RT THREAD!
        device->field = device->response[8];
    }
    return PK_OK;
}
```

You'll create an async implementation:

```c
// Non-blocking request function
int PK_SubsystemGetAsync(sPoKeysDevice* device)
{
    CreateAndSendRequestAsync(device, PK_CMD_SUBSYSTEM_GET, 
                             NULL, 0, NULL, 0, PK_SubsystemParse);
    return PK_OK;  // Returns immediately
}

// Response parser (called by dispatcher)
int PK_SubsystemParse(sPoKeysDevice* device, const uint8_t* response)
{
    device->field = response[PK_RESPONSE_PAYLOAD_START];
    return PK_OK;
}

// HAL pin export
int export_subsystem_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
{
    // Export HAL pins with proper error handling
    // (see skill.md and tasks for complete pattern)
}
```

## Key Principles

1. ✅ **No Magic Numbers** - Use named constants from `pokeys_command_t`
2. ✅ **Protocol Verified** - Cross-check with `PoKeys - protocol specification.pdf`
3. ✅ **HAL Pin Pattern** - Every pin export with error logging
4. ✅ **RT Safe** - No blocking, no allocation, bounded execution
5. ✅ **Self-Documenting** - Named constants, clear structure

## Prerequisites

- C99 programming knowledge
- LinuxCNC HAL architecture familiarity
- Real-time programming constraints understanding
- Access to `PoKeys - protocol specification.pdf`
- Markitdown MCP server (for reading protocol PDF)

## Testing Your Implementation

```bash
# 1. Build the library
sudo make -f Makefile.noqmake install

# 2. Test userspace
halrun <<EOF
loadusr -W pokeys_async
show pin
exit
EOF

# 3. Test real-time
halrun <<EOF
loadrt threads name1=test-thread period1=1000000
loadrt pokeys_async
addf pokeys-async.0 test-thread
start
show pin
exit
EOF
```

## Common Conversions

| Subsystem | Original File | Async File | Status |
|-----------|--------------|------------|--------|
| Encoders | `PoKeysLibEncoders.c` | `PoKeysLibEncodersAsync.c` | ✅ Complete |
| IO | `PoKeysLibIO.c` | `PoKeysLibIOAsync.c` | ✅ Complete |
| Pulse Engine v2 | `PoKeysLibPulseEngine_v2.c` | `PoKeysLibPulseEngine_v2Async.c` | ✅ Complete |
| PoNET | `PoKeysLibPoNET.c` | `PoKeysLibPoNETAsync.c` | ✅ Complete |
| EasySensors | `PoKeysLibEasySensors.c` | `PoKeysLibEasySensorsAsync.c` | 🔄 In Progress |
| 1-Wire | `PoKeysLib1Wire.c` | `PoKeysLib1WireAsync.c` | ⏳ Planned |

## Related Files

- **Architecture Rules**: `.github/instructions/pokeyshal-architecture.instructions.md`
- **Implementation Phase**: `.github/instructions/phase-05-implementation.instructions.md`
- **Protocol Spec**: `PoKeys - protocol specification.pdf`
- **Main Headers**: `PoKeysLibHal.h`, `PoKeysLibAsync.h`

## Examples to Learn From

Review these complete async implementations:
- `PoKeysLibEncodersAsync.c` - Simple GET/SET with bitfield unpacking
- `PoKeysLibPulseEngine_v2Async.c` - Complex with multiple subcommands
- `PoKeysLibPoNETAsync.c` - Multi-step state machine example

## Contribution Guidelines

When adding new async implementations:

1. Follow the [skill.md](./skill.md) step-by-step process
2. Verify protocol in PDF using Markitdown MCP server
3. Add constants to `PoKeysLibAsync.h` (no magic numbers)
4. Implement HAL pin export with error pattern
5. Test both userspace and RT modes
6. Document in code with protocol spec references

## Support

Questions? Check:
1. [skill.md](./skill.md) - Main skill documentation
2. [tasks](./tasks) - Detailed patterns and examples
3. Existing `*Async.c` files in repo
4. `.github/instructions/pokeyshal-architecture.instructions.md`

---

**Ready to convert?** Start with [skill.md](./skill.md) 🚀
