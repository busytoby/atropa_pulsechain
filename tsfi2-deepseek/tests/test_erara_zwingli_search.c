#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "tsfi_zorse_eval.h"

int main(void) {
    printf("[TEST E-RARA ZWINGLI SEARCH] Testing C author search for 'Zwingli' (first 15 titles)...\n");

    // Clean initial test state
    unlink("erara_catalog.dat.bin");
    unlink("erara_catalog.wal.dat.bin");

    // 1. Register 15 digitized rare book titles by Huldrych Zwingli in C RDBMS storage
    const char *zwingli_titles[15] = {
        "Auslegen vnd gründe der schlussreden oder artiklen",
        "Von erkiesen vnd freyheit der speisen",
        "Eine göttliche ermahnung der von Schwyz",
        "De vera et falsa religione commentarius",
        "Fidei ratio ad Carolum Imperatorem",
        "Christianae fidei brevis et clara expositio",
        "Der Hirt: Wie man die rechten hirten erkennenn soll",
        "Von dem predigtampt, wie man das aufrichten soll",
        "Amica exegesis, id est expositio eucharistiae negotii",
        "In evangelium de Jesu Christo secundum Matthaeum",
        "In evangelium secundum Joannem annotationes",
        "Ad illustrissimos Germaniae principes Augustae congregatos",
        "In historiam evangelicam de Jesu Christo annotatoria",
        "Complanatio Isaiae prophetae, foetura prima",
        "Operum Huldrychi Zvinglii tomus primus"
    };

    uint32_t pub_years[15] = {1523, 1522, 1522, 1525, 1530, 1536, 1524, 1525, 1527, 1531, 1533, 1530, 1539, 1529, 1545};
    uint32_t total_pages[15] = {448, 32, 24, 432, 28, 112, 48, 36, 216, 520, 384, 40, 610, 340, 1180};

    for (int i = 0; i < 15; i++) {
        char doi_buf[128];
        char manifest_buf[256];
        snprintf(doi_buf, sizeof(doi_buf), "10.3931/e-rara-%d", 10000 + i * 100);
        snprintf(manifest_buf, sizeof(manifest_buf), "https://www.e-rara.ch/i3f/v20/%d/manifest", 10000 + i * 100);

        assert(tsfi_erara_register_title(
            doi_buf,
            zwingli_titles[i],
            "Huldrych Zwingli",
            pub_years[i],
            total_pages[i],
            manifest_buf
        ) == 0);
    }

    // 2. Perform C Author Search for "Zwingli" querying top 15 results
    vsen_erara_title_record_t results[15];
    int count = 0;
    assert(tsfi_erara_search_by_author("Zwingli", results, 15, &count) == 0);
    assert(count == 15);

    printf("\n--- E-RARA.CH C AUTHOR SEARCH RESULTS FOR 'Zwingli' (%d TITLES FOUND) ---\n", count);
    for (int i = 0; i < count; i++) {
        printf("[%2d] Title:    %s\n", i + 1, results[i].title);
        printf("     Author:   %s (%u)\n", results[i].author, results[i].pub_year);
        printf("     DOI:      %s\n", results[i].doi);
        printf("     Pages:    %u | IIIF: %s\n\n", results[i].total_pages, results[i].iiif_manifest_url);
    }

    // 3. Audit Cryptographic DNA Hash Chain
    assert(tsfi_vsen_audit_chain_verify("erara_catalog.dat.bin") == 0);

    printf("[TEST E-RARA ZWINGLI SEARCH] C author search test passed successfully!\n");
    return 0;
}
