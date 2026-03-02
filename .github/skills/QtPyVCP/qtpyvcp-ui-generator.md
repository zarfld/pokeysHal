# Copilot Skill: LinuxCNC QtPyVCP UI Generator (No Qt Designer)

## Skill Identity

**Name**: QtPyVCP UI Generator Without Designer  
**Version**: 1.0.0  
**Framework**: QtPyVCP (https://www.qtpyvcp.com/)  
**Target**: LinuxCNC machine control interfaces

## Purpose

Generate complete, deployment-ready QtPyVCP user interface artifacts from natural-language specifications **without requiring Qt Designer**. This skill produces hand-authored, valid `.ui` XML files, matching Python implementations, optional `.qss` stylesheets, and YAML configuration files that integrate seamlessly with LinuxCNC's QtPyVCP framework.

## Framework Context: QtPyVCP vs QtVCP

**CRITICAL DISTINCTION**: This skill targets **QtPyVCP** exclusively, NOT LinuxCNC's built-in QtVCP.

### QtPyVCP Architecture
- **YAML-driven configuration**: VCP behavior defined in `<vcp_name>.yml`
- **Plugin system**: Status, positions, tool table, notifications, etc.
- **Custom widget library**: `qtpyvcp.widgets.hal_widgets`, `qtpyvcp.widgets.display_widgets`
- **Action system**: `qtpyvcp.actions` for machine control
- **Rules engine**: Dynamic widget behavior via `qtpyvcp.widgets.rules`
- **Installation**: Standalone Python package (`pip install qtpyvcp`)

### Key QtPyVCP Components
1. **`.ui` file**: Qt Designer XML (hand-authored here)
2. **`<vcp_name>.yml`**: VCP configuration (optional but recommended)
3. **Python module**: VCP class implementation with plugins/actions
4. **`.qss` file**: Qt stylesheet (optional)
5. **HAL file**: LinuxCNC HAL pin connections (`.hal`)

## When to Use This Skill

### ✅ Use when:
- Creating a new QtPyVCP-based LinuxCNC control panel
- Building custom machine-specific UI tabs or panels
- Generating QtPyVCP UI from textual specifications
- Refactoring existing QtPyVCP screens into modular components
- Creating device-specific control panels (e.g., PoKeys diagnostics)
- Building probe panels, toolsetter interfaces, or work offset managers
- Generating VCP configuration without visual design tools

### ❌ DO NOT use when:
- Target framework is LinuxCNC QtVCP (built-in, non-QtPyVCP)
- Creating generic PyQt/PySide desktop applications
- User explicitly requires Qt Designer workflow
- Building GladeVCP interfaces (GTK-based)
- Target is non-LinuxCNC application

## Expected Inputs

When invoking this skill, provide:

1. **VCP Identity**
   - VCP name (e.g., `pokeys_diagnostics`, `probe_panel`)
   - Target type: full VCP, panel, tab, dialog
   - Machine domain: mill, lathe, probe, diagnostics, setup

2. **UI Structure**
   - Layout organization (sections, groups, tabs)
   - Required widgets and hierarchy
   - Widget arrangement (grid, vertical, horizontal, form)

3. **QtPyVCP-Specific Requirements**
   - Required plugins: status, positions, tool_table, notifications, etc.
   - HAL widget types: `hal_label`, `hal_button`, `hal_led`, `hal_spinbox`, etc.
   - Actions needed: machine commands, file operations, program control
   - Rules engine requirements: conditional visibility, value bindings

4. **LinuxCNC Integration**
   - Required HAL pins and signals
   - Status information to display
   - Machine commands to expose
   - G-code interaction requirements

5. **Styling and Behavior**
   - Color scheme and theme requirements
   - Keyboard shortcuts and hotkeys
   - Custom styling for specific widgets
   - Responsive layout requirements

6. **Deployment Context**
   - File placement within LinuxCNC config
   - INI file integration requirements
   - HAL file setup expectations

## Mandatory Behavioral Rules

### Core Principles
1. **Never assume Qt Designer availability**: All `.ui` XML must be hand-generated
2. **Framework accuracy**: Never invent QtPyVCP widgets, plugins, or APIs that don't exist
3. **Consistency first**: Widget object names must match exactly across `.ui`, Python, `.qss`, and YAML
4. **Layout-based design**: Use Qt layouts (QVBoxLayout, QHBoxLayout, QGridLayout, QFormLayout) unless fixed geometry is explicitly requested
5. **Separate concerns**: UI structure in `.ui`, behavior in Python, configuration in YAML, styling in `.qss`
6. **Validate cross-file references**: Every widget referenced in Python/YAML must exist in `.ui` with exact object name match

### QtPyVCP-Specific Rules
7. **Use QtPyVCP widgets correctly**:
   - `qtpyvcp.widgets.hal_widgets.*` for HAL integration
   - `qtpyvcp.widgets.button_widgets.*` for actions
   - `qtpyvcp.widgets.display_widgets.*` for status display
8. **Plugin integration**: Declare required plugins in YAML config
9. **Action binding**: Use QtPyVCP action system for machine commands
10. **HAL pin naming**: Follow LinuxCNC HAL pin conventions (`<component>.<pin>`)

### Quality Standards
11. **Descriptive naming**: Object names reflect purpose (`btnEmergencyStop`, `lblSpindleSpeed`, `grpWorkOffsets`)
12. **No dead code**: Only generate handlers and slots that are actively used
13. **Clear assumptions**: Document all framework version assumptions, widget availability, and integration points
14. **Validation checklist**: Every generation includes cross-file consistency verification

## Generation Process Workflow

### Phase 1: Analysis and Planning
1. **Parse request** and extract requirements
2. **Classify artifact type**: full VCP, panel, tab, dialog
3. **Confirm QtPyVCP target**: Reject if framework is QtVCP/other
4. **Identify required QtPyVCP components**:
   - Widgets (standard Qt + QtPyVCP custom)
   - Plugins (status, positions, etc.)
   - Actions (machine commands)
   - HAL integration points
5. **Define object naming scheme**: Establish consistent prefixes and conventions BEFORE code generation

### Phase 2: Structure Definition
6. **Map UI hierarchy**:
   - Top-level container (QMainWindow, QWidget, QDialog)
   - Major sections (tabs, groups, splitters)
   - Widget organization within sections
   - Layout strategy for each container
7. **Define widget specifications**:
   - Widget type (Qt standard or QtPyVCP custom)
   - Object name (following naming convention)
   - Properties (size, text, alignment, etc.)
   - Layout constraints (stretch, alignment, margins)
8. **Plan YAML configuration** (if needed):
   - Required plugins
   - Application settings
   - HAL component configuration

### Phase 3: File Generation
9. **Generate `.ui` XML**:
   - Valid Qt UI XML structure
   - Correct widget class names (including QtPyVCP custom widgets)
   - Layout definitions (not fixed geometry unless requested)
   - Property assignments
   - Signal/slot connections (if applicable)
10. **Generate Python implementation**:
    - VCP class inheriting from appropriate base
    - Import statements for QtPyVCP modules
    - Widget reference methods
    - Signal/slot handler methods
    - Plugin initialization
    - Action method implementations
11. **Generate YAML configuration** (if needed):
    - Plugin declarations
    - Application metadata
    - HAL component settings
    - Window properties
12. **Generate `.qss` stylesheet** (if styling requested):
    - Scoped selectors matching object names
    - Theme-consistent color palette
    - Widget-specific styling rules

### Phase 4: Integration Artifacts
13. **Generate LinuxCNC integration notes**:
    - INI file snippet (`[DISPLAY]` section)
    - HAL file connection examples
    - File placement instructions
    - Required dependencies
14. **Document QtPyVCP-specific setup**:
    - Plugin requirements
    - Action availability
    - HAL component expectations

### Phase 5: Validation
15. **Cross-file consistency check**:
    - Every Python widget reference exists in `.ui`
    - Every `.qss` selector targets existing object name
    - Every YAML plugin is properly integrated in Python
    - No orphaned signal handlers
    - No undefined widget properties
16. **Generate validation checklist**: Itemized verification steps
17. **Document assumptions and risks**: Framework versions, widget availability, untested edge cases

## Required Output Format

Every skill invocation MUST produce output in this exact structure:

---

### 1. Framework Interpretation
**Framework**: QtPyVCP  
**Artifact Type**: [full VCP | panel | tab | dialog]  
**Domain**: [mill | lathe | probe | diagnostics | setup | custom]  

**Interpretation Summary**:
[Brief explanation of how the request was understood]

**Rejected Interpretations**:
[If any ambiguity was resolved, state what was NOT chosen and why]

---

### 2. Target File Set

**Generated Files**:
- `<vcp_name>.ui` - Qt Designer XML (hand-authored)
- `<vcp_name>/<vcp_name>.py` - VCP Python implementation
- `<vcp_name>/<vcp_name>.yml` - VCP configuration (if applicable)
- `<vcp_name>/<vcp_name>.qss` - Qt stylesheet (if applicable)

**Integration Files** (documentation only, not generated):
- `<config_name>.ini` - LinuxCNC INI snippet
- `postgui_hal.hal` - HAL connection examples

**File Placement**:
```
~/linuxcnc/configs/<config_name>/
  ├── <config_name>.ini
  ├── postgui_hal.hal
  └── <vcp_name>/
      ├── <vcp_name>.ui
      ├── <vcp_name>.py
      ├── <vcp_name>.yml
      └── <vcp_name>.qss
```

---

### 3. Object Naming Map

**Naming Convention**:
- Prefix: `[type][Domain][Purpose]`
  - `btn` = QPushButton, `lbl` = QLabel, `spn` = QSpinBox, etc.
  - Domain: `Main`, `Probe`, `Offset`, `Diag`, etc.
  - Purpose: Descriptive name

**Widget Inventory**:

| Object Name | Widget Type | Purpose | HAL/Action Binding |
|-------------|-------------|---------|-------------------|
| `btnMainPower` | QPushButton | Machine power toggle | `machine.power.on` action |
| `lblSpindleSpeed` | StatusLabel | Display spindle RPM | `status:spindle.0.speed` |
| ... | ... | ... | ... |

---

### 4. Generated `.ui` File

**File**: `<vcp_name>.ui`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class><vcp_name>_MainWindow</class>
 <widget class="QMainWindow" name="<vcp_name>_MainWindow">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>800</width>
    <height>600</height>
   </rect>
  </property>
  <property name="windowTitle">
   <string>[VCP Title]</string>
  </property>
  <widget class="QWidget" name="centralwidget">
   <layout class="QVBoxLayout" name="verticalLayout">
    <!-- Widget tree here -->
   </layout>
  </widget>
 </widget>
 <customwidgets>
  <!-- QtPyVCP custom widgets declared here -->
 </customwidgets>
 <resources/>
 <connections/>
</ui>
```

**Key Sections**:
- Root `<widget>` with proper class and name
- `<customwidgets>` section for QtPyVCP widgets
- Layout-based structure
- All object names from naming map

---

### 5. Generated Python Implementation

**File**: `<vcp_name>/<vcp_name>.py`

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<VCP Name> - QtPyVCP-based LinuxCNC Interface
Generated by Copilot QtPyVCP UI Generator Skill
"""

from qtpyvcp.widgets.form_widgets.main_window import VCPMainWindow

# QtPyVCP imports
from qtpyvcp import hal
from qtpyvcp.actions import machine_actions
from qtpyvcp.plugins import getPlugin
from qtpyvcp.utilities.logger import getLogger

# Qt imports
from PyQt5.QtCore import Qt, pyqtSlot
from PyQt5.QtWidgets import QMessageBox

LOG = getLogger(__name__)


class <VCPName>(VCPMainWindow):
    """Main window class for <VCP Name>"""
    
    def __init__(self, *args, **kwargs):
        super(<VCPName>, self).__init__(*args, **kwargs)
        
        # Get plugins
        self.status = getPlugin('status')
        
        # Initialize HAL component
        self.hal_comp = hal.getComponent()
        
    def initialize(self):
        """Initialize VCP after UI is loaded"""
        super(<VCPName>, self).initialize()
        
        # Connect signals
        self._connectSignals()
        
        # Additional initialization
        LOG.info("<VCP Name> initialized")
        
    def _connectSignals(self):
        """Connect widget signals to slots"""
        # Example: self.btnMainPower.clicked.connect(self.onPowerToggle)
        pass
        
    # Slot implementations
    @pyqtSlot()
    def onPowerToggle(self):
        """Handle power button toggle"""
        # Implementation here
        pass


def main():
    """Entry point for standalone execution"""
    from qtpyvcp.utilities.logger import initBaseLogger
    initBaseLogger(__name__)
    
    from qtpyvcp import VCPApplication
    app = VCPApplication(theme='<theme_name>')
    app.run()


if __name__ == '__main__':
    main()
```

**Key Components**:
- Inheritance from `VCPMainWindow` or appropriate base
- Plugin initialization
- HAL component setup
- Signal/slot connections
- Slot method implementations
- Standalone entry point

---

### 6. Generated YAML Configuration (if applicable)

**File**: `<vcp_name>/<vcp_name>.yml`

```yaml
name: <VCP Name>
version: 1.0.0
description: [Brief description]

provider:
  name: [Author/Organization]
  
window:
  title: <VCP Name>
  size: [800, 600]
  position: [100, 100]
  
ui_file: <vcp_name>.ui
stylesheet: <vcp_name>.qss

plugins:
  status:
    cycle_time: 100
  positions:
    # Position plugin configuration
  notifications:
    # Notification plugin configuration
    
halcomp:
  name: <vcp_name>
  pins:
    # HAL pin definitions
    
actions:
  # Action definitions
```

**Configuration Sections**:
- VCP metadata
- Window properties
- Required plugins
- HAL component specification
- Custom actions

---

### 7. Generated `.qss` Stylesheet (if applicable)

**File**: `<vcp_name>/<vcp_name>.qss`

```css
/* <VCP Name> Stylesheet */

/* Global styling */
QMainWindow {
    background-color: #2b2b2b;
}

/* Specific widgets by object name */
#btnMainPower {
    background-color: #4CAF50;
    color: white;
    font-size: 14pt;
    border-radius: 5px;
}

#btnMainPower:pressed {
    background-color: #45a049;
}

/* Status labels */
QLabel[isStatusLabel="true"] {
    font-family: "Courier New";
    font-size: 12pt;
}
```

**Styling Approach**:
- Object name selectors (`#objectName`)
- Dynamic properties for categorization
- Theme-consistent color palette
- State-based styling (`:hover`, `:pressed`, etc.)

---

### 8. LinuxCNC Integration Notes

#### INI File Configuration

**File**: `<config_name>.ini`

```ini
[DISPLAY]
DISPLAY = qtpyvcp <vcp_name>

# Or for installed VCP:
# DISPLAY = qtpyvcp <vcp_name>.<vcp_name>

[APPLICATIONS]
# Optional: Additional tools
```

#### HAL File Connections

**File**: `postgui_hal.hal`

```hal
# Net HAL signals to QtPyVCP pins
# (Auto-created pins based on .yml configuration)

# Example:
# net spindle-speed => <vcp_name>.spindle-speed
# net machine-on => <vcp_name>.machine-on
```

#### Installation Steps

1. **Place VCP files** in LinuxCNC config directory:
   ```bash
   mkdir -p ~/linuxcnc/configs/<config_name>/<vcp_name>
   cp <vcp_name>.ui <vcp_name>.py <vcp_name>.yml <vcp_name>.qss \
      ~/linuxcnc/configs/<config_name>/<vcp_name>/
   ```

2. **Update INI file** with `DISPLAY = qtpyvcp <vcp_name>`

3. **Create/update HAL file** with pin connections

4. **Test VCP** standalone:
   ```bash
   cd ~/linuxcnc/configs/<config_name>
   qtpyvcp <vcp_name>
   ```

#### Dependencies

- QtPyVCP installed: `pip install qtpyvcp`
- LinuxCNC 2.8+ recommended
- Python 3.6+
- PyQt5

---

### 9. Validation Checklist

**Cross-File Consistency**:
- [ ] Every widget referenced in Python exists in `.ui` with exact object name
- [ ] Every `.qss` selector references existing object names or valid classes
- [ ] Every YAML plugin is initialized in Python implementation
- [ ] No orphaned signal/slot connections
- [ ] All custom widgets declared in `.ui` `<customwidgets>` section

**QtPyVCP Framework Compliance**:
- [ ] Only documented QtPyVCP widgets are used
- [ ] Plugin initialization follows QtPyVCP patterns
- [ ] HAL component creation uses `hal.getComponent()`
- [ ] Actions use QtPyVCP action system (not raw commands)
- [ ] Status information accessed via status plugin

**UI Structure Quality**:
- [ ] Layouts used instead of fixed geometry (unless explicitly requested)
- [ ] Widget hierarchy is logical and maintainable
- [ ] No deeply nested layouts (max 3-4 levels preferred)
- [ ] Responsive design principles applied

**Code Quality**:
- [ ] Python follows PEP 8 style guidelines
- [ ] All imports are valid and available
- [ ] No placeholder methods unless marked as TODO
- [ ] Error handling included where appropriate
- [ ] Logging statements added for debugging

**Integration Readiness**:
- [ ] File placement instructions are clear
- [ ] INI snippet is copy-paste ready
- [ ] HAL examples match expected pin names
- [ ] Dependencies are documented

**Documentation Completeness**:
- [ ] Object naming map is complete
- [ ] All assumptions are documented
- [ ] Open issues are clearly stated
- [ ] Next steps are actionable

---

### 10. Assumptions and Open Issues

#### Framework Assumptions
- **QtPyVCP Version**: [e.g., 0.4.0+]
- **LinuxCNC Version**: [e.g., 2.8+]
- **Python Version**: [e.g., 3.6+]
- **Qt Version**: [e.g., PyQt5]

#### Widget Availability
[List any custom QtPyVCP widgets assumed to exist]

#### Integration Assumptions
[List assumptions about HAL configuration, machine setup, etc.]

#### Known Limitations
[Document any features not implemented or edge cases not handled]

#### Open Issues
[List any ambiguities that require user clarification]

#### Suggested Next Steps
1. [First action item]
2. [Second action item]
3. [etc.]

---

## QtPyVCP Widget Reference

### Common QtPyVCP HAL Widgets

**From `qtpyvcp.widgets.hal_widgets`**:

| Widget Class | Purpose | Key Properties |
|--------------|---------|----------------|
| `HalLabel` | Display HAL pin value | `pin`, `formatter` |
| `HalButton` | Button with HAL output | `pin`, `checked_state_string` |
| `HalCheckBox` | Checkbox with HAL output | `pin` |
| `HalSpinBox` | Numeric input with HAL output | `pin`, `minimum`, `maximum` |
| `HalSlider` | Slider with HAL output | `pin`, `orientation` |
| `HalLed` | LED indicator from HAL pin | `pin`, `on_color`, `off_color` |
| `HalProgressBar` | Progress bar from HAL pin | `pin`, `min_value`, `max_value` |
| `HalDial` | Rotary dial control | `pin` |

### QtPyVCP Display Widgets

**From `qtpyvcp.widgets.display_widgets`**:

| Widget Class | Purpose | Key Properties |
|--------------|---------|----------------|
| `StatusLabel` | Display status channel value | `channel` (status plugin) |
| `DROWidget` | Digital Readout | `axis`, `reference_type` |
| `GcodeViewer` | G-code text display | `gcodes` |
| `GcodeBackplot` | 3D toolpath preview | N/A |

### QtPyVCP Button Widgets

**From `qtpyvcp.widgets.button_widgets`**:

| Widget Class | Purpose | Action Binding |
|--------------|---------|----------------|
| `ActionButton` | Execute QtPyVCP action | `action_name` property |

### Standard Qt Widgets (Always Available)

- `QWidget`, `QPushButton`, `QLabel`, `QLineEdit`, `QSpinBox`, `QDoubleSpinBox`
- `QCheckBox`, `QRadioButton`, `QComboBox`, `QSlider`, `QProgressBar`
- `QGroupBox`, `QTabWidget`, `QStackedWidget`, `QSplitter`
- `QListWidget`, `QTableWidget`, `QTreeWidget`
- `QVBoxLayout`, `QHBoxLayout`, `QGridLayout`, `QFormLayout`

## QtPyVCP Actions Reference

### Machine Control Actions

**From `qtpyvcp.actions.machine_actions`**:

- `machine.power.on()` / `machine.power.off()` - Machine power control
- `machine.home.all()` / `machine.home.axis(axis)` - Homing
- `machine.estop.activate()` / `machine.estop.reset()` - E-stop control
- `machine.mode.manual()` / `machine.mode.auto()` / `machine.mode.mdi()` - Mode switching

### Program Control Actions

**From `qtpyvcp.actions.program_actions`**:

- `program.run()` - Start program execution
- `program.pause()` - Pause execution
- `program.resume()` - Resume from pause
- `program.stop()` - Stop program
- `program.step()` - Single step execution

### File Actions

- `file.open(filename)` - Load G-code file
- `file.reload()` - Reload current file
- `file.edit()` - Open editor

### Configuration Note
Actions can be bound to buttons in `.ui` file or triggered programmatically in Python.

## QtPyVCP Plugins Reference

### Status Plugin
**Access**: `getPlugin('status')`  
**Purpose**: Real-time machine status information

**Key Methods**:
- `status.machine.is_on()` - Machine power state
- `status.estop.is_activated()` - E-stop state
- `status.motion.is_homed()` - Homing state
- `status.spindle.0.speed` - Spindle speed
- `status.axis.position` - Axis positions

### Positions Plugin
**Access**: `getPlugin('positions')`  
**Purpose**: Axis position management

### Tool Table Plugin
**Access**: `getPlugin('tool_table')`  
**Purpose**: Tool table management

### Notifications Plugin
**Access**: `getPlugin('notifications')`  
**Purpose**: User notifications and messages

## Common Patterns and Examples

### Pattern 1: Emergency Stop Button

**In `.ui`**:
```xml
<widget class="QPushButton" name="btnEstop">
  <property name="text">
    <string>E-STOP</string>
  </property>
</widget>
```

**In Python**:
```python
from qtpyvcp.actions.machine_actions import estop

def initialize(self):
    self.btnEstop.clicked.connect(estop.activate)
```

### Pattern 2: Status Display Label

**In `.ui`**:
```xml
<widget class="StatusLabel" name="lblSpindleSpeed">
  <property name="channel">
    <string>status:spindle.0.speed</string>
  </property>
  <property name="formatter">
    <string>%.1f RPM</string>
  </property>
</widget>
```

**Custom widget declaration**:
```xml
<customwidget>
  <class>StatusLabel</class>
  <extends>QLabel</extends>
  <header>qtpyvcp.widgets.display_widgets.status_label</header>
</customwidget>
```

### Pattern 3: HAL LED Indicator

**In `.ui`**:
```xml
<widget class="HalLed" name="ledMachineOn">
  <property name="pin">
    <string>machine-is-on</string>
  </property>
  <property name="on_color">
    <color>
      <red>0</red>
      <green>255</green>
      <blue>0</blue>
    </color>
  </property>
</widget>
```

**Custom widget declaration**:
```xml
<customwidget>
  <class>HalLed</class>
  <extends>QWidget</extends>
  <header>qtpyvcp.widgets.hal_widgets.hal_led</header>
</customwidget>
```

## Anti-Patterns to Avoid

### ❌ NEVER Do These:

1. **Inventing Widgets**: Don't create references to QtPyVCP widgets that don't exist
2. **Wrong HAL Access**: Don't bypass `hal.getComponent()` with direct HAL calls
3. **Status Polling**: Don't poll status; use status plugin reactive channels
4. **Blocking Operations**: Don't block UI thread with long operations
5. **Mixed Frameworks**: Don't mix QtVCP and QtPyVCP patterns
6. **Fixed Geometry**: Don't use absolute positioning unless explicitly required
7. **Magic Numbers**: Don't hardcode widget IDs or indexes
8. **Bypassing Actions**: Don't send raw MDI commands instead of using action system

## Testing and Validation

### Standalone Testing
```bash
qtpyvcp <vcp_name>
```

### With LinuxCNC Simulator
```bash
linuxcnc ~/linuxcnc/configs/<config_name>/<config_name>.ini
```

### HAL Pin Verification
```bash
halcmd show pin <vcp_name>.*
halcmd show sig
```

### Load Testing
```bash
# Show component load
halcmd show comp

# Monitor specific pins
halcmd watch <pin_name>
```

## Version Control and Maintenance

### Git Commit Message Format
```
feat(qtpyvcp): add <vcp_name> diagnostic panel

- Generated .ui with layout-based design
- Implemented Python handler with status plugin
- Added YAML configuration for HAL pins
- Created QSS stylesheet for dark theme

Refs: #<issue_number>
```

### Documentation Updates
When modifying generated VCP:
1. Update object naming map if widgets change
2. Regenerate validation checklist
3. Update assumptions if framework version changes
4. Maintain consistency across `.ui`, Python, YAML, and `.qss`

## Troubleshooting Guide

### Issue: Widget Not Found
**Symptom**: `AttributeError: 'VCPName' object has no attribute 'widgetName'`  
**Fix**: Verify widget `objectName` in `.ui` matches Python reference exactly (case-sensitive)

### Issue: Custom Widget Not Rendering
**Symptom**: Widget shows as placeholder in runtime  
**Fix**: Check `<customwidgets>` section in `.ui` has correct class and header path

### Issue: HAL Pin Not Created
**Symptom**: `halcmd show pin` doesn't show expected pins  
**Fix**: Verify YAML `halcomp.pins` section and ensure `hal.getComponent()` is called in Python

### Issue: Action Not Working
**Symptom**: Button click has no effect  
**Fix**: Check action name is valid, signal connection is made, and LinuxCNC is in correct mode

### Issue: Status Display Not Updating
**Symptom**: Status widgets show stale or zero values  
**Fix**: Ensure status plugin cycle time is reasonable (50-100ms), verify channel name syntax

## Success Criteria

A successfully generated QtPyVCP UI artifact set is complete when:

✅ User can place files in LinuxCNC config directory without modification  
✅ `.ui` file is valid XML and renders without errors in QtPyVCP  
✅ Python implementation loads without import errors  
✅ All widget references resolve correctly  
✅ HAL pins are created as expected  
✅ Actions execute machine commands properly  
✅ Status displays update in real-time  
✅ Styling renders consistently  
✅ YAML configuration is valid and parseable  
✅ Integration notes are accurate and testable  
✅ Validation checklist items all pass  

## Skill Metadata

**Skill Type**: Generative (creates new artifacts)  
**Output Type**: Multi-file artifact set (UI, Python, YAML, CSS, documentation)  
**Complexity**: High (requires framework-specific knowledge)  
**Dependencies**: QtPyVCP framework understanding, Qt UI XML schema, Python, YAML  
**Validation**: Cross-file consistency checks, framework compliance verification  

## References and Resources

### Official QtPyVCP Documentation
- **Main Site**: https://www.qtpyvcp.com/
- **GitHub Repo**: https://github.com/kcjengr/qtpyvcp
- **Configuration Guide**: https://www.qtpyvcp.com/configuration/yml_config.html
- **Widget Reference**: https://www.qtpyvcp.com/widgets/index.html
- **HAL Integration**: https://www.qtpyvcp.com/hal.html
- **Actions Reference**: https://www.qtpyvcp.com/actions/index.html
- **Plugins Guide**: https://www.qtpyvcp.com/plugins/index.html

### Example VCPs
- **Status Labels Example**: https://github.com/kcjengr/qtpyvcp/blob/main/src/examples/status-labels.ui
- **File Chooser Example**: https://github.com/kcjengr/qtpyvcp/blob/main/src/examples/file-chooser.ui
- **Tool Table Example**: https://github.com/kcjengr/qtpyvcp/blob/main/src/examples/tool-table.ui

### QtPyVCP Source Code
- **HAL Widgets**: https://github.com/kcjengr/qtpyvcp/tree/main/src/qtpyvcp/widgets/hal_widgets
- **Display Widgets**: https://github.com/kcjengr/qtpyvcp/tree/main/src/qtpyvcp/widgets/display_widgets
- **Button Widgets**: https://github.com/kcjengr/qtpyvcp/tree/main/src/qtpyvcp/widgets/button_widgets

### LinuxCNC Documentation
- **LinuxCNC HAL**: https://linuxcnc.org/docs/html/hal/intro.html
- **INI Configuration**: https://linuxcnc.org/docs/html/config/ini-config.html

### Qt Documentation
- **Qt Designer UI Format**: https://doc.qt.io/qt-5/designer-ui-file-format.html
- **Qt Layouts**: https://doc.qt.io/qt-5/layout.html
- **Qt Stylesheets (QSS)**: https://doc.qt.io/qt-5/stylesheet-reference.html

---

## Skill Invocation Example

**User Request**:
> "Create a QtPyVCP diagnostic panel for PoKeys device monitoring. It should show device connection status, firmware version, input pin states (8 digital inputs), and output pin states (8 digital outputs). Include refresh and connect/disconnect buttons. Use a dark theme."

**Expected Skill Response**:
[Follow complete output format from sections 1-10 above, generating all files with proper QtPyVCP patterns]

---

**Skill Version**: 1.0.0  
**Last Updated**: 2026-03-02  
**Maintainer**: Copilot QtPyVCP Skill Generator  
**Status**: Active
