# Scope: Git CI Benchmark Pipeline E2E Testing

## Architecture
- **Git Sandbox Manager**: A Python-based utility in the E2E test runner that creates isolated, temporary git repositories to prevent messing up the user's project repository during commit hooks execution tests.
- **Hook Lifecycle Verification**: Programmatically installs the git hook, performs commits, and verifies the automated run.
- **Verification of environment scrubbing**: Verifies that the git hook unsets environment variables like `GIT_DIR`, `GIT_WORK_TREE`, and `GIT_INDEX_FILE` prior to running the compilation.
- **Validation of output artifacts**: Verifies the existence and contents of `benchmark_results.json`, `benchmark_report.html`, and stdout reports.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 1 | Design Test Runner Framework | Setup `tests/e2e/run_e2e_tests.py` with CLI structure and git sandbox creation. | none | PLANNED |
| 2 | Tier 1 Features Tests | Implement 20 test cases covering installation, benchmark trigger, HTML report, and table stdout. | M1 | PLANNED |
| 3 | Tier 2 Boundaries Tests | Implement 20 test cases covering compilation failures, invalid commits, missing binaries. | M2 | PLANNED |
| 4 | Tier 3 Combinations Tests | Implement 4 pairwise feature interaction test cases. | M2 | PLANNED |
| 5 | Tier 4 Scenarios Tests | Implement 5 real-world scenario tests (broken build recovery, git env vars set, etc.). | M3, M4 | PLANNED |
| 6 | Publish TEST_READY.md & Verify | Ensure E2E tests can run, publish `TEST_READY.md` to project root. | M5 | PLANNED |

## Interface Contracts
### E2E Test Suite ↔ Repository Git Hook
- **Installation**: E2E test runner installs/uninstalls hook by modifying `.git/hooks/post-commit`.
- **Test execution**: Output exit code 0 on success, non-zero on failure.
- **Verification channels**: Exit code, standard output (ASCII statistics summary table), and generated HTML/JSON reports.
