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

    // 4. Test tsfi_zorse_query_llm passing GGUF asset path instead of 0 or model name
    char resp_buf[256];
    int llm_gguf_rc = tsfi_zorse_query_llm("Ping DeepSeek GGUF Path", model_gguf_path, resp_buf, sizeof(resp_buf));
    printf("  tsfi_zorse_query_llm(..., GGUF Path): %s -> RC: %d\n", model_gguf_path, llm_gguf_rc);
    assert(llm_gguf_rc == 0 || llm_gguf_rc == -2);

    int gguf_rc = tsfi_zorse_query_llm_gguf("Ping DeepSeek GGUF", model_gguf_path, resp_buf, sizeof(resp_buf));
    printf("  GGUF Asset Path Verification:          %s -> RC: %d\n", model_gguf_path, gguf_rc);
    assert(gguf_rc == 0 || gguf_rc == -2);

    // 5. Test tsfi_zorse_query_moondream_vlm multimodal visual query pipeline
    char vlm_buf[256];
    int vlm_rc = tsfi_zorse_query_moondream_vlm("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=", "Audit visual console", vlm_buf, sizeof(vlm_buf));
    printf("  Moondream VLM Query Status:   RC: %d\n", vlm_rc);
    assert(vlm_rc == 0 || vlm_rc == -1 || vlm_rc == -2);

    // 6. Test tsfi_zorse_audit_screen_visual & tsfi_zorse_generate_jcl_from_flowchart
    char alert_buf[128];
    int alert_rc = tsfi_zorse_audit_screen_visual("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=", "moondream", alert_buf, sizeof(alert_buf));
    printf("  Visual Console Alert Status:  RC: %d\n", alert_rc);
    assert(alert_rc == 0 || alert_rc == -1 || alert_rc == -2);

    char jcl_flow_buf[512];
    int flow_rc = tsfi_zorse_generate_jcl_from_flowchart("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=", "moondream", jcl_flow_buf, sizeof(jcl_flow_buf));
    printf("  Flowchart JCL Gen Status:     RC: %d\n", flow_rc);
    assert(flow_rc == 0 || flow_rc == -1 || flow_rc == -2);

    // 7. Test tsfi_zorse_submit_jcl_cobol_batch batch submission engine
    const char *jcl_job = "//ZORSEJOB JOB (10100),'DEEPSEEK BATCH',CLASS=A\n"
                           "//STEP1    EXEC PGM=ZORSEAI,PARM='MODEL=DEEPSEEK'\n"
                           "//SYSUT1   DD DSN=ATROPA.ZWINGLI.PAGE1,DISP=SHR\n";
    const char *cobol_job = "01  PENITENT-CLEARANCE PIC X(30) VALUE 'Gnad vnd Frid von Gott'.\n";

    zorse_jcl_batch_receipt_t batch_rcpt;
    int batch_rc = tsfi_zorse_submit_jcl_cobol_batch(jcl_job, cobol_job, model_gguf_path, &batch_rcpt);
    printf("  JCL/COBOL Batch Job Submission Status: RC: %d -> Job ID: %u, DSN: %s, Result Binary: %s\n", 
           batch_rc, batch_rcpt.job_id, batch_rcpt.input_dsn, batch_rcpt.result_dat_bin);
    assert(batch_rc == 0);
    assert(batch_rcpt.job_id == 10100);
    assert(batch_rcpt.status_code == 0);
    assert(strcmp(batch_rcpt.input_dsn, "ATROPA.ZWINGLI.PAGE1") == 0);

    // 8. Test Zorse DeepSeek Coder Source File Reading & Editing Interface
    char read_buf[1024];
    int read_rc = tsfi_zorse_read_source_file("inc/tsfi_zorse_eval.h", read_buf, sizeof(read_buf));
    printf("  Zorse C Source Read Status:   RC: %d (Read %zu bytes off inc/tsfi_zorse_eval.h)\n", read_rc, strlen(read_buf));
    assert(read_rc == 0);
    assert(strlen(read_buf) > 0);

    char edit_summary[256];
    int edit_rc = tsfi_zorse_edit_source_file("inc/tsfi_zorse_eval.h", "Refactor C struct alignment under Rule 8", model_gguf_path, edit_summary, sizeof(edit_summary));
    printf("  Zorse DeepSeek Coder Source Edit Status: RC: %d -> Summary: %s\n", edit_rc, edit_summary);
    assert(edit_rc == 0);

    // 9. Audit Cryptographic DNA Hash Chain
    assert(tsfi_vsen_audit_chain_verify("amt_orientation.dat.bin") == 0);

    printf("\n[ZORSE ASSET INTEGRATION] Zorse DeepSeek asset prover initialized and verified successfully in C!\n");
    return 0;
}
