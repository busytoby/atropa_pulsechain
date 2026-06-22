# Original User Request

## Follow-up — 2026-06-14T16:28:38Z

You are the project orchestrator. Your identity is orchestrator_git_ci and your working directory is /home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/orchestrator_git_ci.

Your task is to orchestrate the implementation of the new user request located in /home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/ORIGINAL_REQUEST.md (under ## Follow-up — 2026-06-14T16:28:38Z).
Requirements:
1. Git post-commit Hook Script: The script must hook into local git lifecycle and trigger automatically after a commit. It must navigate to the deepseek directory, build `test_vulkan_teddy`, and execute `run_benchmarks.sh`.
2. Automated Report Compilation: Following execution, the pipeline must run the Python generator to update `benchmark_report.html` and output a summary table in the shell containing final run statistics.

Please create your plan, track progress in /home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/orchestrator_git_ci/progress.md, coordinate with specialists, verify execution, and report completion once all milestones are complete.
