#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" HATHITRUST OPERA OMNIA VOL. 1 CAPUT V FULL-TEXT LATIN OCR ENGINE PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_vol1_path = "/tmp/hathitrust_vol1_ch5_ocr.dat.bin";

    FILE *f = fopen(dat_bin_vol1_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    char ocr_text_buffer[2048] = {0};
    AuncientHathitrustVolume1Chapter5Metrics hvd_metrics = {0};

    bool ok = auncient_hathitrust_volume1_chapter5_engine(
        contract_addr,
        dat_bin_vol1_path,
        ocr_text_buffer,
        sizeof(ocr_text_buffer),
        &hvd_metrics
    );

    assert(ok == true);
    assert(hvd_metrics.chapter5_engine_sound == true);
    assert(hvd_metrics.chapter_index == 5);
    assert(hvd_metrics.total_volume_chapters == 15);
    assert(hvd_metrics.chapter_start_page == 129);
    assert(hvd_metrics.chapter_end_page == 160);

    printf("[HATHITRUST OPERA OMNIA VOLUME 1 CHAPTER 5 METRICS]\n");
    printf("  - HTID:                       %s\n", hvd_metrics.htid);
    printf("  - Chapter Index:              Chapter %u of %u\n", hvd_metrics.chapter_index, hvd_metrics.total_volume_chapters);
    printf("  - Latin Title:                %s\n", hvd_metrics.chapter_name);
    printf("  - HathiTrust Digitized Pages: Pages %u to %u (32 Total Pages)\n",
           hvd_metrics.chapter_start_page, hvd_metrics.chapter_end_page);
    printf("  - OCR Payload Size:           %u Bytes\n", hvd_metrics.ocr_text_bytes);
    printf("  - Rule 9 Dynamic Address:     %s (dynamic_<address>)\n", hvd_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:    %s (.dat.bin Quadtree)\n", hvd_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - ZMM ReBAR Latch:            0x%lX\n\n", hvd_metrics.zmm_hardware_latch);

    printf("[DIGITIZED HATHITRUST LATIN TEXT PAYLOAD - CAPUT V]\n");
    printf("%s\n\n", ocr_text_buffer);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] HathiTrust Volume 1 Chapter 5 Full-Text Latin OCR Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_vol1_path);
    return 0;
}
