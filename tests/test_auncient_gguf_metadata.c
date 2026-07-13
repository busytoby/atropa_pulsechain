#include "../tsfi2-deepseek/src/tsfi_gguf_loader.c"
#include <stdio.h>
#include <stdbool.h>

// Parse and print all metadata key-value pairs inside a target GGUF model
bool tsfi_dump_gguf_metadata(const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "[GGUF ERROR] Cannot open file: %s\n", filepath);
        return false;
    }

    GgufHeader header;
    if (fread(&header, sizeof(GgufHeader), 1, f) != 1 || header.magic != GGUF_MAGIC) {
        fprintf(stderr, "[GGUF ERROR] Invalid GGUF header magic in: %s\n", filepath);
        fclose(f);
        return false;
    }

    printf("=============================================================\n");
    printf("GGUF MODEL METADATA DUMP: %s\n", filepath);
    printf("=============================================================\n");
    printf("  Magic Signature: 0x%08X (Valid GGUF)\n", header.magic);
    printf("  GGUF Version:    %u\n", header.version);
    printf("  Tensor Count:    %lu\n", header.tensor_count);
    printf("  Metadata Count:  %lu\n", header.kv_count);
    printf("-------------------------------------------------------------\n");

    char key_buf[128];
    for (uint64_t i = 0; i < header.kv_count; i++) {
        if (!read_gguf_string(f, key_buf, sizeof(key_buf))) {
            fprintf(stderr, "  [WARN] Failed to read key string at index %lu\n", i);
            break;
        }
        uint32_t val_type;
        if (!read_u32(f, &val_type)) {
            fprintf(stderr, "  [WARN] Failed to read value type for key: %s\n", key_buf);
            break;
        }

        // Print Key name and type ID
        printf("  - %-32s [Type %2u]: ", key_buf, val_type);

        // Fetch and format known basic metadata types
        if (val_type == GGUF_TYPE_UINT32) {
            uint32_t val;
            if (fread(&val, sizeof(uint32_t), 1, f) == 1) {
                printf("%u (uint32)\n", val);
            }
        } else if (val_type == GGUF_TYPE_UINT64) {
            uint64_t val;
            if (fread(&val, sizeof(uint64_t), 1, f) == 1) {
                printf("%lu (uint64)\n", val);
            }
        } else if (val_type == GGUF_TYPE_STRING) {
            char val_str[128];
            if (read_gguf_string(f, val_str, sizeof(val_str))) {
                printf("\"%s\"\n", val_str);
            }
        } else {
            // Print label for other complex structures and skip
            printf("[Complex Value Data]\n");
            if (!skip_gguf_value(f, val_type)) {
                fprintf(stderr, "  [WARN] Failed to skip value for key: %s\n", key_buf);
                break;
            }
        }
    }

    fclose(f);
    printf("=============================================================\n");
    return true;
}

int main(void) {
    // Dump metadata from deepseek_model.gguf
    bool ok = tsfi_dump_gguf_metadata("deepseek_model.gguf");
    return ok ? 0 : 1;
}
