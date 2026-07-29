# Coaxial SCSI Accessor and Displacement Shader Synchronization

The integration of the Coaxial SCSI Hardware Accessor (**SCSIPGM**) bridges low-level hardware handshake signals with vertex mapping calculations in the **Auncient** graphics pipeline. By establishing a direct client/server standard over WinchesterMQ, the system ensures math calculations scale in perfect synchronization with register boundary constraints.

## 1. WinchesterMQ SCSI Handshake Loop

The **SCSIPGM** execution phase operates directly on the `TSFiWinchesterRegisters` structure:
* **`status_reg`**: Coordinates handshake phases (Idle, Request, Ack, Complete).
* **`keycode_reg`**: Intercepts raw SCSI keycodes (e.g. keycode `32` for `d`/`D`) via local loopback sockets.
* **`data_reg`**: Transmits sector-aligned transaction frames directly to the WinchesterMQ bus.

By verifying the SCSI handshakes directly on the hardware state maps, the system establishes a secure, non-preferential transaction pathway that avoids standard driver overhead.

## 2. Displacement Shader Register Alignment

Whenever WinchesterMQ or `wm` registers are accessed, the execution thread returns to the `DisplacementShader` to scale vertex coordinate math in synchronization with register boundary constraints:
* **`Chin`**: Constrains the lower boundary of the projected 3D quaternion shape along the Y-axis.
* **`Monopole`**: Governs global scale boundaries of the projected wireframe envelope.
* **`Identity`**: Resolves the octave step multiplier mapping the coordinate offsets.

$$\text{Vertex Displacement} = \text{eval}(ds, autodin\_time, vertex\_coord)$$

This mathematical coordination guarantees that vertex shifts map cleanly to the modular limits of the hardware registers, preserving visual geometry accuracy during execution cycles.
