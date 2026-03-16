import torch
import sys

print("[INFO] Loading MiDaS DPT_Hybrid for ONNX Export...")
midas_model_type = "DPT_Hybrid"

try:
    # Load the PyTorch model
    midas = torch.hub.load("intel-isl/MiDaS", midas_model_type)
    midas.eval()
    
    # MiDaS DPT_Hybrid expects 384x384 input resolution.
    # We use a static shape for the export to ensure maximum compatibility with C++ backends.
    dummy_input = torch.randn(1, 3, 384, 384)
    
    onnx_path = "assets/models/midas_dpt_hybrid.onnx"
    print(f"[INFO] Exporting static graph to {onnx_path}...")
    
    # We drop the 'dynamic_axes' parameter. For embedded C++ inference (like NCNN or GGML), 
    # fixed-shape ONNX models compile much cleaner and run significantly faster.
    # We can handle the resize to/from 384x384 in the C++ preprocessing layer via OpenCV/stb_image.
    torch.onnx.export(
        midas, 
        dummy_input, 
        onnx_path,
        export_params=True,
        opset_version=16, 
        do_constant_folding=True, 
        input_names=['input'], 
        output_names=['output']
    )
    print("[SUCCESS] MiDaS successfully serialized to a static ONNX graph.")
    
except Exception as e:
    print(f"[ERROR] Failed to export MiDaS: {e}")
    sys.exit(1)
