#include "inc/hathitrust_ef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

// Helper to compress a string to a gzip memory buffer
static unsigned char* compress_to_gzip(const char *src, size_t src_len, size_t *out_len) {
    size_t dest_len = src_len * 2 + 100;
    unsigned char *dest = malloc(dest_len);
    if (!dest) return NULL;
    
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    strm.next_in = (Bytef *)src;
    strm.avail_in = src_len;
    strm.next_out = dest;
    strm.avail_out = dest_len;
    
    // 16 + MAX_WBITS specifies gzip format
    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        free(dest);
        return NULL;
    }
    
    deflate(&strm, Z_FINISH);
    deflateEnd(&strm);
    
    *out_len = strm.total_out;
    return dest;
}

int main(void) {
    printf("[Test] Running HTRC Extracted Features Parser tests...\n");
    
    const char *mock_json = 
        "{\n"
        "  \"id\": \"uc1.31970011037748\",\n"
        "  \"title\": \"L'art mérovingien et carolingien\",\n"
        "  \"author\": \"Léon Coutil\",\n"
        "  \"pubDate\": \"1930\",\n"
        "  \"language\": \"fre\",\n"
        "  \"pages\": [\n"
        "    { \"seq\": \"000001\", \"tokenCount\": 120 },\n"
        "    { \"seq\": \"000002\", \"tokenCount\": 250 }\n"
        "  ]\n"
        "}\n";
        
    size_t gz_len = 0;
    unsigned char *gz_data = compress_to_gzip(mock_json, strlen(mock_json), &gz_len);
    assert(gz_data != NULL);
    printf("[Test] Gzip compression succeeded (compressed size: %zu bytes).\n", gz_len);
    
    HtrcVolumeFeatures volume;
    bool success = htrc_parse_extracted_features(gz_data, gz_len, &volume);
    assert(success);
    
    // Validate parsed metadata
    assert(strcmp(volume.volume_id, "uc1.31970011037748") == 0);
    assert(strcmp(volume.title, "L'art mérovingien et carolingien") == 0);
    assert(strcmp(volume.author, "Léon Coutil") == 0);
    assert(strcmp(volume.pub_date, "1930") == 0);
    assert(strcmp(volume.language, "fre") == 0);
    
    // Validate parsed page features
    assert(volume.page_count == 2);
    assert(volume.pages[0].page_number == 1);
    assert(volume.pages[0].token_count == 120);
    assert(volume.pages[1].page_number == 2);
    assert(volume.pages[1].token_count == 250);
    
    htrc_free_volume_features(&volume);
    free(gz_data);
    
    printf("[Test] HTRC Extracted Features Parser tests passed.\n");
    return 0;
}
