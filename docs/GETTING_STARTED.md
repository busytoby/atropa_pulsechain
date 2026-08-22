# Getting Started Reference Guide

This document provides a thorough walkthrough on compiling, executing, and monitoring all subsystems, dashboards, and automated pipelines.

---

## 1. Prerequisites & Environment Setup

* **Python 3.x**: Ensure `unittest`, `struct`, and standard HTTP server utilities are available.
* **Node.js**: Required to install frontend testing wrappers and modules:
  ```bash
  npm install
  ```
* **Virtualization/Compilation Tools**: gcc/clang (to build low-level interop objects), and git.

---

## 2. Interactive Dashboards Catalogue

The backend server serves multiple diagnostic interface layouts. Launch the server first:
```bash
python3 scripts/dashboard_server.py
```

### A. NoNukes Family Dashboard (`/nonukes/`)
* **Endpoint**: `http://localhost:8080/nonukes/`
* **Features**: Displays the 480 pools from `nonukes_pools.json`. Allows filtering by token groups, viewing reserve trends, executing setup wizards, and monitoring real-time chat outputs from the PulseChain QING contracts.
* **Data Sources**: Reads from `price_cache.json`, `resolved_swaps.json`, and dynamic reserve files.

### B. Twinning Dashboard (`/twinning/`)
* **Endpoint**: `http://localhost:8080/twinning/`
* **Features**: Compares language models and tracks semantic dialect patterns (`twinning_dialects.json`).

### C. LAU Register Gallery (`/lau-gallery/`)
* **Endpoint**: `http://localhost:8080/lau-gallery/`
* **Features**: Visualizes geometric coordinate projections based on the **Auncient** WinchesterMQ SCSI register states fetched from the network interface.

### D. Tomographic Sky Array (`/tomographic-sky/`)
* **Endpoint**: `http://localhost:8080/tomographic-sky/`
* **Features**: Render layers mapping volumetric data grids using shaders and visual elements.

---

## 3. DNA Compilation & Asset Assembly

To generate compiled genomic binary assets (e.g. for character animations in the tournament viewer):
```bash
python3 compile_dna.py
```
* **Options**: Modify the active preset by updating the `ACTIVE_PRESET` parameter in `compile_dna.py` (options: `slugcat` or `teddy_bear`).
* **Output**: Writes the binary file `tsfi2-deepseek/assets/atropa.dna` (334,816 bytes) containing a 16-byte header followed by 10,800 frames of packed coordinates.

---

## 4. Git post-commit Hook Pipeline

To configure the post-commit testing automation hook:
1. Ensure the hook script is present at `.git/hooks/post-commit` and has executable permissions:
   ```bash
   chmod +x .git/hooks/post-commit
   ```
2. The hook runs automatically after commits to compile Vulkan assets, profile execution speeds, and output a diagnostics terminal table.
3. To trigger the pipeline manually for testing:
   ```bash
   python3 tests/e2e/run_e2e_tests.py
   ```
