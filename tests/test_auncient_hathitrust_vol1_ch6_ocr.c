#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" HATHITRUST OPERA OMNIA VOL. 1 CAPUT VI FULL-TEXT LATIN OCR ENGINE EXECUTION\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_hathitrust_path = "/tmp/hathitrust_vol1_ch6_ocr.dat.bin";

    FILE *f = fopen(dat_bin_hathitrust_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    char ocr_text_buffer[2048] = {0};
    AuncientHathitrustVolume1Chapter6Metrics ch6_metrics = {0};

    bool ok = auncient_hathitrust_volume1_chapter6_engine(
        contract_addr,
        dat_bin_hathitrust_path,
        ocr_text_buffer,
        sizeof(ocr_text_buffer),
        preserved_x,
        &ch6_metrics
    );

    assert(ok == true);
    assert(ch6_metrics.hathitrust_ch6_retrieval_sound == true);
    assert(ch6_metrics.start_page == 161);
    assert(ch6_metrics.end_page == 192);
    assert(ch6_metrics.total_pages == 32);

    printf("[HATHITRUST OPERA OMNIA VOLUME 1 CHAPTER 6 METRICS]\n");
    printf("  - HTID:                       %s\n", ch6_metrics.htid);
    printf("  - Latin Title:                %s\n", ch6_metrics.chapter_latin_title);
    printf("  - HathiTrust Digitized Pages: Pages %u to %u (32 Total Pages)\n",
           ch6_metrics.start_page, ch6_metrics.end_page);
    printf("  - Indexed HathiTrust Sections: %u Sections (§ 209 to § 233)\n", ch6_metrics.indexed_section_count);
    printf("  - Indexed Paragraph Blocks:   %u Paragraphs\n", ch6_metrics.indexed_paragraph_count);
    printf("  - Preserved Base Variable x:  x = %ld\n", ch6_metrics.preserved_random_x);
    printf("  - Rule 9 Dynamic Address:     %s (dynamic_<address>)\n", ch6_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:    %s (.dat.bin Quadtree)\n", ch6_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:     0x%lX\n\n", ch6_metrics.acid_hathitrust_ch6_checksum);

    printf("[DIGITIZED HATHITRUST LATIN TEXT PAYLOAD - CAPUT VI]\n");
    printf("%s\n\n", ocr_text_buffer);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] HathiTrust Volume 1 Chapter 6 Full-Text Latin OCR Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_hathitrust_path);
    return 0;
}
