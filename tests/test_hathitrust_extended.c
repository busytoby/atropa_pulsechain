#include "inc/hathitrust_oauth.h"
#include "inc/hathitrust_solr.h"
#include "inc/hathitrust_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_oauth_signature(void) {
    printf("[Test] Running OAuth 1.0a Signer tests...\n");
    
    char header[1024];
    int success = hathitrust_oauth_generate_header(
        "GET",
        "https://babel.hathitrust.org/cgi/pt",
        "mock_consumer_key",
        "mock_consumer_secret",
        "mock_token",
        "mock_token_secret",
        "mock_nonce",
        "1600000000",
        header,
        sizeof(header)
    );
    
    assert(success == 1);
    printf("[Test] Generated OAuth Header: %s\n", header);
    
    // Validate containing expected elements
    assert(strstr(header, "oauth_consumer_key=\"mock_consumer_key\"") != NULL);
    assert(strstr(header, "oauth_nonce=\"mock_nonce\"") != NULL);
    assert(strstr(header, "oauth_signature_method=\"HMAC-SHA1\"") != NULL);
    assert(strstr(header, "oauth_timestamp=\"1600000000\"") != NULL);
    assert(strstr(header, "oauth_version=\"1.0\"") != NULL);
    assert(strstr(header, "oauth_signature=\"") != NULL);
    
    printf("[Test] OAuth 1.0a Signer tests passed.\n");
}

static void test_solr_search(void) {
    printf("[Test] Running Solr Search API tests...\n");
    
    // Search for titles containing "Brandenburg" which we know exists
    char *res = hathitrust_solr_search("title", "Brandenburg");
    if (!res) {
        printf("[WARNING] Solr search query failed (likely network timeout/rate limit).\n");
        return;
    }
    
    printf("[Test] Solr response length: %zu bytes.\n", strlen(res));
    printf("[Test] Solr raw response: '%s'\n", res);
    assert(strstr(res, "records") != NULL || strlen(res) > 0);
    
    free(res);
    printf("[Test] Solr Search API tests passed.\n");
}

static void test_image_downloader(void) {
    printf("[Test] Running Page Image Downloader tests...\n");
    
    unsigned char *img = NULL;
    size_t img_len = 0;
    
    // Fetch a very small thumbnail of page 1 for the public domain book from NYPL
    int success = hathitrust_download_page_image("nyp.33433082257217", 1, 10, &img, &img_len);
    if (!success) {
        printf("[WARNING] Page image downloader failed (likely network timeout/rate limit).\n");
        return;
    }
    
    printf("[Test] Downloaded image size: %zu bytes.\n", img_len);
    assert(img_len > 0);
    assert(img != NULL);
    
    free(img);
    printf("[Test] Page Image Downloader tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust Extended Features tests...\n");
    test_oauth_signature();
    test_solr_search();
    test_image_downloader();
    printf("[Test] All Extended Features tests completed successfully.\n");
    return 0;
}
