# Adversarial Review of Git Post-Commit Hook & E2E Test Suite

## Challenge Summary

**Overall risk assessment**: MEDIUM

While the post-commit hook integration and the E2E test suite are robustly designed and implement comprehensive test coverage (49 test cases), they rely on assumptions regarding sandbox isolation and host environment hygiene that pose risks in active development or real-world build settings.

*Note on Execution*: Terminal execution via `run_command` in this environment was constrained due to interactive user-approval prompts timing out (both for this agent and previous iterations). Therefore, this report is based on a exhaustive static code trace, logic-flow validation of the 49 test cases, and simulation of the mock execution paths.

---

## Attack Surface

### Hypotheses Tested
- **Hypothesis 1**: The sandbox repository in `--real-build` mode is completely isolated from the developer's main working tree. (Result: **FALSE**, see Challenge 1).
- **Hypothesis 2**: The post-commit hook handles a poisoned Git environment cleanly. (Result: **TRUE** for common variables, but potentially **FALSE** for variables like `GIT_COMMON_DIR` or `GIT_OBJECT_DIRECTORY`, see Challenge 3).
- **Hypothesis 3**: Test suite validation survives output directory collisions or permission restrictions. (Result: **TRUE**, tests verify graceful fallback to N/A or warning outputs under directory/file restrictions).

### Vulnerabilities Found
- **Symlink Pollution of Host Tree**: In `--real-build` mode, files from the host repository are recursively symlinked. Modifying these files inside the sandbox follows the symlink and mutates active development files on the host.
- **Incomplete Git Environment Scrubbing**: The hook script unsets only `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE`, leaving other `GIT_*` environment variables intact which can pollute sandbox commands.

### Untested Angles
- **Vulkan / Headless GPU execution**: Real-world driver interactions (Wayland/X11 window managers, Vulkan ICD loader configs) under actual resource constraints remain untested.

---

## Challenges

### [High] Challenge 1: Symlink-based Host Pollution in `--real-build` Mode

- **Assumption challenged**: The test runner assumes that the git sandbox repository created in a temporary directory is isolated from the host/developer environment.
- **Attack scenario**: In `real-build` mode, `GitSandbox.populate_real` recursively symlinks files from the project root (`REAL_TSFI2_DEEPSEEK`) to the sandbox directory. If a test case or downstream compiler/profiler tool opens a symlinked file in write/append mode rather than replacing it, the write operation follows the symlink and overwrites active development files.
- **Blast radius**: Local source code or config files could be corrupted, altered, or wiped during a test run, interfering with active development.
- **Mitigation**: Perform copy-on-write or deep-copy operations instead of symlinking, or execute the runner in a containerized environment (e.g. Docker/Podman).

### [Medium] Challenge 2: Silent Failures of Benchmark Runs & HTML Generation

- **Assumption challenged**: The hook assumes that benchmark runs and report generation execute successfully or that silently swallowing failures is safe.
- **Attack scenario**: If python or shell dependencies change, `generate_report.py` or `run_benchmarks.sh` could fail. The hook intercepts all exit codes and terminates with `exit 0` to prevent blocking the Git commit. If errors occur, developers will not receive visible alerts, causing the HTML report dashboard to display stale or invalid data.
- **Blast radius**: The HTML dashboard and printed metrics display stale or corrupted data indefinitely without developer awareness.
- **Mitigation**: Output error stack traces and exit code warnings to standard error (`stderr`), or propagate a non-zero exit status if a strict validation mode is enabled.

### [Medium] Challenge 3: Incomplete Environment Variable Scrubbing

- **Assumption challenged**: The hook assumes that unsetting `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` is sufficient to scrub the Git environment.
- **Attack scenario**: A user environment might have other Git variables set, such as `GIT_COMMON_DIR`, `GIT_OBJECT_DIRECTORY`, or `GIT_ALTERNATE_OBJECT_DIRECTORIES`. If these leak into the hook subprocesses (like compiling, running benchmarks, or committing inside the sandbox), they can pollute git operations, leading to commit failures or repo metadata pollution.
- **Blast radius**: Sandbox execution fails or writes commit metadata to incorrect directories.
- **Mitigation**: Unset all environment variables starting with `GIT_` dynamically using a loop or prefix filter before spawning hook subprocesses.

---

## Stress Test Results

- **Scenario 1**: Commit from a nested subdirectory → Hook unsets `GIT_DIR`, navigates to root, and executes → **PASS** (Logical validation of `test_45_scenario_commit_from_subdirectory`).
- **Scenario 2**: Environment variables `GIT_DIR` and `GIT_WORK_TREE` pre-set in shell → Hook scrubs variables and resolves paths correctly → **PASS** (Logical validation of `test_48_scenario_env_scrubbing_validation`).
- **Scenario 3**: Broken build recovery (commit breaks compilation, next commit fixes it) → Sandbox successfully tracks state changes and returns correct status → **PASS** (Logical validation of `test_47_scenario_broken_build_recovery`).
- **Scenario 4**: Corrupted/Missing benchmark JSON output → Hook falls back to "N/A" formatting or mock defaults without crashing → **PASS** (Logical validation of `test_34_generate_report_corrupted_json` and `test_36_summary_table_missing_json`).

---

## Unchallenged Areas

- **GPU/Vulkan Hardware Metrics**: Hardware performance counters, frame rendering, or sound device configurations were not challenged as mock mode runs emulated assets and does not communicate with physical Vulkan drivers or sound cards.
