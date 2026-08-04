#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_mets_range.h"
#include "inc/hathitrust_solr_facet.h"
#include "inc/hathitrust_hathifile_diff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_mets_range_mapper(void) {
    printf("[Test] Running METS-to-IIIF Logical Range mapper tests...\n");
    
    char *json = hathitrust_mets_map_range("Chapter 1", 1, 3);
    assert(json != NULL);
    printf("[Test] Generated Range JSON:\n%s\n", json);
    
    assert(strstr(json, "\"label\": \"Chapter 1\"") != NULL);
    assert(strstr(json, "\"http://example.org/iiif/canvas/p1\"") != NULL);
    assert(strstr(json, "\"http://example.org/iiif/canvas/p3\"") != NULL);
    
    free(json);
    printf("[Test] METS-to-IIIF Logical Range mapper tests passed.\n");
}

static void test_solr_facet_compiler(void) {
    printf("[Test] Running Solr Facet Query Compiler tests...\n");
    
    char *url = hathitrust_solr_compile_facet_query("carolingien", "publisher");
    assert(url != NULL);
    printf("[Test] Generated Facet URL: %s\n", url);
    
    assert(strstr(url, "q=carolingien") != NULL);
    assert(strstr(url, "facet=true") != NULL);
    assert(strstr(url, "facet.field=publisher") != NULL);
    
    free(url);
    printf("[Test] Solr Facet Query Compiler tests passed.\n");
}

static void test_hathifile_diff(void) {
    printf("[Test] Running Hathifiles Dataset Diff Engine tests...\n");
    
    const char *old_file = "/tmp/old_hathifile.tsv";
    const char *new_file = "/tmp/new_hathifile.tsv";
    const char *diff_file = "/tmp/diff_hathifile.txt";
    
    FILE *fo = fopen(old_file, "w");
    fprintf(fo, "uc1.100\tpd\t1900\n");
    fclose(fo);
    
    FILE *fn = fopen(new_file, "w");
    fprintf(fn, "uc1.100\tpd\t1900\n");
    fprintf(fn, "uc1.200\tpd\t1910\n");
    fclose(fn);
    
    bool ok = hathitrust_hathifile_diff(old_file, new_file, diff_file);
    assert(ok == true);
    
    FILE *fd = fopen(diff_file, "r");
    char line[256];
    char *got = fgets(line, sizeof(line), fd);
    assert(got != NULL);
    printf("[Test] Diff Output: %s", line);
    assert(strstr(line, "+ uc1.200") != NULL);
    fclose(fd);
    
    remove(old_file);
    remove(new_file);
    remove(diff_file);
    printf("[Test] Hathifiles Dataset Diff Engine tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust extra final tests...\n");
    test_mets_range_mapper();
    test_solr_facet_compiler();
    test_hathifile_diff();
    printf("[Test] All extra final tests completed successfully.\n");
    return 0;
}
