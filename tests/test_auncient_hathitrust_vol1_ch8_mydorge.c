#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" HATHITRUST VOL 1 CAPUT VIII: CLAUDIUS MYDORGIUS CONI SECTIO PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_hathitrust_path = "/tmp/hathitrust_vol1_ch8_mydorge.dat.bin";

    FILE *f = fopen(dat_bin_hathitrust_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    char ocr_text_buffer[2048] = {0};
    AuncientHathitrustVolume1Chapter8Metrics ch8_metrics = {0};

    bool ok = auncient_hathitrust_volume1_chapter8_engine(
        contract_addr,
        dat_bin_hathitrust_path,
        ocr_text_buffer,
        sizeof(ocr_text_buffer),
        preserved_x,
        preserved_y,
        &ch8_metrics
    );

    assert(ok == true);
    assert(ch8_metrics.hathitrust_ch8_retrieval_sound == true);

    printf("[HATHITRUST VOLUME 1 CHAPTER 8 METRICS - CLAUDIUS MYDORGIUS CONI SECTIO]\n");
    printf("  - HTID:                       %s\n", ch8_metrics.htid);
    printf("  - Latin Title:                %s\n", ch8_metrics.chapter_latin_title);
    printf("  - HathiTrust Digitized Pages: Pages %u to %u (32 Total Pages)\n",
           ch8_metrics.start_page, ch8_metrics.end_page);
    printf("  - Indexed Sections & Blocks:  %u Sections, %u Paragraphs\n",
           ch8_metrics.indexed_section_count, ch8_metrics.indexed_paragraph_count);
    printf("  - Preserved Random Pair:      (%ld, %ld)\n",
           ch8_metrics.preserved_random_x, ch8_metrics.preserved_random_y);
    printf("  - Rule 9 Dynamic Address:     %s (dynamic_<address>)\n",
           ch8_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:    %s (.dat.bin Quadtree)\n",
           ch8_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:     0x%lX\n\n", ch8_metrics.acid_hathitrust_ch8_checksum);

    printf("[DIGITIZED LATIN TEXT PAYLOAD - CLAUDIUS MYDORGIUS CONI SECTIO]\n");
    printf("%s\n\n", ocr_text_buffer);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Chapter 8 (Claudius Mydorgius Coni Sectio) Latin OCR sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_hathitrust_path);
    return 0;
}
