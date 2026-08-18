/* SPDX-License-Identifier: GPL-2.0 */
/*
 * C11 Test Prover: CP/M-Tomie Integrated ANNSIM SimAUD RenderMan PageTurner Pipeline
 */

#include "cpm_tomie_pageturner_renderman.h"
#include "cpm_tomie_annsim_reviewer.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

extern bool cpm_tomie_annsim_pageturner_pipeline_run(const char *pdf_path, void *pipe_out);

typedef struct {
    PageTurnerRenderManEngine engine;
    CpmTomieAnnSimReviewRecord record;
    bool pipeline_verified;
    uint32_t grand_checksum;
} CpmTomieAnnSimPageTurnerPipeline;

int main(void) {
    const char *pdf_path = "/home/mariarahel/Downloads/ANNSIM.2025.SimAUD.58.pdf";

    printf("=================================================================\n");
    printf("TEST PROVER: CP/M-TOMIE ANNSIM SIMAUD RENDERMAN PAGETURNER PIPE  \n");
    printf("=================================================================\n");

    CpmTomieAnnSimPageTurnerPipeline pipe;
    bool ok = cpm_tomie_annsim_pageturner_pipeline_run(pdf_path, &pipe);
    assert(ok);
    assert(pipe.pipeline_verified);
    assert(pipe.engine.total_pages == 4);
    assert(pipe.engine.pages[0].num_vertices == 256);
    assert(pipe.grand_checksum > 0);
    assert(pipe.record.living_lab_mentions >= 40);
    assert(pipe.record.digital_twin_mentions >= 15);

    printf(" Multi-Phase Ingestion & Review: PASS (Streams: %u, Bytes: %u)\n",
           pipe.record.total_streams, pipe.record.text_bytes_extracted);
    printf(" Living Lab Digital Twin Domain: PASS (Mentions: %u, BAS: %u)\n",
           pipe.record.living_lab_mentions, pipe.record.honeywell_bas_mentions);
    printf(" RenderMan PageTurner Engine:    PASS (Pages: %u, Plasticity: %.2f)\n",
           pipe.engine.total_pages, pipe.engine.clay_plasticity);
    printf(" Integrated Rule 18 Checksum:    PASS (0x%08X Verified)\n", pipe.grand_checksum);
    printf("=================================================================\n");
    printf("ANNSIM SIMAUD RENDERMAN PAGETURNER PIPELINE VERIFIED SUCCESSFULLY.\n");
    printf("=================================================================\n");
    return 0;
}
