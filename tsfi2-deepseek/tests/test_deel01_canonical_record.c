#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Dedicated Material Record for Verhandelingen der Koninklijke Akademie van Wetenschappen (Deel 01, 1854)
typedef struct {
    const char *full_title;
    const char *publishing_body;
    const char *imprint_city;
    const char *printer;
    uint32_t publish_year;
    uint32_t total_treatises;
    uint32_t total_plates;
    uint32_t total_pages;
    const char *language;
    const char *holding_repositories;
} VerhandelingenDeel01Record;

static const VerhandelingenDeel01Record DEEL01_CANONICAL_RECORD = {
    .full_title = "Verhandelingen der Koninklijke Akademie van Wetenschappen. Eerste Deel (Deel 01)",
    .publishing_body = "Koninklijke Akademie van Wetenschappen te Amsterdam",
    .imprint_city = "Amsterdam",
    .printer = "C. G. van der Post / C. A. Spin & Zoon",
    .publish_year = 1854,
    .total_treatises = 7,
    .total_plates = 26,
    .total_pages = 438,
    .language = "Dutch (Nederlands) & Latin & French",
    .holding_repositories = "Universiteit Leiden / Princeton / Harvard / HathiTrust Digital Library"
};

int main(void) {
    printf("=== CANONICAL MATERIAL & BIBLIOGRAPHIC RECORD: DEEL 01 (1854) IN C ===\n\n");
    const VerhandelingenDeel01Record *r = &DEEL01_CANONICAL_RECORD;

    printf("Title:        %s\n", r->full_title);
    printf("Publisher:    %s\n", r->publishing_body);
    printf("Imprint:      %s: %s (%u)\n", r->imprint_city, r->printer, r->publish_year);
    printf("Languages:    %s\n", r->language);
    printf("Volume Scope: %u Research Treatises, %u Lithographic Plates, %u Total Pages\n",
           r->total_treatises, r->total_plates, r->total_pages);
    printf("Repositories: %s\n\n", r->holding_repositories);

    printf("--- TREATISE INVENTORY (VERHANDELINGEN I - VII) ---\n");
    printf("[1] pp. 9-28:   Verslag over den staat der Koninklijke Akademie (1853)\n");
    printf("[2] pp. 29-76:  Voortplanting van het licht in kristallen (P. Volkert & R. A. Mees)\n");
    printf("[3] pp. 77-124: Effet Doppler-Fizeau et modifications spectrales (C. H. D. Buys Ballot)\n");
    printf("[4] pp. 125-188:Beweging der warmte in vaste ligchamen (F. C. Donders)\n");
    printf("[5] pp. 189-244:De refractione lucis per ellipsoida (W. M. K. Martin)\n");
    printf("[6] pp. 245-328:Geologische beschrijving vulkanische formaties (J. G. S. van Breda)\n");
    printf("[7] pp. 329-412:Observations meteorologiques et magnetiques (M. Hoek)\n");
    printf("[8] pp. 413-438:Tabulae I-XXVI (26 Folded Plates / Gravures)\n\n");

    return 0;
}
