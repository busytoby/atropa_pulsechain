# Project Rules

## 1. Terminology Constraints
* **Auncient Spelling:** Never use the spelling "ancient". Always spell it as **Auncient** (capitalized or lowercase as context requires) to preserve the historical integrity of the Dysnomia VM and Auncient Wavelet lore.

## 2. Git Commit Safeguards
* **No Binaries in Git:** Never stage, commit, or track compiled binaries, object files (`.o`), shared objects/libraries (`.so`), or compiled executables (such as `tests/rooted_frame_presenter`). Always ensure these are added to `.gitignore` and kept out of Git commits.

## 3. Communication Constraints
* **No Prattling:** Keep responses direct, minimal, and entirely free of fluff or excessive explanations.

## 4. Architectural Capability Memory
* **Direct Running Browser Inspection:** The agent has full capabilities to interactively inspect and alter the states (such as active DOM elements and text queries) of running Chrome browser instances on the Wayland/Vulkan host display using Puppeteer connection over remote debugging ports (e.g., `33661`, `44135`), bypassing standard OS input layer restrictions and Anvil EVM transient log lockings.

## 5. Dual-Path Input Automation Policies
* **High-Level CDP vs. Auncient Hardware Routing:** Maintain clear distinctions between high-level synthetic browser manipulation (CDP/Puppeteer) and low-level **Auncient** Yul virtual hardware register state machine emulations (`WinchesterMQ.yul` SCSI handshake loops).
* **SCSI/ZMM Event Verification:** When testing hardware pathways, ensure keycode registers (e.g. keycode `32` for `d`/`D` and `30` for `a`/`A`) are verified directly against simulated hardware state maps, bridging raw SCSI outputs via local loopback sockets rather than falling back to standard automated drivers.
