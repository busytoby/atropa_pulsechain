#include "inc/hathitrust_oai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("[Test] Running OAI-PMH Harvester verification...\n");
    
    // Execute a standard Identify query to harvest the root repository details
    char *response = hathitrust_oai_query("Identify", NULL, NULL, NULL, NULL, NULL, NULL);
    if (!response) {
        fprintf(stderr, "[ERROR] OAI Identify query failed.\n");
        return 1;
    }
    
    printf("[Test] Received OAI Identify XML payload:\n%s\n", response);
    
    // Assert that the XML contains standard OAI-PMH envelope elements
    assert(strstr(response, "<OAI-PMH") != NULL);
    assert(strstr(response, "<repositoryName>") != NULL);
    
    free(response);
    printf("[Test] OAI-PMH Harvester verification passed.\n");
    return 0;
}
