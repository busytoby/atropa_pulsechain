import os
import glob
import time
import sys

def main():
    if not os.path.exists("/tmp/tsfi_llm_in"):
        print("[FRACTURE] Resident LLM daemon is not active. Boot 'bin/tsfi_cockpit' first.")
        sys.exit(1)

    c_files = glob.glob("src/*.c")
    if not c_files:
        print("[FRACTURE] No C files discovered in src/")
        sys.exit(1)

    # Clean the atomic file drop to prevent stale reads
    if os.path.exists("/tmp/tsfi_llm_out.txt"):
        os.remove("/tmp/tsfi_llm_out.txt")

    # Start with a limited geometry subset to verify the architectural pipeline
    target_files = sorted(c_files, key=os.path.getsize)[:3]
    
    print(f"=== Initiating Physical Map-Reduce Audit on {len(target_files)} Code Geometries ===")
    
    with open("COMPUTE_FABRIC_REPORT.md", "w") as report:
        report.write("# TSFi2 Unified Architectural Audit\\n\\n")
        report.write("Generated natively via resident DeepSeek-Coder-V2 hardware boundary.\\n\\n")
        
        for file_path in target_files:
            file_size = os.path.getsize(file_path)
            print(f"  -> Auditing: {file_path} ({file_size} bytes)...")
            
            # Formulate the explicit physical prompt
            prompt = f"Analyze codebase. FILE: {file_path}"
            
            # Dispatch to resident background tensor loop
            with open("/tmp/tsfi_llm_in", "w") as f_in:
                f_in.write(prompt)
                
            # Poll for the atomic file drop indicating completion
            timeout = 180
            elapsed = 0
            while elapsed < timeout:
                if os.path.exists("/tmp/tsfi_llm_out.txt"):
                    with open("/tmp/tsfi_llm_out.txt", "r") as f_out:
                        response = f_out.read().strip()
                        
                    if response:
                        report.write(f"## Target: `{file_path}`\\n")
                        report.write(f"**Size:** {file_size} bytes\\n\\n")
                        report.write(f"{response}\\n\\n---\\n\\n")
                        print(f"     [+] Audit synchronized. {len(response)} bytes returned.")
                        
                        # Wipe atomic drop to prevent logic collision on next cycle
                        os.remove("/tmp/tsfi_llm_out.txt")
                        break
                time.sleep(1)
                elapsed += 1
                
            if elapsed >= timeout:
                print(f"     [FRACTURE] Matrix evaluation for {file_path} timed out.")
                report.write(f"## Target: `{file_path}`\\n\\n[FRACTURE] Evaluation Timed Out.\\n\\n---\\n\\n")
                
            time.sleep(1) # Breathe before next file

    print("\\n=== Architectural Audit Complete. Artifact sealed in COMPUTE_FABRIC_REPORT.md ===")

if __name__ == "__main__":
    main()