# QtPyVCP UI Generator Skill

This directory contains the Copilot Skill for generating QtPyVCP-based LinuxCNC user interfaces **without Qt Designer**.

## Skill Files

- **`qtpyvcp-ui-generator.md`** - Complete skill specification and reference
- **`notes.md`** - Reference links and background information
- **`README.md`** - This file

## Quick Start

### When to Use This Skill

Invoke this skill when you need to:
- Create a new QtPyVCP control panel or screen for LinuxCNC
- Generate custom machine-specific UI components
- Build device diagnostic panels (e.g., PoKeys, ModBus, VFD)
- Create probe panels, tool setters, or work offset managers
- Generate UI from textual specifications without visual design tools

### What This Skill Generates

Each invocation produces a complete, deployment-ready artifact set:
1. **`.ui` file** - Hand-authored Qt Designer XML with proper structure and layouts
2. **Python implementation** - VCP class with plugins, actions, and HAL integration
3. **YAML configuration** - VCP settings, plugin declarations, HAL pins (if needed)
4. **`.qss` stylesheet** - Qt styling (if requested)
5. **Integration documentation** - INI snippets, HAL examples, installation steps
6. **Validation checklist** - Cross-file consistency verification

## Sample Invocations

### Example 1: Simple Status Display Panel

```
Create a QtPyVCP status panel named "machine_status" that displays:
- Current machine mode (Manual/MDI/Auto)
- Spindle speed with RPM readout
- Feed rate override slider
- 3 axis DRO widgets (X, Y, Z)
- E-stop button with red background
Use a vertical layout and include dark theme styling.
```

### Example 2: PoKeys Diagnostic Panel

```
Generate a QtPyVCP diagnostic panel for PoKeys device called "pokeys_diag".
Requirements:
- Device connection status LED (green=connected, red=disconnected)
- Firmware version label
- 8x2 grid of digital input LEDs with pin labels
- 8x2 grid of digital output toggle buttons
- Connect/Disconnect action buttons
- Refresh button to update status
- Use QtPyVCP status plugin for real-time updates
- Dark theme with green/red accent colors
```

### Example 3: Probe Panel

```
Create a QtPyVCP probe panel named "probe_panel" with:
- Work offset selection (G54-G59)
- Probe type selector (Edge Finder, Touch Probe, Tool Setter)
- XYZ position display with current coordinates
- Action buttons: Probe X+, Probe X-, Probe Y+, Probe Y-, Probe Z
- Probe result display showing measured values
- Clear results button
- Use QtPyVCP actions for probe commands
- Include HAL pin configuration for probe input signal
```

## Framework: QtPyVCP vs QtVCP

**CRITICAL**: This skill is for **QtPyVCP** only, NOT LinuxCNC's built-in QtVCP.

### How to Identify QtPyVCP Requirements

Your request likely targets QtPyVCP if it mentions:
- Python-based VCP development
- YAML configuration files
- QtPyVCP plugins (status, positions, notifications)
- Standalone VCP installation (`pip install qtpyvcp`)
- Custom HAL widgets from `qtpyvcp.widgets.hal_widgets`

### How to Identify QtVCP (Not This Skill)

Your request targets QtVCP (standard LinuxCNC, not this skill) if it mentions:
- Built-in LinuxCNC screens (Axis, Gmoccapy, Touchy)
- VCP panels embedded in existing LinuxCNC GUIs
- PyVCP (simple pin-based panels)
- GladeVCP (GTK-based panels)

## Output Structure

Every generation follows this structure:

```
1. Framework Interpretation
   └─ QtPyVCP confirmation, artifact type classification

2. Target File Set
   └─ List of files to be generated with placement instructions

3. Object Naming Map
   └─ Complete widget inventory with naming convention

4. Generated .ui File
   └─ Valid Qt Designer XML with layouts and custom widgets

5. Generated Python Implementation
   └─ VCP class with plugins, HAL, actions, signal handlers

6. Generated YAML Configuration (if applicable)
   └─ VCP settings, plugin config, HAL pin definitions

7. Generated .qss Stylesheet (if applicable)
   └─ Qt styling rules with object name selectors

8. LinuxCNC Integration Notes
   └─ INI snippets, HAL examples, installation steps

9. Validation Checklist
   └─ Cross-file consistency verification items

10. Assumptions and Open Issues
    └─ Framework versions, limitations, next steps
```

## Key Skill Features

### ✅ Always Does
- Generates valid, hand-authored `.ui` XML without Qt Designer
- Ensures exact object name consistency across all files
- Uses layout-based design (QVBoxLayout, QHBoxLayout, QGridLayout)
- Follows QtPyVCP patterns for plugins, actions, and HAL integration
- Validates cross-file references before output
- Documents all assumptions and framework requirements
- Provides copy-paste-ready integration code

### ❌ Never Does
- Assumes Qt Designer is available
- Invents non-existent QtPyVCP widgets or APIs
- Mixes QtVCP and QtPyVCP patterns
- Uses fixed geometry positioning (unless explicitly requested)
- Generates placeholder code without clear documentation
- Bypasses QtPyVCP action system with raw commands
- Creates invalid UI XML structures

## Widget Reference Quick Guide

### QtPyVCP HAL Widgets (from `qtpyvcp.widgets.hal_widgets`)
- `HalLabel` - Display HAL pin value
- `HalButton` - Button with HAL output pin
- `HalCheckBox` - Checkbox with HAL output
- `HalSpinBox` - Numeric input with HAL output
- `HalLed` - LED indicator from HAL pin
- `HalProgressBar` - Progress bar from HAL value
- `HalSlider` - Slider control with HAL output

### QtPyVCP Display Widgets (from `qtpyvcp.widgets.display_widgets`)
- `StatusLabel` - Display LinuxCNC status values
- `DROWidget` - Digital Readout for axis position
- `GcodeViewer` - G-code text display
- `GcodeBackplot` - 3D toolpath preview

### QtPyVCP Actions (from `qtpyvcp.actions`)
- `machine.power.on/off` - Machine power control
- `machine.home.all/axis` - Homing commands
- `machine.estop.activate/reset` - E-stop control
- `program.run/pause/stop` - Program execution

### Standard Qt Widgets (Always Available)
- Buttons: `QPushButton`, `QCheckBox`, `QRadioButton`
- Inputs: `QLineEdit`, `QSpinBox`, `QComboBox`, `QSlider`
- Displays: `QLabel`, `QProgressBar`, `QLCDNumber`
- Containers: `QGroupBox`, `QTabWidget`, `QSplitter`
- Layouts: `QVBoxLayout`, `QHBoxLayout`, `QGridLayout`, `QFormLayout`

## Validation Checklist Template

Use this checklist after each generation:

**Cross-File Consistency**:
- [ ] Every Python widget reference exists in `.ui` with exact object name
- [ ] Every `.qss` selector targets existing object names
- [ ] Every YAML plugin is initialized in Python
- [ ] No orphaned signal/slot connections
- [ ] Custom widgets declared in `.ui` `<customwidgets>` section

**QtPyVCP Compliance**:
- [ ] Only documented QtPyVCP widgets used
- [ ] HAL component via `hal.getComponent()`
- [ ] Status via `getPlugin('status')`
- [ ] Actions use `qtpyvcp.actions` system

**Quality**:
- [ ] Layout-based (not fixed geometry)
- [ ] Descriptive object names
- [ ] No placeholder code unless marked TODO
- [ ] Logging statements included

**Integration**:
- [ ] INI snippet is valid
- [ ] HAL examples match expected pins
- [ ] File placement is clear
- [ ] Dependencies documented

## Troubleshooting Common Issues

### Widget Not Found Error
**Problem**: `AttributeError: 'VCPClass' object has no attribute 'widgetName'`  
**Solution**: Check object name in `.ui` matches Python reference exactly (case-sensitive)

### Custom Widget Renders as Placeholder
**Problem**: Widget shows as grey box at runtime  
**Solution**: Verify `<customwidgets>` section in `.ui` has correct `<class>` and `<header>`

### HAL Pins Not Created
**Problem**: `halcmd show pin` doesn't show expected pins  
**Solution**: Check YAML `halcomp.pins` and ensure `hal.getComponent()` is called

### Action Button Does Nothing
**Problem**: Button click has no effect  
**Solution**: Verify action name is valid and signal connection exists

## Installation and Testing

### Install QtPyVCP
```bash
pip install qtpyvcp
```

### Test VCP Standalone
```bash
cd ~/linuxcnc/configs/<config_name>
qtpyvcp <vcp_name>
```

### Test with LinuxCNC
```bash
linuxcnc ~/linuxcnc/configs/<config_name>/<config_name>.ini
```

### Verify HAL Pins
```bash
halcmd show pin <vcp_name>.*
halcmd show sig
```

## File Placement

Generated files should be placed in LinuxCNC configuration directory:

```
~/linuxcnc/configs/<config_name>/
  ├── <config_name>.ini          # LinuxCNC configuration
  ├── postgui_hal.hal             # HAL connections
  └── <vcp_name>/                 # VCP directory
      ├── <vcp_name>.ui           # UI definition
      ├── <vcp_name>.py           # Python implementation
      ├── <vcp_name>.yml          # VCP configuration
      └── <vcp_name>.qss          # Stylesheet
```

Update `<config_name>.ini`:
```ini
[DISPLAY]
DISPLAY = qtpyvcp <vcp_name>
```

## Best Practices

### Object Naming Convention
- **Prefix by type**: `btn` (button), `lbl` (label), `spn` (spinbox), `led` (LED), `grp` (group)
- **Include domain**: `Main`, `Probe`, `Offset`, `Diag`, `Status`
- **Descriptive suffix**: Purpose of widget
- **Example**: `btnMainEmergencyStop`, `lblStatusSpindleSpeed`, `ledDiagConnection`

### Layout Strategy
1. **Use layouts over fixed geometry** - More maintainable and responsive
2. **Nest layouts logically** - Group related widgets
3. **Set size policies** - Control widget expansion behavior
4. **Add spacers** - Use `QSpacerItem` for elegant spacing
5. **Limit nesting depth** - Max 3-4 levels for readability

### HAL Integration
1. **Use QtPyVCP HAL widgets** - Don't create raw HAL pins in Python unless necessary
2. **Leverage status plugin** - React to status changes, don't poll
3. **Document pin names** - Clear comments in YAML and HAL files
4. **Namespace pins** - Use VCP name prefix: `<vcp_name>.<pin>`

### Code Organization
1. **Separate concerns** - UI structure in `.ui`, behavior in Python, config in YAML
2. **Use logging** - Include `LOG.info/debug/error` statements
3. **Handle errors gracefully** - Try/except around HAL/LinuxCNC calls
4. **Comment plugin usage** - Document which plugins are required

## Example: Complete Invocation

**User Request**:
```
Create a QtPyVCP tool setter panel named "tool_setter" for my mill.
It should have:
- Current tool number display
- Tool length offset display (G43)
- Tool diameter input field
- "Measure Tool Length" button that executes probe routine
- "Set Tool Offset" button
- "Clear Offset" button
- Status message area for feedback
- Dark grey background with blue accent buttons
Use status plugin for real-time tool data.
```

**Skill Output**: Complete artifact set following all 10 sections in output format

## References

- **Skill Specification**: `qtpyvcp-ui-generator.md` (this directory)
- **QtPyVCP Official Docs**: https://www.qtpyvcp.com/
- **QtPyVCP GitHub**: https://github.com/kcjengr/qtpyvcp
- **LinuxCNC HAL**: https://linuxcnc.org/docs/html/hal/intro.html
- **Qt UI XML Format**: https://doc.qt.io/qt-5/designer-ui-file-format.html

## Support and Feedback

For issues or improvements to this skill:
1. Check the validation checklist - most issues are cross-file inconsistencies
2. Review QtPyVCP documentation for widget/plugin availability
3. Test generated VCP in standalone mode before LinuxCNC integration
4. Submit issues with complete generated artifact set for review

---

**Skill Version**: 1.0.0  
**Last Updated**: 2026-03-02  
**Status**: Active and Ready for Use
