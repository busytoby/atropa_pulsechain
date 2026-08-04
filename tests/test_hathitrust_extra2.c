#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_alto.h"
#include "inc/hathitrust_alto_consolidate.h"
#include "inc/hathitrust_speedometer.h"
#include "inc/hathitrust_marc.h"
#include "inc/hathitrust_marc_jsonld.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_alto_consolidation(void) {
    printf("[Test] Running ALTO OCR Text Consolidator tests...\n");
    
    HtrcAltoPage page;
    page.token_count = 3;
    page.tokens = malloc(3 * sizeof(HtrcAltoToken));
    
    page.tokens[0].content = strdup("Auncient");
    page.tokens[0].vpos = 100;
    
    page.tokens[1].content = strdup("Wavelets");
    page.tokens[1].vpos = 100; // Same line
    
    page.tokens[2].content = strdup("Lore");
    page.tokens[2].vpos = 200; // New line (vpos jump > 10)
    
    char *consolidated = hathitrust_alto_consolidate_text(&page);
    assert(consolidated != NULL);
    printf("[Test] Consolidated Text:\n%s\n", consolidated);
    
    assert(strstr(consolidated, "Auncient Wavelets") != NULL);
    assert(strstr(consolidated, "\nLore") != NULL);
    
    free(consolidated);
    hathitrust_alto_free(&page);
    printf("[Test] ALTO OCR Text Consolidator tests passed.\n");
}

static void test_speedometer(void) {
    printf("[Test] Running Speedometer tests...\n");
    
    HtrcSpeedometer speed;
    hathitrust_speedometer_init(&speed);
    
    // Simulate updating chunks
    hathitrust_speedometer_update(&speed, 1024, 4096);
    hathitrust_speedometer_update(&speed, 2048, 4096);
    hathitrust_speedometer_update(&speed, 1024, 4096);
    
    printf("[Test] Speedometer tests completed.\n");
}

static void test_marc_jsonld(void) {
    printf("[Test] Running MARC-to-JSON-LD Transcoder tests...\n");
    
    HtrcMarcBibliographic bib;
    bib.title = strdup("L'art mérovingien et carolingien");
    bib.author = strdup("Coutil, Léon");
    bib.pub_date = strdup("1930");
    bib.isbn = strdup("9780000000000");
    bib.oclc = strdup("(OCoLC)491914940");
    
    char *jsonld = hathitrust_marc_to_jsonld(&bib);
    assert(jsonld != NULL);
    printf("[Test] Generated JSON-LD:\n%s\n", jsonld);
    
    assert(strstr(jsonld, "\"@context\": \"http://schema.org\"") != NULL);
    assert(strstr(jsonld, "\"name\": \"L'art mérovingien et carolingien\"") != NULL);
    assert(strstr(jsonld, "\"datePublished\": \"1930\"") != NULL);
    assert(strstr(jsonld, "\"value\": \"(OCoLC)491914940\"") != NULL);
    
    free(jsonld);
    hathitrust_marc_free(&bib);
    printf("[Test] MARC-to-JSON-LD Transcoder tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust Consolidation, Speedometer, and JSON-LD unit tests...\n");
    test_alto_consolidation();
    test_speedometer();
    test_marc_jsonld();
    printf("[Test] All unit tests completed successfully.\n");
    return 0;
}
