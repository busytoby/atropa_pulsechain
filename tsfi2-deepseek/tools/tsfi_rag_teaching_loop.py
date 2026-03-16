import os
import json
import time
import subprocess
import torch
import re
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor
from llama_cpp import Llama

# The absolute foundation of the Sovereign Anatomy
BASE_GENOME = {
    "torso_length": 120,
    "arm_length": 80,
    "leg_length": 90,
    "head_y_offset": 40,
    "shoulder_width": 40,
    "hip_width": 30
}

def init_rag_system():
    print("=== TSFi Multi-Agent RAG Teaching Laboratory ===", flush=True)
    
    if not os.path.exists("assets/dna_genome.json"):
        with open("assets/dna_genome.json", "w") as f:
            json.dump(BASE_GENOME, f, indent=4)
        print("[INFO] Initialized Genesis Genome JSON.")
        
    if not os.path.exists("audit_lore.pos"):
        with open("audit_lore.pos", "w") as f:
            f.write("[SYSTEM] RAG Laboratory Booted. Awaiting inter-agent communication.\\n")

def run_teaching_loop():
    print("[ACTIVE] Booting Moondream (The Teacher) and DeepSeek (The Architect)...", flush=True)
    
    torch.set_num_threads(4)
    vlm = AutoModelForCausalLM.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256).to("cpu")
    processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
    
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model missing.")
        return
    llm = Llama(model_path=gguf_path, n_ctx=2048, n_threads=8, n_gpu_layers=-1, verbose=False)
    
    cycle = 1
    while True:
        try:
            print(f"\\n--- TEACHING CYCLE {cycle} ---")
            
            # 1. The Visual Audit (Moondream)
            if not os.path.exists("assets/mpx_out.png"):
                print("[WARN] No broadcast image found. Waiting for VTUBE daemon...")
                time.sleep(5)
                continue
                
            img = Image.open("assets/mpx_out.png").convert("RGB")
            enc_image = vlm.encode_image(img)
            
            q = "Critique the biological proportions of this teddy bear. Are the arms too short? Is the body too tall or wide? Identify exactly ONE geometric flaw. Keep it under 2 sentences."
            vlm_critique = vlm.answer_question(enc_image, q, processor).strip()
            print(f"[TEACHER] Moondream Observation: {vlm_critique}")
            
            with open("audit_lore.pos", "a") as f:
                f.write(f"\\n[CYCLE {cycle}] TEACHER (VLM): {vlm_critique}")
            
            # 2. The Architectural Deduction (DeepSeek)
            with open("assets/dna_genome.json", "r") as f:
                current_genome = json.load(f)
                
            prompt = f"""You are the DeepSeek C-Kernel Architect. 
The Vision Model audited your generated Teddy Bear and provided this critique:
"{vlm_critique}"

Here is your current internal geometric DNA matrix:
{json.dumps(current_genome, indent=2)}

TASK: Modify exactly one numerical value in the JSON to physically resolve the Vision Model's critique. 
For example, if it says the arms are short, increase "arm_length". If it says it's too skinny, increase "shoulder_width" or "hip_width".
Output ONLY the raw, updated JSON. Do not use code blocks. Do not add conversational text."""

            formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"
            
            output = llm(formatted_prompt, max_tokens=300, temperature=0.2, stop=["### Instruction:"], echo=False)
            ds_response = output['choices'][0]['text'].strip()
            
            # Clean up potential markdown formatting hallucinated by DeepSeek
            ds_response = ds_response.replace("```json", "").replace("```", "").strip()
            
            try:
                new_genome = json.loads(ds_response)
                
                # Structural Safety Boundary Check
                if "torso_length" in new_genome and "arm_length" in new_genome:
                    with open("assets/dna_genome.json", "w") as f:
                        json.dump(new_genome, f, indent=4)
                    print(f"[ARCHITECT] DeepSeek Deduction: Math successfully patched.")
                    
                    with open("audit_lore.pos", "a") as f:
                        f.write(f"\\n[CYCLE {cycle}] ARCHITECT (DS): Applied JSON fix successfully.")
                else:
                    print(f"[ARCHITECT] DeepSeek hallucinated invalid keys. Rejecting patch.")
                    
            except json.JSONDecodeError:
                print(f"[ARCHITECT] DeepSeek failed to output valid JSON. Matrix rejected.")
                print(f"Raw Output: {ds_response}")
                
            cycle += 1
            # Wait 60 seconds before auditing again so the VTUBE stream has time to read the new JSON and generate new frames
            time.sleep(60) 
            
        except Exception as e:
            print(f"[FRACTURE] Teaching loop crashed: {e}")
            time.sleep(5)

if __name__ == "__main__":
    init_rag_system()
    run_teaching_loop()
