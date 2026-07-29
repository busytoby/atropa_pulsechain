#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_xplos_shell_cbt_cics.h"

int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "Usage: cics_cli [write|read] [tsq|tdq] [name] [val]\n");
        return 1;
    }
    char cmd[512];
    if (strcmp(argv[1], "write") == 0) {
        if (strcmp(argv[2], "tsq") == 0) {
            snprintf(cmd, sizeof(cmd), "cbtcicsts write %s %s", argv[3], argv[4]);
        } else {
            snprintf(cmd, sizeof(cmd), "cbtcicstd write %s %s", argv[3], argv[4]);
        }
    } else {
        if (strcmp(argv[2], "tsq") == 0) {
            snprintf(cmd, sizeof(cmd), "cbtcicsts read %s", argv[3]);
        } else {
            snprintf(cmd, sizeof(cmd), "cbtcicstd read %s", argv[3]);
        }
    }
    tsfi_xplos_shell_cbt_cics(cmd);
    return 0;
}
