# Verification Plan - Relative Path Fixes

This plan outlines the steps to verify that the relative path fixes for the dashboard container in the HTML tournament files correctly point to the benchmark report, and that all project E2E tests pass.

## Step 1: Inspect HTML files and verify file existence
- Verify that the four HTML files exist in the repository:
  1. `/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/teddy_bear_tournament_3d.html`
  2. `/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_bear_tournament_3d.html`
  3. `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/teddy_bear_tournament_3d.html`
  4. `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/frontend/teddy_bear_tournament_3d.html`
- Verify that `benchmark_report.html` (the target report file) exists, or check the pipeline behavior.

## Step 2: Run `test_dashboard_container.py`
- Run command: `python3 tests/e2e/test_dashboard_container.py`
- Verify that all assertions (file existence, diagnostics drawer elements, iframe src values, CSS class names, and JavaScript helper functions) pass.

## Step 3: Run the project E2E tests
- Run command: `python3 tests/e2e/run_e2e_tests.py`
- Run command with real build: `python3 tests/e2e/run_e2e_tests.py --real-build`
- Verify that both commands finish successfully.

## Step 4: Perform Manual/Automated Path Resolution Verification
- Calculate the absolute path of `benchmark_report.html` from each HTML file's iframe src.
- Check if each calculated path resolves to the actual `benchmark_report.html` path on the filesystem.
- Verify that there are no invalid relative path resolutions (which would result in 404s in a web browser context).

## Step 5: Stress Test and Adversarial Verification
- Check what happens if files are accessed or relative path resolution is tested from different working directories.
- Construct challenges or identify scenarios under which the path resolution could fail (e.g. symlinks, directory renames, missing report).
