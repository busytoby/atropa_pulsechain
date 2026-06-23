# Original User Request

## Initial Request — 2026-06-14T06:07:52-07:00

An automated benchmark suite and visual report generator to measure, profile, and plot real-time performance, frames per second (FPS), and ALSA audio latency of the teddy bear editor and drum sequencer components.

Working directory: /home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite
Integrity mode: benchmark

## Requirements

### R1. Benchmarking Engine
The suite must execute the teddy bear editor and drum sequencer binaries, profile their runtime metrics, and record key data points including frames per second (FPS), loop iteration times, and sound trigger latency.

### R2. Web-Based HTML Visualizer
The suite must generate an interactive standalone HTML report. This report must present the profiled metrics using clear visual graphs (e.g., SVG or Canvas plots) showing frame times, latency distributions, and performance comparisons.

## Acceptance Criteria

### Automated Benchmarking & Report Verification
- [ ] A CLI execution script must successfully run the benchmarks and profile execution metrics.
- [ ] The engine must write output data directly to structured JSON or CSV files in the benchmarks directory.
- [ ] An interactive HTML report must be generated containing dynamic, functional SVG/Canvas visual charts displaying frame rate (FPS) and latency graphs.
- [ ] The generated HTML file must load without Console errors or missing asset errors.

## Follow-up — 2026-06-14T16:28:38Z

Implement an automated Git post-commit hook pipeline script that compiles the project, executes the performance profiling suite, updates the benchmark dashboard report, and prints a terminal diagnostic metrics summary after every commit.

Working directory: /home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/git_ci_pipeline
Integrity mode: benchmark

## Requirements

### R1. Git post-commit Hook Script
The script must hook into local git lifecycle and trigger automatically after a commit. It must navigate to the deepseek directory, build `test_vulkan_teddy`, and execute `run_benchmarks.sh`.

### R2. Automated Report Compilation
Following execution, the pipeline must run the Python generator to update `benchmark_report.html` and output a summary table in the shell containing final run statistics (average FPS, average frame time, audio latency records).

## Acceptance Criteria

### Git Hook Execution Verification
- [ ] A functional Git `.git/hooks/post-commit` script must be installed and executable.
- [ ] The hook script must successfully compile and execute the benchmark suite when a Git commit is completed.
- [ ] The hook must update the JSON metrics and HTML visual dashboard without throwing errors or halting git operations.
- [ ] A terminal performance summary block must be printed upon execution.

## Follow-up — 2026-06-14T17:14:13Z

Implement a 2D flat visual dashboard container directly inside teddy_bear_tournament_3d.html to display performance metrics on demand using a clean, flat-pastel theme.

Working directory: /home/mariarahel/src/tsfi2/atropa_pulsechain/benchmarks/dashboard_container
Integrity mode: benchmark

## Requirements

### R1. Slide-out Drawer Container
Modify `teddy_bear_tournament_3d.html` to add a flat button labelled "Performance Diagnostics". Clicking this button must trigger an overlay drawer to slide out from the right edge of the viewport.

### R2. Flat Pastel Aesthetics
The drawer must render metrics (such as the average FPS and audio latency) using a clean flat-pastel color palette (soft pink, blue, cream) with solid black borders, aligning with the 2D flat solids aesthetic.

### R3. Standalone Report Iframe
The slide-out drawer must embed the generated `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html` file using a responsive iframe.

## Acceptance Criteria

### Visual Integration Verification
- [ ] A functional "Performance Diagnostics" button must be present in the UI of `teddy_bear_tournament_3d.html`.
- [ ] Clicking the button must trigger the slide-out drawer animation.
- [ ] The drawer panel must render with clean pastel colors and solid black borders.
- [ ] The iframe within the drawer must successfully display the benchmark report page.


## Follow-up — 2026-06-23T18:04:50Z

An interactive hypervisor dashboard comparing native ZMM VM holdings and Anvil EVM holdings for deployed ERC20 tokens and Yul systems, showing full balances and listing deployed contracts without any mock placeholders.

Working directory: /home/mariarahel/src/tsfi2/atropa_pulsechain
Integrity mode: development

## Requirements

### R1. Dual VM Holdings Explorer
The dashboard must allow querying the local processes (Anvil on port 8545, ZMM VM on port 3000 via `/api/zmm-exec`) to fetch the holdings of the default wallet address (`0x00000000000000000000000000000000000004cc` for ZMM and `0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266` for Anvil).

### R2. Deployed Contracts Inventory
It must show both ERC20 token balances and list deployed Yul objects or system contracts (such as `zmachine` and `zmachineParser`) that may not have standard token balances.

### R3. Automated Comparison Validation
Implement a programmatic validation script that queries both endpoints (ZMM VM and Anvil EVM) directly, confirms they return valid comparable state data, and validates token distribution consistency.

## Acceptance Criteria

### System Integration
- [ ] A validation script successfully executes, querying both port 3000 and port 8545.
- [ ] The dashboard displays actual holdings fetched from the native ZMM VM process and Anvil EVM without placeholders.
- [ ] All deployed tokens and balance-less Yul contracts are listed correctly in the dashboard interface.
