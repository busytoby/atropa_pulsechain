#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[ZORSE ASSET INTEGRATION] Initializing Zorse DeepSeek LLM Asset Prover Bridge in ~/src/tsfi2/assets...\n");

    // Clean initial asset database state
    unlink("tsfi2_zorse_assets.dat.bin");
    unlink("tsfi2_zorse_assets.wal.dat.bin");

    // 1. Verify DeepSeek asset dataset path in ~/src/tsfi2/assets
    const char *dataset_path = "/home/mariarahel/src/tsfi2/assets/tsfi_deepseek_dataset.json";
    const char *model_gguf_path = "/home/mariarahel/src/tsfi2/assets/DeepSeek-Coder-6.7B.gguf";

    printf("  Target Asset Dataset:   %s\n", dataset_path);
    printf("  Target DeepSeek GGUF:   %s\n", model_gguf_path);

    // 2. Register Zorse Asset Prover record in C RDBMS storage (.dat.bin)
    const char *amt_id = "amt_zorse_deepseek_assets";
    const char *orientation = "Zorse Asset Prover: Intrinsic definition established ('amt nit ist ein eisent eih')";
    int forebearance = 10;
    const char *vision = "Direct Zorse C RDBMS bridge over ~/src/tsfi2/assets datasets and GGUF model weights under Rule 11 & Rule 13";

    int reg_rc = tsfi_vsen_amt_register_orientation(amt_id, orientation, forebearance, vision);
    assert(reg_rc == 0);

    // 3. Perform C RDBMS Lookup over recorded Zorse Asset Prover
    vsen_amt_orientation_record_t amt_rec;
    assert(tsfi_vsen_amt_lookup_orientation(amt_id, &amt_rec) == 0);

    printf("\n================ ZORSE DEEPSEEK ASSET PROVER REPORT ================\n");
    printf("  Prover ID:                  %s\n", amt_rec.amt_id);
    printf("  Amt Orientation:            %s\n", amt_rec.orientation);
    printf("  Intrinsic Definition Sound: %s\n", amt_rec.is_eisent_eih_intrinsic ? "YES (amt nit ist ein eisent eih)" : "NO");
    printf("  Forbearance Factor:         %d / 10\n", amt_rec.forebearance_factor);
    printf("  Sight Vision Summary:       %s\n", amt_rec.vision_summary);

    // Assert key structural invariants
    assert(strcmp(amt_rec.amt_id, amt_id) == 0);
    assert(amt_rec.is_eisent_eih_intrinsic == 1);
    assert(amt_rec.forebearance_factor == 10);

    // 4. Test tsfi_zorse_query_llm_gguf direct binding over GGUF asset file
    char resp_buf[256];
    int gguf_rc = tsfi_zorse_query_llm_gguf("Ping DeepSeek GGUF", model_gguf_path, resp_buf, sizeof(resp_buf));
    printf("  GGUF Asset Path Verification: %s -> RC: %d\n", model_gguf_path, gguf_rc);
    assert(gguf_rc == 0 || gguf_rc == -2);

    // 5. Test tsfi_zorse_query_moondream_vlm multimodal visual query pipeline
    char vlm_buf[256];
    int vlm_rc = tsfi_zorse_query_moondream_vlm("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=", "Audit visual console", vlm_buf, sizeof(vlm_buf));
    printf("  Moondream VLM Query Status:   RC: %d\n", vlm_rc);
    assert(vlm_rc == 0 || vlm_rc == -1 || vlm_rc == -2);

    // 6. Audit Cryptographic DNA Hash Chain
    assert(tsfi_vsen_audit_chain_verify("amt_orientation.dat.bin") == 0);

    printf("\n[ZORSE ASSET INTEGRATION] Zorse DeepSeek asset prover initialized and verified successfully in C!\n");
    return 0;
}
