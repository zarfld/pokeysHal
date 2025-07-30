# PoKeys HAL Testing Concept - Debian Integration

**Created:** 2025-07-29  
**Target:** Debian system with LinuxCNC installed  
**Device:** PoKeys device via Ethernet connection  
**Focus:** Real-time component validation with basic functionality first  

---

## 🎯 **TESTING STRATEGY OVERVIEW**

### **Phase 1: Component Loading & Basic Validation (TODAY)**
- Verify RT component builds and installs correctly
- Test component loading in LinuxCNC RT environment
- Validate HAL pin creation and basic connectivity
- Confirm device detection over Ethernet

### **Phase 2: Device Communication Testing**
- Test Ethernet connection stability
- Validate async communication protocols
- Test basic I/O pin operations (digital read/write)
- Monitor connection quality and retry logic

### **Phase 3: HAL Integration Testing**
- Test HAL pin functionality in RT environment
- Validate pin state changes and updates
- Test with minimal LinuxCNC configuration
- Document successful pin mappings

---

## 📋 **DETAILED TEST PLAN**

### **Pre-Test Setup Checklist**
- [ ] PoKeys device powered and connected via Ethernet
- [ ] Device IP address known and pingable
- [ ] LinuxCNC installed and functional
- [ ] RT component built and ready to install
- [ ] Backup of current LinuxCNC configuration

### **Phase 1 Tests: Basic Component Validation**

#### **Test 1.1: Component Build Verification**
```bash
# Run build task to ensure everything compiles
# Verify no compilation errors
# Check that RT component is properly installed
```

#### **Test 1.2: HAL Component Loading**
```bash
# Test loading RT component in halrun
# Verify component loads without errors
# Check HAL pins are created correctly
# Confirm function registration
```

#### **Test 1.3: Device Detection**
```bash
# Test device discovery over network
# Verify connection establishment
# Check device identification and status
```

#### **Test 1.4: Basic HAL Pin Inspection**
```bash
# List all created HAL pins
# Verify pin types and directions
# Check parameter availability
# Document pin naming convention
```

### **Phase 2 Tests: Communication Validation**

#### **Test 2.1: Network Connection Stability**
- Monitor connection establishment time
- Test connection under various network conditions
- Validate retry logic on connection loss
- Document timeout behaviors

#### **Test 2.2: Basic I/O Operations**
- Test digital input pin reading
- Test digital output pin writing
- Verify pin state changes are reflected
- Test analog I/O if available

#### **Test 2.3: Async Communication Performance**
- Monitor communication latency
- Test under RT thread execution
- Validate data integrity
- Check for RT violations

### **Phase 3 Tests: LinuxCNC Integration**

#### **Test 3.1: Minimal Configuration Test**
- Create basic LinuxCNC configuration
- Load PoKeys component in RT environment
- Test HAL pin connections
- Verify component runs in RT context

#### **Test 3.2: Motor Interface Testing**
- Connect HAL pins to motor drivers (safely)
- Test basic enable/disable functionality
- Monitor for proper signal generation
- Document any timing issues

---

## 🛠 **TEST ENVIRONMENT SETUP**

### **Required Tools & Commands**
```bash
# HAL testing tools
halrun          # For component testing
halcmd          # For HAL commands
halmeter        # For pin monitoring
halscope        # For signal analysis (if needed)

# Network tools
ping            # For connectivity testing
netstat         # For connection monitoring
tcpdump         # For packet analysis (if needed)

# LinuxCNC tools
linuxcnc        # For full integration testing
```

### **Test Data Collection**
- Component loading success/failure logs
- HAL pin states and values
- Network connection statistics
- RT performance metrics
- Error messages and debugging info

---

## 📊 **SUCCESS CRITERIA**

### **Phase 1 Success Indicators**
- ✅ RT component loads without errors ✅ **PASSED**
- ✅ HAL pins created and accessible ✅ **PASSED** 
- ✅ Device connection established ✅ **PASSED** (Serial: 27295, IP: 157.247.1.235)
- ✅ Basic pin inspection working ✅ **PASSED** (55 digital I/O, 7 ADC in, 6 PWM out, 26 encoders)

### **Phase 2 Success Indicators**
- ✅ Stable network communication ✅ **PASSED** (RT thread running at 1ms, max time 0.35ms)
- ✅ I/O operations function correctly ✅ **PASSED** (Digital inputs reading TRUE/FALSE, outputs controllable via PEv2)
- ✅ No RT violations detected ✅ **PASSED** (No tmax violations detected)
- ✅ Connection monitoring working ✅ **PASSED** (Device responsive, data flowing correctly)

### **Phase 3 Success Indicators**
- ⚠️ LinuxCNC integration **PARTIALLY SUCCESSFUL** (motmod compatibility issue identified)
- ✅ RT performance acceptable ✅ **PASSED** (Component performs excellently in RT environment)
- ✅ Motor interface connections safe ✅ **PASSED** (All safety connections implemented)
- ✅ Component stable under load ✅ **PASSED** (Stable performance in all tests)

---

## 🚨 **SAFETY PROTOCOLS**

### **Before Each Test**
1. Verify all connections are secure
2. Ensure emergency stop is accessible
3. Confirm motor power can be quickly disabled
4. Have LinuxCNC shutdown procedure ready

### **During Testing**
1. Monitor for unusual behavior
2. Stop immediately if motors behave unexpectedly
3. Log all observations for analysis
4. Keep test sessions short initially

### **Emergency Procedures**
1. LinuxCNC emergency stop: `F1` key
2. Component unload: `halcmd unload all`
3. System restart if needed
4. Device power cycle if communication fails

---

## 📝 **DOCUMENTATION REQUIREMENTS**

### **Test Results Documentation**
- Record all test outcomes (pass/fail)
- Log error messages and debugging info
- Document successful configurations
- Note any workarounds or special procedures

### **Configuration Examples**
- Create working LinuxCNC configuration files
- Document HAL pin mappings
- Provide parameter setting examples
- Create troubleshooting guide

---

## 🔄 **NEXT STEPS AFTER SUCCESSFUL TESTING**

1. **Update Todo.md** with completed items
2. **Create user documentation** based on test results
3. **Optimize performance** based on findings
4. **Plan advanced feature testing** (encoders, analog I/O)
5. **Prepare for production use** documentation

---

## 📞 **SUPPORT & DEBUGGING**

### **Common Issues to Watch For**
- RT violations during communication
- Network timeout issues
- HAL pin naming conflicts
- Device connection drops
- Component loading failures

### **Debugging Tools**
- `dmesg` for kernel messages
- `halcmd show` for HAL state
- LinuxCNC error logs
- Network connection logs

---

*This testing concept provides a structured approach to validating the PoKeys HAL implementation while maintaining safety and ensuring thorough validation of all critical functionality.*
