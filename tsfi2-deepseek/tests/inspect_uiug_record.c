#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== INSPECTING GENUINE BIBLIOGRAPHIC RECORD FOR UIUG.30112109778511 ===\n\n");
    const char *htid = "uiug.30112109778511";

    char *json = hathitrust_query_full("htid", htid);
    if (json) {
        // Find Titles
        char *t_ptr = strstr(json, "\"titles\":[");
        if (t_ptr) {
            char *t_end = strstr(t_ptr, "]");
            if (t_end) {
                char titles[1024] = {0};
                memcpy(titles, t_ptr, t_end - t_ptr + 1);
                printf("Titles: %s\n\n", titles);
            }
        }
        
        // Find Record URL
        char *r_ptr = strstr(json, "\"recordURL\":\"");
        if (r_ptr) {
            r_ptr += 13;
            char *r_end = strchr(r_ptr, '"');
            if (r_end) {
                char url[256] = {0};
                memcpy(url, r_ptr, r_end - r_ptr);
                printf("Catalog Record URL: %s\n\n", url);
            }
        }

        // Find enumcron / volume description for this item
        char *cur = json;
        while ((cur = strstr(cur, "\"htid\":\"uiug.30112109778511\"")) != NULL) {
            char *e_ptr = strstr(cur, "\"enumcron\":\"");
            if (e_ptr) {
                e_ptr += 12;
                char *e_end = strchr(e_ptr, '"');
                if (e_end) {
                    char desc[128] = {0};
                    memcpy(desc, e_ptr, e_end - e_ptr);
                    printf("Volume Description (enumcron): %s\n", desc);
                }
            }
            break;
        }

        free(json);
    }

    return 0;
}
