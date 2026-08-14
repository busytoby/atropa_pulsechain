#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== RESOLVING ACCURATE HTID FOR VERHANDELINGEN DER KONINKLIJKE AKADEMIE (DEEL 01) ===\n\n");

    // Inspecting catalog records to resolve the exact HTID for Deel 1 (1854) of Verhandelingen
    // Testing known volume items under Record 000057211 and serial holdings
    const char *test_htids[] = {
        "njp.32101074872111", // Deel 29 (1892)
        "mdp.39015039789321", // Deel 26 (1888)
        "mdp.39015039789313", // Deel 25 (1886)
        "chi.09876543210012",
        NULL
    };

    for (int i = 0; test_htids[i] != NULL; i++) {
        char *json = hathitrust_query_full("htid", test_htids[i]);
        if (json) {
            printf("[HTID: %s]\n", test_htids[i]);
            // Print titles
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
            // Print record URL
            char *r_ptr = strstr(json, "\"recordURL\":\"");
            if (r_ptr) {
                r_ptr += 13;
                char *r_end = strchr(r_ptr, '"');
                if (r_end) {
                    char url[256] = {0};
                    memcpy(url, r_ptr, r_end - r_ptr);
                    printf("  Catalog URL: %s\n", url);
                }
            }
            printf("\n");
            free(json);
        }
    }

    return 0;
}
