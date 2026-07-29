#include "tsfi_xplos_shell_cbt_diagnostics.h"
#include "tsfi_xplos_kernel_internal.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Globals from virtual file system
extern XplosVirtualDisk g_vfs;

static void resolve_pds_name_diagnostics(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

// -----------------------------------------------------------------------------
// 1. TSO JCL Syntax Checker
// -----------------------------------------------------------------------------
static bool handle_cbtjclchk(const char *cmd) {
    char member[32] = "";
    if (sscanf(cmd + 10, "%31s", member) < 1) {
        printf("[JCLCHK ERROR] Syntax: cbtjclchk <member>\n");
        return true;
    }
    char vfs_filename[128];
    resolve_pds_name_diagnostics(member, vfs_filename, sizeof(vfs_filename));

    int f_idx = -1;
    for (int i = 0; i < g_vfs.count; i++) {
        if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
            f_idx = i;
            break;
        }
    }
    if (f_idx < 0) {
        printf("[JCLCHK ERROR] Member %s not found in VFS.\n", member);
        return true;
    }

    printf("[JCLCHK] Auditing JCL member %s columns and cards...\n", member);
    char *jcl_data = strdup(g_vfs.files[f_idx].data);
    char *line = strtok(jcl_data, "\n");
    int line_num = 1;
    bool has_job_card = false;

    while (line) {
        char *ptr = line;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (strncmp(ptr, "//", 2) == 0) {
            if (strstr(ptr, " JOB ") || strstr(ptr, " job ")) {
                has_job_card = true;
            }
        } else if (strlen(ptr) > 0 && strncmp(ptr, "/*", 2) != 0) {
            printf("[JCLCHK WARNING] Line %d: Card missing '//' prefix.\n", line_num);
        }
        line = strtok(NULL, "\n");
        line_num++;
    }
    free(jcl_data);

    if (!has_job_card) {
        printf("[JCLCHK WARNING] Missing valid JOB statement card at start.\n");
    } else {
        printf("[JCLCHK] Syntax check completed. No blocking errors found. RC=0000\n");
    }
    return true;
}

// -----------------------------------------------------------------------------
// 2. IEBIMAGE Printer Buffer Formatter
// -----------------------------------------------------------------------------
static bool handle_iebimage(const char *cmd) {
    char member[32] = "";
    int margins = 0;
    if (sscanf(cmd + 9, "%31s %d", member, &margins) < 1) {
        printf("[IEBIMAGE ERROR] Syntax: iebimage <member> [margins]\n");
        return true;
    }
    printf("[IEBIMAGE] Formatting printer buffer image for %s...\n", member);
    printf("  - Margin settings: %d columns\n", (margins > 0) ? margins : 8);
    printf("  - Page breaks: 66 lines per page\n");
    printf("[IEBIMAGE] Print image formatted successfully. RC=0000\n");
    return true;
}

// -----------------------------------------------------------------------------
// 3. CICS Transient Storage Directory Explorer
// -----------------------------------------------------------------------------
static bool handle_cbtcicstsd(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSIENT STORAGE QUEUES (TSQ) DIRECTORY                \n");
    printf("================================================================================\n");
    printf(" QUEUE ID | TYPE   | RECORDS | LENGTH | LAST TRANSACTION\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" Q1       | MAIN   |       1 |     12 | CSKP\n");
    printf(" LOGS     | AUX    |      42 |   1024 | TRN1\n");
    printf("================================================================================\n");
    return true;
}

// -----------------------------------------------------------------------------
// 4. VTAM Virtual Route Controller
// -----------------------------------------------------------------------------
static bool handle_vtamroute(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  **AUNCIENT** VTAM PATH TUNNEL ROUTING DIAGNOSTICS\n");
    printf("================================================================================\n");
    printf(" CDRM PATH ROUTE STATUS:\n");
    printf("   - HOST: XPL1   -> TARGET: CDRM_LOCAL (STABLE)\n");
    printf("   - PATH INDEX: 04 (PRIMARY) via LU 1\n");
    printf("   - TRANSMISSION STABILITY: 100%%\n");
    printf("================================================================================\n");
    return true;
}

// -----------------------------------------------------------------------------
// Entry Point / Command Router
// -----------------------------------------------------------------------------
bool tsfi_xplos_shell_cbt_diagnostics(const char *cmd) {
    if (strncmp(cmd, "cbtjclchk ", 10) == 0) {
        return handle_cbtjclchk(cmd);
    }
    if (strncmp(cmd, "iebimage ", 9) == 0) {
        return handle_iebimage(cmd);
    }
    if (strcmp(cmd, "cbtcicstsd") == 0) {
        return handle_cbtcicstsd();
    }
    if (strcmp(cmd, "vtamroute") == 0) {
        return handle_vtamroute();
    }
    return false;
}
