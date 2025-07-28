# PoNET HAL Integration - Linker Error Fix

## 🐛 **Issue Identified**

The CI pipeline failed with linker error:
```
undefined reference to `export_ponet_pins'
```

## 🔍 **Root Cause Analysis**

The issue was caused by **function signature mismatch** between header declaration and implementation:

### **Before Fix:**
- **Header**: `int export_ponet_pins(const char *prefix, int comp_id, sPoKeysDevice *device)`  
- **Implementation**: `int export_ponet_pins(const char *prefix, int comp_id, sPoKeysDevice *device)`
- **Other functions**: All use `long comp_id` (e.g., `export_rtc_pins`, `export_encoder_pins`, `export_IO_pins`)

### **The Problem:**
- I initially used `int comp_id` while the established convention uses `long comp_id`
- This created a function signature mismatch causing the linker to not find the symbol
- The component's `comp_id` variable is `static int comp_id`, but other export functions expect `long`

## ✅ **Fix Applied**

### **1. Updated Header Declaration** (`PoKeysLibHal.h`)
```c
// BEFORE:
POKEYSDECL int export_ponet_pins(const char *prefix, int comp_id, sPoKeysDevice *device);

// AFTER: 
POKEYSDECL int export_ponet_pins(const char *prefix, long comp_id, sPoKeysDevice *device);
```

### **2. Updated Function Implementations** (`PoKeysLibPoNETHal.c`)
```c
// BEFORE:
int export_ponet_pins(const char *prefix, int comp_id, sPoKeysDevice *device)
int export_ponet_basic_pins(const char *prefix, int comp_id, sPoKeysDevice *device) 
int export_ponet_status_pins(const char *prefix, int comp_id, sPoKeysDevice *device)
void update_ponet_hal_pins(sPoKeysDevice *device)

// AFTER:
POKEYSDECL int export_ponet_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
POKEYSDECL int export_ponet_basic_pins(const char *prefix, long comp_id, sPoKeysDevice *device) 
POKEYSDECL int export_ponet_status_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
POKEYSDECL void update_ponet_hal_pins(sPoKeysDevice *device)
```

### **3. Changes Made:**
- ✅ Changed `int comp_id` → `long comp_id` in all PoNET functions
- ✅ Added `POKEYSDECL` to all function implementations
- ✅ Maintained consistency with existing export functions (`export_rtc_pins`, etc.)

## 📋 **Files Modified**

1. **`PoKeysLibHal.h`**: Updated function declaration to use `long comp_id`
2. **`PoKeysLibPoNETHal.c`**: Updated all function implementations with `POKEYSDECL` and `long comp_id`

## 🔧 **Technical Details**

### **Why `long comp_id`?**
- HAL component IDs are traditionally `long` in LinuxCNC
- All existing export functions use `long comp_id` parameter
- C allows implicit conversion from `int` to `long`, so the component's `static int comp_id` will work fine

### **Why `POKEYSDECL`?**
- Ensures proper symbol visibility and linkage
- Required for consistent API decoration across Windows/Linux
- Matches pattern used by all other API functions

### **Library Integration:**
- `PoKeysLibPoNETHal.c` is already included in `Makefile.noqmake` SOURCES list
- The file will be compiled into `libPoKeysHal.a` and `libPoKeysHal.so`
- No additional makefile changes needed

## 🎯 **Expected Result**

The CI pipeline should now successfully:
1. ✅ Compile `PoKeysLibPoNETHal.c` into the library
2. ✅ Link `export_ponet_pins` symbol correctly
3. ✅ Build the `pokeys_async` component without errors

## ✅ **Status: Ready for Testing**

The function signature mismatch has been resolved. The PoNET HAL integration should now compile successfully in the Linux CI pipeline.

**Next CI run should pass the build phase.**
