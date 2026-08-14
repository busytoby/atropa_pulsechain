#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Dedicated Materials Catalog for Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam
typedef struct {
    uint32_t series_id;
    const char *series_title;
    const char *holding_library;
    const char *volume_label;
    uint32_t year_start;
    uint32_t year_end;
    uint32_t total_volumes;
    const char *htid_prefix;
} KoninklijkeAkademieSeries;

static const KoninklijkeAkademieSeries AKADEMIE_SERIES_CATALOG[] = {
    {
        .series_id = 1,
        .series_title = "Verhandelingen der Koninklijke Akademie van Wetenschappen (Original Series)",
        .holding_library = "Universiteit Leiden / Princeton / Harvard",
        .volume_label = "Deel 1 - Deel 29",
        .year_start = 1854,
        .year_end = 1892,
        .total_volumes = 29,
        .htid_prefix = "njp / hvd / mdp"
    },
    {
        .series_id = 2,
        .series_title = "Verhandelingen der Koninklijke Akademie van Wetenschappen. Afdeeling Natuurkunde (Eerste Sectie: Wiskunde, Natuurkunde, Scheikunde, Sterrenkunde, Geologie)",
        .holding_library = "University of Michigan / University of Illinois",
        .volume_label = "Deel 1 - Deel 23",
        .year_start = 1892,
        .year_end = 1940,
        .total_volumes = 23,
        .htid_prefix = "mdp / uiug"
    },
    {
        .series_id = 3,
        .series_title = "Verhandelingen der Koninklijke Akademie van Wetenschappen. Afdeeling Natuurkunde (Tweede Sectie: Plantkunde, Dierkunde, Geneeskunde)",
        .holding_library = "New York Public Library / Harvard",
        .volume_label = "Deel 1 - Deel 45",
        .year_start = 1892,
        .year_end = 1948,
        .total_volumes = 45,
        .htid_prefix = "nyp / hvd"
    },
    {
        .series_id = 4,
        .series_title = "Verslagen van de Gewone Vergaderingen der Wis- en Natuurkundige Afdeeling",
        .holding_library = "University of California / Harvard",
        .volume_label = "Deel 1 - Deel 50",
        .year_start = 1892,
        .year_end = 1942,
        .total_volumes = 50,
        .htid_prefix = "uc1 / hvd"
    },
    {
        .series_id = 5,
        .series_title = "Proceedings of the Section of Sciences (English Translations of Key Treatises)",
        .holding_library = "Koninklijke Akademie / HathiTrust Digital Archive",
        .volume_label = "Vol 1 - Vol 45",
        .year_start = 1898,
        .year_end = 1942,
        .total_volumes = 45,
        .htid_prefix = "uva / ind / pur"
    }
};

int main(void) {
    printf("=== OFFICIAL SERIAL & MATERIALS CATALOG IN C ===\n");
    printf("Subject: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam\n\n");

    size_t count = sizeof(AKADEMIE_SERIES_CATALOG) / sizeof(AKADEMIE_SERIES_CATALOG[0]);
    for (size_t i = 0; i < count; i++) {
        const KoninklijkeAkademieSeries *s = &AKADEMIE_SERIES_CATALOG[i];
        printf("[Series %u]\n", s->series_id);
        printf("  Title:      %s\n", s->series_title);
        printf("  Volumes:    %s (%u - %u, %u Total Volumes)\n", s->volume_label, s->year_start, s->year_end, s->total_volumes);
        printf("  Holdings:   %s\n", s->holding_library);
        printf("  Holders:    %s\n\n", s->htid_prefix);
    }

    return 0;
}
