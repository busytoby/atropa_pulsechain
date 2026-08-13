#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[E-RARA C TEXT ANALYSIS] Analyzing Page 1 of DOI: 10.3931/e-rara-10100 using C RDBMS APIs...\n");

    // Clean initial test database state
    unlink("erara_page_text.dat.bin");
    unlink("erara_page_text.wal.dat.bin");

    // Partial Title-Block Fragment Buffer (Lines 1-3 only) registered for analysis
    const char *title_block_fragment = 
        "Von erkiesen vnd freyheit der speisen. Von aergerniss vnd verbesserung.\n"
        "Ob jemants gewalt habe verbyeten etliche speiss zu eessen zu etlicher zeyt.\n"
        "Huldrychen Zwinglis predig gethan zu Zuerich in der grossen kirchen am .xxiij. tag maertzen .M.D.XXII.";

    // 1. Register partial fragment into C RDBMS storage (erara_page_text.dat.bin)
    int reg_rc = tsfi_erara_register_page_text("10.3931/e-rara-10100", 1, title_block_fragment);
    assert(reg_rc == 0);

    // Manually mark record as a partial fragment in storage struct
    vsen_erara_page_text_record_t analysis;
    int analyze_rc = tsfi_erara_analyze_page_text("10.3931/e-rara-10100", 1, &analysis);
    assert(analyze_rc == 0);

    // 2. Output computed metrics evaluated directly by C code logic
    printf("\n--- C RDBMS ANALYZER REPORT (DOI: %s, Page %u) ---\n", analysis.doi, analysis.page_num);
    printf("  Evaluated Character Count:  %u bytes\n", analysis.character_count);
    printf("  Evaluated Word Count:       %u words\n", analysis.word_count);
    printf("  Payload Completeness State: [PARTIAL FRAGMENT - INCOMPLETE TITLE BLOCK; REMAINING PAGE TEXT UNLOADED]\n");
    printf("  Extracted Incipit (Line 1): \"%s\"\n", analysis.first_line);
    printf("\n  Raw Ingested Fragment Stream:\n--------------------------------------------------------------------\n%s\n--------------------------------------------------------------------\n", analysis.page_text);

    // 3. Audit Cryptographic DNA Hash Chain under Rule 11 & Rule 13
    int audit_rc = tsfi_vsen_audit_chain_verify("erara_page_text.dat.bin");
    assert(audit_rc == 0);

    printf("\n[E-RARA C TEXT ANALYSIS] C analysis of partial fragment completed successfully.\n");
    return 0;
}
