import sys
import os

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Since text prompts sometimes fail against strong ControlNet skeletons, 
# we need a mathematical failsafe. We will add a quick color histogram check to the Spider.
# Humans have flesh tones (peach/pink/light brown). Plushies are strictly bright green, dark brown, and black.
# We will explicitly reject any image with too many "flesh tone" pixels.

spider_code_old = """        lower_brown = np.array([10, 50, 20])
        upper_brown = np.array([25, 255, 200])
        mask_bear = cv2.inRange(hsv, lower_brown, upper_brown)
        bear_pixels = cv2.countNonZero(mask_bear)
        
        total_pixels = img.shape[0] * img.shape[1]
        gator_presence = (gator_pixels / total_pixels) * 100.0
        bear_presence = (bear_pixels / total_pixels) * 100.0"""

spider_code_new = """        lower_brown = np.array([10, 50, 20])
        upper_brown = np.array([25, 255, 200])
        mask_bear = cv2.inRange(hsv, lower_brown, upper_brown)
        bear_pixels = cv2.countNonZero(mask_bear)
        
        # Human Flesh Tone quarantine mask
        lower_flesh = np.array([0, 20, 50])
        upper_flesh = np.array([20, 150, 255])
        mask_flesh = cv2.inRange(hsv, lower_flesh, upper_flesh)
        flesh_pixels = cv2.countNonZero(mask_flesh)
        
        total_pixels = img.shape[0] * img.shape[1]
        gator_presence = (gator_pixels / total_pixels) * 100.0
        bear_presence = (bear_pixels / total_pixels) * 100.0
        flesh_presence = (flesh_pixels / total_pixels) * 100.0"""

code = code.replace(spider_code_old, spider_code_new)

spider_logic_old = """        if brightness < 80.0:
            return "INCREASE_LIGHTING"
        elif gator_presence < 2.0 and "alligator" in original_prompt.lower():
            return "EMPHASIZE_GATOR"
        elif bear_presence < 5.0 and "bear" in original_prompt.lower():
            return "EMPHASIZE_BEAR"
            
        return "NONE"
    except:"""

spider_logic_new = """        if flesh_presence > 5.0:
            # If more than 5% of the image is human skin tone, burn it immediately.
            return "HUMAN_CONTAMINATION"
        elif brightness < 80.0:
            return "INCREASE_LIGHTING"
        elif gator_presence < 2.0 and "alligator" in original_prompt.lower():
            return "EMPHASIZE_GATOR"
        elif bear_presence < 5.0 and "bear" in original_prompt.lower():
            return "EMPHASIZE_BEAR"
            
        return "NONE"
    except:"""

code = code.replace(spider_logic_old, spider_logic_new)

# Add the consequence of HUMAN_CONTAMINATION to the retry loop
retry_old = """                elif advice == "EMPHASIZE_BEAR":
                    prompt = "MASSIVE BROWN TEDDY BEAR PLUSH, " + prompt"""

retry_new = """                elif advice == "EMPHASIZE_BEAR":
                    prompt = "MASSIVE BROWN TEDDY BEAR PLUSH, " + prompt
                elif advice == "HUMAN_CONTAMINATION":
                    # Scramble the seed completely and drop controlnet strength slightly so the text prompt can overpower the human skeleton shape
                    divergent_seed = str(int(divergent_seed) + 12345)
                    dynamic_control = max(0.5, dynamic_control - 0.1)"""

code = code.replace(retry_old, retry_new)

with open(filepath, 'w') as f:
    f.write(code)

