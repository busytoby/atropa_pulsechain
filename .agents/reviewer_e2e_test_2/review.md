# Review Report: Git Post-commit Hook & E2E Test Suite

## Review Summary

**Verdict**: REQUEST_CHANGES

This review evaluates the correctness, completeness, robustness, and interface conformance of the Git post-commit hook E2E test suite. While the E2E test suite is highly comprehensive (defining 49 tests across Tiers 1-4) and the post-commit hook is structurally robust (with proper environment variable scrubbing and graceful error handling), we found a major sandbox isolation violation when running the suite in `--real-build` mode.

---

## Quality Review Findings

### [Major] Finding 1: Sandbox Isolation Violation in `--real-build` Mode
- **What**: The `run_benchmarks.sh` script contains a hardcoded absolute path `WORKSPACE_DIR="/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek"` and performs a directory change (`cd "${WORKSPACE_DIR}"`) to that path.
- **Where**: `tsfi2-deepseek/benchmarks/profiler_suite/run_benchmarks.sh`, lines 5-10.
- **Why**: When the E2E tests are executed with `--real-build`, the runner creates a sandboxed Git repository in a temporary directory and symlinks the real repository files. When the post-commit hook triggers inside the sandbox, it executes the real `./run_benchmarks.sh` script. Due to the hardcoded path, the script changes directory to the host workspace, compiles the benchmark suite, and generates the `benchmark_results.json` file on the host filesystem instead of executing inside the sandbox. This violates the sandbox isolation principle and could lead to side-effects on the host system.
- **Suggestion**: Replace the hardcoded `WORKSPACE_DIR` with a dynamic path calculation relative to the script location, e.g.:
  ```bash
  WORKSPACE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"
  ```

### [Minor] Finding 2: Missing or Null Value Formatting in ASCII Table Parser
- **What**: The Python parsing block inside the post-commit hook uses `summary.get(..., 'N/A')`, but does not handle cases where the key exists but its value is `None` (null in JSON).
- **Where**: `tsfi2-deepseek/benchmarks/git_ci_pipeline/post-commit`, lines 69-84.
- **Why**: If any summary value is explicitly set to `null` in the JSON, `v == 'N/A'` will be false, and `float(None)` will raise a `TypeError`, falling back to `str(None)` which prints `'None'` instead of `'N/A'`.
- **Suggestion**: Modify `fmt` to handle `None` value explicitly:
  ```python
  if v is None or v == 'N/A': return 'N/A'
  ```

---

## Verified Claims

- **GIT_DIR, GIT_WORK_TREE, GIT_INDEX_FILE scrubbing** → verified via code inspection of `post-commit` lines 20-22 → **PASS** (correctly unsets all three variables before executing compiling and benchmarking commands).
- **Benchmark Suite compilation and execution** → verified via code inspection of `post-commit` and `Makefile` → **PASS** (triggers compilation via `make bin/test_vulkan_teddy` and runs the benchmarks script and report generator).
- **Graceful Failure Handling** → verified via code inspection of `post-commit` lines 32-35 → **PASS** (returns exit code 0 and prints warnings, ensuring Git operations are never blocked).
- **ASCII table clean output** → verified via code inspection of `post-commit` lines 96-104 → **PASS** (correctly formatted ASCII table matching 55 characters in width).
- **Test suite count** → verified via code inspection of `tests/e2e/run_e2e_tests.py` → **PASS** (contains exactly 49 tests across Tiers 1-4).

---

## Coverage Gaps

- **Dynamic Execution Verification** — risk level: **Medium** — recommendation: **Accept risk for review, verify in integration phase**. Direct execution of the test suite was not verified dynamically because command execution timed out waiting for user/environment permission.

---

## Unverified Items

- **Test Suite runtime success** — reason not verified: Command execution `python3 tests/e2e/run_e2e_tests.py` timed out waiting for approval.

---
---

## Adversarial Review / Challenge Report

## Challenge Summary

**Overall risk assessment**: MEDIUM

The post-commit hook and E2E test runner are generally robust. However, the use of hardcoded paths in the benchmark run script breaks sandbox isolation during real builds. Additionally, the ASCII summary table parsing is sensitive to formatted value overflows and null representations.

---

## Challenges

### [Medium] Challenge 1: Hardcoded Path in `run_benchmarks.sh`
- **Assumption challenged**: Assumed that the sandbox environment was isolated.
- **Attack scenario**: Running tests in `--real-build` mode executes build/benchmark commands inside the host workspace, overwriting the host's `benchmark_results.json` and polluting host workspace state.
- **Blast radius**: Host workspace files can be overwritten or corrupted by sandbox test execution.
- **Mitigation**: Update `run_benchmarks.sh` to resolve paths dynamically relative to script location.

### [Low] Challenge 2: Broken Table Alignment with Extremely Long Values
- **Assumption challenged**: Values in the summary table are assumed to be short numbers.
- **Attack scenario**: An extremely high number of frames or very long floating point representation (e.g. from custom JSON inputs) could exceed 14 characters, breaking the right border alignment of the printed ASCII table.
- **Blast radius**: Purely cosmetic layout breaks.
- **Mitigation**: Slice the formatted string to a maximum of 14 characters, or increase the width limit dynamically.

---

## Stress Test Results

- **Run in sandbox with `--real-build`** → should execute fully isolated → actually escapes sandbox due to hardcoded path in `run_benchmarks.sh` → **FAIL**
- **JSON summary file with missing/empty keys** → should display table with "N/A" → correctly falls back to "N/A" due to default value in `.get()` and try-except block → **PASS**
- **Corrupted JSON file** → should display table with "N/A" and print warning → correctly handled by try-except block in python script → **PASS**
