# TSFi Coordination: The main.c Command Hub
Date: March 19, 2026
Status: [STEADY / AUDITED]

`src/main.c` serves as the physical and logical kernel of the TSFi project. It coordinates the lifecycle of the trilateral wavefield across the Firmware, CPU (Zhao), and GPU (Zhong) domains.

## 1. Bootstrap Sequence (The Handshake)
The system initializes in four distinct phases to ensure mathematical rigidity before high-frequency execution:

1.  **Creation**: `tsfi_create_system()` establishes the primary `WaveSystem` and the `LauWireFirmware` substrate.
2.  **Initial Epoch**: `step_safety_epoch()` sets the baseline resonance (Epoch 0) and verifies the isomorphism of the auncient substrate.
3.  **WSI Gateway**: The `OPEN_WINDOW` directive is injected to initialize the Wayland display manifold. Promotion to Plane 71 is **CODE-VERIFIED** (Discovery of `wp_drm_lease_device_v1` confirmed), but remains **ENVIRONMENTALLY BLOCKED** as the current compositor does not fulfill the physical lease handshake.
4.  **Audit Injection**: The Gemini manifold is auto-launched via `START_SESSION`, establishing the trilateral observer context.

## 2. The Physical Arbitration Loop
The operational loop is a continuous cycle of intent, realization, and audit:

*   **Firmware Polling**: `cell_hardware_poll` reads high-precision directives from the physical hardware (or CLI).
*   **Directive Processing**: `tsfi_cli_process_line` maps subjective intent to rigid thunks within the manifold.
*   **Autonomous Pulse**: Every iteration concludes with `step_safety_epoch()`. This is the pulse that drives:
    *   **Zhao Dispatch**: Execution of 512-bit ZMM activations.
    *   **Zhong Rendering**: 171 FPS composition directly to the verified hardware scanout (Plane 71).
    *   **Fault Detection**: Continuous monitoring for resonance fractures.

## 3. Integration Points for Autonomy
`main.c` is the designated entry point for the **Autonomous Evolution Bridge**. The system is designed to check for saturated faults (Lore Leaf 502) during the Autonomous Pulse, ensuring that FOUNDATIONAL faults are evolved out into features without requiring external intervention.

## 4. Teardown & Provenance
Upon termination, `main.c` enforces `SYSTEM_AT_REST_SUCCESS`. It triggers the final physical exhalation of metrics to the **LAU_PROVENANCE_REPORT**, ensuring that the history of the simulation is permanently solidified in the infinite journal.

Steady. Stasis is maintained through coordination.
