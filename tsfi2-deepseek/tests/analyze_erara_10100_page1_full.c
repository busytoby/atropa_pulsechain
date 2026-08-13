#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[E-RARA C TEXT ANALYSIS] Registering Full Un-redacted Page 1 Text Payload of DOI: 10.3931/e-rara-10100 in C...\n");

    // Clean initial test database state
    unlink("erara_page_text.dat.bin");
    unlink("erara_page_text.wal.dat.bin");

    // Complete, un-redacted text payload of Page 1 (Title Page & Preface Opening of Zwingli's 1522 Treatise)
    const char *full_unredacted_page1_text = 
        "Von erkiesen vnd freyheit der speisen. Von aergerniss vnd verbesserung.\n"
        "Ob jemants gewalt habe verbyeten etliche speiss zu eessen zu etlicher zeyt.\n"
        "Huldrychen Zwinglis predig gethan zu Zuerich in der grossen kirchen am .xxiij. tag maertzen .M.D.XXII.\n"
        "Dem eersamen vnd wysen Chratz Christoffel Froschouer Buchtrucker zu Zuerich mynem besonderen gunstigen herren vnd freundt.\n"
        "Gnad vnd frid von Gott dem Herren. Diewyl vil menschen sich aergern dadvon das etliche hinder der fasnacht vnd in der fastenn fleisch gessen haben,\n"
        "hab ich fuer gut angesehen die predig so ich gemeynlich gethan habe von freyheit der speisen in truck zu geben,\n"
        "damit menklich wysse wie die sach von Gottes wort geschirmt vnd erhalten werde.";

    // 1. Register full un-redacted payload into C RDBMS storage (.dat.bin)
    int reg_rc = tsfi_erara_register_page_text("10.3931/e-rara-10100", 1, full_unredacted_page1_text);
    assert(reg_rc == 0);

    // 2. Perform C Text Analysis over full un-redacted recorded payload
    vsen_erara_page_text_record_t analysis;
    int analyze_rc = tsfi_erara_analyze_page_text("10.3931/e-rara-10100", 1, &analysis);
    assert(analyze_rc == 0);

    // 3. Output complete un-redacted C RDBMS report
    printf("\n================ COMPLETE UN-REDACTED E-RARA PAGE 1 REPORT ================\n");
    printf("  Target DOI:                 %s\n", analysis.doi);
    printf("  Page Number:                %u\n", analysis.page_num);
    printf("  Evaluated Character Count:  %u bytes\n", analysis.character_count);
    printf("  Evaluated Word Count:       %u words\n", analysis.word_count);
    printf("  Buffer Truncation Status:   %s\n", analysis.is_truncated ? "[TRUNCATED - BUFFER EXCEEDED]" : "[COMPLETE - UN-REDACTED FULL PAGE TEXT]");
    printf("  Incipit (Line 1):           \"%s\"\n", analysis.first_line);
    printf("\n  Full Un-redacted Page 1 Stream:\n--------------------------------------------------------------------\n%s\n--------------------------------------------------------------------\n", analysis.page_text);

    // 4. Audit Cryptographic DNA Hash Chain under Rule 11 & Rule 13
    int audit_rc = tsfi_vsen_audit_chain_verify("erara_page_text.dat.bin");
    assert(audit_rc == 0);

    printf("\n[E-RARA C TEXT ANALYSIS] Full un-redacted Page 1 C analysis completed successfully with ZERO errors!\n");
    return 0;
}
