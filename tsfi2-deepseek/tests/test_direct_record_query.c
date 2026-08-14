#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== DIRECT QUERY FOR VERHANDELINGEN DER KONINKLIJKE AKADEMIE SERIAL RECORDS ===\n\n");

    // Querying verified catalog record identifiers for Koninklijke Akademie serial publications
    // Record 008608831 / 001483827 / other catalog clusters
    const char *bib_keys[] = {
        "000057211", "008883356", "008608831", "000529650", NULL
    };

    for (int i = 0; bib_keys[i] != NULL; i++) {
        char *json = hathitrust_query_full("recordnumber", bib_keys[i]);
        if (json) {
            printf("--- Record Number: %s (Size: %zu bytes) ---\n", bib_keys[i], strlen(json));
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
            free(json);
        }
    }

    return 0;
}
