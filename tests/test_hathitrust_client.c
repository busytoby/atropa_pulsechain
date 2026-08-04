#include "inc/hathitrust_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("[Test] Querying HathiTrust Bibliographic API via raw socket + OpenSSL...\n");
    
    // Querying a known public domain record or catalog item via OCLC
    // OCLC 297775 refers to an old book in the catalog.
    char *response = hathitrust_query_brief("oclc", "297775");
    if (!response) {
        fprintf(stderr, "[ERROR] Brief query failed.\n");
        return 1;
    }
    assert(strstr(response, "\"records\"") != NULL);
    free(response);
    
    char *full_response = hathitrust_query_full("oclc", "297775");
    if (!full_response) {
        fprintf(stderr, "[ERROR] Full query failed.\n");
        return 1;
    }
    assert(strstr(full_response, "\"records\"") != NULL);
    free(full_response);
    
    printf("[Test] HathiTrust Client Test Passed.\n");
    return 0;
}
