#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[ZORSE DEEPSEEK CODER] Reading and Analyzing tests/test_zorse_asset_bridge.c...\n\n");

    const char *target_file = "tests/test_zorse_asset_bridge.c";
    const char *gguf_model = "/home/mariarahel/src/tsfi2/assets/DeepSeek-Coder-6.7B.gguf";

    // 1. Read source file content using Zorse C reader
    char file_content[16384] = {0};
    int read_rc = tsfi_zorse_read_source_file(target_file, file_content, sizeof(file_content));
    assert(read_rc == 0);
    printf("Successfully read %zu bytes from %s.\n", strlen(file_content), target_file);

    // 2. Perform DeepSeek Coder analysis over source file
    char prompt[18000];
    snprintf(prompt, sizeof(prompt), 
             "Analyze the following C source file for code structure, architectural compliance under Rule 8 and Rule 13, and verify intrinsic orientation ('amt nit ist ein eisent eih'):\n\n%s", 
             file_content);

    char analysis_response[2048];
    int llm_rc = tsfi_zorse_query_llm(prompt, gguf_model, analysis_response, sizeof(analysis_response));
    assert(llm_rc == 0);

    printf("\n================ ZORSE DEEPSEEK CODER ANALYSIS REPORT ================\n");
    printf("%s\n", analysis_response);
    printf("======================================================================\n");

    // 3. Write analysis WAL receipt to .dat.bin under Rule 13
    typedef struct {
        uint32_t magic;
        uint32_t file_bytes;
        char     target[128];
    } zorse_analysis_receipt_t;

    zorse_analysis_receipt_t rcpt;
    memset(&rcpt, 0, sizeof(rcpt));
    rcpt.magic = 0x414E4C59; // 'A''N''L''Y'
    rcpt.file_bytes = (uint32_t)strlen(file_content);
    strncpy(rcpt.target, target_file, sizeof(rcpt.target) - 1);

    FILE *wal_fp = fopen("zorse_analysis_test_bridge.dat.bin", "wb");
    if (wal_fp) {
        fwrite(&rcpt, sizeof(rcpt), 1, wal_fp);
        fclose(wal_fp);
    }

    printf("\n[ZORSE DEEPSEEK CODER] Analysis receipt saved to zorse_analysis_test_bridge.dat.bin under Rule 13.\n");
    return 0;
}
