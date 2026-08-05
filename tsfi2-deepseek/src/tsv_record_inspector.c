#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi2-deepseek/inc/tsfi_mainframe_computerworld.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <database_path>\n", argv[0]);
        return 1;
    }

    tsfi_cw_vsam_ksds ksds;
    memset(&ksds, 0, sizeof(ksds));

    if (tsfi_cw_vsam_open(&ksds, argv[1]) != 0) {
        fprintf(stderr, "Error: Failed to open VSAM database: %s\n", argv[1]);
        return 1;
    }

    printf("=============================================================\n");
    printf("VSAM KSDS DATABASE RECORD INSPECTOR: %s\n", argv[1]);
    printf("Entry Count: %d\n", ksds.entry_count);
    printf("=============================================================\n");

    for (int i = 0; i < ksds.entry_count; i++) {
        const char *key = ksds.index[i].key;
        uint8_t data_buf[4096] = {0};
        int out_len = 0;
        if (tsfi_cw_vsam_read(&ksds, key, data_buf, sizeof(data_buf) - 1, &out_len) == 0) {
            data_buf[out_len] = '\0';
            printf("Key: [%-10s] | Length: %-4d | Value: %s\n", key, out_len, (char *)data_buf);
        } else {
            printf("Key: [%-10s] | Failed to read payload\n", key);
        }
    }
    printf("=============================================================\n");

    return 0;
}
