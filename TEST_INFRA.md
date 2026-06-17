# E2E Test Infra: Git CI Benchmark Pipeline

## Test Philosophy
- Opaque-box, requirement-driven.
- Verifies post-commit hook execution under various git workflows without dependencies on internal implementation details.

## Feature Inventory
| # | Feature | Source (requirement) | Tier 1 | Tier 2 | Tier 3 |
|---|---------|---------------------|:------:|:------:|:------:|
| 1 | Git Hook Installation & Trigger | R1. Git post-commit Hook Script | 5 | 5 | ✓ |
| 2 | Benchmark Execution | R1. Git post-commit Hook Script | 5 | 5 | ✓ |
| 3 | HTML Report Updates | R2. Automated Report Compilation | 5 | 5 | ✓ |
| 4 | Summary Table Output | R2. Automated Report Compilation | 5 | 5 | ✓ |

## Test Architecture
- **Test runner**: Written in Python, located at `tests/e2e/run_e2e_tests.py`.
- **Invocation**: `python3 tests/e2e/run_e2e_tests.py`
- **Pass/Fail Semantics**: 0 exit code if all test cases pass; non-zero otherwise.
- **Directory Layout**:
  - `tests/e2e/test_cases/` - Test input configurations and test helper scripts.

## Real-World Application Scenarios (Tier 4)
| # | Scenario | Features Exercised | Complexity |
|---|----------|--------------------|------------|
| 1 | Normal Commit flow | F1, F2, F3, F4 | Low |
| 2 | Commit from Subdirectory | F1, F2, F3, F4 | Medium |
| 3 | Broken Build recovery | F1, F2, F3, F4 | High |
| 4 | Large commit with multiple files | F1, F2, F3 | Medium |
| 5 | Commit with git env vars set | F1, F2, F3, F4 | High |

## Coverage Thresholds
- Tier 1: ≥20 test cases (5 per feature)
- Tier 2: ≥20 test cases (5 per feature boundary/failure modes)
- Tier 3: ≥4 pairwise feature combination tests
- Tier 4: ≥5 real-world scenarios
