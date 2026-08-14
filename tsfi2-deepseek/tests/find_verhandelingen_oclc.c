#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== DIRECT C QUERY FOR VERHANDELINGEN DER KONINKLIJKE AKADEMIE RECORDS ===\n\n");

    // Known OCLC numbers for Verhandelingen der Koninklijke Akademie van Wetenschappen:
    // OCLC: 1756575 - Verhandelingen der Koninklijke Akademie van Wetenschappen (Eerste Sectie)
    // OCLC: 1756576 - Verhandelingen der Koninklijke Akademie van Wetenschappen (Tweede Sectie)
    // OCLC: 1756577 - Verhandelingen der Koninklijke Akademie van Wetenschappen (Letterkunde)
    // OCLC: 4945417 - Verhandelingen (General/Original Series 1854-1892)
    // OCLC: 1761407 - Jaarboek / Verslagen der Koninklijke Akademie

    const char *test_occlcs[] = {
        "4945417", "1756575", "1756576", "1756577", "1761407", "6386457", NULL
    };

    for (int i = 0; test_occlcs[i] != NULL; i++) {
        printf(">>> Querying OCLC: %s <<<\n", test_occlcs[i]);
        char *json = hathitrust_query_full("oclc", test_occlcs[i]);
        if (json) {
            printf("Payload Size: %zu bytes\n", strlen(json));
            // Extract Titles
            char *t_ptr = strstr(json, "\"titles\":[\"");
            if (t_ptr) {
                t_ptr += 11;
                char *t_end = strchr(t_ptr, '"');
                if (t_end) {
                    char title[512] = {0};
                    memcpy(title, t_ptr, t_end - t_ptr);
                    printf("  Title: %s\n", title);
                }
            }
            // Extract Record URL
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
            // List Items
            char *item_ptr = strstr(json, "\"items\":[");
            if (item_ptr) {
                char *cur = item_ptr;
                int count = 0;
                while ((cur = strstr(cur, "\"htid\":\"")) != NULL && count < 6) {
                    cur += 8;
                    char *h_end = strchr(cur, '"');
                    if (h_end) {
                        char htid[64] = {0};
                        memcpy(htid, cur, h_end - cur);
                        
                        char enumcron[64] = "N/A";
                        char *e_ptr = strstr(h_end, "\"enumcron\":\"");
                        if (e_ptr) {
                            e_ptr += 12;
                            char *e_end = strchr(e_ptr, '"');
                            if (e_end && (e_end - e_ptr < (int)sizeof(enumcron))) {
                                memcpy(enumcron, e_ptr, e_end - e_ptr);
                                enumcron[e_end - e_ptr] = '\0';
                            }
                        }
                        printf("    -> Item [%d]: HTID = %s | Vol = %s\n", ++count, htid, enumcron);
                    }
                }
            }
            printf("\n");
            free(json);
        } else {
            printf("  Lookup returned NULL for OCLC %s.\n\n", test_occlcs[i]);
        }
    }

    return 0;
}
