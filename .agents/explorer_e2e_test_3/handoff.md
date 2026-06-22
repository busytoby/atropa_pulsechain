# Handoff Report - Explorer E2E Test 3

## 1. Observation
- **Test Infrastructure Layout**: `TEST_INFRA.md` specifies that the test runner is located at `tests/e2e/run_e2e_tests.py` and test configurations at `tests/e2e/test_cases/`.
- **Hardcoded Path in Report Generator**: `tsfi2-deepseek/benchmarks/profiler_suite/generate_report.py` contains a hardcoded absolute path at line 8:
  ```python
  WORKSPACE_DIR = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek"
  ```
- **Git Environment Scrubbing Contract**: `PROJECT.md` states:
  > "Must unset Git environment variables (`GIT_DIR`, `GIT_WORK_TREE`, `GIT_INDEX_FILE`) to prevent leakage into downstream commands."
- **Scope Thresholds**: `SCOPE.md` details:
  - Tier 1: 20 test cases (5 per feature)
  - Tier 2: 20 test cases (5 per feature boundary/failure modes)
  - Tier 3: 4 pairwise combination tests
  - Tier 4: 5 real-world scenarios
- **Missing Directories/Files**: No `tests/e2e/` folder or `.git/hooks/post-commit` script exists yet in the workspace.

## 2. Logic Chain
- **Sandbox Isolation**: Since Git commands depend on environment variables, executing `git` commands inside a temporary directory while inheriting the parent process's environment variables (`GIT_DIR`, `GIT_WORK_TREE`) will result in operations being directed back to the project's own repository. Consequently, the test runner must sanitize the environment dictionary passed to subprocesses by popping all Git-related variables.
- **Portability**: Because `generate_report.py` has a hardcoded path to the real workspace, invoking it inside a sandbox environment will either fail or write reports to the real project folder. Therefore, `generate_report.py` must be patched to compute its workspace directory dynamically relative to its file location.
- **Dual-Build Mode Necessity**: Building the Vulkan application (`bin/test_vulkan_teddy`) requires specialized graphic drivers, packages, and compiling C source code, which is too slow and fragile for rapid local testing. To solve this, E2E tests should default to a **Mock Build Mode** that simulates compilation and benchmark execution using a shell script, and support a **Real Build Mode** via a command-line flag (`--real-build`) that symlinks the real sources.
- **Graceful Failures**: The Git `post-commit` hook is executed after a commit has already been recorded. Thus, any hook failure (e.g. compile error) must not cause the commit command to fail. The E2E runner can verify this by checking that `git commit` still returns exit code 0 even when `make` is mocked to fail.

## 3. Caveats
- It is assumed that the test environment contains Python 3.x and a functional Git installation.
- Real build execution will require proper installation of compiler toolchains, Vulkan, and ALSA dependencies, which may not be fully set up in headless environments.

## 4. Conclusion
The requirements for the E2E test runner are completely defined and ready for implementation. By constructing `tests/e2e/run_e2e_tests.py` using Python's built-in `unittest` module, introducing a `GitSandbox` manager that cleans the environment dictionary, and patching `generate_report.py` to use relative paths, we can test all 49 test cases under Tiers 1-4 with 100% safety, high speed, and zero side-effects.

## 5. Verification Method
- **Execution Command**: The E2E test suite can be run via:
  ```bash
  python3 tests/e2e/run_e2e_tests.py
  ```
- **Files to Inspect**:
  - `tests/e2e/run_e2e_tests.py` for sandbox setup and unittest test classes.
  - `.git/hooks/post-commit` inside the generated temporary sandbox directories during run.
- **Invalidation Conditions**: If running `git commit` inside the sandbox modifies the parent repository's commit history or files, the sandbox isolation has failed and the design is invalid.
