# Terminal Text Mode

Text Mode provides a human-readable interface for interacting with the terminal. It allows users to execute predefined commands via text input, offering a simple and accessible way to configure and query the device's functionality.

---

## Overview

In Text Mode, commands are entered as plain text strings. Each command may include parameters to specify additional details. The mode supports standard editing keys like backspace and enter for user convenience.

### Key Features:
- Commands are processed line-by-line when the Enter key is pressed.
- Basic editing with backspace.
- Validation of input to prevent invalid commands or parameters.

---

## Supported Commands

Below is a list of commands supported by Text Mode, along with their usage and descriptions:

### 1. `reset`
Resets the device and enters bootloader mode.

**Usage**
```bash
3-key>reset
```

**Description**

Restarts the device and prepares it for firmware updates.

---

### 2. `erase`
Erases all data in flash storage.

**Usage**
```bash
3-key>erase
```

**Description**

- Clears all data stored in flash memory
- Returns a confirmation log message upon success

---

### 3. `change_color`
Changes the color of a specific button.

**Usage**
```bash
3-key>change_color <button_id> <color>
```

**Parameters**

- <button_id>: Numeric ID of the button (must be a valid button ID)
- <color>: Color name (red, green, or blue)

**Example**
```bash
3-key>change_color 1 red
```

**Description**

- Changes the color of the button identified by <button_id> to the specified <color>
- Logs an error message if the button ID or color is invalid

---

### 4. `feature`
Enables or switches to a specific feature.

**Usage**
```bash
3-key>feature <feature_name>
```

**Parameters**

- `<feature_name>`: Name of the feature to enable. Supported values:
    - `none`: Disables all features
    - `ctrl_c_v`: Enables copy-paste functionality
    - `time-tracker`: Activates time tracking

**Example**

```bash
3-key>feature time-tracker
```

**Description**

- Enables the specified feature and disables any previously active features
- Logs a success message or an error if the feature name is invalid

---

### 5. `time`
Fetches time-tracking logs for different categories.

**Usage**
```bash
3-key>time <category>
```

**Parameters**
- `<category>`: Time log category. Supported values:
    - `work`: Fetches work time logs
    - `meetings`: Fetches meeting time logs

**Example**
```bash
3-key>time work
```

**Description**

- Retrieves time logs for the specified category
- Returns an error if the time-tracker feature is not active

---

## Command Parsing and Processing

### Command Execution Workflow
1. User inputs a command string and presses Enter
2. The string is split into the command name and parameters
3. The system maps the command name to a predefined operation
4. Parameters are validated, and the command is dispatched
5. Logs are updated with the result (success or error)

---

## Logs and Feedback

Logs provide immediate feedback on the execution of commands. They include:

- Success messages when commands execute correctly
- Error messages for invalid commands or parameters

**Example Logs**:
```text
Flash storage erased
Changing button 1 color to red
Error: change_color requires 2 parameters
```

---

This documentation provides an overview of Text Mode, its supported commands, and usage guidelines. For advanced configurations or new features, consult the development team or refer to future updates.
