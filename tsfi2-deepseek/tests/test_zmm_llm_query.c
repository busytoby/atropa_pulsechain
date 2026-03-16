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

// Define the async step function exposed in the header
extern void tsfi_zmm_rpc_step_async_llm(TsfiZmmVmState *state);

int main(void) {
    printf("=== ZMM Async LLM DNA Transpilation & Inference Verification ===\n");
    
    // 1. Setup the physical ZMM execution environment natively in memory
    TsfiZmmManifest *manifest = (TsfiZmmManifest*)lau_memalign_wired(512, sizeof(TsfiZmmManifest));
    memset(manifest, 0, sizeof(TsfiZmmManifest));
    
    TsfiZmmVmState rpc_state;
    memset(&rpc_state, 0, sizeof(TsfiZmmVmState));
    rpc_state.manifest = manifest;
    
    // 2. We mock a local transpiled DNA file on disk to test the entire RPC IO architecture
    const char *dna_path = "assets/dna/deepseek_coder_v2/dummy_test_block.dna";
    
    // Synthesize a dummy K0Rn stream
    K0RnStream* dna = (K0RnStream*)lau_memalign_wired(512, sizeof(K0RnStream));
    dna->stream_capacity = 64;
    dna->op_count = 64;
    dna->ops = (K0RnOp*)lau_memalign_wired(512, 64 * sizeof(K0RnOp));
    
    FILE *df = fopen(dna_path, "wb");
    if (df) {
        fwrite(dna, sizeof(K0RnStream), 1, df);
        fwrite(dna->ops, sizeof(K0RnOp) * dna->op_count, 1, df);
        fclose(df);
        printf("  [IO] Created dummy transpiled DNA geometry at %s\n", dna_path);
    }
    
    // 3. Issue the load directive via JSON-RPC to mount the DNA into ReBAR memory
    char load_json[512];
    snprintf(load_json, sizeof(load_json), "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.load_dna_llm\", \"params\": {\"path\": \"%s\"}, \"id\": 1}", dna_path);
    char rpc_output[8192];
    memset(rpc_output, 0, sizeof(rpc_output));
    
    printf("  [RPC] Dispatching Load: %s\n", load_json);
    int res = tsfi_zmm_rpc_dispatch(&rpc_state, load_json, rpc_output, sizeof(rpc_output));
    assert(res == 1);
    assert(strstr(rpc_output, "DNA Model Mounted") != NULL);
    
    // 4. Issue the async JSON-RPC query command
    const char *query_json = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.query_llm\", \"params\": {\"prompt\": \"Write a fast C function to calculate the dot product of two AVX-512 vectors.\"}, \"id\": 2}";
    memset(rpc_output, 0, sizeof(rpc_output));
    
    printf("  [RPC] Submitting Directive Query...\n");
    res = tsfi_zmm_rpc_dispatch(&rpc_state, query_json, rpc_output, sizeof(rpc_output));
    assert(res == 1);
    assert(strstr(rpc_output, "\"receipt\": 1") != NULL);
    assert(strstr(rpc_output, "\"status\": \"pending\"") != NULL);
    
    // 5. Simulate execution epochs where the background logic generates tokens
    printf("  [VM] Executing deterministic polling epochs...\n");
    int epoch = 0;
    int is_done = 0;
    
    const char *receipt_json = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.get_receipt\", \"params\": {\"receipt\": 1}, \"id\": 3}";
    
    for (int i = 0; i < 35; i++) {
        // Physically step the background generation by 1 instruction token
        tsfi_zmm_rpc_step_async_llm(&rpc_state);
        
        memset(rpc_output, 0, sizeof(rpc_output));
        res = tsfi_zmm_rpc_dispatch(&rpc_state, receipt_json, rpc_output, sizeof(rpc_output));
        
        if (strstr(rpc_output, "\"status\": \"done\"") != NULL) {
            is_done = 1;
            epoch = i + 1;
            break;
        }
    }
    
    assert(is_done == 1);
    printf("  [RPC] Final Assembly Reached at Epoch %d:\n%s\n", epoch, rpc_output);
    assert(strstr(rpc_output, "_mm512_fmadd_ps") != NULL);
    
    // 6. Tear down the ZMM environment safely
    printf("  [Teardown] Unlocking ZMM components...\n");
    
    // The load_dna_llm allocated data_ptr internally using lau_memalign_wired. We must free it.
    if (manifest->slots[31].data_ptr) {
        lau_free(manifest->slots[31].data_ptr);
    }
    
    lau_free(manifest);
    lau_free(dna->ops);
    lau_free(dna);
    
    // Remove dummy file
    unlink(dna_path);
    
    // 7. Systematically enforce ZERO memory leaks using the TSFi core tracker
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("=== ZMM Async LLM Transaction Verification Passed ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}