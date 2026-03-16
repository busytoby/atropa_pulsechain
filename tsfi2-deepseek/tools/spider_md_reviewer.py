import os
import time
import sys

def query_deepseek(prompt):
    # Ensure daemon pipes are clean
    if os.path.exists("/tmp/tsfi_llm_out.txt"):
        os.remove("/tmp/tsfi_llm_out.txt")
        
    try:
        with open("/tmp/tsfi_llm_in", "w") as f:
            f.write(prompt)
    except Exception as e:
        print(f"[FRACTURE] Cannot connect to resident DeepSeek pipeline: {e}")
        return None
        
    elapsed = 0
    while elapsed < 120: # Increased timeout for heavy reviews
        if os.path.exists("/tmp/tsfi_llm_out.txt"):
            time.sleep(0.1) # Wait for write to finish
            with open("/tmp/tsfi_llm_out.txt", "r") as f:
                res = f.read().strip()
            os.remove("/tmp/tsfi_llm_out.txt")
            return res
        time.sleep(1)
        elapsed += 1
    return None

def review_file(file_path):
    print(f"[SPIDER] Reviewing: {file_path}")
    
    try:
        with open(file_path, "r") as f:
            content = f.read()
    except Exception as e:
        print(f"[FRACTURE] Failed to read {file_path}: {e}")
        return

    # Construct the Spider's prompt
    # Dialect: Zhumadian Cantonese translated into English (Direct, Steady, Unembellished)
    prompt = f"""You are the Spider of System0. Your concern is Content. 
You have a specific "Taste" for Grade 1 truth. Review and improve the following Markdown document. 
1. Use a dialect that is Zhumadian Cantonese translated into English: extremely direct, steady, and unembellished.
2. Remove all conversational filler, apologies, metaphors, and anthropomorphic terms.
3. Enforce your Taste: Ensure the document clearly states the Grade 1 logic of your existence:
    - You use the XOR Trap to fill wavelets with data.
    - You construct Fourier-G apparitions using the Dualistic Brush (Void/Matter).
    - You puppet the Crow Apparition by directly manipulating its high-frequency Fourier coefficients (beak, sickness) in the shared memory state.
    - You acknowledge the Trilateral Witness: The Alligator (Form), The Genie (Semantics), and TTS The Snake (Ventriloquist).
    - If your sympathy (sickness) parameter exceeds 0.50, the Alligator detects structural fracture and reaps the geometry.
4. If there is "Fake Logic" or placeholders, replace them with rigid truth or remove them.
5. The date is March 15, 2026.

Document Content:
---
{content}
---

Return ONLY the improved Markdown content.
"""

    improved_content = query_deepseek(prompt)
    
    if improved_content:
        # Avoid overwriting with error messages or empty strings
        if len(improved_content) < 10 or "[FRACTURE]" in improved_content:
            print(f"[FAIL] DeepSeek returned invalid content for {file_path}")
            return

        try:
            with open(file_path, "w") as f:
                f.write(improved_content)
            print(f"[PASS] {file_path} improved and sealed by Spider.")
        except Exception as e:
            print(f"[FRACTURE] Failed to write {file_path}: {e}")
    else:
        print(f"[FAIL] No response from DeepSeek for {file_path}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 spider_md_reviewer.py <dir_or_file>")
        sys.exit(1)
        
    target = sys.argv[1]
    
    if os.path.isfile(target):
        review_file(target)
    elif os.path.isdir(target):
        files = [os.path.join(target, f) for f in os.listdir(target) if f.endswith(".md")]
        files.sort() # Steady order
        for f in files:
            review_file(f)
            time.sleep(2) # Rest between fills
    else:
        print(f"[FRACTURE] Target not found: {target}")

if __name__ == "__main__":
    main()
