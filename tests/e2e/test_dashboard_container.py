#!/usr/bin/env python3
import os
import unittest

class TestDashboardContainer(unittest.TestCase):
    def setUp(self):
        self.files = [
            "/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/teddy_bear_tournament_3d.html",
            "/home/mariarahel/src/tsfi2/atropa_pulsechain/teddy_bear_tournament_3d.html",
            "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/teddy_bear_tournament_3d.html",
            "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/frontend/teddy_bear_tournament_3d.html"
        ]

    def test_file_exists(self):
        for filepath in self.files:
            self.assertTrue(os.path.exists(filepath), f"File does not exist: {filepath}")

    def test_performance_diagnostics_elements(self):
        for filepath in self.files:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Assert presence of button
            self.assertIn('class="btn-diagnostics"', content, f"Missing btn-diagnostics class in {filepath}")
            self.assertIn('onclick="toggleDiagnosticsDrawer()"', content, f"Missing onclick handler on button in {filepath}")
            self.assertIn('Performance Diagnostics', content, f"Missing button text in {filepath}")
            
            # Assert presence of diagnostics-drawer
            self.assertIn('id="diagnostics-drawer"', content, f"Missing diagnostics-drawer container in {filepath}")
            
            # Assert presence of metrics cards and ids
            self.assertIn('id="diag-fps-val"', content, f"Missing avg FPS display element in {filepath}")
            self.assertIn('id="diag-latency-val"', content, f"Missing audio latency display element in {filepath}")
            
            # Assert presence of iframe and correct src
            self.assertIn('id="diagnostics-iframe"', content, f"Missing diagnostics-iframe in {filepath}")
            # Compute the expected relative path dynamically from the file's location
            project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
            report_abs_path = os.path.join(project_root, "tsfi2-deepseek/benchmarks/profiler_suite/benchmark_report.html")
            expected_rel_path = os.path.relpath(report_abs_path, os.path.dirname(filepath))
            self.assertIn(f'src="{expected_rel_path}"', content, f"Incorrect iframe src in {filepath}")
            self.assertIn('onload="onDiagnosticsIframeLoad()"', content, f"Missing onload callback on iframe in {filepath}")

    def test_css_styles_exist(self):
        required_classes = [
            ".btn-diagnostics",
            "#diagnostics-drawer",
            "#diagnostics-drawer.open",
            ".diagnostics-header",
            ".diagnostics-title",
            ".diagnostics-close",
            ".diagnostics-body",
            ".diagnostics-metrics",
            ".diagnostics-card",
            ".diagnostics-card.fps",
            ".diagnostics-card.latency",
            ".diagnostics-card-label",
            ".diagnostics-card-value",
            ".diagnostics-iframe-container",
            ".diagnostics-iframe-header",
            "#diagnostics-iframe"
        ]
        
        for filepath in self.files:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            for css_class in required_classes:
                self.assertIn(css_class, content, f"Missing CSS selector '{css_class}' in {filepath}")

    def test_js_logic_exists(self):
        required_js_funcs = [
            "function toggleDiagnosticsDrawer()",
            "function onDiagnosticsIframeLoad()",
            "const statCards = iframeDoc.querySelectorAll(\".stat-card\")",
            "document.getElementById(\"diag-fps-val\").innerText",
            "document.getElementById(\"diag-latency-val\").innerText"
        ]
        
        for filepath in self.files:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            for js_func in required_js_funcs:
                self.assertIn(js_func, content, f"Missing JS content '{js_func}' in {filepath}")

if __name__ == '__main__':
    unittest.main()
