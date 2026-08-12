#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("=== Auncient HathiTrust Opera Omnia Reader Test ===\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_vol1_path = "/tmp/opera_omnia_vol1.dat.bin";

    /* Create dummy .dat.bin asset (Rule 13 layout) */
    FILE *f = fopen(dat_bin_vol1_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[1024] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    /* 1. Test Ingestion Engine */
    AuncientHathitrustVolume1IngestionMetrics ing_metrics = {0};
    bool ing_ok = auncient_hathitrust_volume1_ingestion_engine(
        contract_addr,
        dat_bin_vol1_path,
        &ing_metrics
    );
    assert(ing_ok == true);
    assert(ing_metrics.rule9_address_resolution_sound == true);
    assert(ing_metrics.rule13_dat_bin_verified == true);
    assert(ing_metrics.total_pages_ingested == 512);

    printf("[PASS] Volume 1 Ingestion Engine Verified. Total Pages: %u, HTID: %s\n",
           ing_metrics.total_pages_ingested, ing_metrics.htid);

    /* 2. Review Text of Selected Pages in Opera Omnia Volume 1 */
    uint32_t pages_to_read[] = { 1, 150, 300 };
    for (size_t i = 0; i < sizeof(pages_to_read)/sizeof(pages_to_read[0]); i++) {
        uint32_t page_num = pages_to_read[i];
        char text_buf[2048] = {0};
        AuncientHathitrustVolume1PageReaderMetrics reader_metrics = {0};

        bool read_ok = auncient_hathitrust_volume1_page_reader_engine(
            contract_addr,
            dat_bin_vol1_path,
            page_num,
            text_buf,
            sizeof(text_buf),
            &reader_metrics
        );
        assert(read_ok == true);
        assert(reader_metrics.volume1_page_reader_sound == true);

        printf("\n--- Reading Page %u (%s) ---\n", page_num, reader_metrics.chapter_title);
        printf("%s\n", text_buf);
    }

    remove(dat_bin_vol1_path);
    printf("\n=== All HathiTrust Opera Omnia Reader Tests Passed ===\n");
    return 0;
}
