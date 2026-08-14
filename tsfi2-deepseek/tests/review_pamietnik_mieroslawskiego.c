#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Material record for Pamiętnik Mierosławskiego (1861-1863), edited by Józef Frejlich
typedef struct {
    const char *title;
    const char *author;
    const char *editor;
    const char *coverage_period;
    uint32_t publication_year;
    const char *publication_place;
    const char *publisher;
    const char *language;
    const char *significance;
    const char *holding_repositories;
} PamietnikMieroslawskiegoRecord;

static const PamietnikMieroslawskiegoRecord MIEROSLAWSKI_RECORD = {
    .title = "Pamiętnik Mierosławskiego (1861-1863)",
    .author = "Ludwik Mierosławski (1814–1878)",
    .editor = "Józef Frejlich (1884–1956)",
    .coverage_period = "1861–1863 (January Uprising & Political-Military Mechanics)",
    .publication_year = 1924,
    .publication_place = "Warszawa / Kraków",
    .publisher = "Wydawnictwo Kasy im. Mianowskiego / Instytut Badania Najnowszej Historji Polski",
    .language = "Polish (Polski)",
    .significance = "Primary historical, military-tactical, and political compendium documenting Mierosławski's operational treatises and command during the 1863 Uprising",
    .holding_repositories = "Biblioteka Narodowa w Warszawie / Jagiellonian Library / HathiTrust / Harvard"
};

int main(void) {
    printf("=== CANONICAL MATERIAL REVIEW: PAMIĘTNIK MIEROSŁAWSKIEGO (1861-1863) ===\n\n");
    const PamietnikMieroslawskiegoRecord *r = &MIEROSLAWSKI_RECORD;

    printf("Title:        %s\n", r->title);
    printf("Author:       %s\n", r->author);
    printf("Editor:       %s\n", r->editor);
    printf("Scope Period: %s\n", r->coverage_period);
    printf("Published:    %s, %u (%s)\n", r->publication_place, r->publication_year, r->publisher);
    printf("Language:     %s\n", r->language);
    printf("Significance: %s\n", r->significance);
    printf("Repositories: %s\n\n", r->holding_repositories);

    printf("--- STRUCTURAL COMPENDIUM ANALYSIS ---\n");
    printf("1. Political and Military Doctrine: Critical examination of insurgent mechanics and military engineering.\n");
    printf("2. Critical Edition Apparatus: Józef Frejlich's rigorous archival annotations, collation of manuscripts, and historical notes.\n");
    printf("3. Historical Synthesis: Documents the strategic coordination between Polish diaspora commands and local revolutionary committees.\n\n");

    return 0;
}
