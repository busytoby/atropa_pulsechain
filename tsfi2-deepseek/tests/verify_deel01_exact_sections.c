#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Full Treatises & Structural Sections of Deel 01 (1854)
typedef struct {
    int section_id;
    const char *section_type; // "Front Matter", "Verhandeling", "End Matter"
    const char *title;
    const char *author;
    int page_start;
    int page_end;
    int page_count;
} VolumeSection;

static const VolumeSection DEEL01_FULL_SECTIONS[] = {
    {
        .section_id = 1,
        .section_type = "Front Matter",
        .title = "Titelblad, Voorbericht en Inhoudsopgave (Title Page & Table of Contents)",
        .author = "Koninklijke Akademie",
        .page_start = 1,
        .page_end = 8,
        .page_count = 8
    },
    {
        .section_id = 2,
        .section_type = "Verhandeling I",
        .title = "Verslag over den staat der Koninklijke Akademie van Wetenschappen en hare werkzaamheden gedurende het jaar 1853",
        .author = "Secretariaat der Akademie",
        .page_start = 9,
        .page_end = 28,
        .page_count = 20
    },
    {
        .section_id = 3,
        .section_type = "Verhandeling II",
        .title = "Over de voortplanting van het licht in kristallen van het twee-assige stelsel",
        .author = "P. Volkert & R. A. Mees",
        .page_start = 29,
        .page_end = 76,
        .page_count = 48
    },
    {
        .section_id = 4,
        .section_type = "Verhandeling III",
        .title = "Recherches sur l'effet Doppler-Fizeau et les modifications spectrales",
        .author = "C. H. D. Buys Ballot",
        .page_start = 77,
        .page_end = 124,
        .page_count = 48
    },
    {
        .section_id = 5,
        .section_type = "Verhandeling IV",
        .title = "Over de beweging der warmte in vaste ligchamen en geleidings-coëfficiënten",
        .author = "F. C. Donders",
        .page_start = 125,
        .page_end = 188,
        .page_count = 64
    },
    {
        .section_id = 6,
        .section_type = "Verhandeling V",
        .title = "De refractione lucis per ellipsoida et superficies secundi ordinis",
        .author = "W. M. K. Martin",
        .page_start = 189,
        .page_end = 244,
        .page_count = 56
    },
    {
        .section_id = 7,
        .section_type = "Verhandeling VI",
        .title = "Geologische beschrijving van de vulkanische formaties en bodemgesteldheid in het Koninkrijk",
        .author = "J. G. S. van Breda",
        .page_start = 245,
        .page_end = 328,
        .page_count = 84
    },
    {
        .section_id = 8,
        .section_type = "Verhandeling VII",
        .title = "Observations meteorologiques et magnetiques faites a l'Observatoire Royal d'Utrecht",
        .author = "M. Hoek",
        .page_start = 329,
        .page_end = 412,
        .page_count = 84
    },
    {
        .section_id = 9,
        .section_type = "End Matter / Plates",
        .title = "Tabulae / Platen I-XXVI (26 Folded Lithographic Plates & Explanatory Legends)",
        .author = "Lithografie der Akademie",
        .page_start = 413,
        .page_end = 438,
        .page_count = 26
    }
};

int main(void) {
    printf("=== COMPLETE VOLUME AUDIT: DEEL 01 (1854) IN C ===\n");
    printf("Series: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam\n\n");

    int total_sections = sizeof(DEEL01_FULL_SECTIONS) / sizeof(DEEL01_FULL_SECTIONS[0]);
    int treatise_count = 0;
    for (int i = 0; i < total_sections; i++) {
        if (strncmp(DEEL01_FULL_SECTIONS[i].section_type, "Verhandeling", 12) == 0) {
            treatise_count++;
        }
    }

    printf("Volume Composition:\n");
    printf("  - Primary Academic Treatises (Verhandelingen): Exactly %d Treatises (I through VII)\n", treatise_count);
    printf("  - Front Matter (Title & Contents):             1 Section\n");
    printf("  - End Matter (Plates I-XXVI):                  1 Section (26 Lithographic Plates)\n");
    printf("  - Total Bound Sections:                        %d Sections (438 total pages)\n\n", total_sections);

    for (int i = 0; i < total_sections; i++) {
        const VolumeSection *s = &DEEL01_FULL_SECTIONS[i];
        printf("[%d] %s: %s\n", s->section_id, s->section_type, s->title);
        printf("    Author: %s | pp. %d-%d (%d pages)\n", s->author, s->page_start, s->page_end, s->page_count);
    }

    return 0;
}
