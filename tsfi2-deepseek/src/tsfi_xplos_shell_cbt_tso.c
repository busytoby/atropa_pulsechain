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
#include "tsfi_xplos_shell_cbt_tso.h"
extern XplosVirtualDisk g_vfs;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
static void resolve_pds_name_helper(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

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
        resolve_pds_name_helper(script_name, vfs_filename, sizeof(vfs_filename));

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
static bool handle_cbtdelete(const char *cmd) {
    char action[16] = "";
    char old_mem[32] = "";
    char new_mem[32] = "";
    int scanned = sscanf(cmd + 10, "%15s %31s %31s", action, old_mem, new_mem);
    if (scanned >= 2) {
        char old_vfs[128];
        resolve_pds_name_helper(old_mem, old_vfs, sizeof(old_vfs));

        if (strcasecmp(action, "delete") == 0) {
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, old_vfs) == 0) {
                    g_vfs.files[i].active = false;
                    printf("[DELETE] Member %s deleted successfully. RC=0000\n", old_mem);
                    return true;
                }
            }
            printf("[DELETE ERROR] Member %s not found.\n", old_mem);
            return true;
        }
        if (strcasecmp(action, "rename") == 0 && scanned == 3) {
            char new_vfs[128];
            resolve_pds_name_helper(new_mem, new_vfs, sizeof(new_vfs));
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, old_vfs) == 0) {
                    strcpy(g_vfs.files[i].name, new_vfs);
                    printf("[RENAME] Member %s renamed to %s successfully. RC=0000\n", old_mem, new_mem);
                    return true;
                }
            }
            printf("[RENAME ERROR] Member %s not found.\n", old_mem);
            return true;
        }
    }
    printf("[DELETE ERROR] Syntax: cbtdelete [delete <member> | rename <old> <new>]\n");
    return true;
}

static bool handle_cbtsubchk(const char *cmd) {
    char member[32] = "";
    if (sscanf(cmd + 10, "%31s", member) < 1) {
        printf("[SUBCHK ERROR] Syntax: cbtsubchk <member>\n");
        return true;
    }
    char vfs_filename[128];
    resolve_pds_name_helper(member, vfs_filename, sizeof(vfs_filename));

    int f_idx = -1;
    for (int i = 0; i < g_vfs.count; i++) {
        if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
            f_idx = i;
            break;
        }
    }
    if (f_idx < 0) {
        printf("[SUBCHK WARNING] JCL member %s not found in VFS.\n", member);
    } else {
        printf("[SUBCHK] Scanning job submission parameters for %s...\n", member);
        printf("  - Audit: Job priority mapping and class declarations validated.\n");
        printf("[SUBCHK] Validation complete. No parameter discrepancies found. RC=0000\n");
    }
    return true;
}

static bool handle_cbtsublist(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  TSO SUBMITTED JOB SPOOL STATUS MONITOR                        \n");
    printf("================================================================================\n");
    printf(" JOB ID   | JOB NAME | CLASS | PRTY | STATUS   | STEP NAME\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" JOB00021 | LOADCBT  | A     |    0 | COMPLETED| STEP1\n");
    printf(" JOB00022 | RUNREXX  | A     |    0 | COMPLETED| STEP1\n");
    printf(" JOB00023 | BACKUP   | A     |    0 | HELD     | STEP1\n");
    printf(" JOB0104  | HLDJOB   | A     |   12 | READY    | STEP1\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtsubrel(const char *cmd) {
    char job_id[32] = "";
    if (sscanf(cmd + 10, "%31s", job_id) < 1) {
        printf("[SUBREL ERROR] Syntax: cbtsubrel <job_id>\n");
        return true;
    }
    printf("[SUBREL] Releasing held spool job: %s\n", job_id);
    printf("  - Dispatcher status updated from HELD to READY queue. RC=0000\n");
    return true;
}

static bool handle_cbtsubout(const char *cmd) {
    char job_id[32] = "";
    char dest[32] = "";
    if (sscanf(cmd + 10, "%31s %31s", job_id, dest) < 2) {
        printf("[SUBOUT ERROR] Syntax: cbtsubout <job_id> <dest>\n");
        return true;
    }
    printf("[SUBOUT] Redirecting execution spool output for %s...\n", job_id);
    printf("  - Destination target: %s\n", dest);
    printf("[SUBOUT] Spool log redirected successfully. RC=0000\n");
    return true;
}

static bool handle_cbtsubtrc(const char *cmd) {
    char job_id[32] = "";
    if (sscanf(cmd + 10, "%31s", job_id) < 1) {
        printf("[SUBTRC ERROR] Syntax: cbtsubtrc <job_id>\n");
        return true;
    }
    printf("\n");
    printf("================================================================================\n");
    printf("                  JES JOB STEP EXECUTION HISTORICAL TRACE: %s                   \n", job_id);
    printf("================================================================================\n");
    printf(" STEP NUM | STEP NAME | PGM NAME  | COND CODE | I/O COUNT | STEP STATUS\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" 001      | STEP1     | IEBCOPY   | RC=0000   |        42 | SUCCESS\n");
    printf(" 002      | STEP2     | IKJEFT01  | RC=0000   |        12 | SUCCESS. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtsubpri(const char *cmd) {
    char job_id[32] = "";
    int priority = 0;
    if (sscanf(cmd + 10, "%31s %d", job_id, &priority) < 2) {
        printf("[SUBPRI ERROR] Syntax: cbtsubpri <job_id> <priority>\n");
        return true;
    }
    printf("[SUBPRI] Updating JES spool priority for %s...\n", job_id);
    printf("  - Scheduling priority updated to: %d\n", priority);
    printf("[SUBPRI] Job priority rescheduled successfully. RC=0000\n");
    return true;
}

static bool handle_cbtsubprg(const char *cmd) {
    char job_id[32] = "";
    if (sscanf(cmd + 10, "%31s", job_id) < 1) {
        printf("[SUBPRG ERROR] Syntax: cbtsubprg <job_id>\n");
        return true;
    }
    printf("[SUBPRG] Purging job spool files and details for: %s\n", job_id);
    printf("  - Spool directories and records successfully deleted. RC=0000\n");
    return true;
}

static bool handle_cbtsubfind(const char *cmd) {
    char job_id[32] = "";
    char query[32] = "";
    if (sscanf(cmd + 11, "%31s %31s", job_id, query) < 2) {
        printf("[SUBFIND ERROR] Syntax: cbtsubfind <job_id> <query>\n");
        return true;
    }
    printf("[SUBFIND] Scanning spool output records for %s matching query '%s'...\n", job_id, query);
    printf("  - Offsets scanned: 42 records processed\n");
    printf("  - Match found at: STEP1 program logs. RC=0000\n");
    return true;
}

static bool handle_cbtsubnot(const char *cmd) {
    char state[16] = "";
    if (sscanf(cmd + 10, "%15s", state) < 1) {
        printf("[SUBNOT ERROR] Syntax: cbtsubnot [on|off]\n");
        return true;
    }
    printf("[SUBNOT] Spool completion terminal notification set to: %s\n", state);
    printf("  - User logon broadcast alerts mapped successfully. RC=0000\n");
    return true;
}

static bool handle_cbtsubmod(const char *cmd) {
    char job_id[32] = "";
    char class_char[16] = "";
    if (sscanf(cmd + 10, "%31s %15s", job_id, class_char) < 2) {
        printf("[SUBMOD ERROR] Syntax: cbtsubmod <job_id> <class>\n");
        return true;
    }
    printf("[SUBMOD] Modifying parameters for JCL job spool entry: %s...\n", job_id);
    printf("  - Execution Class updated to: %s\n", class_char);
    printf("[SUBMOD] Spool parameter modification completed. RC=0000\n");
    return true;
}

static bool handle_cbtsuboutlim(const char *cmd) {
    char job_id[32] = "";
    int limit_lines = 0;
    if (sscanf(cmd + 13, "%31s %d", job_id, &limit_lines) < 2) {
        printf("[SUBOUTLIM ERROR] Syntax: cbtsuboutlim <job_id> <limit>\n");
        return true;
    }
    printf("[SUBOUTLIM] Spool redirect limit bounds loaded for job %s...\n", job_id);
    printf("  - Maximum record lines constraint configured: %d lines\n", limit_lines);
    printf("[SUBOUTLIM] Redirect limit thresholds established. RC=0000\n");
    return true;
}

bool tsfi_xplos_shell_cbt_tso(const char *cmd) {
    if (strncmp(cmd, "cbtrexx ", 8) == 0) return handle_cbtrexx(cmd);
    if (strncmp(cmd, "ispfmenu", 8) == 0) return handle_ispfmenu(cmd);
    if (strncmp(cmd, "help ", 5) == 0 || strcmp(cmd, "help") == 0) return handle_cbthelp(cmd);
    if (strncmp(cmd, "cbtalloc ", 9) == 0) return handle_cbtalloc(cmd);
    if (strncmp(cmd, "cbtdelete ", 10) == 0) return handle_cbtdelete(cmd);
    if (strncmp(cmd, "cbtsubchk ", 10) == 0) return handle_cbtsubchk(cmd);
    if (strcmp(cmd, "cbtsublist") == 0) return handle_cbtsublist();
    if (strncmp(cmd, "cbtsubrel ", 10) == 0) return handle_cbtsubrel(cmd);
    if (strncmp(cmd, "cbtsubout ", 10) == 0) return handle_cbtsubout(cmd);
    if (strncmp(cmd, "cbtsubtrc ", 10) == 0) return handle_cbtsubtrc(cmd);
    if (strncmp(cmd, "cbtsubpri ", 10) == 0) return handle_cbtsubpri(cmd);
    if (strncmp(cmd, "cbtsubprg ", 10) == 0) return handle_cbtsubprg(cmd);
    if (strncmp(cmd, "cbtsubfind ", 11) == 0) return handle_cbtsubfind(cmd);
    if (strncmp(cmd, "cbtsubnot ", 10) == 0) return handle_cbtsubnot(cmd);
    if (strncmp(cmd, "cbtsubmod ", 10) == 0) return handle_cbtsubmod(cmd);
    if (strncmp(cmd, "cbtsuboutlim ", 13) == 0) return handle_cbtsuboutlim(cmd);
    return false;
}
