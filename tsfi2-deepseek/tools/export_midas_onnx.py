import torch
import sys

print("[INFO] Loading MiDaS DPT_Hybrid for ONNX Export...")
midas_model_type = "DPT_Hybrid"

try:
    midas = torch.hub.load("intel-isl/MiDaS", midas_model_type)
    midas.eval()
    
    # MiDaS typically expects a 384x384 input tensor for DPT_Hybrid
    # Batch Size = 1, Channels = 3 (RGB), Height = 384, Width = 384
    dummy_input = torch.randn(1, 3, 384, 384)
    
    onnx_path = "assets/models/midas_dpt_hybrid.onnx"
    print(f"[INFO] Exporting to {onnx_path}...")
    
    torch.onnx.export(
        midas, 
        dummy_input, 
        onnx_path,
        export_params=True,
        opset_version=14,          # Opset 14 is generally stable for Vision Transformers
        do_constant_folding=True, 
        input_names=['input'], 
        output_names=['output'],
        dynamic_axes={'input': {0: 'batch_size', 2: 'height', 3: 'width'}, 
                      'output': {0: 'batch_size', 1: 'height', 2: 'width'}}
    )
    print("[SUCCESS] MiDaS successfully serialized to ONNX.")
    
except Exception as e:
    print(f"[ERROR] Failed to export MiDaS: {e}")
    sys.exit(1)
