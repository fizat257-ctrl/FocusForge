# FocusForge

FocusForge is a Windows desktop study organizer for keeping coursework tasks and subjects together and providing a configurable focus timer. The application is built with Qt Widgets and stores task, subject, and study-session data with Qt's `QSettings` API.

**Current version:** 1.0.0

## Features

- **Dashboard:** Overview of task and subject counts, completed tasks, total recorded study time, and the three most recently added tasks.
- **Focus Timer:** Select 1, 2, 3, 5, 10, 15, 20, 25, 30, 45, or 60 minutes, with 25 minutes selected by default. Start, Pause, and Reset controls preserve the selected session duration; Start resumes a paused countdown. Optional subject and task selectors are also available. A completed timer saves one study session; paused time is excluded.
- **Tasks Manager:** Create, edit, complete, and delete tasks. Tasks can have a description, due date, and Low, Medium, or High priority.
- **Subjects Manager:** Create, edit, and delete subjects with a name, optional code, teacher, description, and display color.
- **Study Report and Analytics:** Displays total, today's, and weekly study time, a weekly progress chart, subject study-time totals, and task-completion statistics.

## Windows Requirements

- 64-bit Windows. The distributed MSI and application are built for x64.
- Installing the MSI is configured as a per-machine installation and may request administrator approval through the normal Windows UAC prompt.
- The MSI bundles the Qt and MinGW runtime DLLs and the Qt plugins used by the application; separate Qt or MinGW installations are not required to run the packaged application.
- The minimum supported Windows release has not yet been established or documented. Verify compatibility on the intended Windows versions before broad distribution.

## Installation

1. Obtain `FocusForge-1.0.0-win64.msi` from the official FocusForge distribution. In a source checkout, the built installer is at `dist/FocusForge-1.0.0-win64.msi`.
2. Open the MSI and follow the Windows Installer prompts. Approve the standard UAC prompt if shown.
3. Launch FocusForge from the Start Menu. The MSI also creates a Desktop shortcut.

The current MSI is unsigned. Windows may show an unknown-publisher or reputation warning. Verify that the installer came from the official distribution and check its published checksum when available. Do not disable Smart App Control, Defender, or other security controls to install it. If Windows or an organization policy blocks the installer, stop and contact the maintainer.

## Basic Usage

### Dashboard

Use the navigation buttons to open the timer, task list, subjects, and report. The dashboard shows task and subject counts, completed tasks, total study time from saved sessions, and up to three recent task titles.

### Focus Timer

Open **Focus Timer**, select a duration, and optionally select a subject and an incomplete task. Use **Start**, **Pause**, and **Reset** to control the countdown; click **Start** again to resume. The duration selector is locked during an active or paused session. Pause preserves subsecond remaining time; Reset discards the unfinished session and returns to the selected duration. Reaching zero saves one completed session using the selected duration. Closing the application before completion does not save the unfinished session.

### Tasks Manager

Choose **Tasks Manager** and select **Add New Task**. Enter a title; description, due date, and priority are also available. Use a task's checkbox to mark it complete, or use **Edit** and **Delete** to manage it.

### Subjects Manager

Open **Subjects** and choose **Add New Subject**. A subject name is required; code, teacher, description, and color are available as additional details. Existing subjects can be edited or deleted.

### Study Report

Open **Study Report** to view recorded-session summaries, weekly progress, subject-time totals, and task-completion statistics. Session-time values depend on records in the local `StudySessions` settings array. The Dashboard's total study time uses the same saved session records.

## Data Storage and Persistence

FocusForge uses `QSettings` with the organization and application names `FocusForge` and `FocusForge`. On Windows, Qt's default settings backend stores these values in the current user's registry, under:

```text
HKEY_CURRENT_USER\Software\FocusForge\FocusForge
```

The application stores task, subject, and study-session arrays there. This data is separate from the program files. The current MSI does not include an action to remove this QSettings data during uninstall. Avoid deleting the registry key manually unless you intend to remove the saved application data. The application does not currently document an in-app export or restore workflow.

## Building from Source

The release build was verified with C++17, Qt 6.12 Widgets, CMake 4.4, Ninja, and the 64-bit MinGW 13.1 toolchain. The CMake project declares a 3.16 minimum, but MSI packaging has only been verified with the CMake and WiX versions listed here.

Install Qt 6.12 for MinGW, MinGW 13.1, CMake, and Ninja. Then configure with paths for your local Qt and MinGW installations:

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_PREFIX_PATH="C:/path/to/Qt/6.12.0/mingw_64" `
  -DCMAKE_C_COMPILER="C:/path/to/MinGW/bin/gcc.exe" `
  -DCMAKE_CXX_COMPILER="C:/path/to/MinGW/bin/g++.exe"
cmake --build build --config Release
```

To create the MSI, install the WiX Toolset 4 CLI and its UI extension, then run:

```powershell
cpack --config build/CPackConfig.cmake -G WIX -B dist
```

## Known Limitations

- The minimum supported Windows release has not been confirmed, and the MSI is unsigned.
- No automated test suite is currently configured in the project.
