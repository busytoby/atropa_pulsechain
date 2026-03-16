import sys
import torch
from PIL import Image
from transformers import AutoModelForCausalLM, AutoProcessor

def main():
    if len(sys.argv) < 3:
        print("Usage: python3 tsfi_vlm_critic.py <image_path> <prompt>")
        return

    img_path = sys.argv[1]
    prompt = sys.argv[2]

    try:
        model = AutoModelForCausalLM.from_pretrained(
            "vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256
        ).to("cpu")
        processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
        
        img = Image.open(img_path).convert('RGB')
        enc_image = model.encode_image(img)
        ans = model.answer_question(enc_image, prompt, processor)
        print(f"[VLM CRITIQUE] {ans}")
    except Exception as e:
        print(f"[FRACTURE] VLM Error: {e}")

if __name__ == "__main__":
    main()
