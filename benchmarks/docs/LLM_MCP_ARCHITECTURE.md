# LLM & MCP: The Logic-Reality Bridge

## 1. Core Relationship

In the TSFi2 ecosystem, the relationship between Large Language Models (LLMs) and the Model Context Protocol (MCP) is defined as a **Cybernetic Loop** between reasoning and instrumentation.

*   **The LLM (The Reasoner):** Acts as the high-level cognitive engine. It possesses the capability for complex architectural mapping, code generation (C/Wave512), and strategic decision-making. However, it is fundamentally "at-rest" and isolated from the host's physical state.
*   **The MCP (The Nervous System):** Acts as the standardized interface that connects the LLM to the "Physical Reality" of the simulation. It provides the LLM with eyes (Resources) and hands (Tools).

## 2. Structural Triage

The protocol operates through a tripartite architecture:

### A. Discovery (Capabilities)
The MCP server exports a schema of available functionalities. This allows the LLM to understand the limits of its agency within the project—knowing, for example, that it can "Inspect Zero-Copy Memory" or "Execute Wave512 Vectors" without having to guess the underlying binary interface.

### B. Resources (Contextual Grounding)
MCP allows the LLM to "subscribe" to real-time data streams. In TSFi2, this includes:
*   **Shared Memory Telemetry:** Direct access to `LauTelemetryState`.
*   **Perception Metrics:** High-speed IR data from the Mouse Microscope (Shutter FPS, Surface Hash).
*   **System State:** Versioning and bijective integrity markers.

### C. Tools (Autonomous Execution)
Tools are the mechanism by which the LLM modifies the environment. When the LLM calls an MCP tool, the protocol handles the translation from a high-level request (JSON-RPC) to low-level execution:
*   **Example:** LLM requests a file read $ightarrow$ MCP invokes `bin/tsfi_poll` $ightarrow$ Shared Memory triggers C logic $ightarrow$ Result is returned as text to the LLM.

## 3. The TSFi Implementation (Kr0wZ Stack)

We utilize a dual-backend MCP approach to maximize both flexibility and performance:

1.  **Node.js MCP (The Orchestrator):** Handles complex logic, file system operations, and integration with the Gemini CLI. It manages the high-level tool definitions.
2.  **C-Based MCP Server (`tsfi_mcp_server`):** Handles high-performance tasks that require direct memory access or AVX-512 acceleration. It consumes JSON-RPC directly from the shared memory `request_cmd` channel.

## 4. Strategic Benefits

*   **Token Efficiency:** By using MCP to retrieve only the *necessary* snippets of memory or file data, we avoid flooding the LLM's context window with irrelevant noise.
*   **Physical Grounding:** MCP ensures the LLM's outputs are constrained by the actual state of the RDNA 4 simulation, preventing "hallucinations" regarding system performance or hardware availability.
*   **Bijective Mapping:** The protocol maintains a 1:1 mapping between the LLM's intent and the simulation's state, enabling the "Resident Supervisor" model of development.

## 5. Summary Table

| Component | Role | Data Format | Latency |
| :--- | :--- | :--- | :--- |
| **LLM** | Strategy & Code | Natural Language / Tokens | High (Inference) |
| **MCP** | Interface & Bridge | JSON-RPC | Medium (IPC) |
| **TSFi Core** | Simulation & Math | Binary / Wave512 | Low (Hardware) |
