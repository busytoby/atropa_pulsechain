#include "inc/hathitrust_quadtree.h"
#include "inc/hathitrust_alto.h"
#include "inc/hathitrust_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

static void test_quadtree_indexer(void) {
    printf("[Test] Running Quadtree Indexer tests...\n");
    
    HtrcQuadtreePoint points[3] = {
        { 1930.0, 491914940.0, "uc1.31970011037748", "L'art mérovingien et carolingien" },
        { 1850.0, 5272362.0, "nyp.33433082257217", "The Humming bird" },
        { 1969.0, 2779601.0, "mdp.39015018415946", "Slovenské vyst̕ahovalectvo" }
    };
    
    const char *filepath = "/tmp/hathitrust_quadtree.dat.bin";
    const char *bad_filepath = "/tmp/hathitrust_quadtree.json";
    
    // Assert invalid file extension is rejected
    bool bad_success = hathitrust_quadtree_serialize(bad_filepath, points, 3);
    assert(!bad_success);
    
    // Assert valid serialization
    bool success = hathitrust_quadtree_serialize(filepath, points, 3);
    assert(success);
    
    // Verify file exists
    FILE *f = fopen(filepath, "rb");
    assert(f != NULL);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    assert(size > 0);
    fclose(f);
    
    unlink(filepath);
    printf("[Test] Quadtree Indexer tests passed.\n");
}

static void test_alto_parser(void) {
    printf("[Test] Running ALTO XML Parser tests...\n");
    
    const char *mock_alto =
        "<alto xmlns=\"http://www.loc.gov/standards/alto/ns-v3#\">\n"
        "  <Layout>\n"
        "    <Page ID=\"P1\" PHYSICAL_IMG_NR=\"1\" HEIGHT=\"3000\" WIDTH=\"2000\">\n"
        "      <PrintSpace>\n"
        "        <TextBlock ID=\"TB1\">\n"
        "          <TextLine ID=\"TL1\">\n"
        "            <String CONTENT=\"Auncient\" HPOS=\"100\" VPOS=\"200\" WIDTH=\"150\" HEIGHT=\"40\" />\n"
        "            <String CONTENT=\"Wavelets\" HPOS=\"270\" VPOS=\"200\" WIDTH=\"180\" HEIGHT=\"40\" />\n"
        "          </TextLine>\n"
        "        </TextBlock>\n"
        "      </PrintSpace>\n"
        "    </Page>\n"
        "  </Layout>\n"
        "</alto>\n";
        
    HtrcAltoPage page;
    bool success = hathitrust_alto_parse(mock_alto, &page);
    assert(success);
    
    assert(page.token_count == 2);
    
    assert(strcmp(page.tokens[0].content, "Auncient") == 0);
    assert(page.tokens[0].hpos == 100);
    assert(page.tokens[0].vpos == 200);
    assert(page.tokens[0].width == 150);
    assert(page.tokens[0].height == 40);
    
    assert(strcmp(page.tokens[1].content, "Wavelets") == 0);
    assert(page.tokens[1].hpos == 270);
    assert(page.tokens[1].vpos == 200);
    assert(page.tokens[1].width == 180);
    assert(page.tokens[1].height == 40);
    
    hathitrust_alto_free(&page);
    printf("[Test] ALTO XML Parser tests passed.\n");
}

static void test_token_cache(void) {
    printf("[Test] Running Resumption Token Cache tests...\n");
    
    const char *cache_path = "/tmp/hathitrust_token_cache.dat.bin";
    const char *mock_token = "oai_dc.s(hathitrust:pdus):500-19146-AoEpMDAwMzI3NTg5";
    
    // Test saving/loading token valid for 1 hour in the future
    time_t future_exp = time(NULL) + 3600;
    bool save_ok = hathitrust_cache_save(cache_path, mock_token, future_exp);
    assert(save_ok);
    
    char loaded_token[512];
    bool load_ok = hathitrust_cache_load(cache_path, loaded_token, sizeof(loaded_token));
    assert(load_ok);
    assert(strcmp(loaded_token, mock_token) == 0);
    
    // Test expired token rejection
    time_t past_exp = time(NULL) - 60; // Expired 1 minute ago
    save_ok = hathitrust_cache_save(cache_path, mock_token, past_exp);
    assert(save_ok);
    
    load_ok = hathitrust_cache_load(cache_path, loaded_token, sizeof(loaded_token));
    assert(!load_ok); // Should fail to load expired token
    
    unlink(cache_path);
    printf("[Test] Resumption Token Cache tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust Cache, ALTO, and Quadtree unit tests...\n");
    test_quadtree_indexer();
    test_alto_parser();
    test_token_cache();
    printf("[Test] All unit tests completed successfully.\n");
    return 0;
}
