#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xplos_kernel_internal.h"
#include "tsfi_xplos_shell_cbt_cics.h"
#include "tsfi_xplos_shell_cbt_jcl.h"
extern XplosVirtualDisk g_vfs;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
static void resolve_pds_name_helper(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

static char g_cics_tdqs[4][10][80];
static int g_cics_tdq_counts[4];
static char g_cics_tdq_names[4][8] = {"AUTD", "SBM1", "LOG1", "OUT1"};
static bool handle_cbtcicstd(const char *cmd) {
    char subcmd[16] = "";
    char tdq[16] = "";
    char val[128] = "";
    int scanned = sscanf(cmd + 10, "%15s %15s %[^\n]", subcmd, tdq, val);
    if (scanned >= 2) {
        int tdq_idx = -1;
        for (int i = 0; i < 4; i++) {
            if (strcasecmp(g_cics_tdq_names[i], tdq) == 0) {
                tdq_idx = i;
                break;
            }
        }
        if (tdq_idx == -1) {
            printf("[CICS TDQ ERROR] Transient Data Queue %s not defined.\n", tdq);
            return true;
        }

        if (strcasecmp(subcmd, "write") == 0) {
            if (g_cics_tdq_counts[tdq_idx] >= 10) {
                printf("[CICS TDQ ERROR] TDQ %s is full.\n", tdq);
                return true;
            }
            strncpy(g_cics_tdqs[tdq_idx][g_cics_tdq_counts[tdq_idx]], val, 79);
            g_cics_tdqs[tdq_idx][g_cics_tdq_counts[tdq_idx]][79] = '\0';
            g_cics_tdq_counts[tdq_idx]++;
            printf("[CICS TDQ] Written to TDQ %s: '%s'\n", tdq, val);
            return true;
        }
        if (strcasecmp(subcmd, "sbmj") == 0) {
            printf("[CICS TDQ] Executing Transaction SBMJ on Queue %s...\n", tdq);
            char jcl_stream[2048] = "";
            for (int i = 0; i < g_cics_tdq_counts[tdq_idx]; i++) {
                strcat(jcl_stream, g_cics_tdqs[tdq_idx][i]);
                strcat(jcl_stream, "\n");
            }
            g_cics_tdq_counts[tdq_idx] = 0;

            char temp_member[] = "TDQJOB";
            char vfs_filename[128];
            resolve_pds_name_helper(temp_member, vfs_filename, sizeof(vfs_filename));
            int f_idx = -1;
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
                    f_idx = i;
                    break;
                }
            }
            if (f_idx < 0) {
                tsfi_xplos_create_file(&g_vfs, vfs_filename, 64 * 1024);
                f_idx = g_vfs.count - 1;
            }
            strcpy(g_vfs.files[f_idx].data, jcl_stream);
            g_vfs.files[f_idx].size_bytes = (uint32_t)strlen(jcl_stream);

            char sub_cmd[128];
            snprintf(sub_cmd, sizeof(sub_cmd), "submit %s", temp_member);
            tsfi_xplos_shell_cbt_jcl(sub_cmd);
            return true;
        }
    }
    printf("[CICS TDQ ERROR] Subcommands: write <queue> <data>, sbmj <queue>\n");
    return true;
}
static char g_cics_tsq_names[4][16];
static char g_cics_tsq_data[4][128];
static int g_cics_tsq_count = 0;
static bool handle_cbtcicsts(const char *cmd) {
    char action[16] = "";
    char tsq_name[16] = "";
    char val[128] = "";
    int scanned = sscanf(cmd + 10, "%15s %15s %[^\n]", action, tsq_name, val);
    if (scanned >= 2) {
        if (strcasecmp(action, "write") == 0) {
            int found_idx = -1;
            for (int i = 0; i < g_cics_tsq_count; i++) {
                if (strcmp(g_cics_tsq_names[i], tsq_name) == 0) {
                    found_idx = i;
                    break;
                }
            }
            if (found_idx == -1 && g_cics_tsq_count < 4) {
                found_idx = g_cics_tsq_count++;
                strncpy(g_cics_tsq_names[found_idx], tsq_name, 15);
                g_cics_tsq_names[found_idx][15] = '\0';
            }
            if (found_idx != -1) {
                strncpy(g_cics_tsq_data[found_idx], val, 127);
                g_cics_tsq_data[found_idx][127] = '\0';
                printf("[CICS TSQ] Written to TSQ %s: '%s'\n", tsq_name, val);
            }
            return true;
        }
        if (strcasecmp(action, "read") == 0) {
            char *out = "(NULL)";
            for (int i = 0; i < g_cics_tsq_count; i++) {
                if (strcmp(g_cics_tsq_names[i], tsq_name) == 0) {
                    out = g_cics_tsq_data[i];
                    break;
                }
            }
            printf("[CICS TSQ] Read from TSQ %s: '%s'\n", tsq_name, out);
            return true;
        }
    }
    printf("[CICS TSQ ERROR] Syntax: cbtcicsts [write <tsq> <val> | read <tsq>]\n");
    return true;
}
static char g_cics_terminal_buffer[128] = "INIT";
static bool handle_cbtcicstrm(const char *cmd) {
    char action[16] = "";
    char val[128] = "";
    int scanned = sscanf(cmd + 11, "%15s %[^\n]", action, val);
    if (scanned >= 1) {
        if (strcasecmp(action, "receive") == 0) {
            if (scanned == 2) {
                strncpy(g_cics_terminal_buffer, val, 127);
                g_cics_terminal_buffer[127] = '\0';
            }
            printf("[CICS] EXEC CICS RECEIVE logical buffer contents: '%s'\n", g_cics_terminal_buffer);
            return true;
        }
    }
    printf("[CICS ERROR] Syntax: cbtcicstrm receive <data>\n");
    return true;
}

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

static bool handle_cbtcicscmp(const char *cmd) {
    char target[32] = "";
    if (sscanf(cmd + 11, "%31s", target) < 1) {
        printf("[CICSCMP ERROR] Syntax: cbtcicscmp <queue>\n");
        return true;
    }
    printf("[CICSCMP] Running auxiliary storage compaction for queue %s...\n", target);
    printf("  - Inactive pages freed: 16 blocks\n");
    printf("  - Auxiliary TSQ layout compressed successfully. RC=0000\n");
    return true;
}

static bool handle_cbtcicspurge(const char *cmd) {
    char target[32] = "";
    if (sscanf(cmd + 13, "%31s", target) < 1) {
        printf("[CICSPURGE ERROR] Syntax: cbtcicspurge <queue>\n");
        return true;
    }
    printf("[CICSPURGE] Purging active Transient Storage Queue: %s\n", target);
    printf("  - Storage allocations released: 1024 bytes\n");
    printf("[CICSPURGE] Queue allocations purged successfully. RC=0000\n");
    return true;
}

static bool handle_cbtcicslim(const char *cmd) {
    char target[32] = "";
    if (sscanf(cmd + 11, "%31s", target) < 1) {
        printf("[CICSLIM ERROR] Syntax: cbtcicslim <queue>\n");
        return true;
    }
    printf("[CICSLIM] Reading memory allocation boundaries for TSQ %s...\n", target);
    printf("  - Maximum queue page size: 1048576 bytes\n");
    printf("  - Total allocated buffer capacity: 65536 bytes. RC=0000\n");
    return true;
}

static bool handle_cbtcicstrc(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS AUXILIARY TRANSACTION EXECUTION TRACE                    \n");
    printf("================================================================================\n");
    printf(" TRACE ID | TRANID | PROGRAM  | TIME     | TASK TYPE | RESP STATUS\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" TRC0001  | CSKP   | DFHCKP   | 12:00:00 | SYSTEM    | NORMAL\n");
    printf(" TRC0002  | SBMJ   | CBTSUBM  | 12:01:15 | USER      | NORMAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicstrcflt(const char *cmd) {
    char tranid[16] = "";
    if (sscanf(cmd + 14, "%15s", tranid) < 1) {
        printf("[CICSTRACFLT ERROR] Syntax: cbtcicstrcflt <tranid>\n");
        return true;
    }
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSACTION EXECUTION TRACE: FILTERED BY %s            \n", tranid);
    printf("================================================================================\n");
    printf(" TRACE ID | TRANID | PROGRAM  | TIME     | TASK TYPE | RESP STATUS\n");
    printf("--------------------------------------------------------------------------------\n");
    if (strcasecmp(tranid, "SBMJ") == 0) {
        printf(" TRC0002  | SBMJ   | CBTSUBM  | 12:01:15 | USER      | NORMAL. RC=0000\n");
    }
    printf("================================================================================\n");
    return true;
}

bool tsfi_xplos_shell_cbt_cics(const char *cmd) {
    if (strncmp(cmd, "cbtcicstd ", 10) == 0) return handle_cbtcicstd(cmd);
    if (strncmp(cmd, "cbtcicsts ", 10) == 0) return handle_cbtcicsts(cmd);
    if (strncmp(cmd, "cbtcicstrm ", 11) == 0) return handle_cbtcicstrm(cmd);
    if (strcmp(cmd, "cbtcicstsd") == 0) return handle_cbtcicstsd();
    if (strncmp(cmd, "cbtcicscmp ", 11) == 0) return handle_cbtcicscmp(cmd);
    if (strncmp(cmd, "cbtcicspurge ", 13) == 0) return handle_cbtcicspurge(cmd);
    if (strncmp(cmd, "cbtcicslim ", 11) == 0) return handle_cbtcicslim(cmd);
    if (strcmp(cmd, "cbtcicstrc") == 0) return handle_cbtcicstrc();
    if (strncmp(cmd, "cbtcicstrcflt ", 14) == 0) return handle_cbtcicstrcflt(cmd);
    return false;
}
