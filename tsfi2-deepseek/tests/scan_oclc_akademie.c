#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== SCANNING HATHITRUST SERIAL OCLC RECORDS IN C ===\n\n");

    // Scanning OCLC numbers for Dutch Royal Academy serials:
    // Searching across potential OCLC series IDs:
    const char *test_oclcs[] = {
        "1607567", "1607568", "1607569", "1607570",
        "1756570", "1756571", "1756572", "1756573", "1756574",
        "1761400", "1761401", "1761402", "1761403", "1761404", "1761405", "1761406",
        "2449838", "2449839", "2449840", "3128911", "3128912",
        NULL
    };

    for (int i = 0; test_oclcs[i] != NULL; i++) {
        char *json = hathitrust_query_full("oclc", test_oclcs[i]);
        if (json) {
            if (strlen(json) > 50) {
                printf("[OCLC: %s] Payload = %zu bytes\n", test_oclcs[i], strlen(json));
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
                char *r_ptr = strstr(json, "\"recordURL\":\"");
                if (r_ptr) {
                    r_ptr += 13;
                    char *r_end = strchr(r_ptr, '"');
                    if (r_end) {
                        char url[256] = {0};
                        memcpy(url, r_ptr, r_end - r_ptr);
                        printf("  URL: %s\n", url);
                    }
                }
                printf("\n");
            }
            free(json);
        }
    }

    return 0;
}
