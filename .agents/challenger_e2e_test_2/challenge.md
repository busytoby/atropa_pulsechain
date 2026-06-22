# Adversarial Review of Git Post-Commit Hook & E2E Test Suite

## Challenge Summary

**Overall risk assessment**: MEDIUM

While the mock E2E test suite covers 49 test cases across Tiers 1-4 and demonstrates a highly robust sandbox environment design (using dynamic git initialization and environment scrubbing), several critical assumptions, edge cases, and potential failures exist when running in integration or production environments.

*Note on Execution*: In the current verification environment, executing shell commands via `run_command` timed out due to interactive permission prompts. Thus, this review relies on empirical static code tracing, logical analysis of the 49 test cases, and simulation of the mock setup paths.

---

## Challenges

### [Medium] Challenge 1: Lack of Strict Error Handlers in `post-commit` Script

- **Assumption challenged**: The hook assumes that failures during benchmark run or HTML report generation are safe to ignore, but ignoring them blindly might hide system degradation or misconfiguration.
- **Attack scenario**: A change in the python interpreter or a corrupted dependencies installation could cause `generate_report.py` to fail constantly. Since the post-commit hook doesn't propagate non-zero exit codes (it does not check the exit status of `generate_report.py` or `run_benchmarks.sh`), these silent failures would go unnoticed in CI pipelines until manual inspection.
- **Blast radius**: The HTML dashboard and benchmark summaries will fail to generate/update silently, while developers are falsely assured that commits are succeeding.
- **Mitigation**: Add warning exit logs or trigger a secondary warning hook (e.g. sending metrics to a logging service or printing high-visibility error outputs to stderr during commits).

### [High] Challenge 2: Sandbox Subversion in `--real-build` Mode

- **Assumption challenged**: The test runner assumes the sandbox repository is completely isolated from the host environment.
- **Attack scenario**: In `--real-build` mode, the runner symlinks the real `tsfi2-deepseek` directory to the sandbox. If compilation or benchmark scripts modify local files or global state outside the symlink boundaries, it can contaminate the host project directory or corrupt active development builds.
- **Blast radius**: Running E2E tests with `--real-build` could alter files inside the primary repo or cause build interference with concurrent developer actions.
- **Mitigation**: Use full copy-on-write or containerized environments for real-build tests rather than recursive symlinks.

### [Low] Challenge 3: Disk Space Exhaustion on Sandbox Creation

- **Assumption challenged**: The test runner assumes ample temporary directory space for running tests.
- **Attack scenario**: In environment setups with restricted `/tmp` sizes or disk quota limits, running the suite of 49 test cases (each initializing a fresh `TemporaryDirectory` and copying files/git metadata) could lead to "No space left on device" errors.
- **Blast radius**: Test execution fails halfway, leaving orphaned temporary directories (if cleanup fails).
- **Mitigation**: Explicitly call `cleanup()` in a `finally` block or clean up stale directories proactively, and restrict concurrent execution to limit peak disk usage.

---

## Stress Test Results

- **Scenario 1**: Commit with locked index → Hook executes successfully using `get_clean_env` → Pass (verified via `test_23_hook_run_when_git_index_locked` code logic).
- **Scenario 2**: Execute hook in non-git directory → Script catches error, warns and exits gracefully with 0 → Pass (verified via `test_24_hook_execute_in_non_git_repo` code logic).
- **Scenario 3**: Corrupted benchmark results JSON → Report generator falls back to mock data safely → Pass (verified via `test_34_generate_report_corrupted_json` code logic).
- **Scenario 4**: Directory collision at HTML destination → Report generator fails to write but hook exits 0 without blocking Git commit → Pass (verified via `test_33_html_report_write_permission_denied` code logic).

---

## Unchallenged Areas

- **Vulkan / GPU hardware profiling**: Out of scope since the tests run in mock mode and the host lacks Vulkan/ALSA display and sound resources for real profiling.
