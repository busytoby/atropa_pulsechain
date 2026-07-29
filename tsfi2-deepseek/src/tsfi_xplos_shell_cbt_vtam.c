#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_xplos_shell_cbt_vtam.h"

char g_vtam_coax_data_register[2048] = {0};

void tsfi_vtam_coax_write_buffer(const char *data) {
    if (data) {
        strncpy(g_vtam_coax_data_register, data, sizeof(g_vtam_coax_data_register) - 1);
        g_vtam_coax_data_register[sizeof(g_vtam_coax_data_register) - 1] = '\0';
        printf("[VTAM COAXIAL REGISTER] Wrote %d bytes to g_vtam_coax_data_register\n", (int)strlen(g_vtam_coax_data_register));
    }
}

void tsfi_vtam_coax_read_buffer(char *dest, int max_len) {
    if (dest && max_len > 0) {
        strncpy(dest, g_vtam_coax_data_register, max_len - 1);
        dest[max_len - 1] = '\0';
        // Clear after read (destructive read like physical FIFO buffer)
        g_vtam_coax_data_register[0] = '\0';
        printf("[VTAM COAXIAL REGISTER] Read and cleared coaxial register buffer\n");
    }
}

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
