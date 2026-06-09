# AOT Pipeline Upgrades: Multi-View Synthesis & RAG Self-Improvement Loop

This document specifies the integration of **Phase 1.1** (Orthographic Multi-View Synthesis) and **Phase 7** (RAG Ingestion & Self-Improvement Loop) into the AOT compilation matrix.

---

## 1. Upgraded Pipeline Flow

```
[ Phase 1: Stable Diffusion (Base) ]
          |
[ Phase 1.1: Orthographic Multi-View Synthesis ] <-- (New Phase)
          | (Front, Side, Top Blueprints for Z-axis mapping)
          v
[ Phase 2: VLM Discovery ]
          ...
[ Phase 6: Multi-User Rollup Sync ]
          |
[ Phase 7: RAG Ingestion & Self-Improvement Loop ] <-- (New Phase)
          | (Writes telemetry & code audits to /lore)
          v
[ Lore Directory (/lore) ] -- (Feeds back into Phase 1 / Prompt calibration)
```

---

## 2. Phase 1.1: Orthographic Multi-View Synthesis

To extract accurate depth ($Z$-axis) coordinates alongside standard width and height ($X, Y$), the system generates aligned projection grids:

### A. Core Operation
The text prompt is expanded to enforce a standard triple-axis orthographic layout (e.g. `"orthographic multi-view blueprint sheet, front view, profile side view, top-down view"`).

### B. Depth Extraction Rules
The VLM (Phase 2) analyzes the resulting multi-view sheet using the following coordinate projections:
* **Width ($X$) and Height ($Y$)**: Extracted from the **Front View** bounding box.
* **Depth ($Z$)**: Extracted from the horizontal boundary coordinates of the **Profile Side View**.
* **Overhead Verification ($X, Z$)**: Cross-referenced against the **Top-Down View** to verify aspect ratios and eliminate scaling skew.

---

## 3. Phase 7: RAG Ingestion & Self-Improvement Loop

To ensure the compilation matrix is self-correcting and improves over time, Phase 7 acts as the pipeline's memory consolidation loop.

### A. Telemetry Collection
At the end of each compilation and execution cycle, the runtime engine captures:
1. **Gas Consumption**: Gas used during the Yul rendering loop.
2. **Visual Telemetry**: Frame rates (FPS) and CPU/memory utilization.
3. **Audit Results**: Compiler warning logs and validation errors.

### B. Lore Ingestion
An offline compiler task translates this telemetry into structured Markdown and writes it directly to the [lore/](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/lore) directory (e.g. `lore/telemetry_feedback_epoch_N.md`).

### C. Prompt Calibration (The Loop Closes)
Before executing Phase 1 on subsequent compiles:
1. The prompt compiler queries the newly ingested lore files.
2. It applies modifications (e.g. *"Reduce complexity coefficient: limit joint limits to 2 axes to save 30% gas as noted in feedback log N"*).
3. The prompt is fed back into Phase 1, closing the self-improving loop.
