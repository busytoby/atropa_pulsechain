# Implementation Plan - 2D Flat Visual Dashboard Container

## Objective
Implement a 2D flat visual dashboard container directly inside `teddy_bear_tournament_3d.html` to display performance metrics on demand using a clean, flat-pastel theme (R1, R2, R3).

## Milestones
| # | Milestone Name | Description | Status |
|---|---|---|---|
| 1 | Exploration & Analysis | Investigate duplicate `teddy_bear_tournament_3d.html` files, understand existing layout, and propose exact HTML/CSS/JS edits. | PLANNED |
| 2 | Implementation | Add button, slide-out drawer, and responsive iframe referencing the benchmark report. | PLANNED |
| 3 | Verification & Audit | Verify UI responsiveness, styling constraints, iframe loading, and run the Forensic Auditor to guarantee integrity. | PLANNED |

## Detailed Steps

### Milestone 1: Exploration & Analysis
- Dispatch a `teamwork_preview_explorer` to:
  1. Determine the relationship between the four `teddy_bear_tournament_3d.html` instances (copies vs symlinks).
  2. Inspect the content of `teddy_bear_tournament_3d.html` to find the correct injection point for the CSS, HTML drawer markup, and JS logic.
  3. Locate the existing `benchmark_report.html` to verify its path relative to the HTML file.
  4. Look for existing unit/E2E test suites or verification scripts.

### Milestone 2: Implementation
- Dispatch a `teamwork_preview_worker` to:
  1. Add a flat, stylized button "Performance Diagnostics" adhering to the 2D flat solids aesthetic.
  2. Inject clean, inline or document-level CSS for the slide-out drawer using the flat-pastel palette (soft pink, blue, cream, solid black borders).
  3. Add Javascript to handle toggling the drawer container (open/close).
  4. Embed `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html` via a responsive iframe in the drawer.
  5. Sync changes across all required locations if duplicates exist.

### Milestone 3: Verification & Audit
- Dispatch a `teamwork_preview_reviewer` to review code changes and ensure compliance with constraints.
- Dispatch a `teamwork_preview_challenger` to verify UI action (drawer opening, iframe loading).
- Dispatch a `teamwork_preview_auditor` to perform forensic integrity verification.
