# E2E Test Suite Design and Requirements Analysis
**Explorer 1 Investigation Report**

This document provides a comprehensive design and implementation plan for the Git post-commit hook E2E test suite. It details the setup of an isolated Git sandbox environment, designs for Tiers 1-4 tests, required file layout, and a structured implementation and verification roadmap.

---

## 1. Git Sandbox Environment Design
To prevent tests from mutating the active project repository or causing side effects (such as leaving orphaned lock files or committing unwanted changes), tests must run inside an isolated **Git Sandbox**.

### Sandbox Creation and Lifecycle
Using Python’s standard library, the sandbox lifecycle is managed as follows:
1. **Creation**: A temporary directory is initialized using `tempfile.TemporaryDirectory()`.
2. **Git Initialization**: `git init` is executed inside the directory using the `subprocess` module.
3. **Local Configuration**: Local configurations are explicitly set to insulate the test from global settings:
   - `git config --local user.name "E2E Tester"`
   - `git config --local user.email "e2e@atropa.local"`
   - `git config --local commit.gpgsign false` (disables GPG key sign requirements which can block tests).
4. **Project Structure Setup**: The hook script executes paths relative to the repository work tree. We replicate the expected directory layout in the sandbox:
   - `tsfi2-deepseek/benchmarks/profiler_suite/`
5. **Hook Installation**: The runner copies the hook script under test from the workspace source to `.git/hooks/post-commit` inside the sandbox, granting executable permissions (`chmod +x`).
6. **Cleanup**: Upon test completion, `TemporaryDirectory.cleanup()` recursively removes all files, directories, and git databases.

---

## 2. Test Tier Designs (Tiers 1-4)

### Tier 1: Feature Coverage (>=20 test cases)
Verifies the four core features under normal conditions (5 test cases per feature).

#### Feature 1: Hook Installation & Trigger
- **TC_F1_01**: Assert the hook file exists at `.git/hooks/post-commit` in the sandbox.
- **TC_F1_02**: Assert the hook is executable (has read/execute permissions).
- **TC_F1_03**: Assert committing a change successfully runs the hook script.
- **TC_F1_04**: Assert committing without the hook (uninstalled) succeeds silently.
- **TC_F1_05**: Assert the hook triggers correctly on the initial empty commit in a fresh repo.

#### Feature 2: Benchmark Execution
- **TC_F2_01**: Assert `run_benchmarks.sh` is invoked during the commit.
- **TC_F2_02**: Assert execution generates `benchmark_results.json`.
- **TC_F2_03**: Assert the output JSON contains required keys (`summary`, `frame_times_seconds`, `audio_latency_records`).
- **TC_F2_04**: Assert the benchmark script prints completion logs to stdout.
- **TC_F2_05**: Assert benchmarks trigger on different branches (e.g. `feature-branch`).

#### Feature 3: HTML Report Generation
- **TC_F3_01**: Assert `generate_report.py` is invoked during the commit.
- **TC_F3_02**: Assert `benchmark_report.html` is created.
- **TC_F3_03**: Assert the report does not contain the "Fallback Mode" warning banner if the JSON data is valid.
- **TC_F3_04**: Assert the report compiles successfully and exits with code 0.
- **TC_F3_05**: Assert consecutive commits continuously update the HTML report timestamp.

#### Feature 4: Summary Table Output
- **TC_F4_01**: Assert a formatted ASCII table header is printed to stdout during the commit.
- **TC_F4_02**: Assert the table includes "Render Performance" section (with FPS/Render Time).
- **TC_F4_03**: Assert the table includes "Audio Latency (Avg)" section (with SW/HW/Total).
- **TC_F4_04**: Assert the table includes "Execution Detail" (with Total Frames/Elapsed Time).
- **TC_F4_05**: Assert the table borders and spacing align properly.

---

### Tier 2: Boundary & Corner Cases (>=20 test cases)
Verifies boundary conditions and handles failures gracefully.

#### Feature 1 Boundary: Hook Installation & Trigger
- **TC_F1_B01**: Detached HEAD: Trigger hook when git HEAD is detached.
- **TC_F1_B02**: Merge commit: Trigger hook on merge commits.
- **TC_F1_B03**: Rebase commit: Ensure rebase actions trigger hook cleanly.
- **TC_F1_B04**: Reinstallation: Reinstalling hook overwrites the old hook and executes cleanly.
- **TC_F1_B05**: Missing hook directory: Ensure Git commands function normally when `.git/hooks/` is absent.

#### Feature 2 Boundary: Benchmark Compilation & Execution
- **TC_F2_B01**: Compilation fails: Mock compilation error. Verify commit succeeds and error output is printed.
- **TC_F2_B02**: Missing run script: Verify hook outputs a warning but allows the commit to complete.
- **TC_F2_B03**: Benchmark binary crashes (segfault): Ensure the crash is captured, doesn't block git, and logs warnings.
- **TC_F2_B04**: Timeout: Ensure runner completes gracefully if compilation or execution hangs.
- **TC_F2_B05**: Empty results file: Ensure hook handles zero-byte JSON files without crashing.

#### Feature 3 Boundary: HTML Report Generation
- **TC_F3_B01**: Malformed JSON results: Verify HTML report fallback mode compiles successfully with Mersenne Twister client-side mock data and warning banner.
- **TC_F3_B02**: JSON missing required keys: Verify generator falls back to mock data safely.
- **TC_F3_B03**: Missing report script: Ensure commit succeeds with a warning.
- **TC_F3_B04**: HTML write permission denied: Verify generator logs the write error and exits gracefully.
- **TC_F3_B05**: Generator fails: Ensure generator runtime errors do not block the commit.

#### Feature 4 Boundary: Summary Table Output
- **TC_F4_B01**: Missing JSON results: Verify table prints fallback warning table.
- **TC_F4_B02**: Empty audio latency list: Verify math handles 0 audio records without division-by-zero errors.
- **TC_F4_B03**: Extreme performance metrics (e.g. 100M+ FPS or 0 FPS): Assert alignment stays intact.
- **TC_F4_B04**: Non-TTY redirection: Verify table outputs safely when redirected to files or pipes.
- **TC_F4_B05**: Broken JSON: Verify stdout falls back to warning table cleanly.

---

### Tier 3: Feature Combinations (>=4 test cases)
Tests the pairwise interaction between multiple components:

1. **Compilation Failure + HTML Fallback Report (F2 x F3)**: Verify that when compilation/execution fails, the report generator still generates `benchmark_report.html` in fallback mode with the "Fallback Mode" warning banner.
2. **Missing Results JSON + CLI Table Fallback (F2 x F4)**: Verify that if the benchmark suite fails to write JSON, the post-commit stdout prints a warning block instead of crashing or printing empty columns.
3. **Subdirectory Commit + Root Path Resolution (F1 x F2/F3/F4)**: Verify that committing from a deep subdirectory (e.g. `tsfi2-deepseek/tests/`) triggers the hook, which correctly runs the build, writes reports, and prints tables relative to the workspace root directory.
4. **Reinstallation + Dashboard Retention (F1 x F3)**: Verify that uninstalling, reinstalling, and re-running the hook updates the existing report without altering file paths or ownership.

---

### Tier 4: Real-World Scenarios (>=5 test cases)
Tests comprehensive, multi-step workflows mimicking developer activity.

1. **Scenario 1: Normal Commit Flow**: Verify complete pipeline (commit -> build -> run -> JSON results -> HTML report -> ASCII stdout table).
2. **Scenario 2: Subdirectory Commit**: Verify subdirectory path resolution and environment unsetting from nested folder.
3. **Scenario 3: Broken Build Recovery**:
   - Step 1: Make a commit with a compilation error. Verify warning printed, commit succeeds, HTML report created in fallback mode.
   - Step 2: Make a subsequent commit fixing the compilation error. Verify benchmark compiles and runs successfully, HTML report gets updated with real metrics (fallback banner removed), and ASCII table outputs valid performance stats.
4. **Scenario 4: Large Commit with Multiple Files**: Verify hook execution remains stable and fast when committing a large set of changes (added, modified, deleted files) across directories.
5. **Scenario 5: Commit with Git Env Vars Set**: Explicitly pre-set `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` in the parent environment. Verify that the hook script unsets these prior to compilation/run, avoiding compilation failure or Git database leakage.

---

## 3. Environment Variable Scrubbing & Build Modes

### Git Environment Scrubbing Verification
When a git hook is invoked, Git populates `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` to point to the active repository. If these variables leak into the compilation (`make`) or subprocess execution, subsequent nested git actions or builds can fail or corrupt index files.
- **Verification Design**: The test runner writes a mock `run_benchmarks.sh` script that outputs the active env vars to a file (`env_dump.txt`).
- **Assertion**: The test runner parses `env_dump.txt` and asserts that `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` are either unset or empty, verifying that the hook script scrubbed the environment prior to calling the benchmark runner.

### Real vs Mock Build Handling
To ensure tests can run in headless CI environments without compilers or graphics hardware, the test suite supports two execution modes:
1. **Mock Mode (Default)**:
   - The runner populates `tsfi2-deepseek/benchmarks/profiler_suite/` with mock scripts for `run_benchmarks.sh` and `generate_report.py`.
   - The mock scripts simulate build success/failure and output controlled JSON outputs without compile or Vulkan driver dependencies.
2. **Real Mode (using `--real` flag)**:
   - The runner symlinks the actual project directory `tsfi2-deepseek` into the sandbox.
   - To prevent the actual scripts from writing to the real workspace (due to hardcoded paths), the post-commit hook and scripts must support dynamic path resolution. We recommend:
     - Modifying `run_benchmarks.sh` and `generate_report.py` to check for a `WORKSPACE_DIR` environment variable, falling back to the hardcoded path.
     - The E2E test runner sets `WORKSPACE_DIR` to the sandbox's `tsfi2-deepseek/` path, redirecting all outputs inside the isolated sandbox.

---

## 4. Test Files and Directories Layout
The following layout must be created in the repository root:

```
tests/e2e/
├── run_e2e_tests.py       # Main Python E2E test runner (CLI, Sandbox, Tiers 1-4)
└── test_cases/            # Placeholder for future input/output test configurations
```

---

## 5. Implementation and Verification Plan

### Phase 1: Preparation (Explorer & Developer)
1. Modify `tsfi2-deepseek/benchmarks/profiler_suite/run_benchmarks.sh` to support the `WORKSPACE_DIR` environment variable.
2. Modify `tsfi2-deepseek/benchmarks/profiler_suite/generate_report.py` to support `WORKSPACE_DIR`.
3. Create the post-commit hook script source template at `tsfi2-deepseek/benchmarks/git_ci_pipeline/post-commit`.

### Phase 2: Implementation (Implementer)
1. Create `tests/e2e/run_e2e_tests.py` using the proposed runner template.
2. Verify all mock-mode test cases pass.

### Phase 3: Integration & Real-Mode Verification (Challenger & Auditor)
1. Execute the test runner in default mock mode:
   ```bash
   python3 tests/e2e/run_e2e_tests.py
   ```
   Confirm that all 49 test cases across Tiers 1-4 execute and pass, exiting with 0.
2. Execute the test runner in real mode:
   ```bash
   python3 tests/e2e/run_e2e_tests.py --real
   ```
   Confirm that real compilation and real benchmarks run inside the sandbox without polluting the host repository.
3. Publish `TEST_READY.md` to the root folder.
