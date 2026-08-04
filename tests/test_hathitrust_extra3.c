#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_mets.h"
#include "inc/hathitrust_iiif.h"
#include "inc/hathitrust_region.h"
#include "inc/hathitrust_hathifile.h"
#include "inc/hathitrust_hathifile_search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

static void test_iiif_transcoder(void) {
    printf("[Test] Running IIIF Transcoder tests...\n");
    
    HtrcMetsStructure mets;
    mets.volume_id = strdup("uc1.31970011037748");
    mets.page_count = 2;
    mets.pages = malloc(2 * sizeof(HtrcMetsPage));
    
    mets.pages[0].seq = 1;
    mets.pages[0].label = strdup("Page 1");
    mets.pages[0].file_id = strdup("FID1");
    
    mets.pages[1].seq = 2;
    mets.pages[1].label = strdup("Page 2");
    mets.pages[1].file_id = strdup("FID2");
    
    char *iiif = hathitrust_mets_to_iiif(&mets, "L'art mérovingien et carolingien");
    assert(iiif != NULL);
    printf("[Test] Generated IIIF Manifest Length: %zu bytes\n", strlen(iiif));
    
    assert(strstr(iiif, "\"@context\": \"http://iiif.io/api/presentation/2/context.json\"") != NULL);
    assert(strstr(iiif, "\"label\": \"L'art mérovingien et carolingien\"") != NULL);
    assert(strstr(iiif, "seq=1;size=100") != NULL);
    
    free(iiif);
    hathitrust_mets_free(&mets);
    printf("[Test] IIIF Transcoder tests passed.\n");
}

static void test_region_analyzer(void) {
    printf("[Test] Running Region Analyzer tests...\n");
    
    HtrcAltoPage p1;
    p1.token_count = 5;
    p1.tokens = malloc(5 * sizeof(HtrcAltoToken));
    for (int i = 0; i < 5; i++) {
        p1.tokens[i].content = strdup("Word");
    }
    
    // Low token count should resolve to Title Page
    HtrcStructuralRegionType r1 = hathitrust_region_classify(&p1);
    assert(r1 == REGION_TITLE_PAGE);
    hathitrust_alto_free(&p1);
    
    HtrcAltoPage p2;
    p2.token_count = 40;
    p2.tokens = malloc(40 * sizeof(HtrcAltoToken));
    for (int i = 0; i < 39; i++) {
        p2.tokens[i].content = strdup("Word");
    }
    p2.tokens[39].content = strdup("Index");
    
    // Index keywords should resolve to Index
    HtrcStructuralRegionType r2 = hathitrust_region_classify(&p2);
    assert(r2 == REGION_INDEX_BACK);
    hathitrust_alto_free(&p2);
    
    printf("[Test] Region Analyzer tests passed.\n");
}

static void test_hathifile_search(void) {
    printf("[Test] Running Hathifiles Binary Search tests...\n");
    
    const char *filepath = "/tmp/mock_hathifile.txt";
    FILE *f = fopen(filepath, "w");
    assert(f != NULL);
    
    // Write pre-sorted mock TSV entries (20 columns)
    // format: htid \t access \t rights \t bibkey \t desc \t source \t inst_status \t oclc \t isbn \t issn \t lccn \t title \t imprint \t reason \t timestamp \t gov \t date_used \t place \t lang \t fmt
    fprintf(f, "mdp.39015018415946\tshare\tpd\tgoogle\tdesc1\tsource1\tstatus1\t1001\tisbn1\tissn1\tlccn1\tSlovenské vyst̕ahovalectvo\timprint1\treason1\t1969\tgov1\tdate1\tplace1\tSlovak\ttext\n");
    fprintf(f, "nyp.33433082257217\tshare\tpd\tnypl\tdesc2\tsource2\tstatus2\t1002\tisbn2\tissn2\tlccn2\tThe Humming bird\timprint2\treason2\t1850\tgov2\tdate2\tplace2\tEnglish\ttext\n");
    fprintf(f, "uc1.31970011037748\tshare\tpd\tuc\tdesc3\tsource3\tstatus3\t1003\tisbn3\tissn3\tlccn3\tL'art mérovingien et carolingien\timprint3\treason3\t1930\tgov3\tdate3\tplace3\tFrench\ttext\n");
    fclose(f);
    
    HathifileRow rec;
    bool found = hathitrust_hathifile_binary_search(filepath, "nyp.33433082257217", &rec);
    assert(found);
    
    assert(strcmp(rec.htid, "nyp.33433082257217") == 0);
    assert(strcmp(rec.title, "The Humming bird") == 0);
    assert(strcmp(rec.rights_timestamp, "1850") == 0);
    assert(strcmp(rec.lang, "English") == 0);
    
    hathifile_free_row(&rec);
    unlink(filepath);
    
    printf("[Test] Hathifiles Binary Search tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust IIIF, Region, and Hathifile Search unit tests...\n");
    test_iiif_transcoder();
    test_region_analyzer();
    test_hathifile_search();
    printf("[Test] All unit tests completed successfully.\n");
    return 0;
}
