import onnx
from onnx import numpy_helper
import numpy as np

def export_to_c_header(onnx_path, header_path):
    print(f"Loading {onnx_path}...")
    model = onnx.load(onnx_path)
    
    with open(header_path, 'w') as f:
        f.write("#ifndef MIDAS_WEIGHTS_H\n")
        f.write("#define MIDAS_WEIGHTS_H\n\n")
        f.write("// Auto-generated C header containing raw MiDaS ONNX weights.\n\n")
        
        count = 0
        for initializer in model.graph.initializer:
            # Convert ONNX tensor to NumPy array
            w = numpy_helper.to_array(initializer)
            
            # We only care about the float32 weight matrices
            if w.dtype == np.float32:
                # Clean up the name to be a valid C identifier
                c_name = "midas_weight_" + initializer.name.replace('/', '_').replace('.', '_').replace('-', '_')
                
                # Flatten the array
                w_flat = w.flatten()
                
                f.write(f"static const float {c_name}[{len(w_flat)}] = {{\n")
                
                # Write the floats in chunks to avoid massively long lines
                chunk_size = 10
                for i in range(0, len(w_flat), chunk_size):
                    chunk = w_flat[i:i+chunk_size]
                    f.write("    " + ", ".join([f"{val}f" for val in chunk]) + ",\n")
                    
                f.write("};\n\n")
                count += 1
                
        f.write("#endif // MIDAS_WEIGHTS_H\n")
        print(f"Successfully exported {count} weight matrices to {header_path}.")

if __name__ == "__main__":
    export_to_c_header("assets/models/midas_sim.onnx", "inc/tsfi_midas_weights.h")
