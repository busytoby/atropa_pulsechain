#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "tsfi_zorse_eval.h"

typedef struct {
    const char *title;
    uint32_t pub_year;
    uint32_t total_pages;
    const char *doi;
    const char *manifest;
} zwingli_work_t;

int main(void) {
    printf("[TEST E-RARA ZWINGLI SEARCH] Testing C author search for 'Zwingli' in Strict Chronological Order (1522–1545)...\n");

    // Clean initial test state
    unlink("erara_catalog.dat.bin");
    unlink("erara_catalog.wal.dat.bin");

    // 15 Rare Book Titles by Huldrych Zwingli sorted in strict chronological order by publication year (1522 to 1545)
    zwingli_work_t works[15] = {
        { "Von erkiesen vnd freyheit der speisen", 1522, 32, "10.3931/e-rara-10100", "https://www.e-rara.ch/i3f/v20/10100/manifest" },
        { "Eine göttliche ermahnung der von Schwyz", 1522, 24, "10.3931/e-rara-10200", "https://www.e-rara.ch/i3f/v20/10200/manifest" },
        { "Auslegen vnd gründe der schlussreden oder artiklen", 1523, 448, "10.3931/e-rara-10000", "https://www.e-rara.ch/i3f/v20/10000/manifest" },
        { "Der Hirt: Wie man die rechten hirten erkennenn soll", 1524, 48, "10.3931/e-rara-10600", "https://www.e-rara.ch/i3f/v20/10600/manifest" },
        { "De vera et falsa religione commentarius", 1525, 432, "10.3931/e-rara-10300", "https://www.e-rara.ch/i3f/v20/10300/manifest" },
        { "Von dem predigtampt, wie man das aufrichten soll", 1525, 36, "10.3931/e-rara-10700", "https://www.e-rara.ch/i3f/v20/10700/manifest" },
        { "Amica exegesis, id est expositio eucharistiae negotii", 1527, 216, "10.3931/e-rara-10800", "https://www.e-rara.ch/i3f/v20/10800/manifest" },
        { "Complanatio Isaiae prophetae, foetura prima", 1529, 340, "10.3931/e-rara-11300", "https://www.e-rara.ch/i3f/v20/11300/manifest" },
        { "Fidei ratio ad Carolum Imperatorem", 1530, 28, "10.3931/e-rara-10400", "https://www.e-rara.ch/i3f/v20/10400/manifest" },
        { "Ad illustrissimos Germaniae principes Augustae congregatos", 1530, 40, "10.3931/e-rara-11100", "https://www.e-rara.ch/i3f/v20/11100/manifest" },
        { "In evangelium de Jesu Christo secundum Matthaeum", 1531, 520, "10.3931/e-rara-10900", "https://www.e-rara.ch/i3f/v20/10900/manifest" },
        { "In evangelium secundum Joannem annotationes", 1533, 384, "10.3931/e-rara-11000", "https://www.e-rara.ch/i3f/v20/11000/manifest" },
        { "Christianae fidei brevis et clara expositio", 1536, 112, "10.3931/e-rara-10500", "https://www.e-rara.ch/i3f/v20/10500/manifest" },
        { "In historiam evangelicam de Jesu Christo annotatoria", 1539, 610, "10.3931/e-rara-11200", "https://www.e-rara.ch/i3f/v20/11200/manifest" },
        { "Operum Huldrychi Zvinglii tomus primus", 1545, 1180, "10.3931/e-rara-11400", "https://www.e-rara.ch/i3f/v20/11400/manifest" }
    };

    // 1. Register into C RDBMS storage in strict chronological order
    for (int i = 0; i < 15; i++) {
        assert(tsfi_erara_register_title(
            works[i].doi,
            works[i].title,
            "Huldrych Zwingli",
            works[i].pub_year,
            works[i].total_pages,
            works[i].manifest
        ) == 0);
    }

    // 2. Perform C Author Search for "Zwingli" querying top 15 results
    vsen_erara_title_record_t results[15];
    int count = 0;
    assert(tsfi_erara_search_by_author("Zwingli", results, 15, &count) == 0);
    assert(count == 15);

    printf("\n--- E-RARA.CH C AUTHOR SEARCH (STRICT CHRONOLOGICAL ORDER: 1522–1545) ---\n");
    for (int i = 0; i < count; i++) {
        printf("[%2d] [%u] Title:  %s\n", i + 1, results[i].pub_year, results[i].title);
        printf("          Author: %s | Pages: %u\n", results[i].author, results[i].total_pages);
        printf("          DOI:    %s | IIIF: %s\n\n", results[i].doi, results[i].iiif_manifest_url);

        // Assert chronological ordering
        if (i > 0) {
            assert(results[i].pub_year >= results[i - 1].pub_year);
        }
    }

    // 3. Audit Cryptographic DNA Hash Chain
    assert(tsfi_vsen_audit_chain_verify("erara_catalog.dat.bin") == 0);

    printf("[TEST E-RARA ZWINGLI SEARCH] Chronological order verification passed successfully!\n");
    return 0;
}
