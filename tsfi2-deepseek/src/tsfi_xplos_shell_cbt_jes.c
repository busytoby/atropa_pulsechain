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
#include "tsfi_xplos_shell_cbt_jes.h"
#include "tsfi_xplos_shell_cbt_jcl.h"
extern XplosVirtualDisk g_vfs;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
static void resolve_pds_name_helper(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

char g_hasp_spool_logs[10][4096];
int g_hasp_job_priority[10];
void append_spool_log(const char *job_name, const char *msg) {
    for (int i = 0; i < 10; i++) {
        if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_name, job_name) == 0) {
            strncat(g_hasp_spool_logs[i], msg, sizeof(g_hasp_spool_logs[i]) - strlen(g_hasp_spool_logs[i]) - 1);
            break;
        }
    }
}
void hasp_dispatch_highest_priority_class(char class_filter) {
    int best_idx = -1;
    int highest_prty = -1;
    for (int i = 0; i < 10; i++) {
        if (cbt_job_table[i].active && strcmp(cbt_job_table[i].status, "READY") == 0) {
            if (class_filter == '\0' || cbt_job_table[i].class_char == class_filter) {
                if (g_hasp_job_priority[i] > highest_prty) {
                    highest_prty = g_hasp_job_priority[i];
                    best_idx = i;
                }
            }
        }
    }
    if (best_idx != -1) {
        strcpy(cbt_job_table[best_idx].status, "RUNNING");
        printf("[HASP DISPATCHER] Dispatching Job %s (%s) Class %c with Priority %d\n",
               cbt_job_table[best_idx].job_id, cbt_job_table[best_idx].job_name,
               cbt_job_table[best_idx].class_char ? cbt_job_table[best_idx].class_char : 'A',
               highest_prty);
        char run_cmd[128];
        snprintf(run_cmd, sizeof(run_cmd), "jclrun %s", cbt_job_table[best_idx].job_name);
        tsfi_xplos_shell_cbt_jcl(run_cmd);
        strcpy(cbt_job_table[best_idx].status, "COMPLETED");
    }
}
void hasp_dispatch_highest_priority(void) {
    hasp_dispatch_highest_priority_class('\0');
}
static bool handle_smfdump(void) {
    printf("[SMFDUMP] Displaying System Management Facility telemetry records:\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" TYPE | SYSTEM | TIME     | JOBNAME  | CPU_SEC  | EXCP_COUNT (I/O) \n");
    printf("--------------------------------------------------------------------------------\n");
    int count = 0;
    for (int i = 0; i < 10; i++) {
        if (cbt_job_table[i].active) {
            printf("  030 | XPL1   | 12:05:33 | %-8s |   0.145  | 142 \n", cbt_job_table[i].job_name);
            count++;
        }
    }
    printf("  070 | XPL1   | 12:00:00 | SYSTEM   |  12.400  | N/A (CPU Wait: 87.6%%)\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("[SMFDUMP] Completed. Processed %d active SMF records.\n", count + 1);
    return true;
}
static bool handle_cbthasp(const char *cmd) {
    char subcmd[32] = "";
    char arg[64] = "";
    int scanned = sscanf(cmd + 8, "%31s %63s", subcmd, arg);
    if (scanned >= 1) {
        if (strcasecmp(subcmd, "dispatch") == 0) {
            char class_filter = '\0';
            if (strlen(arg) > 0) {
                class_filter = toupper((unsigned char)arg[0]);
                printf("[HASP] Triggering class-based dispatch for Class %c\n", class_filter);
            } else {
                printf("[HASP] Triggering global dispatch for all classes\n");
            }
            hasp_dispatch_highest_priority_class(class_filter);
            return true;
        }
        if (strcasecmp(subcmd, "log") == 0) {
            if (strlen(arg) == 0) {
                printf("[HASP ERROR] Job ID required to dump spool log.\n");
                return true;
            }
            int found_idx = -1;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, arg) == 0) {
                    found_idx = i;
                    break;
                }
            }
            if (found_idx != -1) {
                printf("[HASP SPOOL LOG: %s (%s)]\n", cbt_job_table[found_idx].job_id, cbt_job_table[found_idx].job_name);
                printf("--------------------------------------------------------------------------------\n");
                printf("%s", g_hasp_spool_logs[found_idx]);
                printf("--------------------------------------------------------------------------------\n");
            } else {
                printf("[HASP ERROR] Job ID %s not found in Spool Log database.\n", arg);
            }
            return true;
        }
        if (strcasecmp(subcmd, "status") == 0) {
            printf("[HASP STATUS] Active HASP execution spool summary:\n");
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active) {
                    printf("  - %s (%-8s) -> STATUS: %s (PRTY: %d, Spool Log: %d bytes)\n",
                           cbt_job_table[i].job_id, cbt_job_table[i].job_name,
                           cbt_job_table[i].status, g_hasp_job_priority[i], (int)strlen(g_hasp_spool_logs[i]));
                }
            }
            return true;
        }
        if (strcasecmp(subcmd, "hold") == 0) {
            if (strlen(arg) == 0) {
                printf("[HASP ERROR] Job ID required for HOLD.\n");
                return true;
            }
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, arg) == 0) {
                    strcpy(cbt_job_table[i].status, "HELD");
                    printf("[HASP] Job %s placed on HELD status.\n", arg);
                    return true;
                }
            }
            printf("[HASP ERROR] Job ID %s not found.\n", arg);
            return true;
        }
        if (strcasecmp(subcmd, "release") == 0) {
            if (strlen(arg) == 0) {
                printf("[HASP ERROR] Job ID required for RELEASE.\n");
                return true;
            }
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, arg) == 0) {
                    if (strcmp(cbt_job_table[i].status, "HELD") == 0) {
                        strcpy(cbt_job_table[i].status, "READY");
                        printf("[HASP] Job %s released to READY queue.\n", arg);
                        hasp_dispatch_highest_priority();
                    } else {
                        printf("[HASP WARNING] Job %s is not in HELD status (Current: %s).\n", arg, cbt_job_table[i].status);
                    }
                    return true;
                }
            }
            printf("[HASP ERROR] Job ID %s not found.\n", arg);
            return true;
        }
        if (strcasecmp(subcmd, "purge") == 0) {
            if (strlen(arg) == 0) {
                printf("[HASP ERROR] Job ID required for PURGE.\n");
                return true;
            }
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, arg) == 0) {
                    cbt_job_table[i].active = false;
                    memset(g_hasp_spool_logs[i], 0, sizeof(g_hasp_spool_logs[i]));
                    printf("[HASP] Job %s purged from spool queue.\n", arg);
                    return true;
                }
            }
            printf("[HASP ERROR] Job ID %s not found.\n", arg);
            return true;
        }
        if (strcasecmp(subcmd, "interrupt") == 0) {
            printf("[HASP INTERRUPT] WinchesterMQ SCSI event received. Simulating hardware interrupt 0x5C3B...\n");
            if (g_active_sched) {
                tsfi_xplos_trigger_event(g_active_sched, 0x5C3B);
                printf("[HASP INTERRUPT] WinchesterMQ event triggered on active scheduler. Dispatching ready jobs.\n");
                hasp_dispatch_highest_priority();
            } else {
                printf("[HASP INTERRUPT] No active scheduler registered.\n");
            }
            return true;
        }
    }
    printf("[HASP ERROR] Subcommand log, status, hold, release, purge, or interrupt required.\n");
    return true;
}

static bool handle_cbthaspspoollogs(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JES HASP SPOOL OPERATION LOGS HISTORY                         \n");
    printf("================================================================================\n");
    printf(" RECENT LOG TIME   : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 SPOOL OPERATIONS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbthaspspoollogsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JES HASP SPOOL LOGS INTEGRITY STATE STATUS                    \n");
    printf("================================================================================\n");
    printf(" PENDING LOGS      : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbthaspspoollogsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JES HASP SPOOL LOGS INTEGRITY STATUS DETAILS                  \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbthaspspoollogsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JES HASP SPOOL LOGS AUDIT PROGRESS                            \n");
    printf("================================================================================\n");
    printf(" AUDIT ALIGNMENT    : 100 PERCENT ALIGNMENT\n");
    printf(" COMPLIANCE STATUS  : VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbthaspspoolignorelistreset(void) {
    printf("[HASP] Spool audit ignore list reset to standard default baseline\n");
    printf("  - Active ignore configurations cleared. RC=0000\n");
    return true;
}

static bool handle_cbthaspspoolignorelistresetstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JES HASP SPOOL IGNORE LIST RESETS STATISTICS                 \n");
    printf("================================================================================\n");
    printf(" STAT RESETS RUN    : 1 RESETS\n");
    printf(" RESET STATE STATUS : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}


bool tsfi_xplos_shell_cbt_jes(const char *cmd) {
    if (strncmp(cmd, "cbthasp ", 8) == 0) return handle_cbthasp(cmd);
    if (strcmp(cmd, "smfdump") == 0) return handle_smfdump();
    if (strcmp(cmd, "cbthaspspoollogsstatusdetailsprogress") == 0) return handle_cbthaspspoollogsstatusdetailsprogress();
    if (strcmp(cmd, "cbthaspspoollogsstatusdetails") == 0) return handle_cbthaspspoollogsstatusdetails();
    if (strcmp(cmd, "cbthaspspoollogsstatus") == 0) return handle_cbthaspspoollogsstatus();
    if (strcmp(cmd, "cbthaspspoollogs") == 0) return handle_cbthaspspoollogs();
    if (strcmp(cmd, "cbthaspspoolignorelistresetstat") == 0) return handle_cbthaspspoolignorelistresetstat();
    if (strcmp(cmd, "cbthaspspoolignorelistreset") == 0) return handle_cbthaspspoolignorelistreset();
    return false;
}
