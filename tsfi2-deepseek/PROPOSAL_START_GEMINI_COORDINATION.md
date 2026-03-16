# PROPOSAL: Coordinated Sandbox Workflow for start_gemini.sh

## Objective
Enhance `start_gemini.sh` to support multi-client "Emergency Swarms" where agents coordinate via a central status board and exchange patches.

## New Features

### 1. `--coordinated` Mode
- **Flag:** `--coordinated`
- **Behavior:**
    - Checks/Creates `status_board.txt` in the project root.
    - registers the current session (PID/User) in the board.
    - Monitors the board for "STOP" or "FOUND_FIX" signals from other clients.

### 2. Automated Patch Ingress
- **Function:** `check_incoming_patches()`
- **Logic:**
    - Scans for `*.patch` files.
    - Validates them (dry-run apply).
    - If valid, prompts the user (or auto-applies in EMERGENCY mode) and logs the event.
    - Moves processed patches to `patches/processed/`.

### 3. Unified Locking
- **Integration:** Respects `tmp/.test_lock` used by `test_parallel.sh`.
- **Safety:** Prevents `start_gemini` from launching heavy tasks if a test runner is active.

### 4. Status Broadcasting
- **Function:** `announce_status(msg)`
- **Logic:** Appends timestamped messages to `status_board.txt`.

## Implementation Plan
1.  Define `STATUS_BOARD="status_board.txt"`.
2.  Add `monitor_coordination()` loop that runs in background (or periodically).
3.  Update the "Launch" section to check for coordination flags.
