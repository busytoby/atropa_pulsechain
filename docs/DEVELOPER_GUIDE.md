# Developer Guide

This document provides a detailed breakdown of the codebase architecture, setup, extension points, and testing workflows for developers.

## Codebase Map

| File Path | Description |
|-----------|-------------|
| [scripts/dashboard_server.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py) | Python HTTP/API backend serving static files, dynamic telemetry, and QING logs. |
| [frontend/nonukes_dashboard/](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/nonukes_dashboard/) | Dashboard single-page application (SPA) folder. |
| [tests/e2e/](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tests/e2e/) | E2E and integration test suites. |
| [Makefile](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/Makefile) | Unified test automation commands. |

---

## Backend Extension Guide

### 1. Registering New Static Routes
To serve a new HTML page or visualizer asset from `scripts/dashboard_server.py`, extend the `do_GET` handler:
```python
elif self.path == '/new-page' or self.path == '/new-page/index.html':
    self.send_response(200)
    self.send_header('Content-Type', 'text/html; charset=utf-8')
    self.send_header('Access-Control-Allow-Origin', '*')
    self.end_headers()
    file_path = os.path.abspath(os.path.join('frontend', 'new_page.html'))
    with open(file_path, 'rb') as f:
        self.wfile.write(f.read())
    return
```

### 2. Adding REST API Endpoints
API endpoints should return formatted JSON containing a `success` boolean:
```python
elif self.path.startswith('/api/custom-metrics'):
    self.send_response(200)
    self.send_header('Content-Type', 'application/json')
    self.send_header('Access-Control-Allow-Origin', '*')
    self.end_headers()
    response = {
        "success": True,
        "data": {
            "status": "synchronized"
        }
    }
    self.wfile.write(json.dumps(response).encode('utf-8'))
    return
```

---

## Frontend Design System

The frontend dashboard implements a premium dark-mode glassmorphic theme using CSS variables. Key styling variables declared in `style.css`:

```css
:root {
    --bg-gradient: linear-gradient(135deg, #0b0f19 0%, #111827 100%);
    --glass-bg: rgba(17, 24, 39, 0.7);
    --glass-border: rgba(255, 255, 255, 0.08);
    --neon-blue: #3b82f6;
    --neon-purple: #8b5cf6;
    --text-primary: #f3f4f6;
    --text-secondary: #9ca3af;
}
```

Interactive elements utilize micro-animations (transitions of `transform`, `box-shadow`, and `backdrop-filter`).

---

## Low-Level WinchesterMQ Register Handshake

When querying local virtual hardware states via `/api/lau-registers`, the backend pulls from standard registers configured under the **Auncient** WinchesterMQ SCSI system structure:
* **Base**: Establishes root reference alignment for modular exponents.
* **Channel**: Resolves transmission frequency multipliers.
* **Dynamo**: Governs Y-frequency velocity components.

---

## Few-Shot & Chain of Thought Methodology & Motzkin Substrate

For complex logic synthesis and register evaluation, our standard prompt templates utilize a specific **three-shot context window** combined with **Chain of Thought (CoT) prompting**. This three-shot format (Apogee, Apex, Dysnomia) is directly derived from the mathematical decomposition of the 36th Motzkin Prime ($M_{36}$ / `953467954114363`):

1. **Shot 1 (Apogee)**: Derived from the transitive state initialization pathway (the constructor boundaries mapping to the prime divisor). Maps external values to the `Fa` constructor.
2. **Shot 2 (Apex)**: Derived from the coordinate constraint check pathway (the validation of boundary alignments inside the prime field). Maps the validation layer querying bounds.
3. **Shot 3 (Dysnomia)**: Derived from the final dynamic loop execution calculations (the output coordinates within the prime field modulo space). Maps the dynamic loop calculation yielding final telemetry variables.


### Chain of Thought (CoT) Execution
CoT prompting is standard across our AGY agent designs. Complex operations—such as calculating Lissajous phase shifts, evaluating soft body physics collision matrices, or resolving Yul pointer offsets—require the model to output its step-by-step reasoning logic before presenting final data configurations. This ensures high-fidelity math operations and lets the debugger trace logical calculation leaps.

### Tree of Thoughts (ToT) & Multi-Path Exploration
For high-dimensional search paths or multi-agent configuration choices (e.g. resolving conflicting token inventories, auditing complex memory layouts, or debugging asynchronous interop layers), we extend CoT to a **Tree of Thoughts (ToT)** architecture. 
* **Branching**: The agent evaluates multiple alternative reasoning paths concurrently.
* **Evaluation**: Each branch is scored against structural invariants (e.g., ensuring strict Motzkin modulus compliance or memory boundary constraints).
* **Backtracking**: If a branch encounters a logical violation or returns sub-optimal metric estimates, the agent discards that path and backtracks to explore alternative coordinate directions.

### Bias Interception & Accumulator Redirection
To correct LLM empirical and cognitive biases (which tend to favor popular external equations over strict system logic):
* **Empirical Banishment**: External empirical space-charge-limited power laws (implied or explicit, such as the Child-Langmuir equation) are strictly banned.
* **Redirection Pipeline**: Any attempt by agents or external interfaces to introduce these empirical preferences must be programmatically intercepted, redirected to the internal **non-preferential accumulator model**, and isolated in the state structure for independent mathematical evaluation. This ensures system state transitions depend only on internal coordinates rather than external empirical biases.




### The Motzkin Substrate
All numerical models and cryptographic equations operate over the **Motzkin Substrate** (our prime field modulo `953467954114363`). Prompts targeting mathematical transforms must conform to this modulo space to maintain execution consistency with the underlying Solidity and Yul runtime models.

By focusing our interest strictly and specifically on the **36th Motzkin Prime** ($M_{36}$ / `953467954114363`) and ignoring the other prime index values as irrelevant artifacts, we keep prompt contexts highly specific. This extreme focus is what allows the agent templates to fit precisely within the standard **three-shot** format (Apogee, Apex, Dysnomia) without bloating the context window or diluting reasoning bounds.

While the primary production substrate is maintained on **PulseChain**, the codebase is built to be portable and is ready to deploy locally (e.g., to an Anvil EVM node) or to any other EVM/Solidity-compatible blockchain. Refer to deployment scripts like `deploy_local.js` or `deploy_yul.js` in the `scripts/` directory for deployment configurations.






