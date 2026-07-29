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
#include "tsfi_xplos_shell_cbt_extra.h"

// Variable references defined in kernel
extern XplosVirtualDisk g_vfs;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;

// Helper to resolve PDS member names to VFS filename
static void resolve_pds_name_extra(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

// -----------------------------------------------------------------------------
// 1. Usenet-over-SNA Newsreader & Poster Implementation
// -----------------------------------------------------------------------------
static bool handle_cbtnet(const char *args) {
    if (strncmp(args, "status", 6) == 0) {
        printf("[CBTNET] Usenet-over-SNA Active Connections:\n");
        printf("  - Virtual LU 1: CDRM_LOCAL -> ACTIVE\n");
        printf("  - Virtual LU 2: CDRM_PEER  -> ACTIVE\n");
        printf("  - Route Group:  net.books  -> REPLICATING via SNA Path Information Units (PIUs)\n");
        printf("[CBTNET] Status check completed successfully.\n");
        return true;
    }

    if (strncmp(args, "post ", 5) == 0) {
        char group[64] = "";
        char author[64] = "";
        char subject[128] = "";
        char body[256] = "";

        // Parse args: cbtnet post <group> | <author> | <subject> | <body>
        const char *p = args + 5;
        char *g_ptr = strchr(p, '|');
        if (!g_ptr) {
            printf("[CBTNET ERROR] Invalid syntax. Format: cbtnet post group | author | subject | body\n");
            return true;
        }
        size_t g_len = g_ptr - p;
        if (g_len >= sizeof(group)) g_len = sizeof(group) - 1;
        memcpy(group, p, g_len);
        group[g_len] = '\0';
        // Trim spaces
        char *g_trimmed = group;
        while (isspace((unsigned char)*g_trimmed)) g_trimmed++;
        char *g_end = g_trimmed + strlen(g_trimmed) - 1;
        while (g_end > g_trimmed && isspace((unsigned char)*g_end)) {
            *g_end = '\0';
            g_end--;
        }

        p = g_ptr + 1;
        char *a_ptr = strchr(p, '|');
        if (!a_ptr) {
            printf("[CBTNET ERROR] Invalid syntax. Missing author separator.\n");
            return true;
        }
        size_t a_len = a_ptr - p;
        if (a_len >= sizeof(author)) a_len = sizeof(author) - 1;
        memcpy(author, p, a_len);
        author[a_len] = '\0';
        char *a_trimmed = author;
        while (isspace((unsigned char)*a_trimmed)) a_trimmed++;
        char *a_end = a_trimmed + strlen(a_trimmed) - 1;
        while (a_end > a_trimmed && isspace((unsigned char)*a_end)) {
            *a_end = '\0';
            a_end--;
        }

        p = a_ptr + 1;
        char *s_ptr = strchr(p, '|');
        if (!s_ptr) {
            printf("[CBTNET ERROR] Invalid syntax. Missing subject separator.\n");
            return true;
        }
        size_t s_len = s_ptr - p;
        if (s_len >= sizeof(subject)) s_len = sizeof(subject) - 1;
        memcpy(subject, p, s_len);
        subject[s_len] = '\0';
        char *s_trimmed = subject;
        while (isspace((unsigned char)*s_trimmed)) s_trimmed++;
        char *s_end = s_trimmed + strlen(s_trimmed) - 1;
        while (s_end > s_trimmed && isspace((unsigned char)*s_end)) {
            *s_end = '\0';
            s_end--;
        }

        strncpy(body, s_ptr + 1, sizeof(body) - 1);
        char *b_trimmed = body;
        while (isspace((unsigned char)*b_trimmed)) b_trimmed++;
        char *b_end = b_trimmed + strlen(b_trimmed) - 1;
        while (b_end > b_trimmed && isspace((unsigned char)*b_end)) {
            *b_end = '\0';
            b_end--;
        }

        char vfs_filename[128];
        snprintf(vfs_filename, sizeof(vfs_filename), "%s.dat.bin", g_trimmed);

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
                file_idx = i;
                break;
            }
        }

        char new_post[1024];
        time_t t = time(NULL);
        struct tm *tm_info = gmtime(&t);
        char date_str[64];
        strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S GMT", tm_info);

        snprintf(new_post, sizeof(new_post),
                 "From: %s\n"
                 "Subject: %s\n"
                 "Date: %s\n"
                 "Newsgroups: %s\n"
                 "\n"
                 "%s\n"
                 "===ARTICLE_END===\n",
                 a_trimmed, s_trimmed, date_str, g_trimmed, b_trimmed);

        if (file_idx >= 0) {
            size_t curr_len = strlen(g_vfs.files[file_idx].data);
            if (curr_len + strlen(new_post) < sizeof(g_vfs.files[file_idx].data)) {
                strcat(g_vfs.files[file_idx].data, new_post);
                g_vfs.files[file_idx].size_bytes = (uint32_t)strlen(g_vfs.files[file_idx].data);
                printf("[CBTNET] Article successfully appended to Usenet spool '%s'\n", vfs_filename);
            } else {
                printf("[CBTNET ERROR] Usenet newsgroup buffer overflow.\n");
            }
        } else {
            if (tsfi_xplos_create_file(&g_vfs, vfs_filename, 64 * 1024)) {
                XplosFile *vf = &g_vfs.files[g_vfs.count - 1];
                strcpy(vf->data, new_post);
                vf->size_bytes = (uint32_t)strlen(vf->data);
                printf("[CBTNET] Created new Usenet newsgroup database '%s' and posted article.\n", vfs_filename);
            } else {
                printf("[CBTNET ERROR] Failed to create newsgroup VFS file.\n");
            }
        }
        return true;
    }

    if (strncmp(args, "read ", 5) == 0) {
        char group[64] = "";
        sscanf(args + 5, "%63s", group);
        char vfs_filename[128];
        snprintf(vfs_filename, sizeof(vfs_filename), "%s.dat.bin", group);

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
                file_idx = i;
                break;
            }
        }

        if (file_idx >= 0) {
            printf("[CBTNET] Reading articles from newsgroup: %s\n", group);
            printf("--------------------------------------------------------------------------------\n");
            printf("%s", g_vfs.files[file_idx].data);
            printf("--------------------------------------------------------------------------------\n");
        } else {
            printf("[CBTNET ERROR] Newsgroup '%s' contains no active articles.\n", group);
        }
        return true;
    }

    printf("[CBTNET ERROR] Unknown sub-command. Options: status, post, read\n");
    return true;
}

// -----------------------------------------------------------------------------
// 2. Native REXX Interpreter / Script Parser Implementation
// -----------------------------------------------------------------------------
static void execute_rexx_script(const char *script_data, const char *script_args) {
    char lines[128][128];
    int line_count = 0;
    
    // Tokenize lines
    char *temp_data = strdup(script_data);
    char *line = strtok(temp_data, "\n");
    while (line && line_count < 128) {
        // Skip leading whitespace
        while (isspace((unsigned char)*line)) line++;
        // Skip comments starting with /* and ending with */
        if (strncmp(line, "/*", 2) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }
        if (strlen(line) > 0) {
            strncpy(lines[line_count], line, sizeof(lines[line_count]) - 1);
            lines[line_count][sizeof(lines[line_count]) - 1] = '\0';
            line_count++;
        }
        line = strtok(NULL, "\n");
    }
    free(temp_data);

    // Simple Variable Registry
    char var_names[32][32];
    char var_vals[32][128];
    int var_count = 0;

    // Pre-populate ARG variable
    if (script_args && strlen(script_args) > 0) {
        strcpy(var_names[var_count], "ARG");
        strncpy(var_vals[var_count], script_args, sizeof(var_vals[var_count]) - 1);
        var_count++;
    }

    printf("[CBTREXX EXECUTION START]\n");

    int pc = 0;
    while (pc < line_count) {
        char *current = lines[pc];
        
        // Handle SAY statement
        if (strncmp(current, "SAY ", 4) == 0 || strncmp(current, "say ", 4) == 0) {
            const char *expr = current + 4;
            // Trim enclosing quotes if present
            if (expr[0] == '"' || expr[0] == '\'') {
                char quote = expr[0];
                char printed[128];
                strncpy(printed, expr + 1, sizeof(printed) - 1);
                printed[sizeof(printed) - 1] = '\0';
                char *end_q = strchr(printed, quote);
                if (end_q) *end_q = '\0';
                printf("  REXX_OUT> %s\n", printed);
            } else {
                // Check if variable
                bool found_var = false;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(var_names[i], expr) == 0) {
                        printf("  REXX_OUT> %s\n", var_vals[i]);
                        found_var = true;
                        break;
                    }
                }
                if (!found_var) {
                    printf("  REXX_OUT> %s\n", expr);
                }
            }
        }
        // Handle variable assignment: VAR = VALUE
        else if (strchr(current, '=')) {
            char name[32] = "";
            char value[128] = "";
            if (sscanf(current, "%31[^=] = %127[^\n]", name, value) == 2) {
                // Trim name
                char *n_trimmed = name;
                while (isspace((unsigned char)*n_trimmed)) n_trimmed++;
                char *n_end = n_trimmed + strlen(n_trimmed) - 1;
                while (n_end > n_trimmed && isspace((unsigned char)*n_end)) {
                    *n_end = '\0';
                    n_end--;
                }
                
                // Trim value quotes
                char *v_trimmed = value;
                while (isspace((unsigned char)*v_trimmed)) v_trimmed++;
                if (v_trimmed[0] == '"' || v_trimmed[0] == '\'') {
                    char q = v_trimmed[0];
                    v_trimmed++;
                    char *v_end = strchr(v_trimmed, q);
                    if (v_end) *v_end = '\0';
                }

                // Check if already registered
                int v_idx = -1;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(var_names[i], n_trimmed) == 0) {
                        v_idx = i;
                        break;
                    }
                }
                if (v_idx == -1 && var_count < 32) {
                    v_idx = var_count++;
                    strncpy(var_names[v_idx], n_trimmed, 31);
                }
                if (v_idx != -1) {
                    strncpy(var_vals[v_idx], v_trimmed, 127);
                }
            }
        }
        // Handle EXIT
        else if (strcasecmp(current, "EXIT") == 0) {
            break;
        }
        
        pc++;
    }
    printf("[CBTREXX EXECUTION COMPLETED]\n");
}

static char g_rexx_pool_names[32][32];
static char g_rexx_pool_vals[32][128];
static int g_rexx_pool_count = 0;

static char g_hasp_spool_logs[10][4096];
static int g_hasp_job_priority[10];

static void append_spool_log(const char *job_name, const char *msg) {
    for (int i = 0; i < 10; i++) {
        if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_name, job_name) == 0) {
            strncat(g_hasp_spool_logs[i], msg, sizeof(g_hasp_spool_logs[i]) - strlen(g_hasp_spool_logs[i]) - 1);
            break;
        }
    }
}

static void hasp_dispatch_highest_priority(void) {
    int best_idx = -1;
    int highest_prty = -1;
    for (int i = 0; i < 10; i++) {
        if (cbt_job_table[i].active && strcmp(cbt_job_table[i].status, "READY") == 0) {
            if (g_hasp_job_priority[i] > highest_prty) {
                highest_prty = g_hasp_job_priority[i];
                best_idx = i;
            }
        }
    }
    if (best_idx != -1) {
        strcpy(cbt_job_table[best_idx].status, "RUNNING");
        printf("[HASP DISPATCHER] Dispatching Job %s (%s) with Priority %d\n",
               cbt_job_table[best_idx].job_id, cbt_job_table[best_idx].job_name, highest_prty);
        char run_cmd[128];
        snprintf(run_cmd, sizeof(run_cmd), "jclrun %s", cbt_job_table[best_idx].job_name);
        tsfi_xplos_shell_cbt_extra(run_cmd);
        strcpy(cbt_job_table[best_idx].status, "COMPLETED");
    }
}

static bool handle_cbtrexx(const char *cmd) {
    char script_name[64] = "";
    char script_args[128] = "";
    int scanned = sscanf(cmd + 8, "%63s %[^\n]", script_name, script_args);
    if (scanned >= 1) {
        if (strcasecmp(script_name, "vput") == 0) {
            char varname[32] = "";
            char value[128] = "";
            if (sscanf(script_args, "%31s %[^\n]", varname, value) >= 1) {
                int found_idx = -1;
                for (int i = 0; i < g_rexx_pool_count; i++) {
                    if (strcmp(g_rexx_pool_names[i], varname) == 0) {
                        found_idx = i;
                        break;
                    }
                }
                if (found_idx == -1 && g_rexx_pool_count < 32) {
                    found_idx = g_rexx_pool_count++;
                    strncpy(g_rexx_pool_names[found_idx], varname, 31);
                }
                if (found_idx != -1) {
                    strncpy(g_rexx_pool_vals[found_idx], value, 127);
                    printf("[CBTREXX POOL] vput variable: %s = '%s'\n", varname, value);
                }
            }
            return true;
        }
        if (strcasecmp(script_name, "vget") == 0) {
            char varname[32] = "";
            sscanf(script_args, "%31s", varname);
            char *val = "(NULL)";
            for (int i = 0; i < g_rexx_pool_count; i++) {
                if (strcmp(g_rexx_pool_names[i], varname) == 0) {
                    val = g_rexx_pool_vals[i];
                    break;
                }
            }
            printf("[CBTREXX POOL] vget variable: %s = '%s'\n", varname, val);
            return true;
        }

        char vfs_filename[128];
        resolve_pds_name_extra(script_name, vfs_filename, sizeof(vfs_filename));

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
                file_idx = i;
                break;
            }
        }

        if (file_idx >= 0) {
            execute_rexx_script(g_vfs.files[file_idx].data, script_args);
        } else {
            // Simulated default fallback script
            printf("[CBTREXX] Warning: script '%s' not found in VFS. Running dynamic built-in simulation.\n", script_name);
            char fallback_code[512] = 
                "/* REXX */\n"
                "SAY \"Initializing cbtrexx interpreter...\"\n"
                "SAY \"Executing dynamic built-in template.\"\n"
                "SAY ARG\n"
                "EXIT\n";
            execute_rexx_script(fallback_code, script_args);
        }
        return true;
    }
    printf("[CBTREXX ERROR] Script name required.\n");
    return true;
}

// -----------------------------------------------------------------------------
// 3. Dynamic JCL Interpreter & Exec Exit Engine Implementation
// -----------------------------------------------------------------------------
static bool handle_jclrun(const char *cmd) {
    char jcl_name[64] = "";
    if (sscanf(cmd + 7, "%63s", jcl_name) == 1) {
        char vfs_filename[128];
        resolve_pds_name_extra(jcl_name, vfs_filename, sizeof(vfs_filename));

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
                file_idx = i;
                break;
            }
        }

        char jcl_data[8192];
        if (file_idx >= 0) {
            strncpy(jcl_data, g_vfs.files[file_idx].data, sizeof(jcl_data) - 1);
            jcl_data[sizeof(jcl_data) - 1] = '\0';
        } else {
            // Simulated default JCL stream
            printf("[JCLRUN] Warning: JCL member '%s' not found. Using fallback JCL execution.\n", jcl_name);
            snprintf(jcl_data, sizeof(jcl_data),
                     "//%s JOB 'CBT TAPE RUN',CLASS=A,MSGCLASS=X\n"
                     "//STEP1 EXEC PGM=IEBCOPY\n"
                     "//SYSUT1 DD DSN=CBT.V510.FILE002,DISP=SHR\n"
                     "//SYSUT2 DD DSN=CBT.V510.FILE003,DISP=OLD\n"
                     "// IF (RC = 0) THEN\n"
                     "//STEP2 EXEC PGM=IBHDRPLY\n"
                     "// ENDIF\n",
                     jcl_name);
        }

        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "[JCLRUN EXECUTION START: %s]\n", jcl_name);
        printf("%s", log_msg);
        append_spool_log(jcl_name, log_msg);

        char *line = strtok(jcl_data, "\n");
        int rc = 0;
        bool skip_block = false;

        while (line) {
            // Trim leading spaces
            while (isspace((unsigned char)*line)) line++;

            if (strncmp(line, "//", 2) == 0) {
                const char *card = line + 2;
                while (isspace((unsigned char)*card)) card++;

                if (strncmp(card, "IF ", 3) == 0 || strncmp(card, "if ", 3) == 0) {
                    snprintf(log_msg, sizeof(log_msg), "  JCL_COND> Checking condition: %s\n", card);
                    printf("%s", log_msg);
                    append_spool_log(jcl_name, log_msg);
                    if (strstr(card, "RC = 0") && rc != 0) {
                        skip_block = true;
                        snprintf(log_msg, sizeof(log_msg), "  JCL_COND> Condition Failed. Bypassing subsequent steps.\n");
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);
                    }
                }
                else if (strncmp(card, "ENDIF", 5) == 0 || strncmp(card, "endif", 5) == 0) {
                    skip_block = false;
                    snprintf(log_msg, sizeof(log_msg), "  JCL_COND> ENDIF reached.\n");
                    printf("%s", log_msg);
                    append_spool_log(jcl_name, log_msg);
                }
                else if (strstr(card, " EXEC ") || strstr(card, " exec ")) {
                    if (skip_block) {
                        snprintf(log_msg, sizeof(log_msg), "  JCL_STEP> Bypassed step: %s\n", card);
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);
                    } else {
                        char pgm_name[64] = "";
                        char *pgm_ptr = strstr(card, "PGM=");
                        if (pgm_ptr) {
                            sscanf(pgm_ptr + 4, "%63[^, \r\n]", pgm_name);
                        }
                        snprintf(log_msg, sizeof(log_msg), "  JCL_STEP> Executing step Program: %s\n", pgm_name);
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);
                        if (strcmp(pgm_name, "IEBCOPY") == 0) {
                            rc = 0;
                            snprintf(log_msg, sizeof(log_msg), "    * IEBCOPY completed successfully. RC=0000\n");
                            printf("%s", log_msg);
                            append_spool_log(jcl_name, log_msg);
                        } else if (strcmp(pgm_name, "IBHDRPLY") == 0) {
                            rc = 0;
                            snprintf(log_msg, sizeof(log_msg), "    * IBHDRPLY Automatic Reply executed. RC=0000\n");
                            printf("%s", log_msg);
                            append_spool_log(jcl_name, log_msg);
                        } else if (strcmp(pgm_name, "IEFBR14") == 0) {
                            rc = 0;
                            snprintf(log_msg, sizeof(log_msg), "    * IEFBR14 Dummy Program executed. Resolving DD dispositions. RC=0000\n");
                            printf("%s", log_msg);
                            append_spool_log(jcl_name, log_msg);
                        } else if (strcmp(pgm_name, "IKJEFT01") == 0) {
                            rc = 0;
                            snprintf(log_msg, sizeof(log_msg), "    * IKJEFT01 Terminal Monitor Program launched. RC=0000\n");
                            printf("%s", log_msg);
                            append_spool_log(jcl_name, log_msg);
                            snprintf(log_msg, sizeof(log_msg), "      TSO_TMP> Executing command from SYSTSIN: cbtrexx vput SYSVAR 953467954114363\n");
                            printf("%s", log_msg);
                            append_spool_log(jcl_name, log_msg);
                            tsfi_xplos_shell_cbt_extra("cbtrexx vput SYSVAR 953467954114363");
                        } else {
                            rc = 4;
                            snprintf(log_msg, sizeof(log_msg), "    * Program %s executed. RC=0004 (Warning)\n", pgm_name);
                            printf("%s", log_msg);
                            append_spool_log(jcl_name, log_msg);
                        }
                    }
                }
                else if (strstr(card, " DD ") || strstr(card, " dd ")) {
                    if (!skip_block) {
                        snprintf(log_msg, sizeof(log_msg), "  JCL_ALLOC> Allocation: %s\n", card);
                        printf("%s", log_msg);
                        append_spool_log(jcl_name, log_msg);
                    }
                }
            }

            line = strtok(NULL, "\n");
        }

        snprintf(log_msg, sizeof(log_msg), "[JCLRUN EXECUTION COMPLETED: Max RC=%04d]\n", rc);
        printf("%s", log_msg);
        append_spool_log(jcl_name, log_msg);
        return true;
    }
    printf("[JCLRUN ERROR] JCL member name required.\n");
    return true;
}

// -----------------------------------------------------------------------------
// 4. Interactive ISPF Terminal Menu Visualizer Implementation
// -----------------------------------------------------------------------------
static bool handle_ispfmenu(const char *cmd) {
    char option[16] = "";
    int scanned = sscanf(cmd + 8, "%15s", option);

    printf("\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("   %s                                                        \n", "Auncient ISPF Primary Option Menu");
    printf("--------------------------------------------------------------------------------\n");
    printf(" OPTION ===> %s\n\n", (scanned > 0) ? option : "");
    printf("   0  SETTINGS      - Terminal and user parameters\n");
    printf("   1  VIEW          - Display source data or listings\n");
    printf("   2  EDIT          - Create or change source data\n");
    printf("   3  UTILITIES     - Perform utility functions (VTOC, Catalog, PDS)\n");
    printf("   S  IMON          - System Status Monitor\n");
    printf("   X  EXIT          - Terminate ISPF\n");
    printf("--------------------------------------------------------------------------------\n");

    if (scanned > 0) {
        printf("[ISPFMENU] Option %s selected:\n", option);
        if (strcmp(option, "0") == 0) {
            printf("  - Terminal ID: 3278-2\n");
            printf("  - Language:    English\n");
        } else if (strcmp(option, "1") == 0) {
            printf("  - PDS Members available for viewing:\n");
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active) {
                    printf("    * %s (%u bytes)\n", g_vfs.files[i].name, g_vfs.files[i].size_bytes);
                }
            }
        } else if (strcmp(option, "2") == 0) {
            printf("  - Editor ready. Load members using standard TSO EDIT commands.\n");
        } else if (strcmp(option, "3") == 0) {
            printf("  - VTOC / Catalog Utility:\n");
            printf("    * Device: 3380 (MVSRES)\n");
            printf("    * VTOC Entries: %d datasets cataloged\n", g_vfs.count);
        } else if (strcasecmp(option, "s") == 0) {
            printf("  - System Status (IMON):\n");
            printf("    * Active tasks count: %d\n", g_active_sched ? g_active_sched->task_count : 0);
            printf("    * Outstanding console replies: 0\n");
        } else if (strcasecmp(option, "x") == 0) {
            printf("  - Exiting ISPF control layout.\n");
        } else {
            printf("  - Invalid option selected.\n");
        }
    } else {
        printf("[ISPFMENU] Displayed primary menu template. Pass option parameter to select.\n");
    }
    printf("\n");
    return true;
}

// -----------------------------------------------------------------------------
// 5. IEBUPDTE Partitioned Dataset Update Emulation
// -----------------------------------------------------------------------------
static bool handle_iebupdte(const char *cmd) {
    char sysin[64] = "";
    if (sscanf(cmd + 9, "%63s", sysin) != 1) {
        printf("[IEBUPDTE ERROR] SYSIN member name required.\n");
        return true;
    }
    char vfs_filename[128];
    resolve_pds_name_extra(sysin, vfs_filename, sizeof(vfs_filename));
    int file_idx = -1;
    for (int i = 0; i < g_vfs.count; i++) {
        if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
            file_idx = i;
            break;
        }
    }
    if (file_idx < 0) {
        printf("[IEBUPDTE ERROR] SYSIN member %s not found in VFS.\n", vfs_filename);
        return true;
    }
    
    printf("[IEBUPDTE] Commencing Partitioned Dataset Update from SYSIN: %s\n", sysin);
    char temp_data[8192];
    strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
    temp_data[sizeof(temp_data) - 1] = '\0';
    
    char *line = strtok(temp_data, "\n");
    char current_member[64] = "";
    char member_data[4096] = "";
    bool collecting = false;
    int members_updated = 0;
    
    while (line) {
        size_t len = strlen(line);
        while (len > 0 && isspace((unsigned char)line[len - 1])) {
            line[len - 1] = '\0';
            len--;
        }
        
        if (strncmp(line, "./ ADD NAME=", 12) == 0) {
            if (collecting && strlen(current_member) > 0) {
                char mem_vfs[128];
                resolve_pds_name_extra(current_member, mem_vfs, sizeof(mem_vfs));
                int target_idx = -1;
                for (int i = 0; i < g_vfs.count; i++) {
                    if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, mem_vfs) == 0) {
                        target_idx = i;
                        break;
                    }
                }
                if (target_idx < 0) {
                    tsfi_xplos_create_file(&g_vfs, mem_vfs, 64 * 1024);
                    target_idx = g_vfs.count - 1;
                }
                XplosFile *vf = &g_vfs.files[target_idx];
                strcpy(vf->data, member_data);
                vf->size_bytes = (uint32_t)strlen(vf->data);
                printf("  - IEBUPDTE: Created/Updated PDS member %s (%d bytes)\n", current_member, vf->size_bytes);
                members_updated++;
            }
            sscanf(line + 12, "%63[^, \r\n]", current_member);
            memset(member_data, 0, sizeof(member_data));
            collecting = true;
        } else if (collecting) {
            if (strncmp(line, "./", 2) == 0) {
                collecting = false;
            } else {
                if (strlen(member_data) + strlen(line) + 2 < sizeof(member_data)) {
                    strcat(member_data, line);
                    strcat(member_data, "\n");
                }
            }
        }
        line = strtok(NULL, "\n");
    }
    if (collecting && strlen(current_member) > 0) {
        char mem_vfs[128];
        resolve_pds_name_extra(current_member, mem_vfs, sizeof(mem_vfs));
        int target_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, mem_vfs) == 0) {
                target_idx = i;
                break;
            }
        }
        if (target_idx < 0) {
            tsfi_xplos_create_file(&g_vfs, mem_vfs, 64 * 1024);
            target_idx = g_vfs.count - 1;
        }
        XplosFile *vf = &g_vfs.files[target_idx];
        strcpy(vf->data, member_data);
        vf->size_bytes = (uint32_t)strlen(vf->data);
        printf("  - IEBUPDTE: Created/Updated PDS member %s (%d bytes)\n", current_member, vf->size_bytes);
        members_updated++;
    }
    printf("[IEBUPDTE] Completed. Updated %d members successfully.\n", members_updated);
    return true;
}

// -----------------------------------------------------------------------------
// 6. TSO SUBMIT Spool Queue Interface
// -----------------------------------------------------------------------------
static bool handle_submit(const char *cmd) {
    char member[64] = "";
    if (sscanf(cmd + 7, "%63s", member) != 1) {
        printf("[SUBMIT ERROR] Member name required.\n");
        return true;
    }
    char vfs_filename[128];
    resolve_pds_name_extra(member, vfs_filename, sizeof(vfs_filename));
    
    int file_idx = -1;
    for (int i = 0; i < g_vfs.count; i++) {
        if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
            file_idx = i;
            break;
        }
    }
    if (file_idx < 0) {
        printf("[SUBMIT WARNING] JCL member %s not found in VFS. Proceeding with virtual mock submission.\n", vfs_filename);
    }
    
    int free_idx = -1;
    for (int i = 0; i < 10; i++) {
        if (!cbt_job_table[i].active) {
            free_idx = i;
            break;
        }
    }
    if (free_idx == -1) {
        printf("[SUBMIT ERROR] JES spool job queue full.\n");
        return true;
    }
    
    sprintf(cbt_job_table[free_idx].job_id, "JOB%04d", 100 + free_idx);
    strncpy(cbt_job_table[free_idx].job_name, member, 15);
    cbt_job_table[free_idx].job_name[15] = '\0';
    
    // Parse priority and held state from file content
    int priority = 1;
    bool held = false;
    if (file_idx >= 0) {
        char *prty_ptr = strstr(g_vfs.files[file_idx].data, "PRTY=");
        if (prty_ptr) {
            sscanf(prty_ptr + 5, "%d", &priority);
        } else {
            prty_ptr = strstr(g_vfs.files[file_idx].data, "PRIORITY=");
            if (prty_ptr) {
                sscanf(prty_ptr + 9, "%d", &priority);
            }
        }
        if (strstr(g_vfs.files[file_idx].data, "TYPRUN=HOLD")) {
            held = true;
        }
    }
    g_hasp_job_priority[free_idx] = priority;

    strcpy(cbt_job_table[free_idx].status, held ? "HELD" : "READY");
    cbt_job_table[free_idx].class_char = 'A';
    cbt_job_table[free_idx].active = true;
    cbt_job_table[free_idx].cics_origin = false;
    memset(g_hasp_spool_logs[free_idx], 0, sizeof(g_hasp_spool_logs[free_idx]));
    
    printf("[SUBMIT] Job %s (%s) submitted to JES Spool (PRTY=%d, STATUS=%s).\n", 
           cbt_job_table[free_idx].job_id, cbt_job_table[free_idx].job_name, priority, cbt_job_table[free_idx].status);
    
    hasp_dispatch_highest_priority();
    return true;
}

// -----------------------------------------------------------------------------
// 7. SMF Record Telemetry Parser
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// 8. VTAM Logon Manager Session Virtualization
// -----------------------------------------------------------------------------
static char g_active_session[32] = "TSO";

static bool handle_vtam_logon(const char *cmd) {
    char applid[32] = "";
    char *app_ptr = strstr(cmd, "APPLID(");
    if (app_ptr) {
        sscanf(app_ptr + 7, "%31[^)]", applid);
    } else {
        sscanf(cmd + 6, "%31s", applid);
    }
    
    if (strlen(applid) == 0) {
        printf("[VTAM ERROR] Subsystem APPLID required.\n");
        return true;
    }
    
    for (int i = 0; applid[i]; i++) {
        applid[i] = toupper((unsigned char)applid[i]);
    }
    
    strncpy(g_active_session, applid, sizeof(g_active_session) - 1);
    g_active_session[sizeof(g_active_session) - 1] = '\0';
    
    printf("[VTAM] Logon request received for subsystem APPLID: %s\n", g_active_session);
    printf("================================================================================\n");
    if (strcmp(g_active_session, "TSO") == 0) {
        printf("                WELCOME TO TSO/E ON **AUNCIENT** SYSTEM XPL1\n");
        printf("           ENTER LOGON PARAMETERS OR PRESS HELP FOR INFORMATION\n");
    } else if (strcmp(g_active_session, "CICS") == 0) {
        printf("                  CICS/ESA VERSION 4.1 SUB-SYSTEM ONLINE\n");
        printf("           ENTER TRANSACTION CODE (E.G. SBMJ) TO BEGIN TRANSACTION\n");
    } else if (strcmp(g_active_session, "USENET") == 0) {
        printf("                USENET-OVER-SNA LOCAL BULLETIN BOARD DISPATCH\n");
        printf("         CDRM PATH ROUTE STABLE. ENTER CBTNET COMMANDS TO POST/READ\n");
    } else {
        printf("            CONNECTED TO APPLICATION SUB-SYSTEM APPLID: %s\n", g_active_session);
    }
    printf("================================================================================\n");
    return true;
}

// -----------------------------------------------------------------------------
// 9. IEBGENER Copy Utility Emulation
// -----------------------------------------------------------------------------
static bool handle_iebgener(const char *cmd) {
    char sysut1[64] = "";
    char sysut2[64] = "";
    char mode[16] = "";
    int scanned = sscanf(cmd + 9, "%63s %63s %15s", sysut1, sysut2, mode);
    if (scanned >= 2) {
        char vfs_ut1[128];
        char vfs_ut2[128];
        resolve_pds_name_extra(sysut1, vfs_ut1, sizeof(vfs_ut1));
        resolve_pds_name_extra(sysut2, vfs_ut2, sizeof(vfs_ut2));

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_ut1) == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx < 0) {
            printf("[IEBGENER ERROR] SYSUT1 source dataset '%s' not found.\n", sysut1);
            return true;
        }

        int target_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_ut2) == 0) {
                target_idx = i;
                break;
            }
        }
        if (target_idx < 0) {
            tsfi_xplos_create_file(&g_vfs, vfs_ut2, 64 * 1024);
            target_idx = g_vfs.count - 1;
        }

        XplosFile *src = &g_vfs.files[file_idx];
        XplosFile *dest = &g_vfs.files[target_idx];

        // Format conversion if requested
        if (strcasecmp(mode, "E2A") == 0) {
            // EBCDIC to ASCII mock copy
            printf("[IEBGENER] Copying %s -> %s with EBCDIC to ASCII translation.\n", sysut1, sysut2);
            strcpy(dest->data, src->data);
        } else if (strcasecmp(mode, "A2E") == 0) {
            // ASCII to EBCDIC mock copy
            printf("[IEBGENER] Copying %s -> %s with ASCII to EBCDIC translation.\n", sysut1, sysut2);
            strcpy(dest->data, src->data);
        } else {
            printf("[IEBGENER] Sequential record copy completed: %s -> %s\n", sysut1, sysut2);
            strcpy(dest->data, src->data);
        }
        dest->size_bytes = (uint32_t)strlen(dest->data);
        printf("  - IEBGENER: Transferred %d bytes successfully. RC=0000\n", dest->size_bytes);
        return true;
    }
    printf("[IEBGENER ERROR] SYSUT1 and SYSUT2 parameters required.\n");
    return true;
}

// -----------------------------------------------------------------------------
// 10. TSO HELP command directory
// -----------------------------------------------------------------------------
static bool handle_cbthelp(const char *cmd) {
    char target[64] = "";
    int scanned = sscanf(cmd + 5, "%63s", target);
    if (scanned <= 0) {
        printf("[CBTHELP] **AUNCIENT** TSO Command Directory Guide:\n");
        printf("  - cbtnet   : Usenet over SNA manager (status, post, read)\n");
        printf("  - cbtrexx  : REXX Script execution & variable pool helper (vput, vget)\n");
        printf("  - jclrun   : Run batch JCL scripts from VFS members\n");
        printf("  - ispfmenu : Launch virtual ISPF panel options\n");
        printf("  - iebupdte : Update PDS members using update decks\n");
        printf("  - submit   : Submit JCL members directly to JES spool\n");
        printf("  - smfdump  : Dump SMF performance metrics\n");
        printf("  - logon    : Request VTAM session logon to subsystem APPLID\n");
        printf("  - iebgener : Copy sequential VFS datasets with translations\n");
        printf("  - cbthasp  : HASP Controller Spool Log reader\n");
        printf("Type 'help <command>' for specific syntax assistance.\n");
        return true;
    }

    printf("[CBTHELP] Command Help for: %s\n", target);
    if (strcasecmp(target, "submit") == 0) {
        printf("  - Syntax: submit <member_name>\n");
        printf("  - Description: Submits the specified JCL member from the PDS VFS to JES spool.\n");
    } else if (strcasecmp(target, "iebupdte") == 0) {
        printf("  - Syntax: iebupdte <sysin_member>\n");
        printf("  - Description: Updates/Creates PDS members from inline data using control cards.\n");
    } else if (strcasecmp(target, "cbtnet") == 0) {
        printf("  - Syntax: cbtnet [status | post group | auth | sub | body | read group]\n");
        printf("  - Description: Connects and replicates Usenet-over-SNA Logical Units.\n");
    } else if (strcasecmp(target, "iebgener") == 0) {
        printf("  - Syntax: iebgener <sysut1> <sysut2> [mode]\n");
        printf("  - Description: Copies dataset contents with optional EBCDIC/ASCII mapping.\n");
    } else {
        printf("  - Command '%s' found. Usage fits standard TSO positional parameters.\n", target);
    }
    return true;
}

// -----------------------------------------------------------------------------
// 11. HASP Spool Log Controller
// -----------------------------------------------------------------------------
static bool handle_cbthasp(const char *cmd) {
    char subcmd[32] = "";
    char arg[64] = "";
    int scanned = sscanf(cmd + 8, "%31s %63s", subcmd, arg);
    if (scanned >= 1) {
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

// -----------------------------------------------------------------------------
// 12. CICS Transient Data Queue (TDQ) Processor
// -----------------------------------------------------------------------------
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
            resolve_pds_name_extra(temp_member, vfs_filename, sizeof(vfs_filename));
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
            tsfi_xplos_shell_cbt_extra(sub_cmd);
            return true;
        }
    }
    printf("[CICS TDQ ERROR] Subcommands: write <queue> <data>, sbmj <queue>\n");
    return true;
}

// -----------------------------------------------------------------------------
// 13. IEBCOMPR Dataset Comparison Utility
// -----------------------------------------------------------------------------
static bool handle_iebcompr(const char *cmd) {
    char sysut1[64] = "";
    char sysut2[64] = "";
    if (sscanf(cmd + 9, "%63s %63s", sysut1, sysut2) < 2) {
        printf("[IEBCOMPR ERROR] SYSUT1 and SYSUT2 parameters required.\n");
        return true;
    }
    char vfs_ut1[128];
    char vfs_ut2[128];
    resolve_pds_name_extra(sysut1, vfs_ut1, sizeof(vfs_ut1));
    resolve_pds_name_extra(sysut2, vfs_ut2, sizeof(vfs_ut2));

    int idx1 = -1, idx2 = -1;
    for (int i = 0; i < g_vfs.count; i++) {
        if (g_vfs.files[i].active) {
            if (strcmp(g_vfs.files[i].name, vfs_ut1) == 0) idx1 = i;
            if (strcmp(g_vfs.files[i].name, vfs_ut2) == 0) idx2 = i;
        }
    }

    if (idx1 < 0 || idx2 < 0) {
        printf("[IEBCOMPR ERROR] One or both datasets not found in VFS.\n");
        return true;
    }

    printf("[IEBCOMPR] Comparing dataset %s vs %s...\n", sysut1, sysut2);
    if (strcmp(g_vfs.files[idx1].data, g_vfs.files[idx2].data) == 0) {
        printf("[IEBCOMPR] Success: Datasets are identical. RC=0000\n");
    } else {
        printf("[IEBCOMPR] Mismatch: Datasets differ in content. RC=0008\n");
    }
    return true;
}

// -----------------------------------------------------------------------------
// 14. VTAM 3270 Terminal Screen Buffer Virtualizer
// -----------------------------------------------------------------------------
static bool handle_vtam3270(const char *cmd) {
    char appl[32] = "";
    int scanned = sscanf(cmd + 9, "%31s", appl);
    printf("\n");
    printf("================================================================================\n");
    printf(" VTAM 3270 DATA STREAM FORMATTER (ACTIVE APPLID: %s)\n", (scanned > 0) ? appl : "TSO");
    printf("================================================================================\n");
    if (scanned > 0 && strcasecmp(appl, "CICS") == 0) {
        printf(" [SF, ATTR(PROT, NUM)]   'CICS/ESA TRANSACTION ENTRY'\n");
        printf(" [SF, ATTR(PROT)]       'TRANID: '\n");
        printf(" [SF, ATTR(UNPROT, INTENS)] '[____]'\n");
    } else {
        printf(" [SF, ATTR(PROT)]       'WELCOME TO TSO/E SIGNON'\n");
        printf(" [SF, ATTR(PROT)]       'USERID: '\n");
        printf(" [SF, ATTR(UNPROT, INTENS)] '[________]'\n");
        printf(" [SF, ATTR(PROT)]       'PASSWORD: '\n");
        printf(" [SF, ATTR(UNPROT, NON_DISPLAY)] '[________]'\n");
    }
    printf("================================================================================\n");
    printf(" [WCC: START_PRINTER, RESTORE_KEYBOARD]\n");
    printf("================================================================================\n");
    return true;
}

// -----------------------------------------------------------------------------
// 15. TSO ALLOCATE & FREE Dataset Processor
// -----------------------------------------------------------------------------
static char g_tso_allocs_dd[5][16];
static char g_tso_allocs_ds[5][64];
static int g_tso_alloc_count = 0;

static bool handle_cbtalloc(const char *cmd) {
    char action[16] = "";
    char ddname[16] = "";
    char dsname[64] = "";
    int scanned = sscanf(cmd + 9, "%15s %15s %63s", action, ddname, dsname);
    if (scanned >= 2) {
        if (strcasecmp(action, "alloc") == 0) {
            if (g_tso_alloc_count >= 5) {
                printf("[ALLOC ERROR] Maximum dynamic allocations reached.\n");
                return true;
            }
            strncpy(g_tso_allocs_dd[g_tso_alloc_count], ddname, 15);
            g_tso_allocs_dd[g_tso_alloc_count][15] = '\0';
            strncpy(g_tso_allocs_ds[g_tso_alloc_count], dsname, 63);
            g_tso_allocs_ds[g_tso_alloc_count][63] = '\0';
            g_tso_alloc_count++;
            printf("[ALLOC] Dataset %s allocated dynamically to DD %s. RC=0000\n", dsname, ddname);
            return true;
        }
        if (strcasecmp(action, "free") == 0) {
            int found_idx = -1;
            for (int i = 0; i < g_tso_alloc_count; i++) {
                if (strcasecmp(g_tso_allocs_dd[i], ddname) == 0) {
                    found_idx = i;
                    break;
                }
            }
            if (found_idx != -1) {
                printf("[FREE] DD %s freed dynamically from dataset %s. RC=0000\n",
                       g_tso_allocs_dd[found_idx], g_tso_allocs_ds[found_idx]);
                // Shift array
                for (int i = found_idx; i < g_tso_alloc_count - 1; i++) {
                    strcpy(g_tso_allocs_dd[i], g_tso_allocs_dd[i + 1]);
                    strcpy(g_tso_allocs_ds[i], g_tso_allocs_ds[i + 1]);
                }
                g_tso_alloc_count--;
            } else {
                printf("[FREE ERROR] DD %s not allocated.\n", ddname);
            }
            return true;
        }
    }
    printf("[ALLOC ERROR] Syntax: cbtalloc [alloc <dd> <ds> | free <dd>]\n");
    return true;
}

// -----------------------------------------------------------------------------
// 16. CICS Transient Storage Queue (TSQ) Processor
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// 17. VTAM Net/USS Logon Solicit Screen Virtualizer
// -----------------------------------------------------------------------------
static bool handle_vtamuss(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  **AUNCIENT** VTAM MULTI-SYSTEM SOLICIT SCREEN                 \n");
    printf("================================================================================\n");
    printf("         XPL1 SYSTEM ONLINE (MVS/XA SP2.1) - DATE: 26/07/29 TIME: 12:00:00\n\n");
    printf(" ENTER APPLID SUB-SYSTEM TO CONNECT LOGICAL TERMINAL PATHWAY:\n");
    printf("   - TSO     (TSO/E Terminal Session Manager)\n");
    printf("   - CICS    (Customer Information Control System)\n");
    printf("   - USENET  (Usenet-Over-SNA Bulletin Board Dispatch)\n\n");
    printf(" APPLID ===> \n");
    printf("================================================================================\n");
    return true;
}

// -----------------------------------------------------------------------------
// Entry Point / Command Router
// -----------------------------------------------------------------------------
bool tsfi_xplos_shell_cbt_extra(const char *cmd) {
    if (strncmp(cmd, "cbtnet ", 7) == 0) {
        return handle_cbtnet(cmd + 7);
    }
    if (strncmp(cmd, "cbtrexx ", 8) == 0) {
        return handle_cbtrexx(cmd);
    }
    if (strncmp(cmd, "jclrun ", 7) == 0) {
        return handle_jclrun(cmd);
    }
    if (strncmp(cmd, "ispfmenu", 8) == 0) {
        return handle_ispfmenu(cmd);
    }
    if (strncmp(cmd, "iebupdte ", 9) == 0) {
        return handle_iebupdte(cmd);
    }
    if (strncmp(cmd, "submit ", 7) == 0) {
        return handle_submit(cmd);
    }
    if (strcmp(cmd, "smfdump") == 0) {
        return handle_smfdump();
    }
    if (strncmp(cmd, "logon ", 6) == 0) {
        return handle_vtam_logon(cmd);
    }
    if (strncmp(cmd, "iebgener ", 9) == 0) {
        return handle_iebgener(cmd);
    }
    if (strncmp(cmd, "help", 4) == 0) {
        return handle_cbthelp(cmd);
    }
    if (strncmp(cmd, "cbthasp ", 8) == 0) {
        return handle_cbthasp(cmd);
    }
    if (strncmp(cmd, "cbtcicstd ", 10) == 0) {
        return handle_cbtcicstd(cmd);
    }
    if (strncmp(cmd, "iebcompr ", 9) == 0) {
        return handle_iebcompr(cmd);
    }
    if (strncmp(cmd, "vtam3270 ", 9) == 0) {
        return handle_vtam3270(cmd);
    }
    if (strncmp(cmd, "cbtalloc ", 9) == 0) {
        return handle_cbtalloc(cmd);
    }
    if (strncmp(cmd, "cbtcicsts ", 10) == 0) {
        return handle_cbtcicsts(cmd);
    }
    if (strcmp(cmd, "vtamuss") == 0) {
        return handle_vtamuss();
    }
    return false;
}
