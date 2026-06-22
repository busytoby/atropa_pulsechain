# Analysis: Git Post-Commit Hook E2E Test Suite

This document provides a detailed requirements analysis and architectural design for the Git post-commit hook E2E test runner, located at `tests/e2e/run_e2e_tests.py`. It details the git sandbox environment creation, Tier 1–4 test suites, directory layout, mock assets, code blueprint, and a verification plan.

---

## 1. Git Sandbox Environment Design

To perform E2E testing of Git commit hooks without polluting or introducing side effects to the project's own repository, we must create a **Git Sandbox**. The sandbox is an isolated, temporary Git repository created on-demand for each test case or suite run.

### Setup and Lifecycle
1. **Creation**: Use Python's `tempfile.TemporaryDirectory` to create a clean directory.
2. **Git Initialization**: Run `git init` inside the temporary directory.
3. **Git Configuration**: Since E2E tests may run in headless environments (like CI) where global Git configuration (`user.name`/`user.email`) is missing or read-only, we must set local Git configuration variables:
   ```bash
   git config --local user.name "E2E Tester"
   git config --local user.email "tester@e2e.atropa"
   ```
4. **Project Structure Emulation**: Create the required directory structure:
   - `<sandbox>/tsfi2-deepseek/benchmarks/profiler_suite/`
   - `<sandbox>/.git/hooks/`
5. **Teardown**: The temporary directory is recursively deleted upon test completion, unless a debugging flag (`--keep-sandbox`) is set.

### Mock vs. Real Build Modes
To ensure test isolation, rapid feedback (milliseconds vs. minutes), and ease of boundary-case injection, the test runner must support two execution modes:

- **Mock Build Mode (Default)**:
  - Writes a mock `Makefile` into `<sandbox>/tsfi2-deepseek/Makefile`. The target `bin/test_vulkan_teddy` compiles a small mock executable or copies a shell script.
  - Writes a mock `run_benchmarks.sh` that checks environment variables, outputs a predefined `benchmark_results.json`, and appends environment states to `env_check.log`.
  - Copy or symlink the real `generate_report.py` (since it is a Python script that runs instantly and handles missing JSON files gracefully by entering fallback mode).
  - Enables injecting compile errors (by forcing mock `make` to exit with status 2) and benchmark crashes (by forcing the mock binary to exit with status 139).

- **Real Build Mode (`--real-build` flag)**:
  - Symlinks or copies the real directories (`src/`, `inc/`, `plugins/`, `tests/`, `assets/`, `tsfi2-deepseek/`) from the current workspace into the sandbox.
  - Uses the real compiler and Vulkan build dependencies to build `bin/test_vulkan_teddy` and run the full benchmark.

---

## 2. Design of Tiers 1-4 Tests

### 2.1 Tier 1: Features (20 test cases, 5 per feature)

#### Feature 1: Git Hook Installation & Trigger
1. `test_install_hook_creates_file`: Verify that invoking hook installation creates `.git/hooks/post-commit`.
2. `test_install_hook_makes_executable`: Verify that `.git/hooks/post-commit` has executable permissions (`0o755`).
3. `test_uninstall_hook_removes_file`: Verify that uninstalling the hook deletes `.git/hooks/post-commit`.
4. `test_hook_triggers_on_simple_commit`: Edit a staged file in the sandbox, commit, and verify the hook is triggered.
5. `test_hook_triggers_on_amend`: Amend a commit and verify the hook triggers.

#### Feature 2: Benchmark Execution
6. `test_benchmark_compilation_triggered`: Verify that `make bin/test_vulkan_teddy` is executed during the hook run.
7. `test_benchmark_script_triggered`: Verify that `run_benchmarks.sh` is executed.
8. `test_benchmark_creates_json_output`: Verify that `benchmark_results.json` is generated in `tsfi2-deepseek/benchmarks/profiler_suite/`.
9. `test_benchmark_binary_arguments`: Verify that the binary is invoked with the `--benchmark` flag.
10. `test_benchmark_execution_environment`: Verify that the benchmark runner executes in the correct directory.

#### Feature 3: HTML Report Updates
11. `test_html_report_created`: Verify that `benchmark_report.html` is generated after the hook runs.
12. `test_html_report_contains_chart`: Verify that the generated HTML contains the `<canvas>` elements for the charts.
13. `test_html_report_contains_json_data`: Verify that the generated HTML contains the serialized JSON data.
14. `test_html_report_reflects_new_results`: Verify that the average FPS in the HTML file matches the average FPS in the JSON results.
15. `test_html_report_non_empty`: Verify that the HTML report size is greater than 0 bytes and has a valid HTML5 structure.

#### Feature 4: Summary Table Output
16. `test_stdout_contains_summary_table`: Verify that the hook outputs an ASCII summary table to stdout during `git commit`.
17. `test_summary_table_headers`: Verify that the summary table contains expected headers ("Render Performance", "Audio Latency", etc.).
18. `test_summary_table_aligns_values`: Verify that the numeric values in the summary table match the JSON values.
19. `test_summary_table_format`: Verify that the table structure uses standard ASCII borders (e.g., `+---+`, `|`).
20. `test_summary_table_fallback_msg`: Verify that if JSON is missing, the summary table is still printed (perhaps with mock data or a warning banner).

---

### 2.2 Tier 2: Boundaries (20 test cases, 5 per feature boundary/failure modes)

#### Feature 1: Git Hook Installation & Trigger
1. `test_install_when_hook_already_exists_backup`: If `.git/hooks/post-commit` already exists, check if the runner backs it up (e.g. `.git/hooks/post-commit.backup`) instead of silently overwriting.
2. `test_uninstall_restores_backup`: Uninstalling the hook restores the backup if it existed.
3. `test_install_without_git_repo`: Run hook installation outside of a git repository; verify it fails gracefully with an informative error.
4. `test_hook_trigger_with_no_changes`: Commit with `--allow-empty` and check hook behavior.
5. `test_hook_trigger_detached_head`: Verify that the hook triggers correctly when the repository is in a detached HEAD state.

#### Feature 2: Benchmark Execution
6. `test_compilation_failure_graceful`: Mock a compilation failure (e.g. `make` exits with status 2). Verify that the commit is NOT aborted (post-commit cannot abort, but verify it returns cleanly and doesn't block the user's shell/process).
7. `test_compilation_failure_logs`: Verify that compilation errors are logged/printed to stderr or a log file for debugging.
8. `test_benchmark_crash_graceful`: Mock `bin/test_vulkan_teddy` crashing (exits with non-zero status). Verify the hook handles it gracefully, prints a warning, and continues.
9. `test_benchmark_results_json_unwritable`: Make the directory for JSON read-only or remove permissions. Verify that the hook handles the write failure gracefully.
10. `test_missing_make_utility`: Mock `make` missing from the system (or not executable). Verify the hook reports this but does not crash the commit workflow.

#### Feature 3: HTML Report Updates
11. `test_html_generation_fails_graceful`: Mock `generate_report.py` failing. Verify the hook logs the error and exits gracefully.
12. `test_html_overwrites_existing`: Run hook twice; verify that the second run successfully overwrites the previous HTML report with new metrics.
13. `test_html_report_path_unwritable`: Mock the HTML file path being read-only. Verify that the generator fails gracefully without throwing unhandled exceptions.
14. `test_html_fallback_mode`: Verify that if `benchmark_results.json` is missing or corrupted, the report generator still runs and uses fallback mock data (with a warning banner in HTML as seen in `generate_report.py`).
15. `test_html_large_json`: Verify that if the JSON file is very large (e.g., thousands of frame times), the HTML generator parses it without memory exhaustion or stack overflow.

#### Feature 4: Summary Table Output
16. `test_stdout_json_corrupted`: Verify that if `benchmark_results.json` is corrupted, the summary table parser prints a fallback warning rather than crashing.
17. `test_stdout_metrics_extreme_values`: Verify that the ASCII table format does not break (remains aligned) when metrics are extremely large or small (e.g., FPS = 1000000.0 or 0.0001).
18. `test_stdout_color_codes_stripped`: If stdout is redirected or is not a TTY, verify that ASCII color escape codes are stripped or handled correctly.
19. `test_stdout_table_suppressed_on_quiet`: If git commit is run with `--quiet` (`-q`), verify whether the hook output is suppressed or prints minimally.
20. `test_stdout_table_no_audio_records`: Verify the summary table behavior when `audio_latency_records` is empty in the JSON.

---

### 2.3 Tier 3: Combinations (4 pairwise feature combination tests)
1. `test_combo_compilation_success_report_success`: Compilation succeeds + benchmark runs + report generated + summary table printed. (Standard happy path).
2. `test_combo_compilation_success_benchmark_fail`: Compilation succeeds + benchmark fails + HTML report uses fallback mock data + summary table printed with mock data/warning.
3. `test_combo_compilation_fail_no_report`: Compilation fails + benchmark skipped + HTML report is not updated (or retains old report) + summary table reports compilation error.
4. `test_combo_environment_scrubbed_with_detached_head`: Running commit in detached HEAD + verification that environment is scrubbed + build and report succeed.

---

### 2.4 Tier 4: Scenarios (5 real-world scenarios)
1. **Scenario 1: Normal Commit flow**
   - Perform a sequence of normal edits and commits. Verify that the hook installs, triggers on each commit, compiles, runs benchmarks, generates JSON, compiles HTML, and prints the ASCII table.
2. **Scenario 2: Commit from Subdirectory**
   - Change directory into a deep nested subdirectory in the git repository (e.g. `tsfi2-deepseek/benchmarks/`) and run `git commit` from there. Verify that the hook executes correctly, unsets `GIT_*` environment variables, compiles from the workspace root, and puts the outputs in the correct paths.
3. **Scenario 3: Broken Build recovery**
   - First commit has a syntax error that breaks compilation. Verify that hook runs, prints compilation error, does not block commit, and doesn't generate report.
   - Second commit fixes the syntax error. Verify that the hook compiles successfully, runs benchmarks, and generates the reports correctly.
4. **Scenario 4: Large commit with multiple files**
   - Stage multiple file changes (e.g., source files, assets, docs) and commit. Verify that the hook runs once (not multiple times) and operates correctly on the workspace state.
5. **Scenario 5: Commit with git env vars set**
   - Explicitly set `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` in the shell, then run `git commit`. Verify that the hook successfully unsets these variables before compilation, preventing the compilation/benchmark script from running in the wrong git context or failing.

---

## 3. Exact Test Files and Directories to Create

To establish the E2E test framework, the following files and directories must be created:

```
tests/e2e/
├── run_e2e_tests.py                    # Main test runner containing CLI and test suites
└── test_cases/
    ├── __init__.py                     # Makes test_cases a package
    └── mock_assets/                    # Mock project assets used for fast sandbox runs
        ├── mock_Makefile               # Simulates make compiler targets
        ├── mock_run_benchmarks.sh      # Simulates benchmark execution
        ├── mock_results_normal.json    # Pre-baked valid benchmark JSON results
        └── mock_results_corrupt.json   # Pre-baked invalid benchmark JSON results
```

---

## 4. Code Blueprint: `run_e2e_tests.py`

This blueprint provides the Python implementation structure for the E2E test runner:

```python
#!/usr/bin/env python3
"""
ATROPA PulseChain Git Post-Commit Hook E2E Test Runner
Covers Tiers 1-4 testing.
"""
import os
import sys
import shutil
import tempfile
import subprocess
import unittest
import argparse
from pathlib import Path

# Paths
WORKSPACE_DIR = Path(__file__).resolve().parents[2]
REAL_GEN_REPORT = WORKSPACE_DIR / "tsfi2-deepseek/benchmarks/profiler_suite/generate_report.py"

class GitSandbox:
    """Manages the lifecycle of an isolated temporary git repository."""
    def __init__(self, use_real_build=False, keep_sandbox=False):
        self.use_real_build = use_real_build
        self.keep_sandbox = keep_sandbox
        self.temp_dir = None
        self.sandbox_path = None

    def __enter__(self):
        self.temp_dir = tempfile.TemporaryDirectory(prefix="git_sandbox_")
        self.sandbox_path = Path(self.temp_dir.name)
        
        # Initialize Git
        subprocess.run(["git", "init"], cwd=self.sandbox_path, check=True, capture_output=True)
        # Local configuration to prevent dependency on host config
        subprocess.run(["git", "config", "user.name", "E2E Tester"], cwd=self.sandbox_path, check=True)
        subprocess.run(["git", "config", "user.email", "tester@e2e.atropa"], cwd=self.sandbox_path, check=True)
        
        # Create directories
        (self.sandbox_path / "tsfi2-deepseek/benchmarks/profiler_suite").mkdir(parents=True, exist_ok=True)
        
        if self.use_real_build:
            self._copy_real_project()
        else:
            self._setup_mock_project()
            
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if not self.keep_sandbox and self.temp_dir:
            self.temp_dir.cleanup()

    def _setup_mock_project(self):
        # Create mock Makefile
        makefile_content = (
            "bin/test_vulkan_teddy:\n"
            "\tmkdir -p bin\n"
            "\techo '#!/bin/sh' > bin/test_vulkan_teddy\n"
            "\techo 'echo \"GIT_DIR=$GIT_DIR\" >> env_check.log' >> bin/test_vulkan_teddy\n"
            "\techo 'echo \"GIT_WORK_TREE=$GIT_WORK_TREE\" >> env_check.log' >> bin/test_vulkan_teddy\n"
            "\techo 'echo \"GIT_INDEX_FILE=$GIT_INDEX_FILE\" >> env_check.log' >> bin/test_vulkan_teddy\n"
            "\tcat ../mock_results_normal.json > benchmarks/profiler_suite/benchmark_results.json\n"
            "\tchmod +x bin/test_vulkan_teddy\n"
        )
        with open(self.sandbox_path / "tsfi2-deepseek/Makefile", "w") as f:
            f.write(makefile_content)

        # Create mock run_benchmarks.sh
        run_benchmarks_content = (
            "#!/bin/bash\n"
            "cd ../..\n"
            "make bin/test_vulkan_teddy\n"
        )
        run_benchmarks_path = self.sandbox_path / "tsfi2-deepseek/benchmarks/profiler_suite/run_benchmarks.sh"
        with open(run_benchmarks_path, "w") as f:
            f.write(run_benchmarks_content)
        run_benchmarks_path.chmod(0o755)

        # Copy real report generator
        shutil.copy(REAL_GEN_REPORT, self.sandbox_path / "tsfi2-deepseek/benchmarks/profiler_suite/generate_report.py")

        # Copy pre-baked mock results
        shutil.copy(
            WORKSPACE_DIR / "tests/e2e/test_cases/mock_assets/mock_results_normal.json",
            self.sandbox_path / "tsfi2-deepseek/mock_results_normal.json"
        )

    def _copy_real_project(self):
        # Implementation of full workspace copying/linking
        pass

    def install_hook(self, hook_content):
        hooks_dir = self.sandbox_path / ".git/hooks"
        hooks_dir.mkdir(parents=True, exist_ok=True)
        hook_path = hooks_dir / "post-commit"
        with open(hook_path, "w") as f:
            f.write(hook_content)
        hook_path.chmod(0o755)

    def make_commit(self, file_name="dummy.txt", content="hello"):
        file_path = self.sandbox_path / file_name
        file_path.write_text(content)
        subprocess.run(["git", "add", file_name], cwd=self.sandbox_path, check=True)
        result = subprocess.run(
            ["git", "commit", "-m", f"update {file_name}"],
            cwd=self.sandbox_path,
            capture_output=True,
            text=True
        )
        return result

# --- Test Cases ---

HOOK_TEMPLATE = """#!/bin/bash
# post-commit hook
unset GIT_DIR
unset GIT_WORK_TREE
unset GIT_INDEX_FILE

REPO_ROOT=$(git rev-parse --show-toplevel)
cd "$REPO_ROOT/tsfi2-deepseek" || exit 1

make bin/test_vulkan_teddy > compile.log 2>&1
if [ $? -ne 0 ]; then
    echo "[ERROR] Compilation failed."
    exit 0
fi

cd benchmarks/profiler_suite || exit 1
./run_benchmarks.sh > run.log 2>&1
python3 generate_report.py > report.log 2>&1

if [ -f "benchmark_results.json" ]; then
    python3 -c "
import json
with open('benchmark_results.json') as f:
    d = json.load(f)
s = d['summary']
print('+-------------------------------------------------------+')
print('|          ATROPA PulseChain Benchmark Summary          |')
print('+-------------------------------------------------------+')
print(f'| Total Frames:     {s.get(\"total_frames\", 0):>35d} |')
print(f'| Average FPS:      {s.get(\"average_fps\", 0.0):>35.2f} |')
print(f'| Avg Render Time:  {s.get(\"average_frame_render_time_seconds\", 0.0)*1000:>32.3f} ms |')
print(f'| Elapsed Time:     {s.get(\"elapsed_time_seconds\", 0.0):>33.2f} s |')
print('+-------------------------------------------------------+')
"
fi
"""

class TestTier1Features(unittest.TestCase):
    def test_install_hook_creates_file(self):
        with GitSandbox() as sandbox:
            sandbox.install_hook(HOOK_TEMPLATE)
            hook_file = sandbox.sandbox_path / ".git/hooks/post-commit"
            self.assertTrue(hook_file.exists())
            self.assertTrue(os.access(hook_file, os.X_OK))

    def test_hook_triggers_on_simple_commit(self):
        with GitSandbox() as sandbox:
            sandbox.install_hook(HOOK_TEMPLATE)
            res = sandbox.make_commit("test.txt", "change")
            self.assertIn("ATROPA PulseChain Benchmark Summary", res.stdout)
            self.assertTrue((sandbox.sandbox_path / "tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html").exists())

    # Further Tier 1 test implementations...

class TestTier2Boundaries(unittest.TestCase):
    def test_compilation_failure_graceful(self):
        with GitSandbox() as sandbox:
            # Inject compilation failure in Makefile
            bad_makefile = "bin/test_vulkan_teddy:\n\treturn 2\n"
            with open(sandbox.sandbox_path / "tsfi2-deepseek/Makefile", "w") as f:
                f.write(bad_makefile)
            
            sandbox.install_hook(HOOK_TEMPLATE)
            res = sandbox.make_commit("test.txt", "change")
            # Commit must still succeed (exit code 0)
            self.assertEqual(res.returncode, 0)
            self.assertIn("[ERROR] Compilation failed.", res.stdout)

    def test_git_env_vars_scrubbed(self):
        with GitSandbox() as sandbox:
            sandbox.install_hook(HOOK_TEMPLATE)
            sandbox.make_commit("test.txt", "change")
            
            env_log_path = sandbox.sandbox_path / "tsfi2-deepseek/env_check.log"
            self.assertTrue(env_log_path.exists())
            
            env_vars = env_log_path.read_text().splitlines()
            for var in env_vars:
                # Assert git vars are empty/unset in the build step
                name, val = var.split("=", 1)
                self.assertEqual(val.strip(), "")

    # Further Tier 2 test implementations...

class TestTier3Combinations(unittest.TestCase):
    # Pairwise feature combination test cases...
    pass

class TestTier4Scenarios(unittest.TestCase):
    # Real-world scenario test cases...
    pass

def main():
    parser = argparse.ArgumentParser(description="Run E2E tests for post-commit hook.")
    parser.add_argument("--real-build", action="store_true", help="Use real Vulkan compilation and run.")
    parser.add_argument("--keep-sandbox", action="store_true", help="Do not delete sandboxes on exit.")
    parser.add_argument("--only-tier", type=int, choices=[1, 2, 3, 4], help="Run only specified tier of tests.")
    args, unknown = parser.parse_known_args()

    # Pass args downstream or use unittest runner configurations
    unittest_args = [sys.argv[0]] + unknown
    unittest.main(argv=unittest_args)

if __name__ == "__main__":
    main()
```

---

## 5. Recommended Implementation & Verification Plan

### Phase 1: Test Infrastructure Preparation
1. Create `tests/e2e/test_cases/mock_assets/` and populate `mock_results_normal.json` and `mock_results_corrupt.json`.
2. Write `tests/e2e/run_e2e_tests.py` with mock sandbox manager, Tier 1 and Tier 2 skeleton test classes.

### Phase 2: Hook Implementation & Tier 1/2 Verification
1. Implement the post-commit hook logic in `.git/hooks/post-commit` (or templates/scripts).
2. Wire up the Git Hook environment cleaning variables in the script.
3. Run `python3 tests/e2e/run_e2e_tests.py --only-tier 1` and `python3 tests/e2e/run_e2e_tests.py --only-tier 2`.
4. Iterate until Tier 1 and Tier 2 tests pass 100%.

### Phase 3: Complex Combinations & Scenarios (Tier 3/4)
1. Add Tier 3 combinations and Tier 4 scenarios (commit from subdirectory, broken build recovery, etc.) in the runner script.
2. Execute tests with environmental overrides to ensure no side effects on the project.
3. Validate that standard output displays correct ASCII tables and exit codes are clean.

### Phase 4: Final Validation
1. Run `python3 tests/e2e/run_e2e_tests.py` to verify all Tiers (1–4) pass synchronously under mock mode.
2. If the build environment supports Vulkan, run with `--real-build` to verify exact integration.
3. Publish `TEST_READY.md` to the workspace root directory.
