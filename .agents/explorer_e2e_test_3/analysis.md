# Git Post-Commit Hook E2E Test Suite Requirements Analysis

This document presents a comprehensive requirements analysis and structural design for the Git post-commit hook E2E test suite. It covers the creation of the git sandbox environment, the detailed test case specifications across Tiers 1–4, handling of mock vs. real builds, validation of environment scrubbing, output verification, graceful failure handling, the exact test directory layout, and a detailed implementation plan.

---

## 1. Executive Summary
The Git post-commit hook E2E test suite is designed as an opaque-box, requirement-driven testing framework located at `tests/e2e/run_e2e_tests.py`. It guarantees the correct behavior of the Git post-commit benchmark pipeline across multiple workflows (standard commits, nested subdirectories, env-var overrides, and build recovery) without polluting the main project repository. It supports a lightweight, fast **Mock Build Mode** for rapid test iteration and boundary-case injection, alongside a **Real Build Mode** for complete system integration testing.

---

## 2. Git Sandbox Environment Design

To perform E2E testing of Git commit hooks safely and prevent any side effects or pollution of the project's own repository, we must create an isolated, temporary Git repository (the "Git Sandbox") on-demand.

### Lifecycle of the Sandbox
1. **Creation**: The test runner utilizes Python's `tempfile.TemporaryDirectory` to allocate a clean directory.
2. **Git Init**: Executes `git init` inside the temporary directory.
3. **Local Identity Config**: To ensure the sandbox can run successfully in headless environments or CI pipelines where global Git configurations are absent or read-only, the runner configures local identity:
   ```bash
   git config --local user.name "E2E Tester"
   git config --local user.email "tester@e2e.atropa"
   ```
4. **Project Layout Replication**: The sandbox recreates the project directory structure under the sandbox path:
   - `<sandbox>/tsfi2-deepseek/benchmarks/profiler_suite/`
   - `<sandbox>/tsfi2-deepseek/bin/`
5. **Teardown**: The temporary directory is automatically deleted upon test completion, unless a command-line flag (`--keep-sandbox`) is passed for debugging.

### Environment Variable Isolation (Avoiding Parent Leaks)
By default, Python's `subprocess.run` inherits the environment variables of the parent process. If the E2E tests are run from inside a Git hook, CI pipeline, or terminal within a Git repository, the environment will contain variables like `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE`. These inherited variables would override the sandbox's local Git configuration, causing Git commands executed within the sandbox to point back to the outer repository.

To prevent this, the test runner must sanitize the environment dictionary passed to all sandbox subprocesses:
```python
def get_clean_env(self):
    env = os.environ.copy()
    git_vars = ["GIT_DIR", "GIT_WORK_TREE", "GIT_INDEX_FILE", "GIT_PREFIX", "GIT_QUIRK_WD"]
    for var in git_vars:
        env.pop(var, None)
    return env
```

---

## 3. Detailed Test Design (Tiers 1-4)

The test runner will cover 49 distinct test cases spanning Tiers 1–4 as specified in `TEST_INFRA.md`.

### 3.1 Tier 1: Feature Verification (20 Test Cases)
Verifies that all core features function correctly under normal operating conditions.

#### Feature 1: Git Hook Installation & Trigger
1. `test_install_hook_creates_file`: Installing the hook creates `.git/hooks/post-commit` relative to the sandbox root.
2. `test_install_hook_makes_executable`: Verifies that `.git/hooks/post-commit` has executable permissions (`0o755`).
3. `test_uninstall_hook_removes_file`: Uninstalling the hook successfully deletes the post-commit hook file.
4. `test_hook_triggers_on_commit`: Staging a file and running `git commit` triggers hook execution.
5. `test_hook_triggers_on_amend`: Amending a commit (`git commit --amend`) triggers hook execution.

#### Feature 2: Benchmark Execution
6. `test_benchmark_compilation_triggered`: Verifies `make bin/test_vulkan_teddy` is called.
7. `test_benchmark_script_triggered`: Verifies `run_benchmarks.sh` is executed.
8. `test_benchmark_results_created`: Verifies `benchmark_results.json` is generated.
9. `test_benchmark_invoked_with_flag`: Verifies that the binary is executed with the `--benchmark` argument.
10. `test_benchmark_run_working_dir`: Verifies the hook executes the benchmark runner in the correct directory.

#### Feature 3: HTML Report Updates
11. `test_html_report_created`: Verifies `benchmark_report.html` is generated.
12. `test_html_report_contains_chart`: Verifies HTML contains chart `<canvas>` tags.
13. `test_html_report_contains_json`: Verifies HTML report embeds the serialized JSON data.
14. `test_html_report_matches_fps`: Verifies that the average FPS printed in the report matches the JSON results.
15. `test_html_report_non_empty`: Verifies HTML report is not zero bytes and has a valid HTML5 structure.

#### Feature 4: Summary Table Output
16. `test_stdout_contains_summary_table`: Verifies that the hook prints an ASCII table to stdout during `git commit`.
17. `test_summary_table_headers`: Verifies table headers ("ATROPA PulseChain Benchmark Summary") are present.
18. `test_summary_table_numeric_match`: Verifies printed values match JSON data.
19. `test_summary_table_ascii_borders`: Verifies standard table borders (`+---+`, `|`) are printed.
20. `test_summary_table_no_crash_on_missing_json`: Verifies table falls back to mock summary if JSON is missing.

---

### 3.2 Tier 2: Boundary & Failure Mode Verification (20 Test Cases)
Verifies that the hook acts robustly in edge cases and fails gracefully.

#### Feature 1: Git Hook Installation & Trigger
21. `test_install_hook_overwrites_existing_backup`: If `.git/hooks/post-commit` already exists, a backup is created (e.g., `.git/hooks/post-commit.backup`) instead of silent deletion.
22. `test_uninstall_restores_backup`: Restores the original backup file if it existed before installation.
23. `test_install_without_git_repo`: Installing outside a Git repository fails gracefully with a clear message.
24. `test_hook_trigger_with_empty_commit`: Committing with `--allow-empty` triggers the hook successfully.
25. `test_hook_trigger_detached_head`: Hook triggers correctly when the repository is in a detached HEAD state.

#### Feature 2: Benchmark Execution
26. `test_compilation_failure_graceful`: Makefile returns non-zero status. Hook exits cleanly without blocking the commit.
27. `test_compilation_failure_logs`: Compilation error output is captured and logged.
28. `test_benchmark_binary_missing`: Compilation succeeds but binary is missing. Hook handles this gracefully.
29. `test_benchmark_crash_graceful`: The benchmark binary crashes (e.g. exit status 139). Hook logs a warning and exits cleanly.
30. `test_missing_make_utility`: `make` command is missing or not executable on the host. Hook logs warning and completes.

#### Feature 3: HTML Report Updates
31. `test_html_generation_fails_graceful`: `generate_report.py` fails (e.g., python syntax error). Hook logs error and continues.
32. `test_html_overwrites_existing`: The report generator successfully overwrites any pre-existing report.
33. `test_html_report_path_unwritable`: HTML report file is read-only. Hook logs warning and exits.
34. `test_html_fallback_mode`: If `benchmark_results.json` is corrupted, report generator enters mock data fallback mode.
35. `test_html_large_json_processing`: Handles parsing and visualization of JSON file containing extremely large datasets.

#### Feature 4: Summary Table Output
36. `test_stdout_json_corrupted`: If JSON results are malformed, summary table prints fallback mock warning.
37. `test_stdout_metrics_extreme_values`: ASCII table does not break formatting when values are extremely large or small.
38. `test_stdout_color_codes_stripped`: Color codes are stripped when stdout is not a TTY or redirected.
39. `test_stdout_table_suppressed_on_quiet`: Git commit is run with `--quiet` (`-q`). Output is suppressed or minimal.
40. `test_stdout_table_no_audio_records`: Verification of table output when `audio_latency_records` is empty in JSON.

---

### 3.3 Tier 3: Pairwise Combinations (4 Test Cases)
Verifies feature interactions under combined failure and boundary conditions.

41. `test_combo_compilation_success_report_success`: Compilation succeeds + benchmark runs + report generated + summary table printed. (Standard happy path).
42. `test_combo_compilation_success_benchmark_fail`: Compilation succeeds + benchmark fails + HTML report uses fallback mock data + summary table printed with mock warning.
43. `test_combo_compilation_fail_no_report`: Compilation fails + benchmark skipped + HTML report is not updated + summary table reports compilation error.
44. `test_combo_environment_scrubbed_with_detached_head`: Committing in detached HEAD + verification that environment is scrubbed + build and report succeed.

---

### 3.4 Tier 4: Real-World Scenarios (5 Test Cases)
Verifies multi-step workflows mimicking developer usage.

45. **Scenario 1: Normal Commit flow**: Multiple successive normal commits. Verifies that compile, run, report, and table print on each commit.
46. **Scenario 2: Commit from Subdirectory**: Running `git commit` from a deep nested directory (e.g., `tsfi2-deepseek/benchmarks/`). Verifies that the hook unsets `GIT_*` environment variables, locates the repo root, compiles relative to the root, and places results in correct locations.
47. **Scenario 3: Broken Build Recovery**:
    - Step A: Introduce syntax error, commit. Hook runs, compile fails, commit succeeds.
    - Step B: Fix syntax error, commit. Hook runs, compile succeeds, benchmark runs, report generated.
48. **Scenario 4: Large commit with multiple files**: Staging dozens of files and committing. Verifies hook triggers once and runs correctly on the full set of files.
49. **Scenario 5: Commit with git env vars set**: Explicitly export `GIT_DIR` and `GIT_WORK_TREE` to invalid locations in the shell before committing. Verifies the hook unsets them, allowing internal compilation and Git status checks to run correctly.

---

## 4. Mock vs. Real Build Strategy

To keep the E2E tests extremely fast (execution within milliseconds per test case) and independent of system Vulkan/ALSA drivers, the test runner must support a dual-build design:

### Mock Build Mode (Default)
In mock mode, the test runner populates the sandbox with mock assets:
- **Mock Makefile**:
  ```make
  bin/test_vulkan_teddy:
  	mkdir -p bin
  	echo '#!/bin/sh' > bin/test_vulkan_teddy
  	echo 'echo "GIT_DIR=$$GIT_DIR" >> env_check.log' >> bin/test_vulkan_teddy
  	echo 'echo "GIT_WORK_TREE=$$GIT_WORK_TREE" >> env_check.log' >> bin/test_vulkan_teddy
  	echo 'echo "GIT_INDEX_FILE=$$GIT_INDEX_FILE" >> env_check.log' >> bin/test_vulkan_teddy
  	cat ../mock_results_normal.json > benchmarks/profiler_suite/benchmark_results.json
  	chmod +x bin/test_vulkan_teddy
  ```
- **Mock `run_benchmarks.sh`**:
  ```bash
  #!/bin/bash
  cd ../..
  make bin/test_vulkan_teddy
  ```
- **Real `generate_report.py`**: The E2E runner copies the actual `generate_report.py` from the project workspace into the sandbox (since it is a Python script that runs instantly and needs to be verified directly).

This enables fast boundary-case testing (e.g. mock compile failures by writing `exit 2` to the mock Makefile).

### Real Build Mode (`--real-build` CLI option)
When `--real-build` is set:
- The runner symlinks the actual directories (`src/`, `inc/`, `plugins/`, `tests/`, `assets/`, `tsfi2-deepseek/`) from the current workspace into the sandbox.
- It invokes the real compiler and Vulkan build system, executing the full benchmark.

---

## 5. Environment Scrubbing Verification

The post-commit hook contract states it must unset `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` prior to compiling. 
- **The Issue**: When Git triggers a post-commit hook, it defines these variables to manage the commit. If downstream C compilations or script operations invoke Git subcommands (like `git log` or `git status`) inside subdirectories, Git will crash with "not a git repository" errors because `GIT_DIR` still points to `.git` relative to the original commit directory.
- **Verification Method**: The mock Makefile writes the environment variables `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` it receives during build execution to a file `env_check.log`. The test runner reads this file and asserts that these variables are empty or unset.

---

## 6. Output Validation

### HTML Visual Report
The runner checks for the existence of `tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html` and parses it to verify:
- It is a valid HTML file.
- It contains key metrics serialized within the script tags or visualized elements.
- If the JSON results file was missing, it contains the visual fallback mode warning:
  `Fallback Mode: Visualizing built-in mock benchmark metrics`

### ASCII Metrics Table on Stdout
The runner captures the standard output of `git commit` and checks for the following format:
```
+-------------------------------------------------------+
|          ATROPA PulseChain Benchmark Summary          |
+-------------------------------------------------------+
| Total Frames:                                     500 |
| Average FPS:                                   285.00 |
| Avg Render Time:                              3.500 ms |
| Elapsed Time:                                  1.75 s |
+-------------------------------------------------------+
```
It verifies the exact numeric values printed in the table match those stored in `benchmark_results.json`.

---

## 7. Graceful Failure Handling

Since the `post-commit` hook is executed *after* a commit is written to the repository, its execution must under no circumstances block or revert the commit.
- **Verification**: If compilation fails (`make` exits with non-zero status) or the benchmark binary crashes, the test runner verifies that `git commit` still exits with `0`, and the commit is successfully registered in `git log -n 1`.

---

## 8. Exact Test Files and Directories to Create

The test suite must be organized under the `tests/e2e/` folder as follows:

```
tests/e2e/
├── run_e2e_tests.py                    # Main test runner (CLI, Sandbox Manager, and suites)
└── test_cases/
    ├── __init__.py                     # Package initializer
    └── mock_assets/                    # Mock resources for sandbox testing
        ├── mock_results_normal.json    # Valid pre-baked benchmark JSON
        └── mock_results_corrupt.json   # Malformed benchmark JSON
```

### Necessary Codebase Patch (Fixing Hardcoded Path)
During our investigation, we identified that `tsfi2-deepseek/benchmarks/profiler_suite/generate_report.py` contains a hardcoded absolute path (line 8):
`WORKSPACE_DIR = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek"`

This will cause report generation to fail or write to the wrong location when executed inside the sandbox environment. We propose the following change to make it dynamic:

```python
# Before (Line 8)
WORKSPACE_DIR = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek"

# After (Dynamic Relative Path)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
WORKSPACE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
```

---

## 9. Recommended Implementation & Verification Plan

We recommend the following four-phase plan for the implementer:

### Phase 1: Patch `generate_report.py` and Setup Mock Assets
1. Apply the dynamic relative path patch to `generate_report.py`.
2. Create directories `tests/e2e/` and `tests/e2e/test_cases/mock_assets/`.
3. Save a valid benchmark sample to `mock_results_normal.json` and a corrupted file to `mock_results_corrupt.json`.

### Phase 2: Implement E2E Test Runner and Sandboxing
1. Write the `GitSandbox` context manager in `tests/e2e/run_e2e_tests.py`, incorporating environment scrubbing and mock file generation.
2. Implement the CLI arguments (`--real-build`, `--keep-sandbox`, `--only-tier`).
3. Setup the skeleton test cases using Python's `unittest` library.

### Phase 3: Implement post-commit Hook and Verify Tiers 1-3
1. Implement the post-commit hook script (or its template) unsetting the Git environment variables and executing `make bin/test_vulkan_teddy`, `./run_benchmarks.sh`, and `generate_report.py`.
2. Implement the stdout ASCII table formatter in the hook script.
3. Run `python3 tests/e2e/run_e2e_tests.py` in mock mode. Fix issues until Tier 1, 2, and 3 tests pass 100%.

### Phase 4: Validate Tier 4 and Publish TEST_READY.md
1. Implement the real-world scenario tests (detached HEAD, subdirectories, recovery from compile error).
2. If the build server supports Vulkan/ALSA, execute `python3 tests/e2e/run_e2e_tests.py --real-build`.
3. Create `TEST_READY.md` in the repository root summarizing test runner usage, capabilities, and Tier 1–4 coverage.
