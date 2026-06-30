import struct
import math

def write_test_gguf(filepath, num_weights):
    with open(filepath, 'wb') as f:
        # GgufHeader: magic (uint32), version (uint32), tensor_count (uint64), kv_count (uint64)
        magic = 0x46554747 # "GGUF" in little-endian
        version = 3
        tensor_count = 0
        kv_count = 0
        
        f.write(struct.pack('<I', magic))
        f.write(struct.pack('<I', version))
        f.write(struct.pack('<Q', tensor_count))
        f.write(struct.pack('<Q', kv_count))
        
        # Align to 32 bytes boundary
        current_pos = f.tell()
        aligned_pos = (current_pos + 31) & ~31
        padding_bytes = aligned_pos - current_pos
        f.write(b'\x00' * padding_bytes)
        
        # Write actual float weights
        weights = []
        for i in range(num_weights):
            val = math.sin(i * 0.012) * math.cos(i * 0.004) * 0.6
            weights.append(val)
            
        f.write(struct.pack(f'<{num_weights}f', *weights))
        print(f"Generated valid binary GGUF file: {filepath} with {num_weights} weights.")

if __name__ == '__main__':
    # Generate 65536 weights for DeepSeek and Stable Diffusion GGUF models
    write_test_gguf("deepseek_model.gguf", 65536)
    write_test_gguf("stable_diffusion_model.gguf", 65536)
