#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== CATALOG SEARCH FOR VERHANDELINGEN DER KONINKLIJKE AKADEMIE IN C ===\n\n");

    // Scanning OCLC catalog keys for Verhandelingen der Koninklijke Akademie
    // Primary OCLC for the early series: 1607567, 1756575, 4945417, 2449838, etc.
    const char *test_records[] = {
        "001483827", "008608831", "000518985", "000067793", "000498010",
        NULL
    };

    for (int i = 0; test_records[i] != NULL; i++) {
        char *json = hathitrust_query_full("recordnumber", test_records[i]);
        if (json) {
            if (strlen(json) > 100) {
                printf("[Record: %s] (%zu bytes)\n", test_records[i], strlen(json));
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
                char *r_ptr = strstr(json, "\"recordURL\":\"");
                if (r_ptr) {
                    r_ptr += 13;
                    char *r_end = strchr(r_ptr, '"');
                    if (r_end) {
                        char url[256] = {0};
                        memcpy(url, r_ptr, r_end - r_ptr);
                        printf("  Record URL: %s\n", url);
                    }
                }
                // List Items
                char *item_ptr = strstr(json, "\"items\":[");
                if (item_ptr) {
                    char *cur = item_ptr;
                    int count = 0;
                    while ((cur = strstr(cur, "\"htid\":\"")) != NULL && count < 5) {
                        cur += 8;
                        char *h_end = strchr(cur, '"');
                        if (h_end) {
                            char htid[64] = {0};
                            memcpy(htid, cur, h_end - cur);
                            
                            char desc[64] = "N/A";
                            char *d_ptr = strstr(h_end, "\"enumcron\":\"");
                            if (d_ptr) {
                                d_ptr += 12;
                                char *d_end = strchr(d_ptr, '"');
                                if (d_end && (d_end - d_ptr < (int)sizeof(desc))) {
                                    memcpy(desc, d_ptr, d_end - d_ptr);
                                    desc[d_end - d_ptr] = '\0';
                                }
                            }
                            printf("    Item %d: HTID=%s | Vol=%s | Link=https://catalog.hathitrust.org/Record/%s?urlappend=%%3Bseq=13\n",
                                   ++count, htid, desc, test_records[i]);
                        }
                    }
                }
                printf("\n");
            }
            free(json);
        }
    }

    return 0;
}
