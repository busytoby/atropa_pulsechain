#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[E-RARA EBCDIC C BACKEND] Retrieving raw EBCDIC Code Page 037 backend page text in C...\n");

    // Clean initial test state
    unlink("erara_ebcdic_page_text.dat.bin");
    unlink("erara_ebcdic_page_text.wal.dat.bin");

    // Full un-redacted Page 1 text stream
    const char *page1_stream = 
        "Von erkiesen vnd freyheit der speisen. Von aergerniss vnd verbesserung.\n"
        "Ob jemants gewalt habe verbyeten etliche speiss zu eessen zu etlicher zeyt.\n"
        "Huldrychen Zwinglis predig gethan zu Zuerich in der grossen kirchen am .xxiij. tag maertzen .M.D.XXII.\n"
        "Dem eersamen vnd wysen Chratz Christoffel Froschouer Buchtrucker zu Zuerich mynem besonderen gunstigen herren vnd freundt.\n"
        "Gnad vnd frid von Gott dem Herren. Diewyl vil menschen sich aergern dadvon das etliche hinder der fasnacht vnd in der fastenn fleisch gessen haben,\n"
        "hab ich fuer gut angesehen die predig so ich gemeynlich gethan habe von freyheit der speisen in truck zu geben,\n"
        "damit menklich wysse wie die sach von Gottes wort geschirmt vnd erhalten werde.";

    // 1. Register Page 1 text into C RDBMS storage in raw EBCDIC Code Page 037 encoding
    assert(tsfi_erara_register_ebcdic_page_text("10.3931/e-rara-10100", 1, page1_stream) == 0);

    // 2. Retrieve raw EBCDIC backend record from RDBMS storage (.dat.bin)
    vsen_erara_ebcdic_page_text_t ebcdic_rec;
    assert(tsfi_erara_get_ebcdic_page_text("10.3931/e-rara-10100", 1, &ebcdic_rec) == 0);

    printf("\n================ RAW EBCDIC CODE PAGE 037 BACKEND PAYLOAD ================\n");
    printf("  Target DOI:         %s (Page %u)\n", ebcdic_rec.doi, ebcdic_rec.page_num);
    printf("  EBCDIC Byte Count:  %u bytes\n", ebcdic_rec.ebcdic_byte_count);

    // Print first 64 bytes of raw EBCDIC hexadecimal payload stream
    printf("\n  Raw EBCDIC Hex Stream (First 64 Bytes):\n  ");
    for (uint32_t i = 0; i < 64 && i < ebcdic_rec.ebcdic_byte_count; i++) {
        printf("%02X ", ebcdic_rec.ebcdic_payload[i]);
        if ((i + 1) % 16 == 0) printf("\n  ");
    }
    printf("\n");

    // 3. Translate raw EBCDIC payload back to ASCII stream to verify zero-loss roundtrip
    char ascii_reconstructed[4096];
    memset(ascii_reconstructed, 0, sizeof(ascii_reconstructed));
    tsfi_ebcdic_cp037_to_ascii(ebcdic_rec.ebcdic_payload, ascii_reconstructed, ebcdic_rec.ebcdic_byte_count);

    printf("\n  Reconstructed ASCII Stream from Raw EBCDIC Backend:\n--------------------------------------------------------------------\n%s\n--------------------------------------------------------------------\n", ascii_reconstructed);

    // Assert exact byte stream match
    assert(ebcdic_rec.ebcdic_byte_count == strlen(page1_stream));
    assert(strncmp(ascii_reconstructed, page1_stream, ebcdic_rec.ebcdic_byte_count) == 0);

    // 4. Audit Cryptographic DNA Hash Chain on EBCDIC storage
    assert(tsfi_vsen_audit_chain_verify("erara_ebcdic_page_text.dat.bin") == 0);

    printf("\n[E-RARA EBCDIC C BACKEND] EBCDIC backend page text retrieval and roundtrip verification passed cleanly!\n");
    return 0;
}
