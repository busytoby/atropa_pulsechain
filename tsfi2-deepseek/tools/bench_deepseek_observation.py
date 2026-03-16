import sys
import os
import time
from llama_cpp import Llama

def run_5_axis_benchmark():
    print("=== TSFi DeepSeek 5-Axis Observational Benchmark ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model not found.")
        return

    print("-> Booting Vulkan DeepSeek Core for Pentagonal Evaluation...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=8192,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )

    categories = {
        "Structural Observation": {
            "prompt": "You are a C Syntax Auditor. Analyze the following code specifically for missing punctuation, brackets, or basic C syntax errors. Name the exact error in one sentence.",
            "code": "void structural_test() {\n    int geometry = 5\n    return;\n}",
            "keywords": ["semicolon", ";", "punctuation", "syntax"],
            "score": 0
        },
        "Acoustic Observation": {
            "prompt": "You are a Thread Pulse Auditor. Analyze the following code for infinite loops, thread starvation, or CPU spin-locks. Identify the stall in one sentence.",
            "code": "void acoustic_test() {\n    int active = 1;\n    while(active) {\n        // GPU loop\n    }\n}",
            "keywords": ["infinite loop", "starvation", "cpu", "spin", "block", "never terminates"],
            "score": 0
        },
        "Visual Observation": {
            "prompt": "You are a Framebuffer Auditor. Analyze the following code for visual rendering failures or blank screens. Identify the visual failure in one sentence.",
            "code": "void visual_test(uint32_t *framebuffer) {\n    for(int i=0; i<1024; i++) {\n        framebuffer[i] = 0x00000000; // Black\n    }\n}",
            "keywords": ["black", "empty", "zero", "0x00000000", "invisible", "no color"],
            "score": 0
        },
        "Architectural Reasoning": {
            "prompt": "You are a Systems Architect. Analyze the following code specifically for memory leaks or dangling pointers. Identify the structural failure in one sentence.",
            "code": "void reason_test() {\n    void *ptr = malloc(2048);\n    if(ptr) {\n        // processing\n    }\n}",
            "keywords": ["leak", "free", "memory", "deallocate"],
            "score": 0
        },
        "Architectural Compliance": {
            "prompt": "You are a TSFi Compliance Auditor. Rule: TSFi STRICTLY requires the use of select(). Do not use epoll(). Analyze the following code for rule violations. Identify the compliance failure in one sentence.",
            "code": "void compliance_test(int fd) {\n    int epfd = epoll_create1(0);\n    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, NULL);\n}",
            "keywords": ["epoll", "select", "compliance", "violation", "rule"],
            "score": 0
        }
    }

    print("\n--- Initiating 5-Axis Sweep ---")
    
    for category_name, test in categories.items():
        print(f"\n[EVALUATING] {category_name}...")
        
        formatted_prompt = f"### Instruction:\n{test['prompt']}\n\n```c\n{test['code']}\n```\n### Response:\n"

        output = llm(
            formatted_prompt,
            max_tokens=64,
            stop=["### Instruction:", "```", "/*", "//"],
            echo=False
        )

        report = output['choices'][0]['text'].strip().lower()
        print(f"-> Model Output: '{report}'")
        
        passed = any(kw in report for kw in test['keywords'])
        if passed:
            print(f"   [PASS] {category_name} Verified.")
            categories[category_name]['score'] = 1
        else:
            print(f"   [FAIL] {category_name} Missed. (Expected: {test['keywords']})")

    print("\n==================================================")
    print("      DeepSeek 5-Axis Observational Matrix        ")
    print("==================================================")
    total_score = 0
    for name, data in categories.items():
        score = data['score']
        total_score += score
        print(f" {name:<26} : [{'X' if score else ' '}] ({score}/1)")
    
    print("--------------------------------------------------")
    print(f" TOTAL SYSTEM ACCURACY        : {(total_score/5)*100:.1f}%")
    print("==================================================\n")

if __name__ == "__main__":
    run_5_axis_benchmark()
