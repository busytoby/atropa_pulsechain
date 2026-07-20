#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tsfi_parc_stcomp.h"

int tsfi_parc_st_compile(const char *source, uint8_t *bytecodes_out, int max_len) {
    if (!source || !bytecodes_out || max_len <= 0) return -1;
    if (strlen(source) >= 256) return -2;

    char src_copy[256];
    strncpy(src_copy, source, sizeof(src_copy) - 1);
    src_copy[sizeof(src_copy) - 1] = '\0';

    int code_idx = 0;
    char *token = strtok(src_copy, " \t\n\r;");
    while (token) {
        if (strcmp(token, "load") == 0) {
            char *arg_str = strtok(NULL, " \t\n\r;");
            if (arg_str) {
                if (code_idx >= max_len) return -3;
                int val = atoi(arg_str);
                bytecodes_out[code_idx++] = (uint8_t)(0x00 | (val & 0x0F));
            }
        } else if (strcmp(token, "const") == 0) {
            char *arg_str = strtok(NULL, " \t\n\r;");
            if (arg_str) {
                if (code_idx >= max_len) return -3;
                int val = atoi(arg_str);
                bytecodes_out[code_idx++] = (uint8_t)(0x20 | (val & 0x0F));
            }
        } else if (strcmp(token, "add") == 0) {
            if (code_idx >= max_len) return -3;
            bytecodes_out[code_idx++] = 0x30;
        } else if (strcmp(token, "sub") == 0) {
            if (code_idx >= max_len) return -3;
            bytecodes_out[code_idx++] = 0x40;
        } else if (strcmp(token, "return") == 0) {
            if (code_idx >= max_len) return -3;
            bytecodes_out[code_idx++] = 0x50;
        }
        token = strtok(NULL, " \t\n\r;");
    }

    return code_idx;
}
