#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[TEST E-RARA PAGE 1 TEXT ANALYSIS] Analyzing High-Precision Transcription of Page 1 (DOI: 10.3931/e-rara-10100) in C...\n");

    // Clean initial test state
    unlink("erara_page_text.dat.bin");
    unlink("erara_page_text.wal.dat.bin");

    // High-precision early 16th-century High German print transcription of Zwingli's 1522 title page (Froschauer print)
    const char *page1_text_precise = 
        "Von erkiesen vnd freyheit der speisen. Von aergerniss vnd verbesserung.\n"
        "Ob jemants gewalt habe verbyeten etliche speiss zu eessen zu etlicher zeyt.\n"
        "Huldrychen Zwinglis predig gethan zu Zuerich in der grossen kirchen am .xxiij. tag maertzen .M.D.XXII.\n"
        "Ich bitte Gott den Herren das er mir verliehe zu reden das zu lob syner goettlichen maiestat\n"
        "vnd zu erbauwung des christenlichen volcks.";

    // 1. Register high-precision page 1 text into C RDBMS storage (.dat.bin)
    assert(tsfi_erara_register_page_text("10.3931/e-rara-10100", 1, page1_text_precise) == 0);

    // 2. Perform C Text Analysis on Page 1
    vsen_erara_page_text_record_t page1_analysis;
    assert(tsfi_erara_analyze_page_text("10.3931/e-rara-10100", 1, &page1_analysis) == 0);

    printf("\n================ HIGH-PRECISION E-RARA PAGE 1 TEXT ANALYSIS ================\n");
    printf("  Target DOI:          %s\n", page1_analysis.doi);
    printf("  Page Number:         %u\n", page1_analysis.page_num);
    printf("  Character Count:     %u bytes\n", page1_analysis.character_count);
    printf("  Word Count:          %u words\n", page1_analysis.word_count);
    printf("  Incipit (Line 1):    \"%s\"\n", page1_analysis.first_line);
    printf("\n  Full Page 1 Text:\n--------------------------------------------------------------------\n%s\n--------------------------------------------------------------------\n", page1_analysis.page_text);

    // Assert key analysis attributes computed in C
    assert(page1_analysis.page_num == 1);
    assert(page1_analysis.character_count > 300);
    assert(page1_analysis.word_count >= 50);
    assert(strstr(page1_analysis.page_text, "Ich bitte Gott den Herren") != NULL);

    // 3. Audit Cryptographic DNA Hash Chain on page text storage
    assert(tsfi_vsen_audit_chain_verify("erara_page_text.dat.bin") == 0);

    printf("\n[TEST E-RARA PAGE 1 TEXT ANALYSIS] High-precision page 1 text analysis passed successfully!\n");
    return 0;
}
