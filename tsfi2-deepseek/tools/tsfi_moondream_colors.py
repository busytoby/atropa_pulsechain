import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor

def extract_color_matrix():
    print("=== TSFi VLM Interrogation: Phase 1 (Color Matrix) ===")
    
    try:
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
    except Exception as e:
        print(f"[FRACTURE] VLM failed to load: {e}")
        return
        
    try:
        img = Image.open('assets/sd_target_concept.png').convert('RGB')
        enc_image = model.encode_image(img)
    except Exception as e:
        print(f"[FRACTURE] Target image missing: {e}")
        return
        
    print("\\n[ACTIVE] Interrogating Aesthetic Color Boundaries...\\n")
    
    queries = [
        "Analyze the bear's fur. Is the brown color warm and saturated, or cool and desaturated? Provide a descriptive color name.",
        "Analyze the green sickness mutation. Is it a bright neon green, or a dark, rotting, earthy green?",
        "Analyze the environmental lighting. Does the light source cast a cool blue tone, or a warm yellow/orange tone?",
        "Analyze the bear's eye. Is it a solid color, or does it contain a glowing gradient? Describe it exactly."
    ]
    
    for i, q in enumerate(queries):
        print(f"QUERY {i+1}: {q}")
        ans = model.answer_question(enc_image, q, processor)
        print(f"RESPONSE: {ans}\\n")

if __name__ == "__main__":
    extract_color_matrix()
