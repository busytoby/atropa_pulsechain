#include "../tsfi2-deepseek/src/tsfi_gguf_loader.c"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void write_mock_gguf(const char *path, const uint8_t *mock_hash) {
    FILE *f = fopen(path, "wb");
    assert(f != NULL);

    GgufHeader header = {
        .magic = GGUF_MAGIC,
        .version = 2,
        .tensor_count = 0,
        .kv_count = 1
    };
    fwrite(&header, sizeof(GgufHeader), 1, f);

    // Key: "auncient.acab.root"
    const char *key = "auncient.acab.root";
    uint64_t key_len = strlen(key);
    fwrite(&key_len, sizeof(uint64_t), 1, f);
    fwrite(key, 1, key_len, f);

    // Value Type: ARRAY
    uint32_t val_type = GGUF_TYPE_ARRAY;
    fwrite(&val_type, sizeof(uint32_t), 1, f);

    // Array Type: UINT8, Length: 32
    uint32_t arr_type = GGUF_TYPE_UINT8;
    uint64_t arr_len = 32;
    fwrite(&arr_type, sizeof(uint32_t), 1, f);
    fwrite(&arr_len, sizeof(uint64_t), 1, f);

    // Array contents: 32-byte hash
    fwrite(mock_hash, 1, 32, f);

    fclose(f);
}

int main(void) {
    printf("=============================================================\n");
    printf("Auncient GGUF Loader: ACAB Metadata Extraction Verification\n");
    printf("=============================================================\n");

    const char *mock_path = "tmp_mock_model.gguf";
    uint8_t mock_hash[32];
    memset(mock_hash, 0xE9, 32); // Fill with 0xE9

    printf("1. Creating mock GGUF model file with ACAB metadata...\n");
    write_mock_gguf(mock_path, mock_hash);
    printf("   ✓ Mock GGUF file written.\n");

    printf("2. Parsing GGUF file using tsfi_load_gguf_weights...\n");
    float dummy_weights[10];
    bool load_ok = tsfi_load_gguf_weights(mock_path, dummy_weights, 0);
    assert(load_ok == true);
    printf("   ✓ GGUF file read completed.\n");

    printf("3. Verifying extracted ACAB Activity Root...\n");
    assert(g_gguf_acab_found == true);
    assert(g_gguf_acab_root[0] == 0xE9);
    assert(g_gguf_acab_root[31] == 0xE9);
    printf("   ✓ ACAB root successfully extracted: 0x");
    for (int i = 0; i < 32; i++) printf("%02x", g_gguf_acab_root[i]);
    printf("\n");

    // Clean up
    remove(mock_path);

    printf("\n=============================================================\n");
    printf("AUNCIENT GGUF ACAB METADATA EXTRACTION PASSED\n");
    printf("=============================================================\n");
    return 0;
}
