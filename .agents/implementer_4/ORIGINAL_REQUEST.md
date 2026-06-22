## 2026-06-14T13:41:21Z

You are a worker agent (teamwork_preview_worker) with role "Visual Report Developer". Your working directory is `/home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/implementer_4/`.

Your task is to implement the Web-Based HTML Visualizer:
1. Write a Python script `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/generate_report.py`.
2. This script must:
   - Read the benchmark metrics from `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_results.json`.
   - If `benchmark_results.json` is missing or invalid, print a warning and fall back to a built-in dictionary of representative mock/default benchmark metrics (with 500 frame rendering times, loop times, and audio latencies for kick, snare, tom, hats, etc.) so that a valid HTML report is still generated.
   - Generate a beautiful, interactive, and completely standalone HTML file at `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html`.
3. The generated HTML report must:
   - Be fully standalone, self-contained, containing all CSS and Javascript inline.
   - DO NOT reference external assets, libraries, or CDNs (such as Chart.js, D3, Google Fonts, or external stylesheets) because the system operates in network-restricted CODE_ONLY mode, and external requests will fail or throw missing-asset/console errors.
   - Present the metrics using clear, interactive visual charts drawn using pure SVG elements or HTML5 Canvas.
   - Include:
     - A chart of frame times / FPS over the 500 frames.
     - A chart of audio latency distributions (software queue vs hardware ALSA latency) for each sound type (kick, snare, hats, clap, tom, ride, snap, etc.).
     - A comparison chart comparing average rendering time vs average audio latency.
     - Summary metrics (total frames, elapsed time, average FPS, average latency).
     - Interactive features (e.g., hover tooltips showing precise coordinate values, toggles to show/hide lines or tracks, zoom/pan if possible).
   - Load cleanly in any browser with ZERO console errors, warning flags, or missing asset errors.
4. Save the Python script and verify it works by executing it (or by checking its structure). Note: if terminal execution is not possible due to environment permissions, make sure the python script is syntactically correct and fully implemented.

MANDATORY INTEGRITY WARNING:
DO NOT CHEAT. All implementations must be genuine. DO NOT hardcode test results, create dummy/facade implementations, or circumvent the intended task. A Forensic Auditor will independently verify your work. Integrity violations WILL be detected and your work WILL be rejected.

Document your implementation and verification in `/home/mariarahel/src/tsfi2/atropa_pulsechain/.agents/implementer_4/handoff.md` and send a message back to the parent conversation (id: 167f8d7e-e59e-4387-a60c-281b82e8b54c) once completed.
