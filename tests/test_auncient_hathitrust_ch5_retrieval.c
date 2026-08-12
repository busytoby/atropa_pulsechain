#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("=== Auncient HathiTrust Full-Text Retrieval Test: Chapter 5 ===\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_vol1_path = "/tmp/opera_omnia_vol1_ch5_text.dat.bin";

    /* Create dummy .dat.bin asset (Rule 13 layout) */
    FILE *f = fopen(dat_bin_vol1_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    /* Retrieve Full Text for Chapter 5 */
    char text_buf[4096] = {0};
    AuncientHathitrustVolume1Chapter5Metrics ch5_metrics = {0};

    bool ok = auncient_hathitrust_volume1_chapter5_engine(
        contract_addr,
        dat_bin_vol1_path,
        text_buf,
        sizeof(text_buf),
        &ch5_metrics
    );

    assert(ok == true);
    assert(ch5_metrics.chapter5_engine_sound == true);
    assert(ch5_metrics.rule9_address_resolution_sound == true);
    assert(ch5_metrics.rule13_dat_bin_verified == true);
    assert(ch5_metrics.chapter_index == 5);
    assert(ch5_metrics.chapter_start_page == 129);
    assert(ch5_metrics.chapter_end_page == 160);

    printf("\n[PASS] Chapter 5 Full-Text Retrieved Successfully:\n");
    printf("%s\n", text_buf);
    printf("\n[METRICS] Pages %u-%u | Bytes: %u | HTID: %s | Latch: 0x%lX\n",
           ch5_metrics.chapter_start_page, ch5_metrics.chapter_end_page,
           ch5_metrics.ocr_text_bytes, ch5_metrics.htid, ch5_metrics.zmm_hardware_latch);

    remove(dat_bin_vol1_path);
    printf("=== HathiTrust Chapter 5 Full-Text Retrieval Test Passed ===\n");
    return 0;
}
