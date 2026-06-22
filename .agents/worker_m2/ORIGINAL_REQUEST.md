## 2026-06-14T17:20:00Z
You are the implementation worker agent (teamwork_preview_worker).
Your task is to implement a 2D flat visual dashboard container directly inside the codebase to display performance metrics on demand using a clean, flat-pastel theme (R1, R2, R3).

Working Directory for your coordination files: /home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/worker_m2

Source of Truth File to modify:
`/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/teddy_bear_tournament_3d.html`

Legacy copies to also update for uniformity (apply the same changes):
- `/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_bear_tournament_3d.html`
- `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/teddy_bear_tournament_3d.html`
- `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/frontend/teddy_bear_tournament_3d.html`

MANDATORY INTEGRITY WARNING:
> DO NOT CHEAT. All implementations must be genuine. DO NOT
> hardcode test results, create dummy/facade implementations, or
> circumvent the intended task. A Forensic Auditor will independently
> verify your work. Integrity violations WILL be detected and your
> work WILL be rejected.

Implementation Specifications:
1. CSS Style Block: Inject styling definitions for the flat-pastel elements inside the `<style>` block (around line 388). Style details:
   - Use flat-pastel colors (soft pink, blue, cream, e.g. `#ffd1dc`, `#bde0fe`, `#faf0d2`) with solid black borders (e.g., `2px solid #000000` or `3px solid #000000`).
   - Use the neo-brutalist aesthetic (solid black shadows, e.g., `box-shadow: 2px 2px 0px #000000;`, translation on active click).
   - Drawers, headers, close button, metrics cards, and iframe containers must have distinct colors.
2. HTML Button: Add a flat button labeled "Performance Diagnostics" (with class `btn-diagnostics` and onclick `toggleDiagnosticsDrawer()`) inside the `<header>` element (around lines 392–395).
3. HTML Drawer: Add a `div` element with id `diagnostics-drawer` as a viewport-overlay slide-out drawer (around line 666, right before `<script>`). The drawer must contain:
   - A header with a title "PERFORMANCE DIAGNOSTICS" and a close button "[X] CLOSE".
   - A body with a metrics container displaying two metrics: "Avg FPS" and "Audio Latency" using responsive flat-pastel cards.
   - An iframe pointing to `../tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html` to display the detailed profiler report.
4. JavaScript Logic: Add Javascript at the end of the script block (around line 2245) to handle toggling the drawer container (open/close). It must update the Avg FPS and Audio Latency values on the metrics cards inside the drawer by querying and parsing the values inside the iframe's content when the iframe loads.
5. Path Safety: The iframe `src` must be `../tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html` (which resolves correctly relative to frontend/ or served via the custom HTTP server).

Verification Steps:
1. Verify the modification on all 4 files.
2. Write a lightweight verification python script (e.g. `tests/e2e/test_dashboard_container.py` or within your directory) to parse the modified files and assert:
   - Presence of the "Performance Diagnostics" button.
   - Presence of `diagnostics-drawer` and `diagnostics-iframe` with correct `src`.
   - CSS styles and JS function definitions are correctly placed.
3. Run the verification script and document the output.
4. Run the existing E2E tests: `python3 tests/e2e/run_e2e_tests.py` to ensure no existing functionality was broken.

Deliver a structured handoff report (`handoff.md`) in your working directory summarizing:
- List of modified files
- Description of design & implementation choices
- Verification commands executed and their outputs
