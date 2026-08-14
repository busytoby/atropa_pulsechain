#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== SCANNING HATHITRUST SERIAL HOLDINGS FOR KONINKLIJKE AKADEMIE IN C ===\n\n");

    // Probing catalog records around Koninklijke Akademie serial blocks
    // In HathiTrust, Academy serials from Netherlands were cataloged under LCCN/OCLC ranges
    const char *test_lccn[] = {
        "01026040", "01026041", "01026042", "01026043", "01026044", "01026045",
        "08018900", "08018901", "08018902", "50047391", "50047392", NULL
    };

    for (int i = 0; test_lccn[i] != NULL; i++) {
        char *json = hathitrust_query_full("lccn", test_lccn[i]);
        if (json) {
            if (strlen(json) > 50) {
                printf("[LCCN: %s] Payload = %zu bytes\n", test_lccn[i], strlen(json));
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
            }
            free(json);
        }
    }

    return 0;
}
