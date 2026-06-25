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

## 7. Mocking vs. Interoperability Constraints
* **No Mocking Permitted:** The implementation of mock components, functions, or libraries is strictly forbidden. Always implement full, thread-safe dynamic interfaces and coordinate registries labeled as **interop** (e.g. `libmozilla_interop.c`). Mocks restrict testing scalability and must not be created. Dynamic components built under **interop** can and should be extended as integration requirements evolve.
## 8. Source File Size Constraints
* **68KB File Limit:** Ensure all source files (`.c`, `.cpp`, `.h`, etc.) remain strictly under 68,000 bytes. Proactively split files into modular subcomponents as they approach this limit to preserve system design structure and modularity.

## 9. Dynamic Contract ZMM Resolution
* **Address-Based Resolution:** Always use contract addresses (e.g., `dynamic_<address>`) to resolve and query dynamic contract states on the ZMM VM, rather than relying on contract names or string transformations of names.

## 10. Physics Constraints
* **FET Discharge Physics:** Soft body physics (Verlet solvers and mass-spring dynamics) should always only apply to the discharge cycles of field-effect transistors (FETs) in simulated low-level hardware structures.

