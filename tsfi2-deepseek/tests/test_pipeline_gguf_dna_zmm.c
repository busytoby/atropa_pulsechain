#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_types.h"
#include "tsfi_zmm_rpc.h"
#include "tsfi_opt_zmm.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_io.h"

#define GGUF_MAGIC 0x46554747 // "GGUF"

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t tensor_count;
    uint64_t kv_count;
} MockGgufHeader;

extern void tsfi_zmm_rpc_step_async_llm(TsfiZmmVmState *state);

int main(void) {
    printf("=== End-to-End Pipeline: GGUF -> DNA -> ZMM Query ===\n");

    const char *gguf_path = "assets/dummy_mini.gguf";
    const char *dna_dir = "assets/dna/deepseek_coder_v2";
    const char *expected_dna = "assets/dna/deepseek_coder_v2/deepseek_coder_v2_moe_block.dna";

    printf("  [IO] Synthesizing mock %s...\n", gguf_path);
    FILE *gf = fopen(gguf_path, "wb");
    assert(gf != NULL);
    MockGgufHeader hdr = { GGUF_MAGIC, 3, 0, 0 };
    fwrite(&hdr, sizeof(MockGgufHeader), 1, gf);
    
    char dummy_tensor[128] = {0};
    fwrite(dummy_tensor, 1, sizeof(dummy_tensor), gf);
    fclose(gf);

    printf("  [Transpiler] Executing binary to translate GGUF to DNA...\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "./bin/tsfi_gguf_transpiler %s %s > /dev/null", gguf_path, dna_dir);
    int ret = system(cmd);
    assert(ret == 0);
    
    assert(access(expected_dna, F_OK) == 0);
    printf("  [Transpiler] Successfully translated GGUF into native .dna K0Rn geometry.\n");

    TsfiZmmManifest *manifest = (TsfiZmmManifest*)lau_memalign_wired(512, sizeof(TsfiZmmManifest));
    memset(manifest, 0, sizeof(TsfiZmmManifest));
    TsfiZmmVmState rpc_state;
    memset(&rpc_state, 0, sizeof(TsfiZmmVmState));
    rpc_state.manifest = manifest;

    char rpc_in[512];
    char rpc_out[8192];
    snprintf(rpc_in, sizeof(rpc_in), "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.load_dna_llm\", \"params\": {\"path\": \"%s\"}, \"id\": 1}", expected_dna);
    
    printf("  [RPC] Loading DNA into ZMM...\n");
    tsfi_zmm_rpc_dispatch(&rpc_state, rpc_in, rpc_out, sizeof(rpc_out));
    assert(strstr(rpc_out, "DNA Model Mounted") != NULL);

    const char *query_json = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.query_llm\", \"params\": {\"prompt\": \"Write a fast C function to calculate the dot product of two AVX-512 vectors.\"}, \"id\": 2}";
    printf("  [RPC] Submitting prompt...\n");
    tsfi_zmm_rpc_dispatch(&rpc_state, query_json, rpc_out, sizeof(rpc_out));
    assert(strstr(rpc_out, "\"status\": \"pending\"") != NULL);

    printf("  [VM] Executing deterministic polling epochs...\n");
    int is_done = 0;
    const char *receipt_json = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.get_receipt\", \"params\": {\"receipt\": 1}, \"id\": 3}";
    
    for (int i = 0; i < 40; i++) {
        tsfi_zmm_rpc_step_async_llm(&rpc_state);
        tsfi_zmm_rpc_dispatch(&rpc_state, receipt_json, rpc_out, sizeof(rpc_out));
        if (strstr(rpc_out, "\"status\": \"done\"") != NULL) {
            is_done = 1;
            break;
        }
    }
    
    assert(is_done == 1);
    printf("  [PASS] Async extraction completed. Full text geometry streamed from DNA.\n");

    printf("  [Teardown] Unlocking ZMM components...\n");
    if (manifest->slots[31].data_ptr) {
        lau_free(manifest->slots[31].data_ptr);
    }
    lau_free(manifest);
    
    unlink(gguf_path);
    unlink(expected_dna);

    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("\n=== Pipeline Verification Complete ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
