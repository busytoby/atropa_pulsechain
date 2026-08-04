#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_oauth_callback.h"
#include "inc/hathitrust_bibframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_oauth_verifier(void) {
    printf("[Test] Running OAuth callback verifier tests...\n");
    
    char verifier[128];
    bool ok = hathitrust_oauth_verify_callback(
        "oauth_token=requestKey&oauth_verifier=verifier12345&state=ok",
        verifier,
        sizeof(verifier)
    );
    assert(ok == true);
    assert(strcmp(verifier, "verifier12345") == 0);
    
    // Fail case
    ok = hathitrust_oauth_verify_callback("state=error", verifier, sizeof(verifier));
    assert(ok == false);
    
    printf("[Test] OAuth callback verifier tests passed.\n");
}

static void test_bibframe_rdf(void) {
    printf("[Test] Running BIB-FRAME RDF serialization tests...\n");
    
    char rdf[512];
    bool ok = hathitrust_to_bibframe_rdf(
        "L'art merovingien et carolingien",
        "Jean Hubert",
        rdf,
        sizeof(rdf)
    );
    assert(ok == true);
    printf("[Test] Generated RDF Triples:\n%s", rdf);
    
    assert(strstr(rdf, "bibframe/title") != NULL);
    assert(strstr(rdf, "L'art merovingien et carolingien") != NULL);
    assert(strstr(rdf, "Jean Hubert") != NULL);
    
    printf("[Test] BIB-FRAME RDF serialization tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust BIB-FRAME & OAuth callback tests...\n");
    test_oauth_verifier();
    test_bibframe_rdf();
    printf("[Test] All BIB-FRAME & OAuth callback tests completed successfully.\n");
    return 0;
}
