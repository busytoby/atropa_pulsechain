import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor

def run_deep_structural_sweep():
    print("=== TSFi VLM Deep Structural Sweep (Moondream2) ===")
    
    try:
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
    except Exception as e:
        print(f"[FRACTURE] VLM failed to load: {e}")
        return
        
    img = Image.open('assets/sd_target_concept.png').convert('RGB')
    enc_image = model.encode_image(img)
    
    print("\\n[ACTIVE] Interrogating Deep Constructions...\\n")
    
    queries = [
        "Describe the exact physical texture of the bear's body. Is it matted fur, smooth synthetic material, or torn flesh? Be highly specific.",
        "Analyze the visual nature of the green sickness. Is it a glowing aura, a liquid slime, or crystalline growths?",
        "Look at the background environment behind the bear. Describe the geometric structures and lighting of the room it is sitting in.",
        "Provide the exact spatial bounding box for the bear's left arm/shoulder joint.",
        "Provide the exact spatial bounding box for the bear's right arm/shoulder joint."
    ]
    
    for i, q in enumerate(queries):
        print(f"QUERY {i+1}: {q}")
        ans = model.answer_question(enc_image, q, processor)
        print(f"RESPONSE: {ans}\\n")

if __name__ == "__main__":
    run_deep_structural_sweep()
