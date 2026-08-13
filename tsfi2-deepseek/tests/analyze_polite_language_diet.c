#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[E-RARA C ANALYSIS] Analyzing Polite Language & Dietary Terminology from Page 1 (DOI: 10.3931/e-rara-10100)...\n");

    // Clean initial test state
    unlink("erara_page_text.dat.bin");
    unlink("erara_page_text.wal.dat.bin");

    // Registered un-redacted Page 1 text stream
    const char *page1_stream = 
        "Von erkiesen vnd freyheit der speisen. Von aergerniss vnd verbesserung.\n"
        "Ob jemants gewalt habe verbyeten etliche speiss zu eessen zu etlicher zeyt.\n"
        "Huldrychen Zwinglis predig gethan zu Zuerich in der grossen kirchen am .xxiij. tag maertzen .M.D.XXII.\n"
        "Dem eersamen vnd wysen Chratz Christoffel Froschouer Buchtrucker zu Zuerich mynem besonderen gunstigen herren vnd freundt.\n"
        "Gnad vnd frid von Gott dem Herren. Diewyl vil menschen sich aergern dadvon das etliche hinder der fasnacht vnd in der fastenn fleisch gessen haben,\n"
        "hab ich fuer gut angesehen die predig so ich gemeynlich gethan habe von freyheit der speisen in truck zu geben,\n"
        "damit menklich wysse wie die sach von Gottes wort geschirmt vnd erhalten werde.";

    // 1. Register Page 1 text stream into C RDBMS storage
    assert(tsfi_erara_register_page_text("10.3931/e-rara-10100", 1, page1_stream) == 0);

    // 2. Perform C Polite Language & Dietary Terminology Analysis
    vsen_erara_polite_language_diet_t diet_analysis;
    assert(tsfi_erara_analyze_polite_language_diet("10.3931/e-rara-10100", 1, &diet_analysis) == 0);

    printf("\n================ E-RARA POLITE LANGUAGE & DIET C ANALYSIS ================\n");
    printf("  Target DOI:             %s (Page %u)\n", diet_analysis.doi, diet_analysis.page_num);
    printf("  Honorific Salutation:   \"%s\"\n", diet_analysis.honorific_salutation);
    printf("  Benediction Greeting:   \"%s\"\n", diet_analysis.benediction_greeting);
    printf("  Diet Term (Speisen):    %s\n", diet_analysis.diet_term_speisen);
    printf("  Diet Term (Fleisch):    %s\n", diet_analysis.diet_term_fleisch);
    printf("  Diet Term (Fastenn):    %s\n", diet_analysis.diet_term_fastenn);
    printf("  Core Thesis Clause:     \"%s\"\n", diet_analysis.core_thesis_clause);

    // Assert key analysis extractions computed by C logic
    assert(strstr(diet_analysis.honorific_salutation, "Christoffel Froschouer") != NULL);
    assert(strstr(diet_analysis.benediction_greeting, "Gnad vnd frid") != NULL);
    assert(strlen(diet_analysis.diet_term_speisen) > 0);
    assert(strlen(diet_analysis.diet_term_fleisch) > 0);
    assert(strlen(diet_analysis.diet_term_fastenn) > 0);
    assert(strstr(diet_analysis.core_thesis_clause, "von Gottes wort geschirmt") != NULL);

    // 3. Audit Cryptographic DNA Hash Chain
    assert(tsfi_vsen_audit_chain_verify("erara_page_text.dat.bin") == 0);

    printf("\n[E-RARA C ANALYSIS] Polite language & diet analysis completed successfully!\n");
    return 0;
}
