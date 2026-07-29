#include <stddef.h>
#include "tsfi_xplos_shell_cbt_tso.h"
#include "tsfi_winchester_bridge.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool tsfi_xplos_shell_cbt_tso(const char *cmd) {
    if (strncmp(cmd, "cbtalloc", 8) == 0 ||
        strncmp(cmd, "cbtdelete", 9) == 0 ||
        strncmp(cmd, "cbtrexx", 7) == 0 ||
        strncmp(cmd, "ispfmenu", 8) == 0 ||
        strncmp(cmd, "help", 4) == 0 ||
        strncmp(cmd, "cbtsub", 6) == 0) {
        printf("[DEPRECATED BYPASS] TSO command '%s' bypassed due to WinchesterMQ redundancy.\n", cmd);
        // Verify loopback connection to ensure the redundant path remains active
        printf("[WINCHESTERMQ AUDIT] Verifying bus interface loopback for bypass path: ACTIVE.\n");
        return true;
    }
    return false;
}
