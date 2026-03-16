import onnx
from onnx import numpy_helper
import numpy as np
import sys
import os

def export_to_c_weights(onnx_path, header_path):
    print(f"Loading {onnx_path}...")
    model = onnx.load(onnx_path)
    
    with open(header_path, 'w') as f:
        f.write("#ifndef MIDAS_WEIGHTS_H\n")
        f.write("#define MIDAS_WEIGHTS_H\n\n")
        f.write("// Auto-generated C header containing raw MiDaS ONNX weights.\n")
        f.write("// ALIGNED FOR AVX-512 (64-byte boundaries)\n\n")
        
        count = 0
        for initializer in model.graph.initializer:
            w = numpy_helper.to_array(initializer)
            
            if w.dtype == np.float32:
                c_name = "midas_weight_" + initializer.name.replace('/', '_').replace('.', '_').replace('-', '_')
                w_flat = w.flatten()
                
                # The compiler directive aligns the arrays so AVX-512 can load them via _mm512_load_ps instead of _mm512_loadu_ps
                f.write(f"__attribute__((aligned(64))) static const float {c_name}[{len(w_flat)}] = {{\n")
                
                chunk_size = 10
                for i in range(0, len(w_flat), chunk_size):
                    chunk = w_flat[i:i+chunk_size]
                    f.write("    " + ", ".join([f"{val}f" for val in chunk]) + ",\n")
                    
                f.write("};\n\n")
                count += 1
                
        f.write("#endif // MIDAS_WEIGHTS_H\n")
        print(f"Successfully exported {count} weight matrices to {header_path}.")

if __name__ == "__main__":
    export_to_c_weights("assets/models/midas_sim.onnx", "inc/tsfi_midas_weights.h")
