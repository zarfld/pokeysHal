# LinuxCNC Integration Testing Plan

**Date:** 2025-07-29 (Created for next session)
**Status:** 📋 PLANNED
**Priority:** High
**Type:** Integration Testing

## Objective
Test the completed RT component with actual LinuxCNC installation and physical PoKeys device to validate real-world functionality.

## Prerequisites Completed ✅
- RT component compiles successfully
- All dependencies resolved (PK57i_Disconnect fixed)
- Connection monitoring and retry logic restored
- HAL pins properly defined
- Build system working (both userspace and RT)

## Test Environment Setup

### Required Hardware
- [ ] PoKeys device (USB or Ethernet)
- [ ] Test machine with LinuxCNC installed
- [ ] Basic I/O components for testing (LEDs, switches, etc.)

### LinuxCNC Configuration
- [ ] Create test HAL configuration file
- [ ] Load pokeys_async RT component
- [ ] Verify HAL pins are available
- [ ] Test basic pin connectivity

## Testing Phases

### Phase 1: Component Loading ⏳
```bash
# Test RT component loading
halrun
loadrt threads name1=test-thread period1=1000000
loadrt pokeys_async
show funct
show pin
show param
addf pokeys-async.0 test-thread
start
```

**Expected Results:**
- Component loads without errors
- HAL pins visible
- Functions available for thread assignment

### Phase 2: Device Communication ⏳
- [ ] Test device discovery and connection
- [ ] Validate async communication
- [ ] Test connection monitoring features
- [ ] Verify retry logic under failure conditions

### Phase 3: HAL Pin Functionality ⏳
- [ ] Test digital input pins
- [ ] Test digital output pins
- [ ] Test analog input pins
- [ ] Test encoder pins
- [ ] Verify pin state updates

### Phase 4: Performance Validation ⏳
- [ ] Measure RT performance characteristics
- [ ] Test under various load conditions
- [ ] Validate timing requirements
- [ ] Check for RT violations

## Key Features to Validate

### Connection Monitoring (NEW - Fixed Today)
```c
// Test new connection quality features
PK_DeviceConnectionQualityAsync(device, &quality);
PK_DeviceConnectionFailureAsync(device); // Failure handling
```

### Device Status Monitoring (NEW - Fixed Today)
- CPU load monitoring
- USB/Network load tracking
- Temperature monitoring
- Buffer utilization
- Error count tracking

### Retry Logic (NEW - Fixed Today)
- Consecutive failure counting
- Connection quality degradation
- Automatic recovery attempts

## Success Criteria

### Functional Requirements
- [x] RT component compiles ✅
- [ ] Component loads in LinuxCNC ⏳
- [ ] Device connects successfully ⏳  
- [ ] HAL pins respond to device state ⏳
- [ ] Real-time performance acceptable ⏳

### Reliability Requirements
- [ ] Handles connection loss gracefully ⏳
- [ ] Recovers from device errors ⏳
- [ ] Maintains timing under load ⏳
- [ ] No RT violations detected ⏳

## Test Scripts and Configurations

### Basic HAL Test Configuration
```hal
# Load the RT component
loadrt pokeys_async

# Load threads
loadrt threads name1=servo-thread period1=1000000

# Add function to thread
addf pokeys-async.0 servo-thread

# Start realtime
start

# Test basic functionality
show pin pokeys*
setp pokeys.0.digital-out-00 1
show pin pokeys.0.digital-in-00
```

### Connection Test Script
```bash
#!/bin/bash
# Test connection reliability
for i in {1..100}; do
    echo "Test iteration $i"
    halcmd setp pokeys.0.test-connection 1
    sleep 0.1
    halcmd show pin pokeys.0.connection-status
done
```

## Documentation Tasks
- [ ] Document successful test procedures
- [ ] Create troubleshooting guide for common issues
- [ ] Update HAL pin reference with actual pin names
- [ ] Create example configurations for different use cases

## Known Limitations & Workarounds
- Windows development environment cannot run LinuxCNC tests
- Need Linux environment for actual RT testing  
- Physical device required for full validation
- Network latency may affect connection quality metrics

## Next Steps After Testing
1. **If successful:** Create user documentation and examples
2. **If issues found:** Debug and fix remaining problems
3. **Performance optimization:** Based on test results
4. **Extended testing:** Multiple devices, complex configurations

---

**Preparation for Next Session:**
1. Ensure Linux test environment available
2. Have PoKeys device ready for testing
3. LinuxCNC properly installed and configured
4. Review test scripts and modify as needed

**Files to Reference:**
- `experimental/pokeys_async.c` - RT component implementation
- `experimental/Submakefile.rt` - Build configuration  
- `docs/tasks/completed/RT_Compilation_Header_Organization_Fix.md` - Today's fixes
- HAL pin definitions in component source
