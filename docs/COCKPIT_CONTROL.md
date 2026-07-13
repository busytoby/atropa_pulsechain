# Cockpit and Synthesizer Control Processes

This document details the interface architecture connecting the Cockpit control console with the **Auncient** guest virtual machine and the TCP-based MCP synthesizer server.

---

## 1. Interface Protocols & Port Assignments

The integration server runs as a background process listening for JSON-RPC 2.0 messages:

* **Control Port**: `10042` (default, overridden via `TSFI_MCP_PORT` env)
* **Transport Protocol**: TCP Sockets
* **Client Driver**: [tests/test_final_mcp_synth.sh](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tests/test_final_mcp_synth.sh)

---

## 2. Supported JSON-RPC Methods

### A. Mount Instrument (`manifold.mount_instrument`)
Registers a target channel output mapping.
* **Params**: `{"target": "<instrument_name>"}` (e.g. `lead`, `bass`, `growl`, `drums`)
* **Gas Requirement**: Submits a transaction verifying local account permissions.
* **Implementation**: [tsfi2-deepseek/src/tsfi_zmm_rpc.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_zmm_rpc.c#L980-L1010)

### B. Play BIO Arrangement (`manifold.play_bio`)
Parses a tempo-synchronized `.bio` track layout, queries EDO-22 pitch scales, and streams synthesis packets.
* **Params**: `{"path": "<path_to_bio>", "project": 1, "programmer": 2, "key_id": 11}`
* **Execution**: Streams synthesized audio packets to the local speaker systems using PulseAudio Simple API.
* **Implementation**: [tsfi2-deepseek/src/tsfi_zmm_rpc.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_zmm_rpc.c#L1015-L1060)

### C. Sysop Peer Observation (`sysop.observe_peers`)
Queries the ring buffer logs capturing peer execution, thunk evaluations, and telemetry anomalies.
* **Params**: `{"limit": <int>}`
* **Result**: Array of peer observation log entries.

### D. Sysop Command Injection (`sysop.inject_command`)
Injects control signals directly into a target peer's VM execution state.
* **Params**: `{"target_project": <int>, "target_programmer": <int>, "command": "<command_string>"}` (e.g. `CLEAR_LOCKOUT` to clear validation failure locks).

### E. Fortell Memory Diagnostics (`fortell.detect_fault`)
Executes loop fault diagnostics audits on the virtual memory layout space.
* **Params**: `{"address": "<hex_address>", "size": <int>}`
* **Result**: Diagnostic enum status indicating `FAULT_NONE`, `FAULT_OPEN`, or `FAULT_GROUND`.

---

## 3. Auncient VM Register Alignments

The synthesis loop directly maps VM execution parameters to Lissajous waveform modulations:

* **Base**: Initial Twist ($\phi_w$)
* **Channel**: X-frequency scaling ($f_x$)
* **Dynamo**: Y-frequency scaling ($f_y$)
* **Foundation**: Z-frequency depth multiplier ($f_z$)
* **Chin**: Hemisphere coordinate clamping (negative Y warping)
