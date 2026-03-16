import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor

def run_kinematic_structural_sweep():
    print("=== TSFi VLM Kinematic Trajectory Sweep (Moondream2) ===")
    
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
    
    print("\\n[ACTIVE] Interrogating Spatial Kinematics...\\n")
    
    queries = [
        "Analyze the vertical empty space above the sitting bear's head. If the bear were to abruptly stand up on its hind legs, approximately what percentage of the total vertical frame would its full standing body occupy?",
        "Analyze the empty space to the left of the bear. To completely walk off the left side of the stage and disappear from the image, what is the exact physical trajectory it must travel? Be specific about the visual space.",
        "When the bear stands up, what is the geometric relationship between its head and the light source in the top left corner? Would standing move it closer to the light or further into the shadows?"
    ]
    
    for i, q in enumerate(queries):
        print(f"QUERY {i+1}: {q}")
        ans = model.answer_question(enc_image, q, processor)
        print(f"RESPONSE: {ans}\\n")

if __name__ == "__main__":
    run_kinematic_structural_sweep()
