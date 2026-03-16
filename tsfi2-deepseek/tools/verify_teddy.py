import subprocess
import time

print("Starting Editor...")
p = subprocess.Popen(['./bin/tsfi_teddy_editor'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, start_new_session=True)

time.sleep(1)

p.poll()
if p.returncode is not None:
    print(f"Editor crashed or exited with code {p.returncode}")
    out, err = p.communicate()
    print("STDOUT:", out.decode('utf-8'))
    print("STDERR:", err.decode('utf-8'))
else:
    print("Editor running successfully in background!")
    p.kill()

