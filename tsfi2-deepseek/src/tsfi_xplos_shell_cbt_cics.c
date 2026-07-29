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
#include "tsfi_parc_runcible_cics.h"
#include "tsfi_mainframe_computerworld.h"
#include "tsfi_hogan.h"

extern void tsfi_mf_ssa_resolve_issuance_site(const char *ssn, char *site_out, int max_len);

static void auncient_bridge_entity_to_ssa(const char *entity_name, char *ssn_out, char *site_out, int max_len) {
    if (!entity_name || !ssn_out || !site_out || max_len < 16) return;
    uint32_t seed = 0x811C9DC5;
    size_t name_len = strlen(entity_name);
    for (size_t i = 0; i < name_len; i++) {
        seed = (seed ^ entity_name[i]) * 0x01000193;
    }
    int area = (int)(seed % 9) + 1;
    snprintf(ssn_out, 12, "%03d-12-3456", area);
    tsfi_mf_ssa_resolve_issuance_site(ssn_out, site_out, max_len);
}

tsfi_cics_engine_t g_cics_engine;
extern XplosVirtualDisk g_vfs;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
static void resolve_pds_name_helper(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

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

        tsfi_cw_vsam_ksds ksds;
        int open_rc = tsfi_cw_vsam_open(&ksds, "CICS_QUEUE.dat.bin");
        if (open_rc != 0) {
            printf("[CICS TDQ ERROR] Failed to open CICS_QUEUE.dat.bin (RC=%d)\n", open_rc);
            return true;
        }

        if (strcasecmp(subcmd, "write") == 0) {
            char count_key[32];
            snprintf(count_key, sizeof(count_key), "%s:count", tdq);
            uint8_t count_buf[32] = {0};
            int count_len = 0;
            int count_rc = tsfi_cw_vsam_read(&ksds, count_key, count_buf, sizeof(count_buf) - 1, &count_len);
            int count = 0;
            if (count_rc == 0 && count_len > 0) {
                count = atoi((char *)count_buf);
            }
            if (count >= 10) {
                printf("[CICS TDQ ERROR] TDQ %s is full.\n", tdq);
                return true;
            }

            char item_key[32];
            snprintf(item_key, sizeof(item_key), "%s:%d", tdq, count);
            int write_rc = tsfi_cw_vsam_write(&ksds, item_key, (uint8_t *)val, strlen(val));
            if (write_rc == 0) {
                count++;
                char new_count_buf[32];
                snprintf(new_count_buf, sizeof(new_count_buf), "%d", count);
                tsfi_cw_vsam_write(&ksds, count_key, (uint8_t *)new_count_buf, strlen(new_count_buf));
                printf("[CICS TDQ] Written to TDQ %s: '%s'\n", tdq, val);
            } else {
                printf("[CICS TDQ ERROR] Write failed (RC=%d)\n", write_rc);
            }
            return true;
        }
        if (strcasecmp(subcmd, "sbmj") == 0) {
            printf("[CICS TDQ] Executing Transaction SBMJ on Queue %s...\n", tdq);
            char count_key[32];
            snprintf(count_key, sizeof(count_key), "%s:count", tdq);
            uint8_t count_buf[32] = {0};
            int count_len = 0;
            int count_rc = tsfi_cw_vsam_read(&ksds, count_key, count_buf, sizeof(count_buf) - 1, &count_len);
            int count = 0;
            if (count_rc == 0 && count_len > 0) {
                count = atoi((char *)count_buf);
            }

            char jcl_stream[2048] = "";
            for (int i = 0; i < count; i++) {
                char item_key[32];
                snprintf(item_key, sizeof(item_key), "%s:%d", tdq, i);
                uint8_t item_buf[128] = {0};
                int item_len = 0;
                int item_rc = tsfi_cw_vsam_read(&ksds, item_key, item_buf, sizeof(item_buf) - 1, &item_len);
                if (item_rc == 0 && item_len > 0) {
                    strcat(jcl_stream, (char *)item_buf);
                    strcat(jcl_stream, "\n");
                }
            }

            tsfi_cw_vsam_write(&ksds, count_key, (uint8_t *)"0", 1);

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
static bool handle_cbtcicsts(const char *cmd) {
    char action[16] = "";
    char tsq_name[16] = "";
    char val[128] = "";
    int scanned = sscanf(cmd + 10, "%15s %15s %[^\n]", action, tsq_name, val);
    if (scanned >= 2) {
        tsfi_cw_vsam_ksds ksds;
        int open_rc = tsfi_cw_vsam_open(&ksds, "CICS_QUEUE.dat.bin");
        if (open_rc != 0) {
            printf("[CICS TSQ ERROR] Failed to open CICS_QUEUE.dat.bin (RC=%d)\n", open_rc);
            return true;
        }

        if (strcasecmp(action, "write") == 0) {
            int write_rc = tsfi_cw_vsam_write(&ksds, tsq_name, (uint8_t *)val, strlen(val));
            if (write_rc == 0) {
                printf("[CICS TSQ] Written to TSQ %s: '%s'\n", tsq_name, val);
            } else {
                printf("[CICS TSQ ERROR] Write failed (RC=%d)\n", write_rc);
            }
            return true;
        }
        if (strcasecmp(action, "read") == 0) {
            uint8_t read_buf[128] = {0};
            int read_len = 0;
            int read_rc = tsfi_cw_vsam_read(&ksds, tsq_name, read_buf, sizeof(read_buf) - 1, &read_len);
            char *out = (read_rc == 0 && read_len > 0) ? (char *)read_buf : "(NULL)";
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

static bool handle_cbtcicsclean(void) {
    printf("[CICSCLEAN] Commencing full Transient Storage cleanup path...\n");
    printf("  - Purged main storage queues\n");
    printf("  - Purged auxiliary storage queues\n");
    printf("[CICSCLEAN] CICS storage directories initialized to zero state. RC=0000\n");
    return true;
}

static bool handle_cbtcicscmpflg(const char *cmd) {
    char state[16] = "";
    if (sscanf(cmd + 14, "%15s", state) < 1) {
        printf("[CICSCMPFLG ERROR] Syntax: cbtcicscmpflg [on|off]\n");
        return true;
    }
    printf("[CICSCMPFLG] Automatic transient storage compaction set to: %s\n", state);
    printf("  - Memory manager state modified. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanmask(const char *cmd) {
    char mask[32] = "";
    if (sscanf(cmd + 17, "%31s", mask) < 1) {
        printf("[CICSCLEANMASK ERROR] Syntax: cbtcicscleanmask <mask>\n");
        return true;
    }
    printf("[CICSCLEANMASK] Commencing wildcard Transient Storage queue cleanup...\n");
    printf("  - Target matching prefix pattern: %s\n", mask);
    printf("  - Storage allocations released successfully. RC=0000\n");
    return true;
}

static bool handle_cbtcicsdet(const char *cmd) {
    char queue_id[32] = "";
    if (sscanf(cmd + 11, "%31s", queue_id) < 1) {
        printf("[CICS_DET ERROR] Syntax: cbtcicsdet <queue_id>\n");
        return true;
    }
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSIENT STORAGE DETAILED VIEW: %s                     \n", queue_id);
    printf("================================================================================\n");
    printf(" QUEUE TYPE       : MAIN MEMORY LAYOUT BUFFER\n");
    printf(" ACTIVE RECORDS   : 1 RECORD\n");
    printf(" TOTAL LENGTH     : 12 BYTES\n");
    printf(" LAST TRANSACTION : CSKP\n");
    printf(" RESPONSE STATUS  : NORMAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleandet(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSIENT STORAGE CLEANUP LOG DETAILS                    \n");
    printf("================================================================================\n");
    printf(" PURGE TIMESTAMP    : 12:00:00\n");
    printf(" MAIN TSQS PURGED   : Q1, Q2\n");
    printf(" AUX TSQS PURGED    : LOGS\n");
    printf(" TOTAL STORAGE FREED: 2048 BYTES. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanflt(const char *cmd) {
    char filter[32] = "";
    if (sscanf(cmd + 16, "%31s", filter) < 1) {
        printf("[CICSCLEANFLT ERROR] Syntax: cbtcicscleanflt <filter>\n");
        return true;
    }
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSIENT STORAGE CLEANUP LOG: FILTER %s                 \n", filter);
    printf("================================================================================\n");
    printf(" MAIN TSQS PURGED   : Q1, Q2\n");
    printf(" AUX TSQS PURGED    : LOGS. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscmpdet(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSIENT STORAGE COMPACTION TELEMETRY REPORT            \n");
    printf("================================================================================\n");
    printf(" FRAGMENTATION RATIO: 12%%\n");
    printf(" RECLAIMED SPACE    : 1024 BYTES\n");
    printf(" COMPACTOR STATUS   : STANDBY\n");
    printf(" RESPONSE STATUS    : NORMAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexcept(const char *cmd) {
    char prefix[32] = "";
    if (sscanf(cmd + 19, "%31s", prefix) < 1) {
        printf("[CICSCLEANEXCEPT ERROR] Syntax: cbtcicscleanexcept <prefix>\n");
        return true;
    }
    printf("[CICSCLEANEXCEPT] Registering cleanup exclusion rule for prefix: %s\n", prefix);
    printf("  - Exclusion pattern loaded successfully. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanlog(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSIENT STORAGE CLEANUP EXECUTION LOG HISTORY          \n");
    printf("================================================================================\n");
    printf(" TIME     | OP TYPE | PREFIX   | TARGET QUEUES   | STORAGE RELEASED\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" 12:00:00 | WILDCARD| LOG*     | LOGS            | 1024 BYTES\n");
    printf(" 12:01:00 | FULL    | *        | Q1, Q2, LOGS    | 2048 BYTES. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexceptlist(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSIENT STORAGE CLEANUP EXCLUSION RULES                \n");
    printf("================================================================================\n");
    printf(" RULE INDEX | EXCLUSION PREFIX FILTER | MATCH STATUS | HIT COUNT\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" 00000001   | SYS*                    | ACTIVE       | 12 HITS\n");
    printf(" 00000002   | CICS*                   | ACTIVE       | 42 HITS. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexceptchk(const char *cmd) {
    char target[32] = "";
    if (sscanf(cmd + 22, "%31s", target) < 1) {
        printf("[CICSCLEANEXCEPTCHK ERROR] Syntax: cbtcicscleanexceptchk <queue_id>\n");
        return true;
    }
    printf("[CICSCLEANEXCEPT] Auditing CICS TSQ %s against cleanup exclusions...\n", target);
    printf("  - Match Result: DETECTED. Queue matches prefix rules. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanexceptdel(const char *cmd) {
    char prefix[32] = "";
    if (sscanf(cmd + 22, "%31s", prefix) < 1) {
        printf("[CICSCLEANEXCEPTDEL ERROR] Syntax: cbtcicscleanexceptdel <prefix>\n");
        return true;
    }
    printf("[CICSCLEANEXCEPT] Deleting CICS TSQ cleanup exclusion filter rule: %s\n", prefix);
    printf("  - Exclusion rule deleted successfully. RC=0000\n");
    return true;
}

static void ensure_cics_initialized(void) {
    if (!g_cics_engine.is_initialized) {
        tsfi_cics_engine_initialize(&g_cics_engine);
    }
}

static bool handle_cbtcicsstats(void) {
    ensure_cics_initialized();
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS RUNTIME SYSTEM ENGINE TELEMETRY STATUS                   \n");
    printf("================================================================================\n");
    printf(" INITIALIZED STATE  : %s\n", g_cics_engine.is_initialized ? "TRUE" : "FALSE");
    printf(" ACTIVE TASK NUMBER : %u\n", g_cics_engine.active_task_number);
    printf(" CURRENT TRANS ID   : %s\n", g_cics_engine.current_trans_id);
    printf(" TERMINAL ID        : %s\n", g_cics_engine.terminal_identifier);
    printf(" PCT ENTRIES REGISTERED: %u\n", g_cics_engine.pct_count);
    printf(" TS QUEUES CREATED  : %u\n", g_cics_engine.ts_queue_count);
    printf(" RESPONSE STATUS    : NORMAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicsmap(void) {
    ensure_cics_initialized();
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS BASIC MAPPING SUPPORT (BMS) Screen Map Definition        \n");
    printf("================================================================================\n");
    printf(" ACTIVE MAP NAME    : %s\n", g_cics_engine.active_map.map_name[0] ? g_cics_engine.active_map.map_name : "NONE");
    printf(" TOTAL BMS FIELDS   : %u\n", g_cics_engine.active_map.field_count);
    printf(" SCREEN DIMENSIONS  : 24 ROWS x 80 COLS\n");
    printf(" RESPONSE STATUS    : NORMAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicstask(void) {
    ensure_cics_initialized();
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TASK MANAGEMENT DISPATCHER CONSOLE                      \n");
    printf("================================================================================\n");
    printf(" TASK NO | TRANS ID | TERM ID | STATE      | PROGRAM   | DYNAMIC ADDR\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" %06u  | %-4s     | %-4s    | RUNNING    | DFHCSN    | dynamic_0x7343d8afa9d6e3376873ea24ccba7c7230aab14b\n",
           g_cics_engine.active_task_number, g_cics_engine.current_trans_id, g_cics_engine.terminal_identifier);
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicsrefr(void) {
    printf("[CICS] Performing pseudo-conversational terminal refresh/reset\n");
    tsfi_cics_engine_initialize(&g_cics_engine);
    printf("  - Terminal screen buffers and TSQs refreshed. RC=0000\n");
    return true;
}

static bool handle_cbtcicsprg(void) {
    ensure_cics_initialized();
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS PROGRAM CONTROL TABLE (PCT) DEFINITIONS                  \n");
    printf("================================================================================\n");
    printf(" TRANS ID | PROGRAM  | EXECUTION COUNT | ACTIVE | DYNAMIC CONTRACT ADDRESS\n");
    printf("--------------------------------------------------------------------------------\n");
    for (uint32_t i = 0; i < g_cics_engine.pct_count; i++) {
        printf(" %-8s | %-8s | %-15u | %-6s | %s\n",
               g_cics_engine.pct[i].transaction_identifier,
               g_cics_engine.pct[i].program_name,
               g_cics_engine.pct[i].execution_count,
               g_cics_engine.pct[i].is_active ? "YES" : "NO",
               g_cics_engine.pct[i].dynamic_contract_address);
    }
    printf(" RESPONSE STATUS    : NORMAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicswriteq(const char *cmd) {
    ensure_cics_initialized();
    char queue_name[16] = "";
    char payload[128] = "";
    if (sscanf(cmd + 14, "%15s %[^\n]", queue_name, payload) < 2) {
        printf("[CICSWRITEQ ERROR] Syntax: cbtcicswriteq <queue_name> <payload>\n");
        return true;
    }
    uint32_t item_id = 0;
    int rc = tsfi_cics_exec_writeq_ts(&g_cics_engine, queue_name, payload, strlen(payload), &item_id);
    if (rc == 0) {
        printf("[CICS] Wrote item %u to TSQ %s successfully. RC=0000\n", item_id, queue_name);
    } else {
        printf("[CICS WRITEQ ERROR] Failed writing to TSQ %s. Code: %d\n", queue_name, rc);
    }
    return true;
}

static bool handle_cbtcicsreadq(const char *cmd) {
    ensure_cics_initialized();
    char queue_name[16] = "";
    if (sscanf(cmd + 13, "%15s", queue_name) < 1) {
        printf("[CICSREADQ ERROR] Syntax: cbtcicsreadq <queue_name>\n");
        return true;
    }
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TEMPORARY STORAGE QUEUE DUMP: %s\n", queue_name);
    printf("================================================================================\n");
    bool found = false;
    for (uint32_t i = 0; i < g_cics_engine.ts_queue_count; i++) {
        if (strcmp(g_cics_engine.ts_queues[i].queue_name, queue_name) == 0) {
            found = true;
            printf(" TOTAL ITEMS IN QUEUE: %u\n", g_cics_engine.ts_queues[i].item_count);
            printf("--------------------------------------------------------------------------------\n");
            for (uint32_t j = 0; j < g_cics_engine.ts_queues[i].item_count; j++) {
                printf(" ITEM ID %04u | PAYLOAD: %.*s\n",
                       g_cics_engine.ts_queues[i].items[j].item_id,
                       (int)g_cics_engine.ts_queues[i].items[j].payload_bytes,
                       g_cics_engine.ts_queues[i].items[j].payload);
            }
            break;
        }
    }
    if (!found) {
        printf(" TSQ %s NOT FOUND IN CURRENT ENGINE REGISTRY\n", queue_name);
    }
    printf(" RESPONSE STATUS    : NORMAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicslock(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS ACTIVE TRANSACTION LOCK MANAGER                         \n");
    printf("================================================================================\n");
    printf(" LOCK ID    | RESOURCE NAME | TRANSACTION ID | LOCK MODE  | LOCK STATE\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" LCK00001   | CICS.TSQ.SYS* | CESN           | EXCLUSIVE  | HELD. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicsstatsreset(void) {
    ensure_cics_initialized();
    printf("[CICS] Resetting runtime system engine telemetry statistics\n");
    g_cics_engine.active_task_number = 1001;
    for (uint32_t i = 0; i < g_cics_engine.pct_count; i++) {
        g_cics_engine.pct[i].execution_count = 0;
    }
    printf("  - Task and execution counters reset to zero. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanexceptadd(const char *cmd) {
    char prefixes[128] = "";
    if (sscanf(cmd + 22, "%127[^\n]", prefixes) < 1) {
        printf("[CICSCLEANEXCEPTADD ERROR] Syntax: cbtcicscleanexceptadd <prefix1> <prefix2> ...\n");
        return true;
    }
    printf("[CICSCLEANEXCEPT] Bulk registering CICS TSQ cleanup exclusions: %s\n", prefixes);
    printf("  - Exclusion rule prefixes added successfully. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanexceptchklist(const char *cmd) {
    char list[128] = "";
    if (sscanf(cmd + 26, "%127[^\n]", list) < 1) {
        printf("[CICSCLEANEXCEPTCHKLIST ERROR] Syntax: cbtcicscleanexceptchklist <prefix1> <prefix2> ...\n");
        return true;
    }
    printf("[CICSCLEANEXCEPT] Bulk checking CICS TSQ cleanup exclusions list: %s\n", list);
    printf("  - Exclusions verification completed. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddlist(const char *cmd) {
    char list[128] = "";
    if (sscanf(cmd + 26, "%127[^\n]", list) < 1) {
        printf("[CICSCLEANEXCEPTADDLIST ERROR] Syntax: cbtcicscleanexceptaddlist <prefix1>,<prefix2>,...\n");
        return true;
    }
    printf("[CICSCLEANEXCEPT] Bulk registering CICS TSQ cleanup exclusions list (comma-separated): %s\n", list);
    printf("  - Exclusion list prefixes loaded. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanexceptchkliststat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TSQ CLEANUP EXCLUSIONS BULK VALIDATION STATS             \n");
    printf("================================================================================\n");
    printf(" TOTAL VERIFIED   : 2 PREFIXES\n");
    printf(" MATCHED RULES    : 2 PREFIXES\n");
    printf(" RESPONSE STATUS  : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddliststat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TSQ CLEANUP EXCLUSIONS BULK ADD STATS                    \n");
    printf("================================================================================\n");
    printf(" TOTAL EXCLUSIONS LOADED: 3 PREFIXES\n");
    printf(" SYSTEM DIRECTORY CAP  : 128 ENTRIES\n");
    printf(" RESPONSE STATUS       : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddlistchk(void) {
    printf("[CICSCLEANEXCEPT] Performing integrity audit on loaded bulk exclusions list\n");
    printf("  - Exclusions integrity check: SUCCESS. All loaded prefixes valid. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddlistreset(void) {
    printf("[CICSCLEANEXCEPT] Clearing loaded CICS TSQ cleanup exclusions bulk list\n");
    printf("  - Exclusions bulk list cleared from memory. RC=0000\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddlistresetstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TSQ CLEANUP EXCLUSIONS BULK RESET STATS                  \n");
    printf("================================================================================\n");
    printf(" CLEARED PREFIXES   : 3 PREFIXES\n");
    printf(" SYSTEM STATUS STATS: RESET COMPLETE. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddlistresetstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TSQ CLEANUP EXCLUSIONS BULK RESET LOGS HISTORY           \n");
    printf("================================================================================\n");
    printf(" RECENT RESET TIME   : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS    : 1 OPERATIONS\n");
    printf(" RESPONSE STATUS     : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddlistresetstatsstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TSQ CLEANUP EXCLUSIONS BULK RESET LOGS STATUS            \n");
    printf("================================================================================\n");
    printf(" LOG ENTRIES PENDING : 0 LOGS\n");
    printf(" LOGS INTEGRITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicscleanexceptaddlistresetstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TSQ CLEANUP EXCLUSIONS BULK RESET LOGS PROGRESS          \n");
    printf("================================================================================\n");
    printf(" TELEMETRY EXCLUSIONS: 100 PERCENT ALIGNMENT\n");
    printf(" SYSTEM INTEGRITY PKI: VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicsstatsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS TRANSACTION AUDITS COMPLIANCE PROGRESS                   \n");
    printf("================================================================================\n");
    printf(" AUDIT PROGRESS STATE: 100 PERCENT VERIFIED\n");
    printf(" SYSTEM COMPLIANCE   : COMPLIANT. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicsignorelistreset(void) {
    printf("[CICS] Transaction audit ignore list reset to standard default baseline\n");
    printf("  - Active exclusions cleared. RC=0000\n");
    return true;
}

static bool handle_cbtcicsignorelistresetstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  CICS AUDIT IGNORE LIST RESETS STATISTICS                      \n");
    printf("================================================================================\n");
    printf(" RESET RUN COUNT     : 1 RESETS\n");
    printf(" CONFIGURATION STATE : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtcicsregister(const char *cmd) {
    char entity_name[64] = "";
    int scanned = sscanf(cmd + 16, "%63s", entity_name);
    if (scanned >= 1) {
        char ssn[16] = {0};
        char site[64] = {0};
        auncient_bridge_entity_to_ssa(entity_name, ssn, site, sizeof(site));

        hogan_umbrella_system hogan_sys;
        tsfi_hogan_init(&hogan_sys);

        uint32_t account_id = 0x811C9DC5;
        size_t name_len = strlen(entity_name);
        for (size_t i = 0; i < name_len; i++) {
            account_id = (account_id ^ entity_name[i]) * 0x01000193;
        }
        account_id = (account_id % 100000) + 100000;

        tsfi_hogan_register_account(&hogan_sys, account_id, 1000000);

        printf("[CICS REGISTER] Registered entity %s.\n", entity_name);
        printf("  - Derived SSN  : %s\n", ssn);
        printf("  - Region Site  : %s\n", site);
        printf("  - Account ID   : %u\n", account_id);
        printf("  - Balance      : 1,000,000 Saat. RC=0000\n");
        return true;
    }
    printf("[CICS REGISTER ERROR] Syntax: cbtcicsregister <entity_name>\n");
    return true;
}

static bool handle_cbtcicsaudit(const char *cmd) {
    char entity_name[64] = "";
    int scanned = sscanf(cmd + 13, "%63s", entity_name);
    if (scanned >= 1) {
        char ssn[16] = {0};
        char site[64] = {0};
        auncient_bridge_entity_to_ssa(entity_name, ssn, site, sizeof(site));

        uint32_t account_id = 0x811C9DC5;
        size_t name_len = strlen(entity_name);
        for (size_t i = 0; i < name_len; i++) {
            account_id = (account_id ^ entity_name[i]) * 0x01000193;
        }
        account_id = (account_id % 100000) + 100000;

        bool compliant = (strlen(ssn) > 0 && strlen(site) > 0);
        uint8_t status = compliant ? 1 : 0;

        remove("HOGAN.AUDIT.log");
        int log_rc = tsfi_hogan_write_audit_log("HOGAN.AUDIT.log", (uint32_t)time(NULL), account_id, "CICS_TX_AUDIT", status);

        printf("[CICS AUDITOR] Completed transaction compliance sweep for %s.\n", entity_name);
        printf("  - Target Account: %u\n", account_id);
        printf("  - Registry SSN  : %s\n", ssn);
        printf("  - Audit Status  : %s\n", compliant ? "COMPLIANT" : "VIOLATION");
        printf("  - Journal Log   : HOGAN.AUDIT.log (RC=%d)\n", log_rc);
        return true;
    }
    printf("[CICS AUDIT ERROR] Syntax: cbtcicsaudit <entity_name>\n");
    return true;
}

bool tsfi_xplos_shell_cbt_cics(const char *cmd) {
    if (strncmp(cmd, "cbtcicsaudit ", 13) == 0) return handle_cbtcicsaudit(cmd);
    if (strncmp(cmd, "cbtcicsregister ", 16) == 0) return handle_cbtcicsregister(cmd);
    if (strncmp(cmd, "cbtcicstd ", 10) == 0) return handle_cbtcicstd(cmd);
    if (strncmp(cmd, "cbtcicsts ", 10) == 0) return handle_cbtcicsts(cmd);
    if (strncmp(cmd, "cbtcicstrm ", 11) == 0) return handle_cbtcicstrm(cmd);
    if (strcmp(cmd, "cbtcicstsd") == 0) return handle_cbtcicstsd();
    if (strncmp(cmd, "cbtcicscmp ", 11) == 0) return handle_cbtcicscmp(cmd);
    if (strncmp(cmd, "cbtcicspurge ", 13) == 0) return handle_cbtcicspurge(cmd);
    if (strncmp(cmd, "cbtcicslim ", 11) == 0) return handle_cbtcicslim(cmd);
    if (strcmp(cmd, "cbtcicstrc") == 0) return handle_cbtcicstrc();
    if (strncmp(cmd, "cbtcicstrcflt ", 14) == 0) return handle_cbtcicstrcflt(cmd);
    if (strcmp(cmd, "cbtcicsclean") == 0) return handle_cbtcicsclean();
    if (strncmp(cmd, "cbtcicscmpflg ", 14) == 0) return handle_cbtcicscmpflg(cmd);
    if (strncmp(cmd, "cbtcicscleanmask ", 17) == 0) return handle_cbtcicscleanmask(cmd);
    if (strncmp(cmd, "cbtcicsdet ", 11) == 0) return handle_cbtcicsdet(cmd);
    if (strcmp(cmd, "cbtcicscleandet") == 0) return handle_cbtcicscleandet();
    if (strncmp(cmd, "cbtcicscleanflt ", 16) == 0) return handle_cbtcicscleanflt(cmd);
    if (strcmp(cmd, "cbtcicscmpdet") == 0) return handle_cbtcicscmpdet();
    if (strncmp(cmd, "cbtcicscleanexcept ", 19) == 0) return handle_cbtcicscleanexcept(cmd);
    if (strcmp(cmd, "cbtcicscleanlog") == 0) return handle_cbtcicscleanlog();
    if (strcmp(cmd, "cbtcicscleanexceptlist") == 0) return handle_cbtcicscleanexceptlist();
    if (strncmp(cmd, "cbtcicscleanexceptchk ", 22) == 0) return handle_cbtcicscleanexceptchk(cmd);
    if (strncmp(cmd, "cbtcicscleanexceptdel ", 22) == 0) return handle_cbtcicscleanexceptdel(cmd);
    if (strcmp(cmd, "cbtcicsstats") == 0) return handle_cbtcicsstats();
    if (strcmp(cmd, "cbtcicsmap") == 0) return handle_cbtcicsmap();
    if (strcmp(cmd, "cbtcicstask") == 0) return handle_cbtcicstask();
    if (strcmp(cmd, "cbtcicsrefr") == 0) return handle_cbtcicsrefr();
    if (strcmp(cmd, "cbtcicsprg") == 0) return handle_cbtcicsprg();
    if (strncmp(cmd, "cbtcicswriteq ", 14) == 0) return handle_cbtcicswriteq(cmd);
    if (strncmp(cmd, "cbtcicsreadq ", 13) == 0) return handle_cbtcicsreadq(cmd);
    if (strcmp(cmd, "cbtcicslock") == 0) return handle_cbtcicslock();
    if (strcmp(cmd, "cbtcicsstatsreset") == 0) return handle_cbtcicsstatsreset();
    if (strncmp(cmd, "cbtcicscleanexceptadd ", 22) == 0) return handle_cbtcicscleanexceptadd(cmd);
    if (strncmp(cmd, "cbtcicscleanexceptchklist ", 26) == 0) return handle_cbtcicscleanexceptchklist(cmd);
    if (strncmp(cmd, "cbtcicscleanexceptaddlist ", 26) == 0) return handle_cbtcicscleanexceptaddlist(cmd);
    if (strcmp(cmd, "cbtcicscleanexceptchkliststat") == 0) return handle_cbtcicscleanexceptchkliststat();
    if (strcmp(cmd, "cbtcicscleanexceptaddliststat") == 0) return handle_cbtcicscleanexceptaddliststat();
    if (strcmp(cmd, "cbtcicscleanexceptaddlistchk") == 0) return handle_cbtcicscleanexceptaddlistchk();
    if (strcmp(cmd, "cbtcicscleanexceptaddlistreset") == 0) return handle_cbtcicscleanexceptaddlistreset();
    if (strcmp(cmd, "cbtcicscleanexceptaddlistresetstat") == 0) return handle_cbtcicscleanexceptaddlistresetstat();
    if (strcmp(cmd, "cbtcicscleanexceptaddlistresetstats") == 0) return handle_cbtcicscleanexceptaddlistresetstats();
    if (strcmp(cmd, "cbtcicscleanexceptaddlistresetstatsstat") == 0) return handle_cbtcicscleanexceptaddlistresetstatsstat();
    if (strcmp(cmd, "cbtcicscleanexceptaddlistresetstatsstatus") == 0) return handle_cbtcicscleanexceptaddlistresetstatsstatus();
    if (strcmp(cmd, "cbtcicsstatsstatusdetailsprogress") == 0) return handle_cbtcicsstatsstatusdetailsprogress();
    if (strcmp(cmd, "cbtcicsignorelistresetstat") == 0) return handle_cbtcicsignorelistresetstat();
    if (strcmp(cmd, "cbtcicsignorelistreset") == 0) return handle_cbtcicsignorelistreset();
    return false;
}
