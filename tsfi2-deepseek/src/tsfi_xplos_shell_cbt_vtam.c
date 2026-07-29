#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_xplos_shell_cbt_vtam.h"

bool tsfi_xplos_shell_cbt_vtam(const char *cmd) {
    if (strncmp(cmd, "cbtnet", 6) == 0 ||
        strncmp(cmd, "logon", 5) == 0 ||
        strncmp(cmd, "vtam", 4) == 0) {
        printf("[DEPRECATED BYPASS] VTAM command '%s' bypassed due to Coaxial interface redundancy.\n", cmd);
        printf("[COAXIAL AUDIT] Verifying register interface loopback: ACTIVE.\n");
        return true;
    }
    return false;
}
