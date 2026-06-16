#!/usr/bin/env python3
import os
import sys
import shutil
import tempfile
import subprocess
import unittest
import argparse

# Configuration
class TestConfig:
    real_build = False
    keep_sandbox = False
    only_tier = None

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
REAL_TSFI2_DEEPSEEK = os.path.join(PROJECT_ROOT, "tsfi2-deepseek")
MOCK_ASSETS_DIR = os.path.join(PROJECT_ROOT, "tests", "e2e", "test_cases", "mock_assets")
NORMAL_JSON_PATH = os.path.join(MOCK_ASSETS_DIR, "mock_results_normal.json")
CORRUPT_JSON_PATH = os.path.join(MOCK_ASSETS_DIR, "mock_results_corrupt.json")

def symlink_recursive(src_dir, dst_dir, ignore_dirs=None):
    if ignore_dirs is None:
        ignore_dirs = []
    os.makedirs(dst_dir, exist_ok=True)
    for name in os.listdir(src_dir):
        if name in ignore_dirs:
            continue
        src_path = os.path.join(src_dir, name)
        dst_path = os.path.join(dst_dir, name)
        if os.path.isdir(src_path):
            symlink_recursive(src_path, dst_path, ignore_dirs)
        else:
            if os.path.exists(dst_path) or os.path.islink(dst_path):
                try:
                    os.remove(dst_path)
                except Exception:
                    pass
            os.symlink(src_path, dst_path)

def load_mock_normal_json():
    with open(NORMAL_JSON_PATH, "r") as f:
        return f.read()

def load_mock_corrupt_json():
    with open(CORRUPT_JSON_PATH, "r") as f:
        return f.read()

class GitSandbox:
    def __init__(self, real_build=False, keep_sandbox=False):
        self.real_build = real_build
        self.keep_sandbox = keep_sandbox
        self.temp_dir_obj = tempfile.TemporaryDirectory(prefix="git_sandbox_")
        self.repo_dir = os.path.abspath(self.temp_dir_obj.name)
        self.tsfi2_deepseek_dir = os.path.join(self.repo_dir, "tsfi2-deepseek")
        
    def __enter__(self):
        self.init_repo()
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        if not self.keep_sandbox:
            try:
                self.temp_dir_obj.cleanup()
            except Exception:
                pass
        else:
            print(f"[INFO] Keeping sandbox repository directory at: {self.repo_dir}")

    def run_git(self, args, cwd=None, env=None):
        if cwd is None:
            cwd = self.repo_dir
        if env is None:
            env = self.get_clean_env()
        res = subprocess.run(["git"] + args, cwd=cwd, env=env, capture_output=True, text=True)
        return res

    def get_clean_env(self, extra=None):
        env = os.environ.copy()
        for k in list(env.keys()):
            if k.startswith("GIT_"):
                del env[k]
        if extra:
            env.update(extra)
        return env

    def init_repo(self):
        self.run_git(["init"])
        self.run_git(["config", "user.name", "Test User"])
        self.run_git(["config", "user.email", "test@example.com"])
        
        os.makedirs(os.path.join(self.repo_dir, ".git", "hooks"), exist_ok=True)
        
        if self.real_build:
            self.populate_real()
        else:
            self.populate_mock()

    def populate_real(self):
        symlink_recursive(REAL_TSFI2_DEEPSEEK, self.tsfi2_deepseek_dir, ignore_dirs=[".git", "sandbox", "obj", "bin"])
        os.makedirs(os.path.join(self.tsfi2_deepseek_dir, "bin"), exist_ok=True)
        os.makedirs(os.path.join(self.tsfi2_deepseek_dir, "obj"), exist_ok=True)

    def populate_mock(self):
        os.makedirs(os.path.join(self.tsfi2_deepseek_dir, "bin"), exist_ok=True)
        os.makedirs(os.path.join(self.tsfi2_deepseek_dir, "benchmarks", "git_ci_pipeline"), exist_ok=True)
        os.makedirs(os.path.join(self.tsfi2_deepseek_dir, "benchmarks", "profiler_suite"), exist_ok=True)
        
        real_hook_src = os.path.join(REAL_TSFI2_DEEPSEEK, "benchmarks", "git_ci_pipeline", "post-commit")
        sandbox_hook_src = os.path.join(self.tsfi2_deepseek_dir, "benchmarks", "git_ci_pipeline", "post-commit")
        shutil.copy(real_hook_src, sandbox_hook_src)
        os.chmod(sandbox_hook_src, 0o755)
        
        self.write_mock_makefile()
        self.write_mock_run_benchmarks()
        self.write_mock_generate_report()

    def install_hook(self):
        hook_src = os.path.join(self.tsfi2_deepseek_dir, "benchmarks", "git_ci_pipeline", "post-commit")
        hook_dest = os.path.join(self.repo_dir, ".git", "hooks", "post-commit")
        if os.path.exists(hook_dest) or os.path.islink(hook_dest):
            os.remove(hook_dest)
        shutil.copy(hook_src, hook_dest)
        os.chmod(hook_dest, 0o755)

    def write_mock_makefile(self, fail=False):
        makefile_path = os.path.join(self.tsfi2_deepseek_dir, "Makefile")
        if os.path.exists(makefile_path) or os.path.islink(makefile_path):
            os.remove(makefile_path)
        if fail:
            content = """
bin/test_vulkan_teddy:
	echo "Mock compilation error!"
	exit 1
"""
        else:
            content = """
bin/test_vulkan_teddy:
	mkdir -p bin
	touch bin/test_vulkan_teddy
	chmod +x bin/test_vulkan_teddy
"""
        with open(makefile_path, "w") as f:
            f.write(content)

    def write_mock_run_benchmarks(self, fail=False, custom_json=None):
        script_path = os.path.join(self.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "run_benchmarks.sh")
        if os.path.exists(script_path) or os.path.islink(script_path):
            os.remove(script_path)
        if fail:
            content = """#!/bin/bash
echo "Mock benchmark run error!"
exit 1
"""
        else:
            if custom_json is not None:
                json_data = custom_json
            else:
                json_data = '{"summary": {"total_frames": 500, "elapsed_time_seconds": 50.18, "total_render_time_seconds": 50.18, "average_fps": 9.96, "average_frame_render_time_seconds": 0.1}}'
            content = f"""#!/bin/bash
echo '{json_data}' > benchmark_results.json
"""
        with open(script_path, "w") as f:
            f.write(content)
        os.chmod(script_path, 0o755)

    def write_mock_generate_report(self, fail=False):
        script_path = os.path.join(self.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "generate_report.py")
        if os.path.exists(script_path) or os.path.islink(script_path):
            os.remove(script_path)
        if fail:
            content = """#!/usr/bin/env python3
import sys
print("Mock generate_report error!")
sys.exit(1)
"""
        else:
            content = """#!/usr/bin/env python3
with open("benchmark_report.html", "w") as f:
    f.write("<html>Mock Report</html>")
"""
        with open(script_path, "w") as f:
            f.write(content)
        os.chmod(script_path, 0o755)

    def make_commit(self, file_name="dummy.txt", content="hello", cwd=None):
        if cwd is None:
            cwd = self.repo_dir
        file_path = os.path.join(cwd, file_name)
        os.makedirs(os.path.dirname(file_path), exist_ok=True)
        with open(file_path, "w") as f:
            f.write(content)
        self.run_git(["add", file_name], cwd=cwd)
        res = self.run_git(["commit", "-m", f"Commit {file_name}"], cwd=cwd)
        return res

class Tier1Tests(unittest.TestCase):
    # Git Hook Installation & Trigger
    def test_01_hook_installation_success(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            hook_path = os.path.join(sandbox.repo_dir, ".git", "hooks", "post-commit")
            self.assertTrue(os.path.exists(hook_path))
            self.assertTrue(os.access(hook_path, os.X_OK))

    def test_02_hook_trigger_on_commit(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("[INFO]", res.stdout)

    def test_03_hook_trigger_multiple_commits(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res1 = sandbox.make_commit("file1.txt", "content1")
            self.assertEqual(res1.returncode, 0)
            self.assertIn("[INFO]", res1.stdout)
            res2 = sandbox.make_commit("file2.txt", "content2")
            self.assertEqual(res2.returncode, 0)
            self.assertIn("[INFO]", res2.stdout)

    def test_04_hook_no_trigger_on_non_commit(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.run_git(["status"])
            self.assertNotIn("[INFO] Compiling Vulkan benchmark...", res.stdout)
            with open(os.path.join(sandbox.repo_dir, "test.txt"), "w") as f:
                f.write("hello")
            res_add = sandbox.run_git(["add", "test.txt"])
            self.assertNotIn("[INFO] Compiling Vulkan benchmark...", res_add.stdout)

    def test_05_hook_manual_execution(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            hook_path = os.path.join(sandbox.repo_dir, ".git", "hooks", "post-commit")
            res = subprocess.run([hook_path], cwd=sandbox.repo_dir, env=sandbox.get_clean_env(), capture_output=True, text=True)
            self.assertEqual(res.returncode, 0)
            self.assertIn("[INFO]", res.stdout)

    # Benchmark Execution
    def test_06_benchmark_execution_success(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertIn("Compiling Vulkan benchmark...", res.stdout)

    def test_07_benchmark_results_json_created(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            sandbox.make_commit()
            results_json = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
            self.assertTrue(os.path.exists(results_json))

    def test_08_run_benchmarks_script_called(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertIn("Running benchmarks...", res.stdout)

    def test_09_benchmark_non_zero_exit_compilation_success(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)

    def test_10_benchmark_binary_built(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            sandbox.make_commit()
            binary_path = os.path.join(sandbox.tsfi2_deepseek_dir, "bin", "test_vulkan_teddy")
            self.assertTrue(os.path.exists(binary_path))

    # HTML Report Updates
    def test_11_html_report_generation(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            sandbox.make_commit()
            report_html = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_report.html")
            self.assertTrue(os.path.exists(report_html))

    def test_12_html_report_contains_valid_content(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            sandbox.make_commit()
            report_html = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_report.html")
            with open(report_html, "r") as f:
                content = f.read()
            self.assertGreater(len(content), 0)

    def test_13_html_report_overwrite(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            sandbox.make_commit()
            report_html = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_report.html")
            mtime1 = os.path.getmtime(report_html)
            import time
            time.sleep(0.1)
            sandbox.make_commit("another.txt", "content")
            mtime2 = os.path.getmtime(report_html)
            self.assertNotEqual(mtime1, mtime2)

    def test_14_generate_report_script_called(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertIn("Generating HTML report...", res.stdout)

    def test_15_html_report_fallback_mode_flag(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_run_benchmarks(fail=False, custom_json="")
                real_gen_rep = os.path.join(REAL_TSFI2_DEEPSEEK, "benchmarks", "profiler_suite", "generate_report.py")
                sandbox_gen_rep = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "generate_report.py")
                shutil.copy(real_gen_rep, sandbox_gen_rep)
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path):
                    os.remove(json_path)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            report_html = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_report.html")
            self.assertTrue(os.path.exists(report_html))
            with open(report_html, "r") as f:
                html_content = f.read()
            self.assertIn("Fallback Mode:", html_content)

    # Summary Table Output
    def test_16_summary_table_ascii_formatting(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertIn("+------------------------------------+----------------+", res.stdout)

    def test_17_summary_table_contains_fps(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertIn("Average FPS", res.stdout)

    def test_18_summary_table_contains_total_frames(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertIn("Total Frames", res.stdout)

    def test_19_summary_table_contains_render_time(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            res = sandbox.make_commit()
            self.assertIn("Average Frame Render Time", res.stdout)

    def test_20_summary_table_normal_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                normal_json = load_mock_normal_json()
                sandbox.write_mock_run_benchmarks(custom_json=normal_json)
            res = sandbox.make_commit()
            self.assertIn("9.96", res.stdout)
            self.assertIn("100.373 ms", res.stdout)

class Tier2Tests(unittest.TestCase):
    # Feature A Boundary/Failure Modes
    def test_21_hook_missing_source(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            hook_src = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "git_ci_pipeline", "post-commit")
            if os.path.exists(hook_src) or os.path.islink(hook_src):
                os.remove(hook_src)
            with self.assertRaises(FileNotFoundError):
                sandbox.install_hook()

    def test_22_hook_install_overwrites_existing(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            hook_dest = os.path.join(sandbox.repo_dir, ".git", "hooks", "post-commit")
            os.makedirs(os.path.dirname(hook_dest), exist_ok=True)
            with open(hook_dest, "w") as f:
                f.write("dummy hook content")
            sandbox.install_hook()
            with open(hook_dest, "r") as f:
                content = f.read()
            self.assertNotIn("dummy hook content", content)
            self.assertIn("unset GIT_DIR", content)

    def test_23_hook_run_when_git_index_locked(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            lock_path = os.path.join(sandbox.repo_dir, ".git", "index.lock")
            with open(lock_path, "w") as f:
                f.write("locked")
            hook_path = os.path.join(sandbox.repo_dir, ".git", "hooks", "post-commit")
            res = subprocess.run([hook_path], cwd=sandbox.repo_dir, env=sandbox.get_clean_env(), capture_output=True, text=True)
            self.assertEqual(res.returncode, 0)
            if os.path.exists(lock_path):
                os.remove(lock_path)

    def test_24_hook_execute_in_non_git_repo(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            hook_src = os.path.join(REAL_TSFI2_DEEPSEEK, "benchmarks", "git_ci_pipeline", "post-commit")
            hook_dest = os.path.join(temp_dir, "post-commit")
            shutil.copy(hook_src, hook_dest)
            os.chmod(hook_dest, 0o755)
            res = subprocess.run([hook_dest], cwd=temp_dir, env=os.environ.copy(), capture_output=True, text=True)
            self.assertEqual(res.returncode, 0)
            self.assertIn("[WARNING] Could not navigate to", res.stdout)

    def test_25_hook_non_executable_source(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            hook_src = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "git_ci_pipeline", "post-commit")
            os.chmod(hook_src, 0o644)
            sandbox.install_hook()
            hook_dest = os.path.join(sandbox.repo_dir, ".git", "hooks", "post-commit")
            self.assertTrue(os.access(hook_dest, os.X_OK))

    # Feature B Boundary/Failure Modes
    def test_26_compilation_fails_warning(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=True)
            else:
                makefile_path = os.path.join(sandbox.tsfi2_deepseek_dir, "Makefile")
                if os.path.exists(makefile_path) or os.path.islink(makefile_path):
                    os.remove(makefile_path)
                with open(makefile_path, "w") as f:
                    f.write("bin/test_vulkan_teddy:\n\techo 'compilation error'\n\texit 1\n")
            
            res = sandbox.make_commit()
            self.assertIn("[WARNING] Compilation of Vulkan benchmark failed.", res.stdout)

    def test_27_compilation_fails_exit_code(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=True)
            else:
                makefile_path = os.path.join(sandbox.tsfi2_deepseek_dir, "Makefile")
                if os.path.exists(makefile_path) or os.path.islink(makefile_path):
                    os.remove(makefile_path)
                with open(makefile_path, "w") as f:
                    f.write("bin/test_vulkan_teddy:\n\techo 'compilation error'\n\texit 1\n")
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)

    def test_28_run_benchmarks_script_missing(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            script_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "run_benchmarks.sh")
            if os.path.exists(script_path) or os.path.islink(script_path):
                os.remove(script_path)
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("run_benchmarks.sh not found", res.stdout)

    def test_29_run_benchmarks_fails(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_run_benchmarks(fail=True)
            else:
                script_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "run_benchmarks.sh")
                if os.path.exists(script_path) or os.path.islink(script_path):
                    os.remove(script_path)
                with open(script_path, "w") as f:
                    f.write("#!/bin/bash\nexit 1\n")
                os.chmod(script_path, 0o755)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("Mock benchmark run error!" if not sandbox.real_build else "", res.stdout)

    def test_30_vulkan_teddy_binary_execution_fails(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                makefile_content = """
bin/test_vulkan_teddy:
	mkdir -p bin
	echo '#!/bin/bash' > bin/test_vulkan_teddy
	echo 'exit 1' >> bin/test_vulkan_teddy
	chmod +x bin/test_vulkan_teddy
"""
                makefile_path = os.path.join(sandbox.tsfi2_deepseek_dir, "Makefile")
                if os.path.exists(makefile_path) or os.path.islink(makefile_path):
                    os.remove(makefile_path)
                with open(makefile_path, "w") as f:
                    f.write(makefile_content)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)

    # Feature C Boundary/Failure Modes
    def test_31_generate_report_script_missing(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            script_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "generate_report.py")
            if os.path.exists(script_path) or os.path.islink(script_path):
                os.remove(script_path)
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("generate_report.py not found", res.stdout)

    def test_32_generate_report_fails(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_generate_report(fail=True)
            else:
                script_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "generate_report.py")
                if os.path.exists(script_path) or os.path.islink(script_path):
                    os.remove(script_path)
                with open(script_path, "w") as f:
                    f.write("#!/usr/bin/env python3\nimport sys\nsys.exit(1)\n")
                os.chmod(script_path, 0o755)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("Mock generate_report error!" if not sandbox.real_build else "", res.stdout)

    def test_33_html_report_write_permission_denied(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            report_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_report.html")
            os.makedirs(report_path, exist_ok=True)
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            if os.path.isdir(report_path):
                shutil.rmtree(report_path)

    def test_34_generate_report_corrupted_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            corrupt_json = load_mock_corrupt_json()
            if not sandbox.real_build:
                real_gen_rep = os.path.join(REAL_TSFI2_DEEPSEEK, "benchmarks", "profiler_suite", "generate_report.py")
                sandbox_gen_rep = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "generate_report.py")
                if os.path.exists(sandbox_gen_rep) or os.path.islink(sandbox_gen_rep):
                    os.remove(sandbox_gen_rep)
                shutil.copy(real_gen_rep, sandbox_gen_rep)
                os.chmod(sandbox_gen_rep, 0o755)
                sandbox.write_mock_run_benchmarks(custom_json=corrupt_json)
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
                with open(json_path, "w") as f:
                    f.write(corrupt_json)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            report_html = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_report.html")
            self.assertTrue(os.path.exists(report_html))

    def test_35_generate_report_missing_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                real_gen_rep = os.path.join(REAL_TSFI2_DEEPSEEK, "benchmarks", "profiler_suite", "generate_report.py")
                sandbox_gen_rep = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "generate_report.py")
                if os.path.exists(sandbox_gen_rep) or os.path.islink(sandbox_gen_rep):
                    os.remove(sandbox_gen_rep)
                shutil.copy(real_gen_rep, sandbox_gen_rep)
                os.chmod(sandbox_gen_rep, 0o755)
                sandbox.write_mock_run_benchmarks(custom_json="")
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            report_html = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_report.html")
            self.assertTrue(os.path.exists(report_html))

    # Feature D Boundary/Failure Modes
    def test_36_summary_table_missing_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_run_benchmarks(custom_json="")
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
            res = sandbox.make_commit()
            self.assertIn("N/A", res.stdout)

    def test_37_summary_table_corrupted_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            corrupt_json = load_mock_corrupt_json()
            if not sandbox.real_build:
                sandbox.write_mock_run_benchmarks(custom_json=corrupt_json)
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
                with open(json_path, "w") as f:
                    f.write(corrupt_json)
            res = sandbox.make_commit()
            self.assertIn("N/A", res.stdout)

    def test_38_summary_table_empty_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_run_benchmarks(custom_json="{}")
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
                with open(json_path, "w") as f:
                    f.write("{}")
            res = sandbox.make_commit()
            self.assertIn("N/A", res.stdout)

    def test_39_summary_table_partial_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            partial_json = '{"summary": {"total_frames": 999}}'
            if not sandbox.real_build:
                sandbox.write_mock_run_benchmarks(custom_json=partial_json)
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
                with open(json_path, "w") as f:
                    f.write(partial_json)
            res = sandbox.make_commit()
            self.assertIn("999", res.stdout)
            self.assertIn("N/A", res.stdout)

    def test_40_summary_table_non_numeric_values(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            bad_json = '{"summary": {"total_frames": "many", "average_fps": "fast"}}'
            if not sandbox.real_build:
                sandbox.write_mock_run_benchmarks(custom_json=bad_json)
            else:
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
                with open(json_path, "w") as f:
                    f.write(bad_json)
            res = sandbox.make_commit()
            self.assertIn("many", res.stdout)
            self.assertIn("fast", res.stdout)

class Tier3Tests(unittest.TestCase):
    # Pairwise Combination Tests
    def test_41_combination_broken_compilation_and_missing_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=True)
                sandbox.write_mock_run_benchmarks(custom_json="")
            else:
                makefile_path = os.path.join(sandbox.tsfi2_deepseek_dir, "Makefile")
                if os.path.exists(makefile_path) or os.path.islink(makefile_path):
                    os.remove(makefile_path)
                with open(makefile_path, "w") as f:
                    f.write("bin/test_vulkan_teddy:\n\techo 'compilation error'\n\texit 1\n")
                
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("[WARNING] Compilation of Vulkan benchmark failed.", res.stdout)
            self.assertNotIn("+------------------------------------+----------------+", res.stdout)

    def test_42_combination_compilation_success_but_benchmark_fails_and_corrupt_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            corrupt_json = load_mock_corrupt_json()
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=False)
                sandbox.write_mock_run_benchmarks(fail=True)
                script_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "run_benchmarks.sh")
                if os.path.exists(script_path) or os.path.islink(script_path):
                    os.remove(script_path)
                with open(script_path, "w") as f:
                    f.write(f"#!/bin/bash\necho '{corrupt_json}' > benchmark_results.json\nexit 1\n")
                os.chmod(script_path, 0o755)
            else:
                script_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "run_benchmarks.sh")
                if os.path.exists(script_path) or os.path.islink(script_path):
                    os.remove(script_path)
                with open(script_path, "w") as f:
                    f.write(f"#!/bin/bash\necho '{corrupt_json}' > benchmark_results.json\nexit 1\n")
                os.chmod(script_path, 0o755)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("N/A", res.stdout)

    def test_43_combination_compilation_success_but_report_fails_with_normal_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            normal_json = load_mock_normal_json()
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=False)
                sandbox.write_mock_run_benchmarks(custom_json=normal_json)
                sandbox.write_mock_generate_report(fail=True)
            else:
                script_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "generate_report.py")
                if os.path.exists(script_path) or os.path.islink(script_path):
                    os.remove(script_path)
                with open(script_path, "w") as f:
                    f.write("#!/usr/bin/env python3\nimport sys\nsys.exit(1)\n")
                os.chmod(script_path, 0o755)
                json_path = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite", "benchmark_results.json")
                if os.path.exists(json_path) or os.path.islink(json_path):
                    os.remove(json_path)
                with open(json_path, "w") as f:
                    f.write(normal_json)
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("9.96", res.stdout)

    def test_44_combination_broken_compilation_but_existing_valid_json(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            normal_json = load_mock_normal_json()
            
            json_dir = os.path.join(sandbox.tsfi2_deepseek_dir, "benchmarks", "profiler_suite")
            os.makedirs(json_dir, exist_ok=True)
            json_path = os.path.join(json_dir, "benchmark_results.json")
            with open(json_path, "w") as f:
                f.write(normal_json)
                
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=True)
            else:
                makefile_path = os.path.join(sandbox.tsfi2_deepseek_dir, "Makefile")
                if os.path.exists(makefile_path) or os.path.islink(makefile_path):
                    os.remove(makefile_path)
                with open(makefile_path, "w") as f:
                    f.write("bin/test_vulkan_teddy:\n\techo 'compilation error'\n\texit 1\n")
            
            res = sandbox.make_commit()
            self.assertEqual(res.returncode, 0)
            self.assertIn("[WARNING] Compilation of Vulkan benchmark failed.", res.stdout)
            self.assertNotIn("+------------------------------------+----------------+", res.stdout)

class Tier4Tests(unittest.TestCase):
    # Real-world Scenarios
    def test_45_scenario_commit_from_subdirectory(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            sub_dir = os.path.join(sandbox.repo_dir, "some", "deep", "dir")
            os.makedirs(sub_dir, exist_ok=True)
            res = sandbox.make_commit("some/deep/dir/file.txt", "hello", cwd=sub_dir)
            self.assertEqual(res.returncode, 0)
            self.assertIn("[INFO] Compiling Vulkan benchmark...", res.stdout)
            self.assertIn("+------------------------------------+----------------+", res.stdout)

    def test_46_scenario_detached_head(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            sandbox.make_commit("initial.txt", "initial")
            log_res = sandbox.run_git(["log", "-n", "1", "--format=%H"])
            commit_hash = log_res.stdout.strip()
            sandbox.run_git(["checkout", commit_hash])
            res = sandbox.make_commit("detached.txt", "detached")
            self.assertEqual(res.returncode, 0)
            self.assertIn("[INFO] Compiling Vulkan benchmark...", res.stdout)

    def test_47_scenario_broken_build_recovery(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=True)
            else:
                makefile_path = os.path.join(sandbox.tsfi2_deepseek_dir, "Makefile")
                if os.path.exists(makefile_path) or os.path.islink(makefile_path):
                    os.remove(makefile_path)
                with open(makefile_path, "w") as f:
                    f.write("bin/test_vulkan_teddy:\n\techo 'broken'\n\texit 1\n")
            
            res1 = sandbox.make_commit("break.txt", "break")
            self.assertEqual(res1.returncode, 0)
            self.assertIn("[WARNING] Compilation of Vulkan benchmark failed.", res1.stdout)
            
            if not sandbox.real_build:
                sandbox.write_mock_makefile(fail=False)
            else:
                makefile_path = os.path.join(sandbox.tsfi2_deepseek_dir, "Makefile")
                if os.path.exists(makefile_path) or os.path.islink(makefile_path):
                    os.remove(makefile_path)
                os.symlink(os.path.join(REAL_TSFI2_DEEPSEEK, "Makefile"), makefile_path)
            
            res2 = sandbox.make_commit("fix.txt", "fix")
            self.assertEqual(res2.returncode, 0)
            self.assertIn("[INFO] Compiling Vulkan benchmark...", res2.stdout)
            self.assertNotIn("[WARNING] Compilation of Vulkan benchmark failed.", res2.stdout)
            self.assertIn("+------------------------------------+----------------+", res2.stdout)

    def test_48_scenario_env_scrubbing_validation(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            dirty_env = sandbox.get_clean_env({
                "GIT_DIR": "/nonexistent/path/.git",
                "GIT_WORK_TREE": "/nonexistent/path",
                "GIT_INDEX_FILE": "/nonexistent/path/index"
            })
            hook_path = os.path.join(sandbox.repo_dir, ".git", "hooks", "post-commit")
            res = subprocess.run([hook_path], cwd=sandbox.repo_dir, env=dirty_env, capture_output=True, text=True)
            self.assertEqual(res.returncode, 0)
            self.assertIn("[INFO] Compiling Vulkan benchmark...", res.stdout)

    def test_49_scenario_large_payload_commit(self):
        with GitSandbox(real_build=TestConfig.real_build, keep_sandbox=TestConfig.keep_sandbox) as sandbox:
            sandbox.install_hook()
            for i in range(100):
                file_path = os.path.join(sandbox.repo_dir, f"file_{i}.txt")
                with open(file_path, "w") as f:
                    f.write(f"content {i}")
                sandbox.run_git(["add", f"file_{i}.txt"])
            res = sandbox.run_git(["commit", "-m", "Commit 100 files"])
            self.assertEqual(res.returncode, 0)
            self.assertIn("[INFO] Compiling Vulkan benchmark...", res.stdout)
            self.assertIn("+------------------------------------+----------------+", res.stdout)

def main():
    parser = argparse.ArgumentParser(description="E2E Test Runner for Git CI Pipeline Hook")
    parser.add_argument('--real-build', action='store_true', help='Use real build and compilation')
    parser.add_argument('--keep-sandbox', action='store_true', help='Do not delete temporary sandbox directories')
    parser.add_argument('--only-tier', type=int, choices=[1, 2, 3, 4], help='Only run tests in the specified Tier')
    
    args, unknown = parser.parse_known_args()
    
    TestConfig.real_build = args.real_build
    TestConfig.keep_sandbox = args.keep_sandbox
    TestConfig.only_tier = args.only_tier
    
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    
    tiers = {
        1: Tier1Tests,
        2: Tier2Tests,
        3: Tier3Tests,
        4: Tier4Tests
    }
    
    if TestConfig.only_tier is not None:
        suite.addTests(loader.loadTestsFromTestCase(tiers[TestConfig.only_tier]))
    else:
        for t_cls in tiers.values():
            suite.addTests(loader.loadTestsFromTestCase(t_cls))
            
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    if result.wasSuccessful():
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__':
    main()
