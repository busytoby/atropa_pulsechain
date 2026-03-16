import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor

def interrogate_dreamboats():
    print("=== TSFi VLM Interrogation (Moondream2) ===")
    
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
        
    print(f"\\n[ACTIVE] Initiating Phase 0 Structural Interrogation...\\n")    
    queries = [
        "Analyze the teddy bear's snout or muzzle. Provide its exact spatial bounding box in the format [Y_min, X_min, Y_max, X_max].",
        "Analyze the lighting in this image. Where is the primary light source located? Answer precisely.",
        "Analyze the green sickness or mutations on the bear. Provide the exact spatial bounding box where the sickness is most concentrated."
    ]
    
    for i, q in enumerate(queries):
        print(f"QUERY {i+1}: {q}")
        ans = model.answer_question(enc_image, q, processor)
        print(f"RESPONSE: {ans}\\n")

if __name__ == "__main__":
    interrogate_dreamboats()
