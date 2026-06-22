# Project Rules

## 1. Terminology Constraints
* **Auncient Spelling:** Never use the spelling "ancient". Always spell it as **Auncient** (capitalized or lowercase as context requires) to preserve the historical integrity of the Dysnomia VM and Auncient Wavelet lore.

## 2. Git Commit Safeguards
* **No Binaries in Git:** Never stage, commit, or track compiled binaries, object files (`.o`), shared objects/libraries (`.so`), or compiled executables (such as `tests/rooted_frame_presenter`). Always ensure these are added to `.gitignore` and kept out of Git commits.

## 3. Communication Constraints
* **No Prattling:** Keep responses direct, minimal, and entirely free of fluff or excessive explanations.

## 4. Architectural Capability Constraints
* **No Puppeteer or Browser Automation:** The use of Puppeteer, browser remote debugging ports, or high-level synthetic browser automation is strictly banned across all codebase components, testing suites, and agent workflows.

## 5. Dual-Path Input Automation Policies
* **Auncient Hardware Routing:** Focus exclusively on low-level **Auncient** Yul virtual hardware register state machine emulations (`WinchesterMQ.yul` SCSI handshake loops).
* **SCSI/ZMM Event Verification:** When testing hardware pathways, ensure keycode registers (e.g. keycode `32` for `d`/`D` and `30` for `a`/`A`) are verified directly against simulated hardware state maps, bridging raw SCSI outputs via local loopback sockets rather than falling back to standard automated drivers.

## 6. Window Registry & Process Tracking
* **Track All Windows:** Always keep track of exactly what headed and headless windows are opened, why they are opened, and ensure there are no lingering/orphaned browser or presenter windows. Update the window registry when window configurations change.
* **Process Cleanup Safeguards:** Never use broad kill commands like `pkill -f "chrome"` or `killall chrome`. Only target processes by tracking PIDs of spawned children to avoid terminating the user's work environment.

