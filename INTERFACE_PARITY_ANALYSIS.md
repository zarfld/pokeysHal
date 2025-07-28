# PoKeys HAL Component Interface Parity Analysis

## Overview
This document compares the current RT implementation with the userspace implementation to ensure compatibility.

## Current Implementation Status

### ✅ **Already Implemented**
- Basic digital I/O (limited)
- Analog I/O (limited) 
- RTC functionality
- Basic encoder support
- Device connection management

### ❌ **Missing Critical Features**

#### 1. **Device Information Pins**
Current implementation lacks comprehensive device capability reporting:
```c
// Missing from current implementation:
hal_u32_t *info_PinCount;
hal_u32_t *info_PWMCount; 
hal_u32_t *info_EncodersCount;
hal_u32_t *info_PulseEnginev2;
// ... 25+ more info pins
```

#### 2. **Complete Encoder Interface**
Missing standard LinuxCNC encoder pins:
```c
// Missing encoder pins per channel:
hal_s32_t *count;        // Current count
hal_float_t *position;   // Scaled position  
hal_float_t *velocity;   // Velocity
hal_bit_t *reset;        // Reset command
hal_bit_t *index_enable; // Index enable
hal_float_t scale;       // Scale parameter
```

#### 3. **PulseEngine v2 Support** 
Critical for CNC applications - completely missing:
```c
// Missing PEv2 interface:
hal_float_t *joint_pos_cmd[8];    // Position command
hal_float_t *joint_vel_cmd[8];    // Velocity command  
hal_float_t *joint_pos_fb[8];     // Position feedback
hal_bit_t *joint_in_position[8];  // In position status
hal_u32_t *AxesState[8];          // Axis states
// ... 100+ more PEv2 pins
```

#### 4. **Advanced I/O Features**
Missing scaling, inversion, and configuration:
```c
// Missing I/O features:
hal_float_t adcin_scale[7];       // ADC scaling
hal_float_t adcin_offset[7];      // ADC offset
hal_bit_t digout_invert[55];      // Output inversion
hal_u32_t Pin_Function[55];       // Pin function config
```

#### 5. **PoExtBus and PoNET Support**
Missing expansion bus interfaces:
```c
// Missing PoExtBus:
hal_bit_t *PoExtBus_digin_in[16];
hal_bit_t *PoExtBus_digout_out[16];

// Missing PoNET:
hal_bit_t *kbd48CNC_Button[48];
hal_bit_t *kbd48CNC_LED[48];
```

## Required Implementation Steps

### Phase 1: Core Interface Expansion
1. **Add device info pins** - Essential for capability detection
2. **Complete encoder interface** - Standard LinuxCNC compatibility  
3. **Expand I/O interface** - Add scaling/inversion/configuration

### Phase 2: Motion Control
1. **Implement PulseEngine v2** - Critical for CNC functionality
2. **Add homing support** - Automated homing sequences
3. **Add limit switch handling** - Safety features

### Phase 3: Advanced Features  
1. **PoExtBus support** - I/O expansion
2. **PoNET support** - Networked devices
3. **INI file integration** - Configuration persistence

## Compatibility Requirements

### HAL Pin and Parameter Interface (CRITICAL)
Must match userspace implementation exactly for LinuxCNC configuration compatibility:

**Pin Naming Convention:**
```c
// Required HAL pin names (userspace compatible):
"pokeys.0.encoder.00.count"      // Encoder count
"pokeys.0.encoder.00.position"   // Encoder position  
"pokeys.0.encoder.00.velocity"   // Encoder velocity
"pokeys.0.encoder.00.reset"      // Encoder reset
"pokeys.0.encoder.00.index-enable" // Index enable

"pokeys.0.joint.00.pos-cmd"      // Joint position command
"pokeys.0.joint.00.vel-cmd"      // Joint velocity command
"pokeys.0.joint.00.pos-fb"       // Joint position feedback

"pokeys.0.info.PinCount"         // Device info pins
"pokeys.0.info.EncodersCount"    // Number of encoders
"pokeys.0.info.PulseEnginev2"    // PEv2 capability

"pokeys.0.pin.00.in"             // Digital input
"pokeys.0.pin.00.out"            // Digital output
"pokeys.0.adc.00.value"          // Analog input

// Current implementation (INCOMPATIBLE):
"pokeys-async.0.in-00"           // ❌ Wrong naming
"pokeys-async.0.ain-00"          // ❌ Wrong naming
```

**Parameter Names:**
```c
// Required parameter names:
"pokeys.0.devSerial"             // Device serial number
"pokeys.0.encoder.00.scale"      // Encoder scale factor
"pokeys.0.adc.00.scale"          // ADC scale factor
"pokeys.0.adc.00.offset"         // ADC offset
```

### Internal Implementation (FLEXIBLE)
Function names and internal structure can be different - these are implementation details:
```c
// Internal functions can have any names:
static void my_encoder_update(...)     // ✅ OK - internal function
static void my_io_update(...)          // ✅ OK - internal function  
static void my_pev2_update(...)        // ✅ OK - internal function
```

## Implementation Priority

### **HIGH PRIORITY** (Required for basic compatibility)
1. Device info pins
2. Complete encoder interface
3. Enhanced I/O with scaling
4. Standard pin naming convention

### **MEDIUM PRIORITY** (CNC functionality)
1. PulseEngine v2 basic support
2. Homing sequences  
3. Position/velocity control
4. Limit switch handling

### **LOW PRIORITY** (Advanced features)
1. PoExtBus expansion
2. PoNET devices
3. Advanced motion features
4. Full INI integration

## Testing Requirements

### Interface Compatibility Test
```bash
# Must expose same HAL pins as userspace version:
halcmd show pin pokeys.0.encoder.00.count
halcmd show pin pokeys.0.encoder.00.position  
halcmd show pin pokeys.0.joint.00.pos-cmd
halcmd show pin pokeys.0.info.PinCount
halcmd show param pokeys.0.encoder.00.scale

# Current implementation fails these tests:
halcmd show pin pokeys-async.0.in-00    # ❌ Wrong naming convention
```

### LinuxCNC Configuration Compatibility
```ini
# LinuxCNC HAL file must work with same pin names:
[HAL]
# These commands must work with RT implementation:
net encoder-count <= pokeys.0.encoder.00.count
net joint-pos-cmd => pokeys.0.joint.00.pos-cmd  
setp pokeys.0.encoder.00.scale 1000

# Current implementation breaks existing configs
```

## Conclusion

The current implementation provides **~10%** of the userspace HAL interface compatibility. 
To achieve full compatibility, the HAL pin and parameter interface must be expanded significantly.

**Critical Issue:** Pin naming convention is completely incompatible:
- Userspace: `pokeys.0.encoder.00.count`
- Current RT: `pokeys-async.0.in-00`

**Priority for HAL Interface Compatibility:**
1. **Fix pin naming convention** - Change component name from "pokeys_async" to "pokeys"
2. **Add missing HAL pins** - Encoder, motion control, device info pins  
3. **Add missing parameters** - Scale factors, configuration parameters
4. **Maintain same pin semantics** - Data types, directions, ranges

**Implementation Strategy:**
- Keep existing internal async functions (they work well)
- Change only the HAL pin export layer for compatibility
- Add missing pin types while reusing existing communication code
- Ensure drop-in replacement for userspace driver

This will ensure existing LinuxCNC configurations work without modification when switching from userspace to RT driver.

## Real-Time Implementation Concepts

### Core Real-Time Design Principles

#### 1. **Separation of Concerns**
```c
// RT-safe data structures (shared between RT and non-RT contexts)
typedef struct {
    // RT thread only reads, non-RT thread only writes
    volatile hal_float_t joint_pos_cmd[8];
    volatile hal_float_t joint_vel_cmd[8];
    
    // RT thread only writes, non-RT thread only reads  
    volatile hal_float_t joint_pos_fb[8];
    volatile hal_u32_t axis_state[8];
    
    // Atomic flags for coordination
    volatile bool config_changed;
    volatile bool emergency_stop;
} rt_shared_data_t;

// RT Thread: Fast, deterministic, no blocking calls
void pokeys_rt_thread(void *arg) {
    // Only read commands, write feedback
    // No malloc, no printf, no blocking I/O
    update_outputs_from_hal();
    read_device_status_cache();
    update_hal_inputs();
}

// Non-RT Thread: Handles async communication
void pokeys_async_thread(void *arg) {
    // Handle all PoKeysLib async calls here
    // Update shared cache when data arrives
    process_async_responses();
    send_async_commands();
}
```

#### 2. **Lock-Free Communication Pattern**
```c
// Double-buffering for RT-safe data exchange
typedef struct {
    device_status_t buffer[2];
    volatile int write_index;
    volatile int read_index;
    volatile bool data_ready;
} double_buffer_t;

// Non-RT thread updates back buffer
void async_callback(device_status_t *status) {
    int write_idx = (shared_data.read_index + 1) % 2;
    memcpy(&status_buffer.buffer[write_idx], status, sizeof(device_status_t));
    
    // Atomic switch
    status_buffer.write_index = write_idx;
    status_buffer.data_ready = true;
}

// RT thread reads front buffer
void rt_read_status(void) {
    if (status_buffer.data_ready) {
        shared_data.read_index = status_buffer.write_index;
        status_buffer.data_ready = false;
        // Now use buffer[read_index] safely
    }
}
```

#### 3. **Async Command Queue Pattern**
```c
// Command queue for RT -> Async communication
typedef struct {
    enum { CMD_SET_POSITION, CMD_START_HOMING, CMD_ENABLE_AXIS } type;
    int axis;
    float value;
    bool processed;
} async_command_t;

#define MAX_COMMANDS 64
typedef struct {
    async_command_t commands[MAX_COMMANDS];
    volatile int head, tail;
    volatile int count;
} command_queue_t;

// RT thread: Queue commands (non-blocking)
bool queue_async_command(command_queue_t *q, async_command_t *cmd) {
    if (q->count >= MAX_COMMANDS) return false; // Queue full
    
    q->commands[q->head] = *cmd;
    q->head = (q->head + 1) % MAX_COMMANDS;
    __atomic_fetch_add(&q->count, 1, __ATOMIC_SEQ_CST);
    return true;
}

// Non-RT thread: Process commands
void process_command_queue(command_queue_t *q) {
    while (q->count > 0) {
        async_command_t *cmd = &q->commands[q->tail];
        
        switch (cmd->type) {
            case CMD_SET_POSITION:
                PK_PEv2_PositionSetAsync(dev, cmd->axis, cmd->value);
                break;
            case CMD_START_HOMING:
                PK_PEv2_HomingStartAsync(dev, 1 << cmd->axis);
                break;
        }
        
        q->tail = (q->tail + 1) % MAX_COMMANDS;
        __atomic_fetch_sub(&q->count, 1, __ATOMIC_SEQ_CST);
    }
}
```

### Implementation Strategy by Feature

#### 1. **Device Info Pins (Non-Critical Path)**
```c
// Read once at startup, cache in RT-accessible memory
typedef struct {
    hal_u32_t *info_PinCount;
    hal_u32_t *info_EncodersCount;
    hal_u32_t *info_PulseEnginev2;
    // ... other info pins
} device_info_pins_t;

// Initialize once during HAL setup
void init_device_info_pins(sPoKeysDevice *dev) {
    // These values don't change during runtime
    *device_info.info_PinCount = dev->info.iPinCount;
    *device_info.info_EncodersCount = dev->info.iEncodersCount;
    *device_info.info_PulseEnginev2 = dev->info.iPulseEnginev2;
}
```

#### 2. **Encoder Interface (RT-Critical)**
```c
// RT-safe encoder implementation
typedef struct {
    hal_s32_t *count;
    hal_float_t *position;
    hal_float_t *velocity;
    hal_bit_t *reset;
    hal_bit_t *index_enable;
    hal_float_t scale;
    
    // RT-safe state
    int32_t last_count;
    int32_t current_count;
    bool reset_requested;
} encoder_data_t;

void rt_update_encoders(void) {
    for (int i = 0; i < encoder_count; i++) {
        encoder_data_t *enc = &encoders[i];
        
        // Read from cached device data (populated by async thread)
        enc->current_count = device_cache.encoder_counts[i];
        
        // Handle reset request
        if (*enc->reset && !enc->reset_requested) {
            enc->reset_requested = true;
            // Queue async reset command
            async_command_t cmd = {CMD_RESET_ENCODER, i, 0, false};
            queue_async_command(&cmd_queue, &cmd);
        }
        
        // Calculate position and velocity
        *enc->count = enc->current_count;
        *enc->position = enc->current_count / enc->scale;
        *enc->velocity = (enc->current_count - enc->last_count) * thread_freq / enc->scale;
        
        enc->last_count = enc->current_count;
    }
}
```

#### 3. **PulseEngine v2 (Complex RT-Critical)**
```c
// Hierarchical state management
typedef enum {
    PE_STATE_IDLE,
    PE_STATE_POSITIONING, 
    PE_STATE_HOMING,
    PE_STATE_ERROR
} pe_axis_state_t;

typedef struct {
    // HAL interface
    hal_float_t *pos_cmd;
    hal_float_t *vel_cmd;
    hal_float_t *pos_fb;
    hal_bit_t *in_position;
    hal_bit_t *homing_active;
    
    // RT-safe state machine
    pe_axis_state_t current_state;
    pe_axis_state_t target_state;
    float last_pos_cmd;
    float last_vel_cmd;
    bool state_changed;
} pe_axis_data_t;

void rt_update_pe_axis(int axis) {
    pe_axis_data_t *ax = &pe_axes[axis];
    
    // Detect command changes
    if (*ax->pos_cmd != ax->last_pos_cmd) {
        // Queue position command
        async_command_t cmd = {CMD_SET_POSITION, axis, *ax->pos_cmd, false};
        queue_async_command(&cmd_queue, &cmd);
        ax->last_pos_cmd = *ax->pos_cmd;
    }
    
    // Update feedback from cached data
    *ax->pos_fb = device_cache.axis_positions[axis] / axis_scale[axis];
    *ax->in_position = device_cache.axis_in_position & (1 << axis);
    
    // State machine updates
    rt_update_axis_state_machine(axis);
}

void rt_update_axis_state_machine(int axis) {
    pe_axis_data_t *ax = &pe_axes[axis];
    
    switch (ax->current_state) {
        case PE_STATE_IDLE:
            if (ax->target_state == PE_STATE_HOMING) {
                *ax->homing_active = true;
                ax->current_state = PE_STATE_HOMING;
                // Queue homing start command
                async_command_t cmd = {CMD_START_HOMING, axis, 0, false};
                queue_async_command(&cmd_queue, &cmd);
            }
            break;
            
        case PE_STATE_HOMING:
            // Check if homing completed (from device cache)
            if (device_cache.homing_status & (1 << axis)) {
                *ax->homing_active = false;
                ax->current_state = PE_STATE_IDLE;
            }
            break;
    }
}
```

### Threading Architecture

#### 1. **Multi-Threading Model**
```c
// Main RT thread: Fixed frequency (typically 1kHz)
void rt_main_thread(void *arg) {
    while (!shutdown_requested) {
        rt_wait_for_period();  // Wait for next RT cycle
        
        // Fast, deterministic operations only
        rt_update_digital_io();    // ~10µs
        rt_update_encoders();      // ~20µs  
        rt_update_pe_axes();       // ~30µs
        rt_update_hal_outputs();   // ~10µs
        
        // Total: ~70µs per cycle (plenty of margin for 1ms period)
    }
}

// Async communication thread: Variable timing
void async_comm_thread(void *arg) {
    while (!shutdown_requested) {
        // Process queued commands
        process_command_queue(&cmd_queue);
        
        // Handle async responses
        process_async_responses();
        
        // Periodic status updates
        if (time_for_status_update()) {
            request_device_status_async();
        }
        
        usleep(1000);  // 1ms sleep - not RT critical
    }
}
```

#### 2. **Error Handling Strategy**
```c
// RT-safe error handling
typedef enum {
    ERR_NONE = 0,
    ERR_COMMUNICATION,
    ERR_DEVICE_FAULT,
    ERR_LIMIT_SWITCH,
    ERR_EMERGENCY_STOP
} error_code_t;

typedef struct {
    volatile error_code_t error_code;
    volatile bool error_active;
    volatile uint32_t error_count;
} error_state_t;

void rt_handle_errors(void) {
    if (error_state.error_active) {
        // Immediate RT-safe actions
        disable_all_outputs();
        set_emergency_stop_state();
        
        // Queue async error recovery
        async_command_t cmd = {CMD_ERROR_RECOVERY, 0, error_state.error_code, false};
        queue_async_command(&cmd_queue, &cmd);
    }
}
```

### Memory Management

#### 1. **Pre-allocated Structures**
```c
// All memory allocated at startup - no runtime allocation
#define MAX_AXES 8
#define MAX_ENCODERS 25
#define MAX_DIGITAL_PINS 55

static pe_axis_data_t pe_axes[MAX_AXES];
static encoder_data_t encoders[MAX_ENCODERS];
static digital_pin_data_t digital_pins[MAX_DIGITAL_PINS];
static device_cache_t device_cache;
static command_queue_t cmd_queue;

// No malloc/free in RT context - everything pre-allocated
```

#### 2. **Cache-Friendly Data Layout**
```c
// Structure data for cache efficiency
typedef struct {
    // Hot data - accessed every RT cycle
    float positions[MAX_AXES];
    float velocities[MAX_AXES];
    uint32_t digital_inputs;
    uint32_t axis_states;
    
    // Cold data - accessed less frequently  
    uint32_t device_serial;
    uint32_t firmware_version;
    device_info_t device_info;
} __attribute__((packed)) device_cache_t;
```

### Configuration and Initialization

#### 1. **RT-Safe Parameter Updates**
```c
// Parameter changes through non-RT thread
void update_encoder_scale(int encoder, float new_scale) {
    // Validate parameter
    if (new_scale == 0.0) return;
    
    // Atomic update - RT thread will see consistent value
    __atomic_store_n(&encoders[encoder].scale, new_scale, __ATOMIC_SEQ_CST);
}

// RT thread reads parameters atomically
void rt_update_encoder_position(int encoder) {
    float scale = __atomic_load_n(&encoders[encoder].scale, __ATOMIC_SEQ_CST);
    *encoders[encoder].position = encoders[encoder].current_count / scale;
}
```

### Performance Considerations

#### 1. **Timing Budgets**
```c
// Target timing for 1kHz RT thread (1000µs period):
// - Digital I/O update:     ~20µs
// - Encoder updates:        ~50µs  
// - PEv2 state machines:    ~100µs
// - HAL pin updates:        ~30µs
// - Error handling:         ~10µs
// - Margin for jitter:      ~790µs

// Monitor actual timing
static struct timespec start_time, end_time;
void rt_timing_monitor(void) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // ... RT operations ...
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    long duration_us = (end_time.tv_nsec - start_time.tv_nsec) / 1000;
    if (duration_us > 200) {  // Warning threshold
        // Log timing violation (non-RT context)
        queue_timing_warning(duration_us);
    }
}
```

This architecture ensures:
- **Deterministic RT performance** - No blocking calls in RT thread
- **Robust error handling** - Graceful degradation under fault conditions  
- **Scalable design** - Easy to add features without breaking RT constraints
- **HAL compatibility** - Standard LinuxCNC interface while maintaining RT safety
