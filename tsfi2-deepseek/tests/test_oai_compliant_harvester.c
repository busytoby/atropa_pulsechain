#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_oai.h"
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== LEGITIMATE OAI-PMH & BIBLIOGRAPHIC HARVESTER IN C ===\n\n");

    // 1. Identify Repository
    printf("[1] Querying OAI-PMH Identify to verify compliant gateway handshake...\n");
    char *ident = hathitrust_oai_query("Identify", NULL, NULL, NULL, NULL, NULL, NULL);
    if (ident) {
        printf("    -> Status: SUCCESS (%zu bytes)\n", strlen(ident));
        printf("    -> Snippet: %.250s...\n\n", ident);
        free(ident);
    } else {
        printf("    -> Status: FAILED\n\n");
    }

    // 2. List Sets to find legitimate Academy serial sets
    printf("[2] Querying OAI-PMH ListSets...\n");
    char *sets = hathitrust_oai_query("ListSets", NULL, NULL, NULL, NULL, NULL, NULL);
    if (sets) {
        printf("    -> Status: SUCCESS (%zu bytes)\n", strlen(sets));
        printf("    -> Snippet: %.300s...\n\n", sets);
        free(sets);
    } else {
        printf("    -> Status: FAILED\n\n");
    }

    return 0;
}
