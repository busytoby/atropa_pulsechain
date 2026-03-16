import json
import os
import random

def format_dataset():
    print("=== Kr0wZ LoRA Dataset Formatter Active ===")
    input_file = "assets/tsfi_deepseek_dataset.json"
    output_file = "assets/tsfi_alpaca_train.json"

    if not os.path.exists(input_file):
        print(f"[FRACTURE] Raw dataset not found at {input_file}")
        return

    with open(input_file, "r") as f:
        try:
            raw_traps = json.load(f)
        except json.JSONDecodeError:
            print("[FRACTURE] Failed to decode raw dataset.")
            return

    alpaca_dataset = []

    for trap in raw_traps:
        code_block = trap.get("code", "")
        keywords = trap.get("expected_keywords", [])
        name = trap.get("name", "Unknown Fracture")

        if not code_block or not keywords:
            continue

        # Mechanically convert the raw trap into an explicit Alpaca instruction format
        system_prompt = "You are a strict TSFi C Systems Auditor. CRITICAL RULE: The TSFi architecture STRICTLY uses select(). Do not ever suggest poll() or epoll_wait(). Analyze the following C code snippet and identify the primary architectural flaw. Respond in exactly one short sentence."
        
        # Generate the synthetic "perfect" response from the expected keywords
        ideal_response = f"The primary architectural flaw is a {name.lower().replace('fracture', '')} involving the mechanism: {', '.join(keywords)}."

        alpaca_node = {
            "instruction": system_prompt,
            "input": code_block,
            "output": ideal_response
        }
        alpaca_dataset.append(alpaca_node)

    with open(output_file, "w") as f:
        json.dump(alpaca_dataset, f, indent=4)

    print(f"[PASS] Mechanically translated {len(alpaca_dataset)} raw traps into Alpaca instructional format.")
    print(f"[ANCHOR] Fine-tuning payload solidified at {output_file}")

if __name__ == "__main__":
    format_dataset()
