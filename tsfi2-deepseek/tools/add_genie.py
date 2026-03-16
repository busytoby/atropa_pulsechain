import sys

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# 1. Add Genie Globals and Imports
import_section = """from PIL import Image, ImageDraw, ImageFont

# --- Genie Globals ---
try:
    import torch
    from transformers import AutoModelForCausalLM, AutoProcessor
    GENIE_ENABLED = True
except ImportError:
    GENIE_ENABLED = False

genie_model = None
genie_processor = None
genie_current_thought = "GENIE IS AWAKENING..."
genie_last_observation_time = 0

def init_genie():
    global genie_model, genie_processor
    if GENIE_ENABLED and genie_model is None:
        print("[INFO] Conjuring the Moondream Genie from the Void...")
        try:
            genie_model = AutoModelForCausalLM.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02", pad_token_id=50256).to("cpu")
            genie_processor = AutoProcessor.from_pretrained("vikhyatk/moondream2", trust_remote_code=True, revision="2024-04-02")
            print("[SUCCESS] Genie is watching.")
        except Exception as e:
            print(f"[WARN] Genie failed to materialize: {e}")

if GENIE_ENABLED:
    threading.Thread(target=init_genie, daemon=True).start()

def genie_observation_thread(frame_image):
    global genie_current_thought
    if genie_model is None or genie_processor is None: return
    try:
        enc_image = genie_model.encode_image(frame_image)
        q = "You are an AI daemon observing three soft plush toys (a teddy bear, an alligator, and a spider). What are they doing together? Answer in one short, poetic sentence."
        ans = genie_model.answer_question(enc_image, q, genie_processor)
        genie_current_thought = "DAEMON SEES: " + ans.upper()
    except:
        pass
"""

code = code.replace("from PIL import Image, ImageDraw", import_section)

# 2. Add Matrix Rain to Skeleton
skeleton_start = """def build_openpose_skeleton(f_idx, phase="sit"):
    W, H = 512, 512
    img = Image.new("RGB", (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)"""

matrix_rain = """
    try:
        font = ImageFont.truetype("assets/fonts/Hack-Regular.ttf", 14)
    except IOError:
        font = ImageFont.load_default()
        
    thought = genie_current_thought + " " + genie_current_thought + " "
    for column in range(0, W, 14):
        offset = (f_idx * 5 + column * 13) % H
        char_idx_base = column % len(thought)
        for row in range(0, H, 14):
            y_pos = (row + offset) % H
            if y_pos > H - 50: continue
            char_idx = (char_idx_base + (row // 14)) % len(thought)
            char = thought[char_idx]
            intensity = int(255 * (1.0 - (y_pos / float(H))))
            draw.text((column, y_pos), char, font=font, fill=(0, intensity, 0))
"""

code = code.replace(skeleton_start, skeleton_start + matrix_rain)


# 3. Trigger Genie in Generation Loop
trigger_old = """            painter_img = Image.open(out_path).convert("RGB")
            
            # Multiplexing
            mpx_img = Image.new("RGB", (512, 1024))
            mpx_img.paste(skeleton, (0, 0)) 
            mpx_img.paste(painter_img, (0, 512)) 
            
            jpeg_io = io.BytesIO()
            mpx_img.save(jpeg_io, format="JPEG", quality=80)
            
            latest_jpeg_bytes = jpeg_io.getvalue()
            latest_frame_bytes = mpx_img.tobytes()"""

trigger_new = """            painter_img = Image.open(out_path).convert("RGB")
            
            global genie_last_observation_time
            current_time = time.time()
            if GENIE_ENABLED and (current_time - genie_last_observation_time > 4.0) and local_f_idx > 5:
                genie_last_observation_time = current_time
                obs_img = painter_img.copy().resize((256, 256))
                threading.Thread(target=genie_observation_thread, args=(obs_img,), daemon=True).start()
                
            # Multiplexing
            mpx_img = Image.new("RGB", (512, 1024))
            mpx_img.paste(skeleton, (0, 0)) 
            mpx_img.paste(painter_img, (0, 512)) 
            
            jpeg_io = io.BytesIO()
            mpx_img.save(jpeg_io, format="JPEG", quality=80)
            
            latest_jpeg_bytes = jpeg_io.getvalue()
            latest_frame_bytes = mpx_img.tobytes()"""

code = code.replace(trigger_old, trigger_new)

with open(filepath, 'w') as f:
    f.write(code)

