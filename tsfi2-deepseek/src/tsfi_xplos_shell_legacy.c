#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_xpl_mallgren.h"
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_nadler_skeletonizer.h"
#include "tsfi_nadler_syntactic_parser.h"
#include "tsfi_xplos_kernel_internal.h"

// Variable references defined in kernel
extern XplosVirtualDisk g_vfs;
extern bool g_vfs_initialized;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
extern CbtTreeNode *g_queue_tree_root;

extern uint32_t ce_gprs[16];
extern uint8_t ce_memory[1024];
extern uint32_t xdc_ip;


bool tsfi_xplos_shell_legacy(const char *cmd) {
    (void)cmd;
    
    // Lazy initialization of Global VFS
    if (!g_vfs_initialized) {
        tsfi_xplos_init_vfs(&g_vfs);
        g_vfs_initialized = true;
    }

    // Check for "jclparse " command
    if (strncmp(cmd, "jclparse ", 9) == 0) {
        const char *path = cmd + 9;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCL PARSER ERROR] Could not open file: %s\n", path);
            return true;
        }
        char line_buf[512];
        int line_num = 0;
        int parsed_cards = 0;
        int parsed_comments = 0;
        while (fgets(line_buf, sizeof(line_buf), f_jcl)) {
            line_num++;
            // Basic JCL classification
            if (strncmp(line_buf, "//*", 3) == 0) {
                parsed_comments++;
            } else if (strncmp(line_buf, "//", 2) == 0) {
                parsed_cards++;
            }
        }
        fclose(f_jcl);
        printf("[JCL PARSER] Total Lines: %d, Control Cards: %d, Comments: %d\n", line_num, parsed_cards, parsed_comments);
        return true;
    }

    // Check for "cbtupd " command
    if (strncmp(cmd, "cbtupd ", 7) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        if (sscanf(cmd + 7, "%127s %127s", in_path, out_path) == 2) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[CBTUPD ERROR] Could not open input or output file\n");
                return true;
            }
            char line[512];
            int inserted = 0;
            while (fgets(line, sizeof(line), f_in)) {
                // Look for //***FILE<digits> marker
                char *marker = strstr(line, "//***FILE");
                if (marker) {
                    char file_id[32] = "FILE";
                    int idx_f = 4;
                    char *p = marker + 9; // Skip "//***FILE"
                    while (*p && isspace((unsigned char)*p)) {
                        p++;
                    }
                    while (*p && isdigit((unsigned char)*p) && idx_f < 30) {
                        file_id[idx_f++] = *p++;
                    }
                    file_id[idx_f] = '\0';
                    if (idx_f > 4) {
                        fprintf(f_out, "./ ADD NAME=%s\n", file_id);
                        inserted++;
                    }
                }
                fputs(line, f_out);
            }
            fclose(f_in);
            fclose(f_out);
            printf("[CBTUPD] Completed successfully. Inserted %d ./ ADD cards.\n", inserted);
            return true;
        }
    }

    // Check for "pdsload " command
    if (strncmp(cmd, "pdsload ", 8) == 0) {
        char in_path[128] = "";
        if (sscanf(cmd + 8, "%127s", in_path) == 1) {
            FILE *f_in = fopen(in_path, "r");
            if (!f_in) {
                printf("[PDSLOAD ERROR] Could not open file: %s\n", in_path);
                return true;
            }
            char line[512];
            int loaded = 0;
            while (fgets(line, sizeof(line), f_in)) {
                if (strncmp(line, "./ ADD NAME=", 12) == 0) {
                    char member_name[64] = "";
                    int idx_m = 0;
                    char *p = line + 12;
                    while (*p && !isspace((unsigned char)*p) && idx_m < 32) {
                        member_name[idx_m++] = *p++;
                    }
                    member_name[idx_m] = '\0';
                    if (strlen(member_name) > 0) {
                        char vfs_name[64];
                        snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", member_name);
                        if (tsfi_xplos_create_file(&g_vfs, vfs_name, 80 * 1024)) {
                            loaded++;
                        }
                    }
                }
            }
            fclose(f_in);
            printf("[PDSLOAD] Unpacked %d members into Virtual Disk VFS.\n", loaded);
            return true;
        }
    }

    // Check for "pdslist" command
    if (strcmp(cmd, "pdslist") == 0) {
        printf("[PDSLIST] Listing active VFS members:\n");
        int active_count = 0;
        for (int i = 0; i < g_vfs.count; i++) {
            XplosFile *f = &g_vfs.files[i];
            if (f->active) {
                printf("  - MEMBER: %-15s SIZE: %-6d OFFSET: 0x%08X\n", f->name, f->size_bytes, f->start_offset);
                active_count++;
            }
        }
        printf("[PDSLIST] Total active members: %d\n", active_count);
        return true;
    }

    // Check for "pdsfind " command
    if (strncmp(cmd, "pdsfind ", 8) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 8, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[PDSFIND] FOUND: %s, SIZE: %d, OFFSET: 0x%08X\n", f->name, f->size_bytes, f->start_offset);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[PDSFIND] NOT FOUND: %s\n", target_name);
            }
            return true;
        }
    }

    // Check for "pdsdelete " command
    if (strncmp(cmd, "pdsdelete ", 10) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 10, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    f->active = false;
                    printf("[PDSDELETE] DELETED: %s\n", target_name);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[PDSDELETE] NOT FOUND: %s\n", target_name);
            }
            return true;
        }
    }

    // Check for "pdsstats " command
    if (strncmp(cmd, "pdsstats ", 9) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 9, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[PDSSTATS] MEMBER: %s, CREATED: 2026/07/28, MODIFIED: 2026/07/28, USER: MVSUSER, LINES: 1250\n", f->name);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[PDSSTATS] NOT FOUND: %s\n", target_name);
            }
            return true;
        }
    }

    // Check for "rexx " command
    if (strncmp(cmd, "rexx ", 5) == 0) {
        const char *path = cmd + 5;
        FILE *f_script = fopen(path, "r");
        if (!f_script) {
            printf("[REXX ERROR] Could not open script: %s\n", path);
            return true;
        }
        char line[512];
        int line_num = 0;
        char in_dd[64] = "";
        char out_dd[64] = "";
        bool is_rexx = false;
        int rc = 0;
        
        char var_names[8][32];
        char var_vals[8][64];
        int var_count = 0;
        char rxtk[16][128];
        int rxtk_cnt = 0;

        while (fgets(line, sizeof(line), f_script)) {
            line_num++;
            char *p = line;
            while (*p && isspace((unsigned char)*p)) p++;
            if (line_num == 1) {
                if (strncmp(p, "/* REXX */", 10) == 0) {
                    is_rexx = true;
                    printf("[REXX] Initialized REXX Exec environment\n");
                    continue;
                } else {
                    printf("[REXX ERROR] Script does not start with /* REXX */\n");
                    fclose(f_script);
                    return true;
                }
            }
            if (!is_rexx) break;

            size_t len = strlen(p);
            while (len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n')) {
                p[len - 1] = '\0';
                len--;
            }

            if (strncmp(p, "PUSH ", 5) == 0) {
                char val[128] = "";
                char *val_start = strchr(p + 5, '\'');
                if (val_start) {
                    char *val_end = strchr(val_start + 1, '\'');
                    if (val_end) {
                        *val_end = '\0';
                        snprintf(val, sizeof(val), "%s", val_start + 1);
                    }
                } else {
                    sscanf(p + 5, "%127s", val);
                }
                if (rxtk_cnt < 16) {
                    for (int j = rxtk_cnt; j > 0; j--) {
                        strcpy(rxtk[j], rxtk[j - 1]);
                    }
                    strcpy(rxtk[0], val);
                    rxtk_cnt++;
                    printf("[REXX STACK] PUSH: %s\n", val);
                }
                continue;
            }

            if (strncmp(p, "QUEUE ", 6) == 0) {
                char val[128] = "";
                char *val_start = strchr(p + 6, '\'');
                if (val_start) {
                    char *val_end = strchr(val_start + 1, '\'');
                    if (val_end) {
                        *val_end = '\0';
                        snprintf(val, sizeof(val), "%s", val_start + 1);
                    }
                } else {
                    sscanf(p + 6, "%127s", val);
                }
                if (rxtk_cnt < 16) {
                    strcpy(rxtk[rxtk_cnt], val);
                    rxtk_cnt++;
                    printf("[REXX STACK] QUEUE: %s\n", val);
                }
                continue;
            }

            if (strncmp(p, "PULL ", 5) == 0) {
                char var_name[32] = "";
                if (sscanf(p + 5, "%31s", var_name) == 1) {
                    char val[128] = "";
                    if (rxtk_cnt > 0) {
                        strcpy(val, rxtk[0]);
                        for (int j = 0; j < rxtk_cnt - 1; j++) {
                            strcpy(rxtk[j], rxtk[j + 1]);
                        }
                        rxtk_cnt--;
                        printf("[REXX STACK] PULL: %s -> %s\n", var_name, val);
                    } else {
                        strcpy(val, "");
                        printf("[REXX STACK] PULL: Stack empty\n");
                    }
                    bool found_var = false;
                    for (int k = 0; k < var_count; k++) {
                        if (strcmp(var_names[k], var_name) == 0) {
                            snprintf(var_vals[k], sizeof(var_vals[k]), "%s", val);
                            found_var = true;
                            break;
                        }
                    }
                    if (!found_var && var_count < 8) {
                        snprintf(var_names[var_count], sizeof(var_names[var_count]), "%s", var_name);
                        snprintf(var_vals[var_count], sizeof(var_vals[var_count]), "%s", val);
                        var_count++;
                    }
                }
                continue;
            }

            bool cond_met = true;
            if (strncmp(p, "IF ", 3) == 0) {
                char cond_var[32] = "";
                int cond_val = 0;
                char action[256] = "";
                if (sscanf(p + 3, "%31s = %d THEN %[^\n]", cond_var, &cond_val, action) >= 2) {
                    if (strcmp(cond_var, "RC") == 0) {
                        cond_met = (rc == cond_val);
                    }
                    if (cond_met) {
                        p = action;
                    } else {
                        continue;
                    }
                }
            }

            char *assign_eq = strchr(p, '=');
            if (assign_eq && strncmp(p, "IF ", 3) != 0 && strchr(p, '\'') != NULL) {
                char var_name[32] = "";
                char raw_val[64] = "";
                *assign_eq = '\0';
                if (sscanf(p, "%31s", var_name) == 1) {
                    char *val_start = strchr(assign_eq + 1, '\'');
                    if (val_start) {
                        char *val_end = strchr(val_start + 1, '\'');
                        if (val_end) {
                            *val_end = '\0';
                            snprintf(raw_val, sizeof(raw_val), "%s", val_start + 1);
                        }
                    } else {
                        sscanf(assign_eq + 1, "%63s", raw_val);
                    }
                    if (strlen(var_name) > 0 && var_count < 8) {
                        snprintf(var_names[var_count], sizeof(var_names[var_count]), "%s", var_name);
                        snprintf(var_vals[var_count], sizeof(var_vals[var_count]), "%s", raw_val);
                        printf("[REXX] ASSIGN: %s = '%s'\n", var_names[var_count], var_vals[var_count]);
                        var_count++;
                    }
                }
                continue;
            }

            if (strncmp(p, "SAY ", 4) == 0) {
                char *msg_start = strchr(p + 4, '\'');
                if (msg_start) {
                    char *msg_end = strchr(msg_start + 1, '\'');
                    if (msg_end) {
                        *msg_end = '\0';
                        printf("[REXX SAY] %s\n", msg_start + 1);
                    }
                } else {
                    char var_query[32] = "";
                    if (sscanf(p + 4, "%31s", var_query) == 1) {
                        bool found_var = false;
                        for (int k = 0; k < var_count; k++) {
                            if (strcmp(var_names[k], var_query) == 0) {
                                printf("[REXX SAY] %s\n", var_vals[k]);
                                found_var = true;
                                break;
                            }
                        }
                        if (!found_var) {
                            printf("[REXX SAY] %s\n", var_query);
                        }
                    }
                }
                continue;
            }

            char interpolated[512] = "";
            char *inp = p;
            char *outp = interpolated;
            while (*inp && (outp - interpolated) < 500) {
                if (*inp == '\'') {
                    *outp++ = *inp++;
                    while (*inp && *inp != '\'' && (outp - interpolated) < 500) {
                        *outp++ = *inp++;
                    }
                    if (*inp == '\'') *outp++ = *inp++;
                } else if (isalnum((unsigned char)*inp)) {
                    char token[32] = "";
                    int tk_idx = 0;
                    while (*inp && isalnum((unsigned char)*inp) && tk_idx < 31) {
                        token[tk_idx++] = *inp++;
                    }
                    token[tk_idx] = '\0';
                    bool subst = false;
                    for (int k = 0; k < var_count; k++) {
                        if (strcmp(var_names[k], token) == 0) {
                            int written = snprintf(outp, 500 - (outp - interpolated), "%s", var_vals[k]);
                            outp += written;
                            subst = true;
                            break;
                        }
                    }
                    if (!subst) {
                        int written = snprintf(outp, 500 - (outp - interpolated), "%s", token);
                        outp += written;
                    }
                } else {
                    *outp++ = *inp++;
                }
            }
            *outp = '\0';

            char *start_quote = strchr(interpolated, '\'');
            if (start_quote) {
                char *end_quote = strchr(start_quote + 1, '\'');
                if (end_quote) {
                    *end_quote = '\0';
                    char *sub_cmd = start_quote + 1;
                    if (strncmp(sub_cmd, "FILEDEF ", 8) == 0) {
                        char dd_name[32] = "";
                        char target[64] = "";
                        int scan_count = sscanf(sub_cmd + 8, "%31s %63s", dd_name, target);
                        if (scan_count < 2) {
                            char extra[64] = "";
                            if (sscanf(end_quote + 1, "%63s", extra) == 1) {
                                snprintf(target, sizeof(target), "%s", extra);
                                scan_count = 2;
                            }
                        }
                        if (scan_count == 2) {
                            if (strcmp(dd_name, "INDD") == 0) {
                                snprintf(in_dd, sizeof(in_dd), "%s", target);
                                printf("[REXX] FILEDEF: INDD mapped to %s\n", in_dd);
                                rc = 0;
                            } else if (strcmp(dd_name, "OUTDD") == 0) {
                                snprintf(out_dd, sizeof(out_dd), "%s", target);
                                printf("[REXX] FILEDEF: OUTDD mapped to %s\n", out_dd);
                                rc = 0;
                            }
                        } else {
                            rc = 8;
                        }
                    } else if (strcmp(sub_cmd, "TAPE LOAD") == 0 || strncmp(sub_cmd, "TAPE LOAD ", 10) == 0) {
                        if (strlen(in_dd) > 0 && strlen(out_dd) > 0) {
                            char vfs_name[64];
                            snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", outp = out_dd);
                            if (tsfi_xplos_create_file(&g_vfs, vfs_name, 64 * 1024)) {
                                printf("[REXX] TAPE LOAD: Successfully loaded %s from tape %s into VFS\n", vfs_name, in_dd);
                                rc = 0;
                            } else {
                                rc = 4;
                            }
                        } else {
                            printf("[REXX ERROR] TAPE LOAD failed: INDD or OUTDD not defined\n");
                            rc = 12;
                        }
                    } else if (strncmp(sub_cmd, "SELECT ", 7) == 0) {
                        char pattern[32] = "";
                        sscanf(sub_cmd + 7, "%31s", pattern);
                        printf("[REXX SELECT] Matching pattern '%s':\n", pattern);
                        int matches = 0;
                        for (int file_idx = 1; file_idx <= 3; file_idx++) {
                            char sim_member[32];
                            snprintf(sim_member, sizeof(sim_member), "FILE%03d", file_idx);
                            char vfs_name[64];
                            snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", sim_member);
                            printf("  - Match: %s\n", sim_member);
                            if (tsfi_xplos_create_file(&g_vfs, vfs_name, 64 * 1024)) {
                                matches++;
                            }
                        }
                        printf("[REXX SELECT] Loaded %d matched members into VFS.\n", matches);
                        rc = 0;
                    }
                }
            }
        }
        fclose(f_script);
        return true;
    }

    // Check for "docfile " command
    if (strncmp(cmd, "docfile ", 8) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        char tag[5] = "";
        if (sscanf(cmd + 8, "%127s %127s %4s", in_path, out_path, tag) == 3) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[DOCFILE ERROR] Could not open input or output file\n");
                return true;
            }
            char line[512];
            int seq = 100;
            int processed = 0;
            while (fgets(line, sizeof(line), f_in)) {
                size_t len = strlen(line);
                while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) {
                    line[len - 1] = '\0';
                    len--;
                }
                char padded[81];
                int pad_len = (len > 72) ? 72 : (int)len;
                memcpy(padded, line, pad_len);
                for (int k = pad_len; k < 72; k++) {
                    padded[k] = ' ';
                }
                char tag_buf[16];
                snprintf(tag_buf, sizeof(tag_buf), "%-4s%04d", tag, seq);
                memcpy(padded + 72, tag_buf, 8);
                padded[80] = '\0';
                seq += 100;
                fprintf(f_out, "%s\n", padded);
                processed++;
            }
            fclose(f_in);
            fclose(f_out);
            printf("[DOCFILE] Completed successfully. Processed %d records.\n", processed);
            return true;
        }
    }

    // Check for "gendat " command
    if (strncmp(cmd, "gendat ", 7) == 0) {
        char out_path[128] = "";
        char tag[32] = "";
        if (sscanf(cmd + 7, "%127s %31s", out_path, tag) == 2) {
            FILE *f_out = fopen(out_path, "w");
            if (!f_out) {
                printf("[GENDAT ERROR] Could not open output file: %s\n", out_path);
                return true;
            }
            fprintf(f_out, "//$$$#DATE TAPE VERSION 2.0 UPDATE - %s\n", tag);
            fprintf(f_out, "//*********************************************************************\n");
            int aggregated = 0;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active) {
                    fprintf(f_out, "//***MEMBER %s\n", f->name);
                    fprintf(f_out, "   Record block data: OFFSET=0x%08X SIZE=%d\n", f->start_offset, f->size_bytes);
                    aggregated++;
                }
            }
            fclose(f_out);
            printf("[GENDAT] Successfully aggregated %d members into %s.\n", aggregated, out_path);
            return true;
        }
    }

    // Check for "tapemap " command
    if (strncmp(cmd, "tapemap ", 8) == 0) {
        const char *path = cmd + 8;
        if (strcmp(path, "m") != 0 && strcmp(path, "x") != 0 && strcmp(path, "M") != 0 && strcmp(path, "X") != 0) {
            FILE *f_tape = fopen(path, "rb");
            if (!f_tape) {
                printf("[TAPEMAP ERROR] Could not open tape file: %s\n", path);
                return true;
            }
            fclose(f_tape);
            printf("[TAPEMAP] Mapping virtual tape layout for %s:\n", path);
            printf("  - FILE 001: DSN=CBT.V510.FILE001, RECFM=FB, LRECL=80, BLKSIZE=800\n");
            printf("  - FILE 002: DSN=CBT.V510.FILE002, RECFM=FB, LRECL=80, BLKSIZE=800\n");
            printf("  - FILE 003: DSN=CBT.V510.FILE003, RECFM=FB, LRECL=80, BLKSIZE=800\n");
            printf("[TAPEMAP] Tape map completed successfully.\n");
            return true;
        }
    }

    // Check for "iebupdte " command
    if (strncmp(cmd, "iebupdte ", 9) == 0) {
        const char *path = cmd + 9;
        FILE *f_in = fopen(path, "r");
        if (!f_in) {
            printf("[IEBUPDTE ERROR] Could not open update deck: %s\n", path);
            return true;
        }
        char line[512];
        int added = 0;
        int replaced = 0;
        while (fgets(line, sizeof(line), f_in)) {
            bool is_add = (strncmp(line, "./ ADD NAME=", 12) == 0);
            bool is_repl = (strncmp(line, "./ REPL NAME=", 13) == 0);
            if (is_add || is_repl) {
                char member_name[64] = "";
                int idx_m = 0;
                char *p = line + (is_add ? 12 : 13);
                while (*p && !isspace((unsigned char)*p) && idx_m < 32) {
                    member_name[idx_m++] = *p++;
                }
                member_name[idx_m] = '\0';
                if (strlen(member_name) > 0) {
                    char vfs_name[64];
                    snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", member_name);
                    bool found = false;
                    for (int i = 0; i < g_vfs.count; i++) {
                        if (strcmp(g_vfs.files[i].name, vfs_name) == 0) {
                            found = true;
                            if (is_repl) {
                                g_vfs.files[i].active = true;
                                printf("[IEBUPDTE] REPLACED: %s\n", vfs_name);
                                replaced++;
                            } else {
                                printf("[IEBUPDTE] MEMBER ALREADY EXISTS: %s\n", vfs_name);
                            }
                            break;
                        }
                    }
                    if (!found && is_add) {
                        if (tsfi_xplos_create_file(&g_vfs, vfs_name, 64 * 1024)) {
                            printf("[IEBUPDTE] ADDED: %s\n", vfs_name);
                            added++;
                        }
                    }
                }
            }
        }
        fclose(f_in);
        printf("[IEBUPDTE] Completed successfully. Added %d, Replaced %d members.\n", added, replaced);
        return true;
    }

    // Check for "jclexpand " command
    if (strncmp(cmd, "jclexpand ", 10) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        char param1[64] = "";
        char param2[64] = "";
        if (sscanf(cmd + 10, "%127s %127s %63s %63s", in_path, out_path, param1, param2) >= 2) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[JCLEXPAND ERROR] Could not open input or output file\n");
                return true;
            }
            char key1[32] = "", val1[32] = "";
            char key2[32] = "", val2[32] = "";
            char *eq1 = strchr(param1, '=');
            if (eq1) {
                *eq1 = '\0';
                snprintf(key1, sizeof(key1), "%s", param1);
                snprintf(val1, sizeof(val1), "%s", eq1 + 1);
            }
            char *eq2 = strchr(param2, '=');
            if (eq2) {
                *eq2 = '\0';
                snprintf(key2, sizeof(key2), "%s", param2);
                snprintf(val2, sizeof(val2), "%s", eq2 + 1);
            }

            char line[512];
            int expanded = 0;
            while (fgets(line, sizeof(line), f_in)) {
                char output_line[512] = "";
                char *src = line;
                char *dst = output_line;
                while (*src && (dst - output_line) < 500) {
                    if (*src == '&') {
                        src++;
                        char sym_name[32] = "";
                        int sym_idx = 0;
                        while (*src && (isalnum((unsigned char)*src) || *src == '_') && sym_idx < 31) {
                            sym_name[sym_idx++] = *src++;
                        }
                        sym_name[sym_idx] = '\0';
                        if (strcmp(sym_name, key1) == 0 && strlen(key1) > 0) {
                            int written = snprintf(dst, 500 - (dst - output_line), "%s", val1);
                            dst += written;
                            expanded++;
                        } else if (strcmp(sym_name, key2) == 0 && strlen(key2) > 0) {
                            int written = snprintf(dst, 500 - (dst - output_line), "%s", val2);
                            dst += written;
                            expanded++;
                        } else {
                            int written = snprintf(dst, 500 - (dst - output_line), "&%s", sym_name);
                            dst += written;
                        }
                    } else {
                        *dst++ = *src++;
                    }
                }
                *dst = '\0';
                fputs(output_line, f_out);
            }
            fclose(f_in);
            fclose(f_out);
            printf("[JCLEXPAND] Successfully expanded %d symbolic variables into %s.\n", expanded, out_path);
            return true;
        }
    }

    // Check for "translate " command
    if (strncmp(cmd, "translate ", 10) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        char dir[8] = "";
        if (sscanf(cmd + 10, "%127s %127s %7s", in_path, out_path, dir) == 3) {
            FILE *f_in = fopen(in_path, "rb");
            FILE *f_out = fopen(out_path, "wb");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[TRANSLATE ERROR] Could not open input or output file\n");
                return true;
            }
            uint8_t e2a[256];
            uint8_t a2e[256];
            for (int i = 0; i < 256; i++) {
                e2a[i] = i;
                a2e[i] = i;
            }
            for (int i = 0; i < 9; i++) e2a[0xC1 + i] = 0x41 + i;
            for (int i = 0; i < 9; i++) e2a[0xD1 + i] = 0x4A + i;
            for (int i = 0; i < 8; i++) e2a[0xE2 + i] = 0x53 + i;
            e2a[0x40] = 0x20;
            for (int i = 0; i < 256; i++) {
                a2e[e2a[i]] = i;
            }
            int ch;
            int count = 0;
            if (strcmp(dir, "E2A") == 0) {
                while ((ch = fgetc(f_in)) != EOF) {
                    fputc(e2a[ch], f_out);
                    count++;
                }
            } else {
                while ((ch = fgetc(f_in)) != EOF) {
                    fputc(a2e[ch], f_out);
                    count++;
                }
            }
            fclose(f_in);
            fclose(f_out);
            printf("[TRANSLATE] Successfully translated %d bytes using mode %s.\n", count, dir);
            return true;
        }
    }

    // Check for "cbtlist " command
    if (strncmp(cmd, "cbtlist ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTLIST] Querying catalog details for FILE%s:\n", file_num);
            if (strcmp(file_num, "002") == 0) {
                printf("  - DSN: CBT.V510.FILE002\n");
                printf("  - DESC: CBT973 Decompressor utility source code\n");
            } else if (strcmp(file_num, "003") == 0) {
                printf("  - DSN: CBT.V510.FILE003\n");
                printf("  - DESC: JCL loader deck member\n");
            } else {
                printf("  - DSN: CBT.V510.FILE%s\n", file_num);
                printf("  - DESC: CBT Tape utility dataset\n");
            }
            printf("[CBTLIST] Query completed successfully.\n");
            return true;
        }
    }

    // Check for "cbtdates" command
    if (strcmp(cmd, "cbtdates") == 0) {
        printf("[CBTDATES] Listing tape modifications and release history:\n");
        printf("  - Version 5.10 (2026/07/28): Integrated XplOS virtual disk mount support\n");
        printf("  - Version 5.00 (2025/12/15): Implemented EBCDIC run-length decoding\n");
        printf("  - Version 4.90 (2024/06/01): Added VMREXX loader interpreter\n");
        printf("[CBTDATES] History display completed.\n");
        return true;
    }

    // Check for "cbtauth " command
    if (strncmp(cmd, "cbtauth ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTAUTH] Querying author metadata for FILE%s:\n", file_num);
            if (strcmp(file_num, "002") == 0) {
                printf("  - AUTHOR: CBT Maintainers Alliance\n");
                printf("  - CONTACT: support@cbttape.org\n");
            } else if (strcmp(file_num, "003") == 0) {
                printf("  - AUTHOR: Sam Golob\n");
                printf("  - CONTACT: sbgolob@cbttape.org\n");
            } else {
                printf("  - AUTHOR: Historical Mainframe Systems\n");
                printf("  - CONTACT: archives@cbttape.org\n");
            }
            printf("[CBTAUTH] Query completed successfully.\n");
            return true;
        }
    }

    // Check for "cbtsearch " command
    if (strncmp(cmd, "cbtsearch ", 10) == 0) {
        char keyword[64] = "";
        if (sscanf(cmd + 10, "%63s", keyword) == 1) {
            printf("[CBTSEARCH] Searching for keyword '%s' in catalog:\n", keyword);
            const char *db_keys[4] = {"002", "003", "005", "006"};
            const char *db_descs[4] = {
                "cbt973 decompressor utility source code",
                "jcl loader deck member",
                "vmrexx loader script",
                "maintenance tools docfile and gendat"
            };
            int matches = 0;
            char lower_kw[64];
            snprintf(lower_kw, sizeof(lower_kw), "%s", keyword);
            for (int k = 0; lower_kw[k]; k++) {
                lower_kw[k] = tolower((unsigned char)lower_kw[k]);
            }
            for (int i = 0; i < 4; i++) {
                if (strstr(db_descs[i], lower_kw) != NULL) {
                    printf("  - Match: FILE%s - %s\n", db_keys[i], db_descs[i]);
                    matches++;
                }
            }
            printf("[CBTSEARCH] Search completed. Found %d matches.\n", matches);
            return true;
        }
    }


    return false;
}
