#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== SEARCHING EXACT METS/MARC RECORDS FOR VERHANDELINGEN DEEL 01 (1854) IN C ===\n\n");

    // Probing specific volume identifiers for Deel 1 (1854)
    // Testing LCCN / OCLC variations associated with Royal Netherlands Academy of Arts and Sciences (KNAW)
    const char *test_queries[][2] = {
        {"oclc", "4945417"},
        {"oclc", "1607567"},
        {"oclc", "6386457"},
        {"lccn", "01026041"},
        {"lccn", "13013457"},
        {NULL, NULL}
    };

    for (int i = 0; test_queries[i][0] != NULL; i++) {
        const char *id_type = test_queries[i][0];
        const char *id_val = test_queries[i][1];
        printf("--- Querying %s: %s ---\n", id_type, id_val);
        char *json = hathitrust_query_full(id_type, id_val);
        if (json) {
            if (strlen(json) > 50) {
                printf("  [SUCCESS] Payload received (%zu bytes)\n", strlen(json));
                char *t_ptr = strstr(json, "\"titles\":[\"");
                if (t_ptr) {
                    t_ptr += 11;
                    char *t_end = strchr(t_ptr, '"');
                    if (t_end) {
                        char title[256] = {0};
                        memcpy(title, t_ptr, t_end - t_ptr);
                        printf("  Title: %s\n", title);
                    }
                }
            } else {
                printf("  [EMPTY] %s\n", json);
            }
            free(json);
        } else {
            printf("  [FAILED]\n");
        }
        printf("\n");
    }

    return 0;
}
