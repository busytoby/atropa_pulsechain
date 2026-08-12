#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" HATHITRUST OPERA OMNIA VOL 1 CH 6 PARAGRAPH SCANNER & VERIFICATION\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_toc_path = "/tmp/hathitrust_vol1_ch6_scan.dat.bin";

    FILE *f = fopen(dat_bin_toc_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    uint32_t start_p = 0;
    AuncientHathitrustChapterIndexerMetrics index_metrics = {0};

    bool ok = auncient_hathitrust_chapter_indexer_engine(
        contract_addr,
        dat_bin_toc_path,
        "hvd.32044089123456",
        "Caput VI: De logarithmis in genere",
        &start_p,
        &index_metrics
    );

    assert(ok == true);

    printf("[HATHITRUST TABLE OF CONTENTS DISCOVERY]\n");
    printf("  - Target HTID:             %s\n", index_metrics.htid);
    printf("  - Chapter Query:           Caput VI: De logarithmis in genere\n");
    printf("  - Discovered Start Page:   Page %u\n", start_p);
    printf("  - Rule 9 Dynamic Address:  %s\n", index_metrics.rule9_address_resolution_sound ? "YES" : "NO");
    printf("  - Rule 13 .dat.bin Format: %s\n\n", index_metrics.rule13_dat_bin_verified ? "YES" : "NO");

    printf("================================================================================\n");
    printf(" [SCAN COMPLETE] Initiating HathiTrust OCR stream parser for exact section count.\n");
    printf("================================================================================\n");

    remove(dat_bin_toc_path);
    return 0;
}
