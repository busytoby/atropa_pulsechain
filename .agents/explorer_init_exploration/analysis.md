# ATROPA PulseChain Codebase Exploration: Benchmarks & Git Integration

## Executive Summary
This report documents the exploration of the ATROPA PulseChain codebase with respect to building the headless Vulkan/audio benchmark suite, running performance diagnostics, compiling HTML visualizer reports, and designing a local Git hook pipeline (`post-commit`) to automate these steps on every commit.

---

## 1. Build Process for `test_vulkan_teddy`
- **Source File Path**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_vulkan_teddy.c`
- **Output Binary Path**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/bin/test_vulkan_teddy`
- **Makefile Path**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/Makefile`
- **Build Target**: `bin/test_vulkan_teddy`

### Compilation Command
To build `test_vulkan_teddy`, run the following command:
```bash
cd /home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek
make bin/test_vulkan_teddy
```

### Makefile Build Rule
The rule defining the target compilation is located in `tsfi2-deepseek/Makefile` on lines 257-258:
```make
bin/test_vulkan_teddy: tests/test_vulkan_teddy.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
```

---

## 2. Benchmark Runner Script (`run_benchmarks.sh`)
- **Script Path**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/run_benchmarks.sh`

### Usage & Mechanics
The script executes as follows:
```bash
cd /home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite
./run_benchmarks.sh
```

### Execution Details
1. Navigates to `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek`.
2. Re-compiles `test_vulkan_teddy` with `make bin/test_vulkan_teddy`.
3. Runs `./bin/test_vulkan_teddy --benchmark`, executing a headless benchmark for 500 frames.
4. Outputs raw performance metrics to `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_results.json`.
5. Prints the JSON summary block (using `jq` if available, or a fallback grep/cat block).

---

## 3. Visual Report Generator (`generate_report.py`)
- **Script Path**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/generate_report.py`

### Usage
Execute using Python 3:
```bash
cd /home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite
python3 generate_report.py
```

### Behavior
- **Input File**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_results.json`
- **Output File**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html`
- **Fallback Mode**: If the input JSON is missing or corrupted, the script prints a warning and generates mock data, inserting a fallback warning banner at the top of the HTML report.

---

## 4. Local Git Hooks Setup
- **Local Git Hooks Directory**: `/home/mariarahel/src/tsfi2/atropa_pulsechain/.git/hooks`

### Structure
The directory currently contains only standard Git sample hook scripts (with the `.sample` extension). No active hooks are currently installed. The files present include:
- `applypatch-msg.sample`
- `commit-msg.sample`
- `fsmonitor-watchman.sample`
- `post-update.sample`
- `pre-applypatch.sample`
- `pre-commit.sample`
- `pre-merge-commit.sample`
- `pre-push.sample`
- `pre-rebase.sample`
- `pre-receive.sample`
- `prepare-commit-msg.sample`
- `push-to-checkout.sample`
- `sendemail-validate.sample`
- `update.sample`

---

## 5. Recommended Implementation Design for the `post-commit` Hook
To integrate the benchmark runs automatically after every local commit, we recommend installing a `post-commit` hook script.

### Key Considerations
1. **Unsetting Git Environment Variables**: When Git fires hooks, it sets environment variables such as `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` which can cause downstream git operations or commands called by compilation tools to fail. These should be unset within the hook context.
2. **Path Resolution**: The script should resolve paths absolutely to ensure reliability regardless of the subdirectory from which the commit was executed.
3. **Execution Safety**: If the build or benchmark run fails, it should display a clean stderr notice but **not** abort git workflows (the commit has already succeeded).
4. **Summary Printing**: An inline Python parsing command is used to output a cleanly formatted summary table of the stats to the terminal.

### Proposed Hook Script: `/home/mariarahel/src/tsfi2/atropa_pulsechain/.git/hooks/post-commit`
```bash
#!/bin/bash

# Prevent Git environment variables from leaking into downstream tools
unset GIT_DIR GIT_WORK_TREE GIT_INDEX_FILE

# Set paths
REPO_DIR="/home/mariarahel/src/tsfi2/atropa_pulsechain"
DEEPSEEK_DIR="${REPO_DIR}/tsfi2-deepseek"
PROFILER_DIR="${DEEPSEEK_DIR}/benchmarks/profiler_suite"
RESULTS_JSON="${PROFILER_DIR}/benchmark_results.json"

echo "========================================="
echo "[POST-COMMIT HOOK] Starting CI Pipeline..."
echo "========================================="

# 1. Navigate to deepseek folder
cd "${DEEPSEEK_DIR}" || exit 1

# 2. Build test_vulkan_teddy
echo "[CI] Compiling test_vulkan_teddy..."
make bin/test_vulkan_teddy > /dev/null
if [ $? -ne 0 ]; then
    echo "[CI ERROR] Compilation failed! Skipping benchmark." >&2
    exit 0
fi

# 3. Execute run_benchmarks.sh
echo "[CI] Executing benchmark suite..."
chmod +x "${PROFILER_DIR}/run_benchmarks.sh"
"${PROFILER_DIR}/run_benchmarks.sh" > /dev/null
if [ $? -ne 0 ]; then
    echo "[CI ERROR] Benchmark execution failed." >&2
    exit 0
fi

# 4. Compile HTML Visual Report
echo "[CI] Compiling visual report..."
python3 "${PROFILER_DIR}/generate_report.py" > /dev/null
if [ $? -ne 0 ]; then
    echo "[CI ERROR] HTML Report generation failed." >&2
    exit 0
fi

# 5. Output Diagnostic Metrics Summary Table in Terminal
if [ -f "${RESULTS_JSON}" ]; then
    echo ""
    python3 -c "
import json, sys
try:
    with open('${RESULTS_JSON}') as f:
        data = json.load(f)
    sum_data = data['summary']
    print('+' + '-'*50 + '+')
    print('|{:^50}|'.format('ATROPA BENCHMARK RUN SUMMARY'))
    print('+' + '-'*50 + '+')
    print('| {:<30} | {:>15} |'.format('Metric', 'Value'))
    print('+' + '-'*50 + '+')
    print('| {:<30} | {:>15.2f} |'.format('Total Frames', sum_data['total_frames']))
    print('| {:<30} | {:>15.4f} s |'.format('Elapsed Time', sum_data['elapsed_time_seconds']))
    print('| {:<30} | {:>15.4f} s |'.format('Total Render Time', sum_data['total_render_time_seconds']))
    print('| {:<30} | {:>15.4f} |'.format('Average FPS', sum_data['average_fps']))
    print('| {:<30} | {:>15.4f} ms |'.format('Avg Frame Render Time', sum_data['average_frame_render_time_seconds'] * 1000))
    print('+' + '-'*50 + '+')
    
    records = data.get('audio_latency_records', [])
    if records:
        print('\n+' + '-'*65 + '+')
        print('|{:^65}|'.format('AUDIO LATENCY METRICS BY SOUND TYPE'))
        print('+' + '-'*65 + '+')
        print('| {:<15} | {:>10} | {:>15} | {:>15} |'.format('Sound Type', 'Count', 'Avg Latency(ms)', 'Max Latency(ms)'))
        print('+' + '-'*65 + '+')
        by_type = {}
        for r in records:
            t = r['sound_type']
            lat = r['total_latency_seconds'] * 1000
            by_type.setdefault(t, []).append(lat)
        for t, lats in sorted(by_type.items()):
            avg_lat = sum(lats) / len(lats)
            max_lat = max(lats)
            print('| {:<15} | {:>10} | {:>15.3f} | {:>15.3f} |'.format(t, len(lats), avg_lat, max_lat))
        print('+' + '-'*65 + '+')
except Exception as e:
    print('Error printing summary table:', e, file=sys.stderr)
"
fi

echo "========================================="
echo "[POST-COMMIT HOOK] Pipeline Completed."
echo "========================================="
```
