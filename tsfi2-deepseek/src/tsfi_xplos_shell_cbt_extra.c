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

static bool handle_cbtrexx(const char *cmd) {
    char script_name[64] = "";
    char script_args[128] = "";
    int scanned = sscanf(cmd + 8, "%63s %[^\n]", script_name, script_args);
    if (scanned >= 1) {
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

        printf("[JCLRUN EXECUTION START: %s]\n", jcl_name);

        char *line = strtok(jcl_data, "\n");
        int rc = 0;
        bool skip_block = false;

        while (line) {
            // Trim leading spaces
            while (isspace((unsigned char)*line)) line++;

            if (strncmp(line, "//", 2) == 0) {
                // Parse JCL control card
                const char *card = line + 2;
                while (isspace((unsigned char)*card)) card++;

                if (strncmp(card, "IF ", 3) == 0 || strncmp(card, "if ", 3) == 0) {
                    printf("  JCL_COND> Checking condition: %s\n", card);
                    // Mock conditional evaluation
                    if (strstr(card, "RC = 0") && rc != 0) {
                        skip_block = true;
                        printf("  JCL_COND> Condition Failed. Bypassing subsequent steps.\n");
                    }
                }
                else if (strncmp(card, "ENDIF", 5) == 0 || strncmp(card, "endif", 5) == 0) {
                    skip_block = false;
                    printf("  JCL_COND> ENDIF reached.\n");
                }
                else if (strstr(card, " EXEC ") || strstr(card, " exec ")) {
                    if (skip_block) {
                        printf("  JCL_STEP> Bypassed step: %s\n", card);
                    } else {
                        char pgm_name[64] = "";
                        char *pgm_ptr = strstr(card, "PGM=");
                        if (pgm_ptr) {
                            sscanf(pgm_ptr + 4, "%63[^, \r\n]", pgm_name);
                        }
                        printf("  JCL_STEP> Executing step Program: %s\n", pgm_name);
                        // Simulate program execution exit codes
                        if (strcmp(pgm_name, "IEBCOPY") == 0) {
                            rc = 0;
                            printf("    * IEBCOPY completed successfully. RC=0000\n");
                        } else if (strcmp(pgm_name, "IBHDRPLY") == 0) {
                            rc = 0;
                            printf("    * IBHDRPLY Automatic Reply executed. RC=0000\n");
                        } else {
                            rc = 4;
                            printf("    * Program %s executed. RC=0004 (Warning)\n", pgm_name);
                        }
                    }
                }
                else if (strstr(card, " DD ") || strstr(card, " dd ")) {
                    if (!skip_block) {
                        printf("  JCL_ALLOC> Allocation: %s\n", card);
                    }
                }
            }

            line = strtok(NULL, "\n");
        }

        printf("[JCLRUN EXECUTION COMPLETED: Max RC=%04d]\n", rc);
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
        // Trim trailing whitespace
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
    strcpy(cbt_job_table[free_idx].status, "READY");
    cbt_job_table[free_idx].class_char = 'A';
    cbt_job_table[free_idx].active = true;
    cbt_job_table[free_idx].cics_origin = false;
    
    printf("[SUBMIT] Job %s (%s) submitted to JES Spool dynamically.\n", cbt_job_table[free_idx].job_id, cbt_job_table[free_idx].job_name);
    
    // Trigger the JCL parser
    char run_cmd[128];
    snprintf(run_cmd, sizeof(run_cmd), "jclrun %s", member);
    tsfi_xplos_shell_cbt_extra(run_cmd);
    
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
    return false;
}
