#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi2-deepseek/inc/tsfi_mainframe_computerworld.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <database_path> <query_isbn_or_oclc>\n", argv[0]);
        return 1;
    }

    tsfi_cw_vsam_ksds ksds;
    memset(&ksds, 0, sizeof(ksds));

    if (tsfi_cw_vsam_open(&ksds, argv[1]) != 0) {
        fprintf(stderr, "Error: Failed to open VSAM database: %s\n", argv[1]);
        return 1;
    }

    const char *query = argv[2];
    printf("[HathiTrust Query] Scanning catalog indexes for query: %s\n", query);

    bool found = false;
    for (int i = 0; i < ksds.entry_count; i++) {
        const char *key = ksds.index[i].key;
        uint8_t data_buf[4096] = {0};
        int out_len = 0;
        
        if (tsfi_cw_vsam_read(&ksds, key, data_buf, sizeof(data_buf) - 1, &out_len) == 0) {
            data_buf[out_len] = '\0';
            // Check if ISBN or OCLC matches
            if (strstr((char *)data_buf, query) != NULL) {
                found = true;
                printf("\n[MATCH FOUND] Key: %s\n", key);
                printf("Record Data: %s\n", (char *)data_buf);

                // Preserve by reference
                FILE *ref_f = fopen("assets/hathitrust_references.txt", "a");
                if (ref_f) {
                    fprintf(ref_f, "PRESERVED REFERENCE: Key=%s | Source=%s\n", key, (char *)data_buf);
                    fclose(ref_f);
                    printf("[Preservation] Reference successfully appended to assets/hathitrust_references.txt\n");
                }
            }
        }
    }

    if (!found) {
        printf("[HathiTrust Query] No matching records found for query: %s\n", query);
    }

    return 0;
}
