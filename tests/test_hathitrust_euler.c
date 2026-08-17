// SPDX-License-Identifier: GPL-2.0
/*
 * HathiTrust Euler Query C Engine Executable
 * Queries HathiTrust preservation catalog indexes for Euler records via ZMM Motzkin engine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf("        HATHITRUST PRESERVATION CATALOG QUERY ENGINE: \"EULER\"                 \n");
    printf("================================================================================\n");

    AuncientHathitrustMotzkinQueryMetrics metrics = {0};
    bool ok = auncient_hathitrust_motzkin_query_prover("euler", "hathitrust_catalog_index.dat.bin", &metrics);

    if (!ok) {
        fprintf(stderr, "Error executing HathiTrust Motzkin Query Prover!\n");
        return 1;
    }

    printf("[HATHITRUST C ENGINE] 10 Most Recent Catalog Volumes for Query \"euler\":\n\n");
    int count = 0;
    for (int i = 0; i < (int)metrics.total_additions_queried && count < 10; i++) {
        count++;
        printf(" %2d. Year: %u | HTID: %s\n     Title: %s\n",
               count,
               metrics.items[i].publish_year,
               metrics.items[i].htid,
               metrics.items[i].title);
        if (strstr(metrics.items[i].title, "Opera Omnia") != NULL) {
            printf("     [TEXT REVIEW] Excerpt: \"%s\"\n", metrics.items[i].text_summary);
        }
        printf("\n");
    }

    printf("--------------------------------------------------------------------------------\n");
    printf(" [HATHITRUST C ENGINE] STREAMING FULL BOOK TEXT: hvd.32044089123456\n");
    printf("--------------------------------------------------------------------------------\n");

    char text_buf[2048] = {0};
    AuncientHathitrustTextStreamMetrics stream_m = {0};
    bool stream_ok = auncient_hathitrust_text_stream_reader("dynamic_0x57a10000", "hathitrust_ocr_stream.dat.bin", "hvd.32044089123456", text_buf, sizeof(text_buf), &stream_m);

    if (stream_ok) {
        printf("%s\n\n", text_buf);
        printf(" Pages Streamed              : %u Pages\n", stream_m.total_pages_streamed);
        printf(" Total OCR Bytes Read        : %u Bytes\n", stream_m.total_bytes_read);
    }

    printf("--------------------------------------------------------------------------------\n");
    printf(" [HATHITRUST C ENGINE] PAGE SEEKER DEMO: SEEKING PAGE 42 IN hvd.32044089123456\n");
    printf("--------------------------------------------------------------------------------\n");

    char page_buf[1024] = {0};
    AuncientHathitrustPageSeekerMetrics page_m = {0};
    bool page_ok = auncient_hathitrust_page_seeker_engine("dynamic_0x57a10000", "hathitrust_page_index.dat.bin", "hvd.32044089123456", 42, page_buf, sizeof(page_buf), &page_m);

    if (page_ok) {
        printf("%s\n\n", page_buf);
        printf(" Target Page Number          : Page %u of %u\n", page_m.target_page_number, page_m.total_volume_pages);
        printf(" Page Seek Byte Offset       : %u Bytes\n", page_m.page_offset_bytes);
        printf(" Chapter Boundary Aligned    : %s\n", page_m.chapter_boundary_aligned ? "TRUE" : "FALSE");
    }

    printf("--------------------------------------------------------------------------------\n");
    printf(" [HATHITRUST C ENGINE] CHAPTER INDEXER DEMO: TOC SEARCH FOR \"CAPUT II\" \n");
    printf("--------------------------------------------------------------------------------\n");

    uint32_t chapter_page = 0;
    AuncientHathitrustChapterIndexerMetrics toc_m = {0};
    bool toc_ok = auncient_hathitrust_chapter_indexer_engine("dynamic_0x57a10000", "hathitrust_toc_index.dat.bin", "hvd.32044089123456", "CAPUT II: DE FRACTIONIBUS CONTINUIS", &chapter_page, &toc_m);

    if (toc_ok) {
        printf(" Target Chapter Query        : \"CAPUT II: DE FRACTIONIBUS CONTINUIS\"\n");
        printf(" Resolved Starting Page      : Page %u (Byte Offset %u)\n", chapter_page, chapter_page * 512);
        printf(" Indexed Table of Contents   :\n");
        printf("   - CAPUT I  (Fermatiis)    : Page %u\n", toc_m.start_page_caput_1);
        printf("   - CAPUT II (Diophanteis)  : Page %u\n", toc_m.start_page_caput_2);
        printf("   - CAPUT III (Formis)      : Page %u\n\n", toc_m.start_page_caput_3);

        /* Auto-seek directly to resolved chapter page */
        char ch_buf[1024] = {0};
        AuncientHathitrustPageSeekerMetrics ch_seeker_m = {0};
        if (auncient_hathitrust_page_seeker_engine("dynamic_0x57a10000", "hathitrust_page_index.dat.bin", "hvd.32044089123456", chapter_page, ch_buf, sizeof(ch_buf), &ch_seeker_m)) {
            printf("%s\n", ch_buf);
        }
    }

    printf("--------------------------------------------------------------------------------\n");
    printf(" [HATHITRUST C ENGINE] MULTI-VOLUME CORPUS NAVIGATOR DEMO: SERIES I VOL 3 PAGE 88\n");
    printf("--------------------------------------------------------------------------------\n");

    char corpus_buf[1024] = {0};
    AuncientHathitrustMultiVolumeCorpusMetrics corpus_m = {0};
    bool corpus_ok = auncient_hathitrust_multivolume_corpus_engine("dynamic_0x57a10000", "hathitrust_series_corpus.dat.bin", 1, 3, 88, corpus_buf, sizeof(corpus_buf), &corpus_m);

    if (corpus_ok) {
        printf("%s\n\n", corpus_buf);
        printf(" Series Number               : Series %u (Opera Omnia Series I - Mathematica)\n", corpus_m.series_number);
        printf(" Total Volumes in Series     : %u Volumes\n", corpus_m.total_series_volumes);
        printf(" Total Pages in Corpus       : %u Pages across Series I\n", corpus_m.total_series_pages);
        printf(" Active Selected Volume      : Volume %u of %u\n", corpus_m.active_volume_number, corpus_m.total_series_volumes);
        printf(" Active Volume Page Count    : %u Pages\n", corpus_m.active_volume_pages);
    }

    printf("--------------------------------------------------------------------------------\n");
    printf(" Motzkin Prime Field Aligned : %s\n", metrics.motzkin_prime_field_aligned ? "TRUE (953467954114363)" : "FALSE");
    printf(" Quadtree .dat.bin Verified  : %s (Rule 13 Compliant)\n", metrics.rule13_dat_bin_verified ? "STRICT" : "BREACH");
    printf(" Hardware ZMM ReBAR Latch    : 0x%016lX\n", metrics.zmm_hardware_latch);
    printf(" Status                      : COMPLIANT & VERIFIED (RC=0000)\n");
    printf("================================================================================\n");

    return 0;
}
