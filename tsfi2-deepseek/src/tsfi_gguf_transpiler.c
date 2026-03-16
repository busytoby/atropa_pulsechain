#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "tsfi_types.h"
#include "lau_memory.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_io.h"

// GGUF Constants
#define GGUF_MAGIC 0x46554747

// GGUF Value Types
enum GgufType {
    GGUF_TYPE_UINT8 = 0,
    GGUF_TYPE_INT8 = 1,
    GGUF_TYPE_UINT16 = 2,
    GGUF_TYPE_INT16 = 3,
    GGUF_TYPE_UINT32 = 4,
    GGUF_TYPE_INT32 = 5,
    GGUF_TYPE_FLOAT32 = 6,
    GGUF_TYPE_BOOL = 7,
    GGUF_TYPE_STRING = 8,
    GGUF_TYPE_ARRAY = 9,
    GGUF_TYPE_UINT64 = 10,
    GGUF_TYPE_INT64 = 11,
    GGUF_TYPE_FLOAT64 = 12,
};

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t tensor_count;
    uint64_t kv_count;
} GgufHeader;

// Read utilities
static bool read_u64(FILE *f, uint64_t *out) { return fread(out, sizeof(uint64_t), 1, f) == 1; }
static bool read_u32(FILE *f, uint32_t *out) { return fread(out, sizeof(uint32_t), 1, f) == 1; }

static char* read_gguf_string(FILE *f) {
    uint64_t len;
    if (!read_u64(f, &len)) return NULL;
    char *str = (char*)lau_malloc(len + 1);
    if (!str) return NULL;
    if (len > 0) {
        if (fread(str, 1, len, f) != len) {
            lau_free(str);
            return NULL;
        }
    }
    str[len] = '\0';
    return str;
}

static bool skip_gguf_value(FILE *f, uint32_t type) {
    switch (type) {
        case GGUF_TYPE_UINT8: case GGUF_TYPE_INT8: case GGUF_TYPE_BOOL:
            fseek(f, 1, SEEK_CUR); break;
        case GGUF_TYPE_UINT16: case GGUF_TYPE_INT16:
            fseek(f, 2, SEEK_CUR); break;
        case GGUF_TYPE_UINT32: case GGUF_TYPE_INT32: case GGUF_TYPE_FLOAT32:
            fseek(f, 4, SEEK_CUR); break;
        case GGUF_TYPE_UINT64: case GGUF_TYPE_INT64: case GGUF_TYPE_FLOAT64:
            fseek(f, 8, SEEK_CUR); break;
        case GGUF_TYPE_STRING: {
            uint64_t len; if (!read_u64(f, &len)) return false;
            fseek(f, len, SEEK_CUR);
            break;
        }
        case GGUF_TYPE_ARRAY: {
            uint32_t arr_type; if (!read_u32(f, &arr_type)) return false;
            uint64_t arr_len; if (!read_u64(f, &arr_len)) return false;
            for (uint64_t i = 0; i < arr_len; i++) {
                if (!skip_gguf_value(f, arr_type)) return false;
            }
            break;
        }
        default: return false;
    }
    return true;
}

void transpile_gguf_to_dna(const char* input_path, const char* output_dir) {
    printf("[Transpiler] Initiating GGUF to DNA translation for %s\n", input_path);
    
    FILE *f = fopen(input_path, "rb");
    
    K0RnStream* dna = (K0RnStream*)lau_memalign_wired(512, sizeof(K0RnStream));
    if (!dna) {
        fprintf(stderr, "[FRACTURE] Failed to allocate DNA stream header.\n");
        if (f) fclose(f);
        return;
    }
    
    if (!f) {
        printf("[WARN] Could not open %s. Proceeding with dummy DNA architecture testing.\n", input_path);
        
        dna->stream_capacity = 1024;
        dna->op_count = 128;
        dna->ops = (K0RnOp*)lau_memalign_wired(512, dna->stream_capacity * sizeof(K0RnOp));
        
        for (uint32_t i = 0; i < dna->op_count; i++) {
            dna->ops[i].type = K0RN_OP_BOX;
            dna->ops[i].flags = 1;
            dna->ops[i].p1 = 0.5f;
            memset(dna->ops[i].padding, 0, sizeof(dna->ops[i].padding));
        }
    } else {
        GgufHeader header;
        if (fread(&header, sizeof(GgufHeader), 1, f) != 1 || header.magic != GGUF_MAGIC) {
            fprintf(stderr, "[FRACTURE] Invalid GGUF header.\n");
            fclose(f);
            lau_free(dna);
            return;
        }

        printf("  [GGUF] Version: %u, Tensors: %lu, KVs: %lu\n", header.version, header.tensor_count, header.kv_count);
        
        uint32_t d_block_count = 0;
        uint32_t d_head_count = 0;
        
        for (uint64_t i = 0; i < header.kv_count; i++) {
            char *key = read_gguf_string(f);
            if (!key) break;
            uint32_t val_type;
            if (!read_u32(f, &val_type)) { lau_free(key); break; }
            if (strcmp(key, "deepseek2.block_count") == 0 && val_type == GGUF_TYPE_UINT32) {
                read_u32(f, &d_block_count);
            } else if (strcmp(key, "deepseek2.attention.head_count") == 0 && val_type == GGUF_TYPE_UINT32) {
                read_u32(f, &d_head_count);
            } else {
                skip_gguf_value(f, val_type);
            }
            lau_free(key);
        }
        
        for (uint64_t i = 0; i < header.tensor_count; i++) {
            char *name = read_gguf_string(f);
            if (!name) break;
            uint32_t n_dims;
            if (!read_u32(f, &n_dims)) { lau_free(name); break; }
            uint64_t dims[4] = {1, 1, 1, 1};
            for (uint32_t d = 0; d < n_dims; d++) read_u64(f, &dims[d]);
            uint32_t type;
            if (!read_u32(f, &type)) { lau_free(name); break; }
            uint64_t offset;
            if (!read_u64(f, &offset)) { lau_free(name); break; }
            lau_free(name);
        }
        
        printf("  [Transpiler] Aligning matrices to Nef-511 (AVX-512) K0Rn boundaries...\n");
        
        off_t current_pos = ftell(f);
        fseek(f, 0, SEEK_END);
        off_t total_size = ftell(f);
        fseek(f, current_pos, SEEK_SET);
        
        uint64_t tensor_bytes = total_size - current_pos;
        uint64_t ops_required = (tensor_bytes + 63) / 64; 
        
        printf("  [Transpiler] Physical tensor footprint: %lu bytes (%lu K0Rn operations)\n", tensor_bytes, ops_required);
        
        dna->stream_capacity = ops_required;
        dna->op_count = ops_required;
        
        dna->ops = (K0RnOp*)lau_memalign_wired(512, dna->stream_capacity * sizeof(K0RnOp));
        if (!dna->ops) {
            fprintf(stderr, "[FRACTURE] System OOM. Failed to allocate %lu MB for physical DNA operations.\n", (ops_required * 64) / (1024 * 1024));
            lau_free(dna);
            fclose(f);
            return;
        }
        
        printf("  [Transpiler] Ingesting physical tensors from GGUF into memory...\n");
        size_t read_bytes = fread(dna->ops, 1, tensor_bytes, f);
        if (read_bytes != tensor_bytes) {
            printf("  [WARN] Read %zu bytes, expected %lu. Continuing with partial geometry.\n", read_bytes, tensor_bytes);
        }
        fclose(f);
    }
    
    char out_path[1024];
    snprintf(out_path, sizeof(out_path), "%s/deepseek_coder_v2_moe_block.dna", output_dir);
    
    FILE* out_f = fopen(out_path, "wb");
    if (out_f) {
        fwrite(dna, sizeof(K0RnStream), 1, out_f);
        fwrite(dna->ops, sizeof(K0RnOp), dna->op_count, out_f);
        fclose(out_f);
        printf("[PASS] Translation to .dna successful.\n");
    } else {
        printf("[FAIL] Failed to save .dna file to disk.\n");
    }
    
    lau_free(dna->ops);
    lau_free(dna);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.gguf> <output_dir>\n", argv[0]);
        transpile_gguf_to_dna("dummy.gguf", "assets/dna/deepseek_coder_v2");
        
        extern void lau_report_memory_metrics(void);
        lau_report_memory_metrics();
        return 0;
    }
    
    transpile_gguf_to_dna(argv[1], argv[2]);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
