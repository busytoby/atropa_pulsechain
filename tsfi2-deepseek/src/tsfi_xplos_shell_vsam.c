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


bool tsfi_xplos_shell_vsam(const char *cmd) {
    (void)cmd;
    // Check for "jclfree " command
    if (strncmp(cmd, "jclfree ", 8) == 0) {
        char ddname[32] = "";
        char free_param[16] = "";
        if (sscanf(cmd + 8, "%31s %15s", ddname, free_param) == 2) {
            printf("[JCLFREE] DDNAME %s registered with FREE=%s. Dataset will be released dynamically.\n", ddname, free_param);
            return true;
        }
    }

    // Check for "jclrestart " command
    if (strncmp(cmd, "jclrestart ", 11) == 0) {
        char restart_step[32] = "";
        char current_step[32] = "";
        if (sscanf(cmd + 11, "%31s %31s", restart_step, current_step) == 2) {
            static bool s_active = false;
            if (strcmp(restart_step, current_step) == 0) {
                s_active = true;
            }
            if (s_active) {
                printf("[JCLRESTART] RESTART target reached. Executing step %s.\n", current_step);
            } else {
                printf("[JCLRESTART] RESTART target is %s. Bypassing execution of step %s.\n", restart_step, current_step);
            }
            return true;
        }
    }

    // Check for "jcllike " command
    if (strncmp(cmd, "jcllike ", 8) == 0) {
        char target_dsn[64] = "";
        char ref_dsn[64] = "";
        if (sscanf(cmd + 8, "%63s %63s", target_dsn, ref_dsn) == 2) {
            char target_ref[80];
            snprintf(target_ref, sizeof(target_ref), "%s.dat.bin", ref_dsn);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, target_ref) == 0) {
                    found = true;
                    break;
                }
            }
            printf("[JCLLIKE] Allocated %s mimicking attributes of %s:\n", target_dsn, ref_dsn);
            if (found) {
                printf("  - RECFM: FB, LRECL: 80, BLKSIZE: 800 (Source found in VFS catalog)\n");
            } else {
                printf("  - RECFM: FB, LRECL: 80, BLKSIZE: 800 (Default parameters applied)\n");
            }
            return true;
        }
    }

    // Check for "cbtpdsinit " command (Rule 13 enforcement for output extension)
    if (strncmp(cmd, "cbtpdsinit ", 11) == 0) {
        const char *path = cmd + 11;
        if (strstr(path, ".dat.bin") == NULL) {
            printf("[CBTPDSINIT ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return true;
        }
        FILE *f = fopen(path, "wb");
        if (!f) {
            printf("[CBTPDSINIT ERROR] Could not create file: %s\n", path);
            return true;
        }
        uint8_t dir_block[256];
        memset(dir_block, 0, sizeof(dir_block));
        dir_block[0] = 0;
        dir_block[1] = 2; // initial size is just these 2 bytes
        fwrite(dir_block, 1, 256, f);
        fclose(f);
        printf("[CBTPDSINIT] Initialized empty MVS-compatible PDS: %s\n", path);
        return true;
    }

    // Check for "cbtpdsadd " command
    if (strncmp(cmd, "cbtpdsadd ", 10) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        char input_path[128] = "";
        if (sscanf(cmd + 10, "%127s %31s %127s", pds_path, member_name, input_path) == 3) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSADD ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return true;
            }
            FILE *f_pds = fopen(pds_path, "r+b");
            if (!f_pds) {
                printf("[CBTPDSADD ERROR] Could not open PDS: %s\n", pds_path);
                return true;
            }
            FILE *f_in = fopen(input_path, "r");
            if (!f_in) {
                fclose(f_pds);
                printf("[CBTPDSADD ERROR] Could not open input file: %s\n", input_path);
                return true;
            }
            uint8_t dir_block[256];
            fseek(f_pds, 0, SEEK_SET);
            fread(dir_block, 1, 256, f_pds);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            
            fseek(f_pds, 0, SEEK_END);
            long offset = ftell(f_pds);
            
            char line[256];
            uint32_t size = 0;
            while (fgets(line, sizeof(line), f_in)) {
                line[strcspn(line, "\r\n")] = '\0';
                char card[80];
                memset(card, ' ', 80);
                int len = strlen(line);
                if (len > 80) len = 80;
                memcpy(card, line, len);
                fwrite(card, 1, 80, f_pds);
                size += 80;
            }
            fclose(f_in);
            
            if (used + 16 <= 256) {
                uint8_t *entry = &dir_block[used];
                memset(entry, ' ', 8);
                int m_len = strlen(member_name);
                if (m_len > 8) m_len = 8;
                memcpy(entry, member_name, m_len);
                entry[8] = (offset >> 16) & 0xFF;
                entry[9] = (offset >> 8) & 0xFF;
                entry[10] = offset & 0xFF;
                entry[11] = 1;
                entry[12] = (size >> 24) & 0xFF;
                entry[13] = (size >> 16) & 0xFF;
                entry[14] = (size >> 8) & 0xFF;
                entry[15] = size & 0xFF;
                
                used += 16;
                dir_block[0] = (used >> 8) & 0xFF;
                dir_block[1] = used & 0xFF;
                
                fseek(f_pds, 0, SEEK_SET);
                fwrite(dir_block, 1, 256, f_pds);
                printf("[CBTPDSADD] Added member %s (offset: 0x%06lX, size: %d bytes) to PDS: %s\n", member_name, offset, size, pds_path);
            } else {
                printf("[CBTPDSADD ERROR] PDS Directory block full.\n");
            }
            fclose(f_pds);
            return true;
        }
    }

    // Check for "cbtpdslist " command
    if (strncmp(cmd, "cbtpdslist ", 11) == 0) {
        const char *path = cmd + 11;
        if (strstr(path, ".dat.bin") == NULL) {
            printf("[CBTPDSLIST ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return true;
        }
        FILE *f = fopen(path, "rb");
        if (!f) {
            printf("[CBTPDSLIST ERROR] Could not open PDS: %s\n", path);
            return true;
        }
        uint8_t dir_block[256];
        fread(dir_block, 1, 256, f);
        fclose(f);
        uint16_t used = (dir_block[0] << 8) | dir_block[1];
        printf("[CBTPDSLIST] Listing PDS Directory Members for: %s\n", path);
        int ptr = 2;
        while (ptr < used) {
            char name[9];
            memcpy(name, &dir_block[ptr], 8);
            name[8] = '\0';
            for (int k = 7; k >= 0; k--) {
                if (name[k] == ' ') name[k] = '\0';
                else break;
            }
            uint32_t offset = (dir_block[ptr + 8] << 16) | (dir_block[ptr + 9] << 8) | dir_block[ptr + 10];
            uint32_t size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
            printf("  - MEMBER: %-8s OFFSET: 0x%06X SIZE: %d bytes\n", name, offset, size);
            ptr += 16;
        }
        printf("[CBTPDSLIST] List completed.\n");
        return true;
    }

    // Check for "cbtjesspool " command
    if (strncmp(cmd, "cbtjesspool ", 12) == 0) {
        const char *jobname = cmd + 12;
        printf("[CBTJESSPOOL] Spool logs for JOB: %s\n", jobname);
        printf("  - STEP 1 (IEBCOPY): RC=0000, Records read=120, Records written=120\n");
        printf("  - STEP 2 (PDSLOAD): RC=0000, Records processed=85\n");
        printf("[CBTJESSPOOL] Output spool closed.\n");
        return true;
    }

    // Check for "cbtclist " command
    if (strncmp(cmd, "cbtclist ", 9) == 0) {
        printf("[CBTCLIST] Executing TSO CLIST command list:\n");
        char copy_cmd[256];
        strncpy(copy_cmd, cmd + 9, sizeof(copy_cmd) - 1);
        copy_cmd[sizeof(copy_cmd) - 1] = '\0';
        char *token = strtok(copy_cmd, ";");
        while (token != NULL) {
            while (*token == ' ') token++;
            int len = strlen(token);
            while (len > 0 && token[len - 1] == ' ') {
                token[len - 1] = '\0';
                len--;
            }
            if (strlen(token) > 0) {
                printf("  - Executing statement: %s\n", token);
                shell_task_handler(token);
            }
            token = strtok(NULL, ";");
        }
        printf("[CBTCLIST] Execution complete.\n");
        return true;
    }

    // Check for "cbtsearch " command
    if (strncmp(cmd, "cbtsearch ", 10) == 0) {
        const char *keyword = cmd + 10;
        printf("[CBTSEARCH] Searching mounted VFS datasets for '%s':\n", keyword);
        int matches = 0;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strstr(g_vfs.files[i].name, keyword) != NULL) {
                printf("  - MATCH: %s (size: %u bytes)\n", g_vfs.files[i].name, (unsigned int)g_vfs.files[i].size_bytes);
                matches++;
            }
        }
        printf("[CBTSEARCH] Found %d matches.\n", matches);
        return true;
    }

    // Check for "cbtauth " command
    if (strncmp(cmd, "cbtauth ", 8) == 0) {
        const char *author = cmd + 8;
        printf("[CBTAUTH] Querying contributions for author '%s':\n", author);
        if (strcasecmp(author, "UCLA") == 0) {
            printf("  - FILE007: UCLA Tapemap utility program\n");
            printf("  - FILE012: UCLA SMF reporting package\n");
        } else if (strcasecmp(author, "IBM") == 0) {
            printf("  - FILE001: System directory index\n");
            printf("  - FILE002: CBT973 decompilation exit\n");
        } else {
            printf("  - No direct contributions listed in current VFS segment registry.\n");
        }
        printf("[CBTAUTH] Query completed.\n");
        return true;
    }

    // Check for "cbtpdsread " command
    if (strncmp(cmd, "cbtpdsread ", 11) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 11, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSREAD ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return true;
            }
            FILE *f = fopen(pds_path, "rb");
            if (!f) {
                printf("[CBTPDSREAD ERROR] Could not open PDS: %s\n", pds_path);
                return true;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            uint32_t offset = 0;
            uint32_t size = 0;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    offset = (dir_block[ptr + 8] << 16) | (dir_block[ptr + 9] << 8) | dir_block[ptr + 10];
                    size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                printf("[CBTPDSREAD] Reading member %s from %s (offset: 0x%06X, size: %u bytes):\n", member_name, pds_path, offset, size);
                fseek(f, offset, SEEK_SET);
                uint32_t read_bytes = 0;
                while (read_bytes < size) {
                    char card[81];
                    if (fread(card, 1, 80, f) != 80) break;
                    card[80] = '\0';
                    int trim_idx = 79;
                    while (trim_idx >= 0 && card[trim_idx] == ' ') {
                        card[trim_idx] = '\0';
                        trim_idx--;
                    }
                    printf("  %s\n", card);
                    read_bytes += 80;
                }
            } else {
                printf("[CBTPDSREAD ERROR] Member %s not found in PDS.\n", member_name);
            }
            fclose(f);
            return true;
        }
    }

    // Check for "cbtpdscompress " command
    if (strncmp(cmd, "cbtpdscompress ", 15) == 0) {
        const char *pds_path = cmd + 15;
        if (strstr(pds_path, ".dat.bin") == NULL) {
            printf("[CBTPDSCOMPRESS ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return true;
        }
        FILE *f = fopen(pds_path, "r+b");
        if (!f) {
            printf("[CBTPDSCOMPRESS ERROR] Could not open PDS: %s\n", pds_path);
            return true;
        }
        uint8_t dir_block[256];
        fread(dir_block, 1, 256, f);
        uint16_t used = (dir_block[0] << 8) | dir_block[1];
        
        int count = (used - 2) / 16;
        if (count <= 0) {
            printf("[CBTPDSCOMPRESS] PDS is already compact (0 active members).\n");
            fclose(f);
            return true;
        }
        
        uint8_t **buffers = malloc(count * sizeof(uint8_t*));
        uint32_t *sizes = malloc(count * sizeof(uint32_t));
        uint32_t *old_offsets = malloc(count * sizeof(uint32_t));
        
        int ptr = 2;
        int idx = 0;
        while (ptr < used) {
            uint32_t offset = (dir_block[ptr + 8] << 16) | (dir_block[ptr + 9] << 8) | dir_block[ptr + 10];
            uint32_t size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
            
            buffers[idx] = malloc(size);
            sizes[idx] = size;
            old_offsets[idx] = offset;
            
            fseek(f, offset, SEEK_SET);
            fread(buffers[idx], 1, size, f);
            
            ptr += 16;
            idx++;
        }
        
        long current_offset = 256;
        ptr = 2;
        idx = 0;
        while (ptr < used) {
            fseek(f, current_offset, SEEK_SET);
            fwrite(buffers[idx], 1, sizes[idx], f);
            
            dir_block[ptr + 8] = (current_offset >> 16) & 0xFF;
            dir_block[ptr + 9] = (current_offset >> 8) & 0xFF;
            dir_block[ptr + 10] = current_offset & 0xFF;
            
            printf("  - Repositioned member %d: Offset 0x%06X -> 0x%06lX (%u bytes)\n", idx + 1, old_offsets[idx], current_offset, sizes[idx]);
            
            current_offset += sizes[idx];
            free(buffers[idx]);
            ptr += 16;
            idx++;
        }
        
        free(buffers);
        free(sizes);
        free(old_offsets);
        
        fflush(f);
        int fd = fileno(f);
        if (ftruncate(fd, current_offset) != 0) {
            printf("[CBTPDSCOMPRESS WARNING] Could not truncate file.\n");
        }
        
        fseek(f, 0, SEEK_SET);
        fwrite(dir_block, 1, 256, f);
        fclose(f);
        
        printf("[CBTPDSCOMPRESS] PDS compression completed. Reclaimed space gaps. New file size: %ld bytes.\n", current_offset);
        return true;
    }

    // Check for "cbtpdsdel " command
    if (strncmp(cmd, "cbtpdsdel ", 10) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 10, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSDEL ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return true;
            }
            FILE *f = fopen(pds_path, "r+b");
            if (!f) {
                printf("[CBTPDSDEL ERROR] Could not open PDS: %s\n", pds_path);
                return true;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    int entries_to_shift = (used - (ptr + 16)) / 16;
                    if (entries_to_shift > 0) {
                        memmove(&dir_block[ptr], &dir_block[ptr + 16], entries_to_shift * 16);
                    }
                    used -= 16;
                    memset(&dir_block[used], 0, 16);
                    dir_block[0] = (used >> 8) & 0xFF;
                    dir_block[1] = used & 0xFF;
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                fseek(f, 0, SEEK_SET);
                fwrite(dir_block, 1, 256, f);
                printf("[CBTPDSDEL] Deleted member %s from PDS: %s\n", member_name, pds_path);
            } else {
                printf("[CBTPDSDEL ERROR] Member %s not found in PDS.\n", member_name);
            }
            fclose(f);
            return true;
        }
    }

    // Check for "cbtrexx " command
    if (strncmp(cmd, "cbtrexx ", 8) == 0) {
        char exec_name[64] = "";
        char exec_args[128] = "";
        int scanned = sscanf(cmd + 8, "%63s %[^\n]", exec_name, exec_args);
        if (scanned >= 1) {
            printf("[CBTREXX] Executing REXX exec '%s' from File #020:\n", exec_name);
            if (strcasecmp(exec_name, "LISTDSPD") == 0) {
                printf("  - Parsing target PDS attributes for dataset: %s\n", exec_args);
                printf("  - LISTDSPD REXX Status: Space allocated=15 tracks, directory blocks=5, used=1\n");
            } else if (strcasecmp(exec_name, "DISPMOD") == 0) {
                printf("  - Module name resolved: %s\n", exec_args);
                printf("  - DISPMOD REXX Status: AMODE=31, RMODE=ANY, ATTRS=RENT, REUS\n");
            } else {
                printf("  - Executed script '%s' with arguments: '%s'\n", exec_name, exec_args);
            }
            printf("[CBTREXX] Return Code: 0000 (Success)\n");
            return true;
        }
    }

    // Check for "cbtpdsstats " command
    if (strncmp(cmd, "cbtpdsstats ", 12) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 12, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSSTATS ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return true;
            }
            FILE *f = fopen(pds_path, "rb");
            if (!f) {
                printf("[CBTPDSSTATS ERROR] Could not open PDS: %s\n", pds_path);
                return true;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            uint32_t size = 0;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
                    found = true;
                    break;
                }
                ptr += 16;
            }
            fclose(f);
            if (found) {
                uint32_t lines = size / 80;
                printf("[CBTPDSSTATS] ISPF Statistics for member %s in %s:\n", member_name, pds_path);
                printf("  - Created:        2026/07/28\n");
                printf("  - Modified:       2026/07/28 17:18\n");
                printf("  - Current Lines:  %u\n", lines);
                printf("  - Version:        01.00\n");
                printf("  - User ID:        MVSUSER\n");
            } else {
                printf("[CBTPDSSTATS ERROR] Member %s not found in PDS.\n", member_name);
            }
            return true;
        }
    }

    // Check for "cbtwho" command
    if (strncmp(cmd, "cbtwho", 6) == 0) {
        char target_user[32] = "";
        int scanned = sscanf(cmd + 6, "%31s", target_user);
        printf("[CBTWHO] Querying active TSO sessions:\n");
        if (scanned <= 0) {
            printf("  - USER: MVSUSER   LINE: TTY001   LOGON: 2026/07/28 12:00   ALLOCS: 3\n");
            printf("  - USER: ZUSER     LINE: TTY002   LOGON: 2026/07/28 14:15   ALLOCS: 1\n");
        } else {
            if (strcasecmp(target_user, "MVSUSER") == 0) {
                printf("  - USER: MVSUSER   LINE: TTY001   LOGON: 2026/07/28 12:00\n");
                printf("    * ALLOC: SYS1.MACLIB.dat.bin (SHR)\n");
                printf("    * ALLOC: CBT.FILE001.dat.bin (SHR)\n");
                printf("    * ALLOC: SYS1.PROCLIB.dat.bin (SHR)\n");
            } else if (strcasecmp(target_user, "ZUSER") == 0) {
                printf("  - USER: ZUSER     LINE: TTY002   LOGON: 2026/07/28 14:15\n");
                printf("    * ALLOC: ZUSER.LOAD.dat.bin (OLD)\n");
            } else {
                printf("  - User ID '%s' not currently logged on.\n", target_user);
            }
        }
        printf("[CBTWHO] Query completed.\n");
        return true;
    }

    // Check for "cbtpdsren " command
    if (strncmp(cmd, "cbtpdsren ", 10) == 0) {
        char pds_path[128] = "";
        char old_name[32] = "";
        char new_name[32] = "";
        if (sscanf(cmd + 10, "%127s %31s %31s", pds_path, old_name, new_name) == 3) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSREN ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return true;
            }
            FILE *f = fopen(pds_path, "r+b");
            if (!f) {
                printf("[CBTPDSREN ERROR] Could not open PDS: %s\n", pds_path);
                return true;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, old_name) == 0) {
                    memset(&dir_block[ptr], ' ', 8);
                    int len = strlen(new_name);
                    if (len > 8) len = 8;
                    memcpy(&dir_block[ptr], new_name, len);
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                fseek(f, 0, SEEK_SET);
                fwrite(dir_block, 1, 256, f);
                printf("[CBTPDSREN] Renamed PDS member %s to %s inside: %s\n", old_name, new_name, pds_path);
            } else {
                printf("[CBTPDSREN ERROR] Member %s not found in PDS.\n", old_name);
            }
            fclose(f);
            return true;
        }
    }

    // Check for "cbtpdsdump " command
    if (strncmp(cmd, "cbtpdsdump ", 11) == 0) {
        const char *pds_path = cmd + 11;
        if (strstr(pds_path, ".dat.bin") == NULL) {
            printf("[CBTPDSDUMP ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return true;
        }
        FILE *f = fopen(pds_path, "rb");
        if (!f) {
            printf("[CBTPDSDUMP ERROR] Could not open PDS: %s\n", pds_path);
            return true;
        }
        uint8_t dir_block[256];
        fread(dir_block, 1, 256, f);
        fclose(f);
        printf("[CBTPDSDUMP] Raw Directory Block Hex Dump for: %s\n", pds_path);
        for (int i = 0; i < 256; i += 16) {
            printf("  %04X: ", i);
            for (int j = 0; j < 16; j++) {
                printf("%02X ", dir_block[i + j]);
            }
            printf(" | ");
            for (int j = 0; j < 16; j++) {
                char ch = (char)dir_block[i + j];
                if (isprint((unsigned char)ch)) {
                    printf("%c", ch);
                } else {
                    printf(".");
                }
            }
            printf("\n");
        }
        printf("[CBTPDSDUMP] Dump completed.\n");
        return true;
    }

    // Check for "cbtcicssubmit " command
    if (strncmp(cmd, "cbtcicssubmit ", 14) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 14, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTCICSSUBMIT ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return true;
            }
            FILE *f = fopen(pds_path, "rb");
            if (!f) {
                printf("[CBTCICSSUBMIT ERROR] Could not open PDS: %s\n", pds_path);
                return true;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            fclose(f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                int free_idx = -1;
                for (int i = 0; i < 10; i++) {
                    if (!cbt_job_table[i].active) {
                        free_idx = i;
                        break;
                    }
                }
                if (free_idx != -1) {
                    sprintf(cbt_job_table[free_idx].job_id, "JOB000%d", 21 + free_idx);
                    strncpy(cbt_job_table[free_idx].job_name, member_name, 15);
                    cbt_job_table[free_idx].job_name[15] = '\0';
                    strcpy(cbt_job_table[free_idx].status, "READY");
                    cbt_job_table[free_idx].class_char = 'A';
                    cbt_job_table[free_idx].active = true;
                    cbt_job_table[free_idx].cics_origin = true;
                    
                    printf("[CBTCICSSUBMIT] CICS Transaction 'SBMJ' submitting member %s:\n", member_name);
                    printf("  - Job submitted: %s (%s) via INTRDR spool interface\n", cbt_job_table[free_idx].job_name, cbt_job_table[free_idx].job_id);
                    printf("  - CICS Return Code: EIBRESP=0 (NORMAL)\n");
                } else {
                    printf("[CBTCICSSUBMIT ERROR] JES Input Queue Full.\n");
                }
            } else {
                printf("[CBTCICSSUBMIT ERROR] Member %s not found in PDS.\n", member_name);
            }
            return true;
        }
    }

    // Check for "cbtjeslist" command
    if (strncmp(cmd, "cbtjeslist", 10) == 0) {
        printf("[CBTJESLIST] Active JES Spool Job Queue:\n");
        for (int i = 0; i < 10; i++) {
            if (cbt_job_table[i].active) {
                printf("  - %s   %-10s MVSUSER   CLASS %c   STATUS: %s (CICS Coupled: %s)\n",
                       cbt_job_table[i].job_id,
                       cbt_job_table[i].job_name,
                       cbt_job_table[i].class_char,
                       cbt_job_table[i].status,
                       cbt_job_table[i].cics_origin ? "YES" : "NO");
            }
        }
        printf("[CBTJESLIST] Queue status displayed successfully.\n");
        return true;
    }

    // Check for "cbtjespurge " command
    if (strncmp(cmd, "cbtjespurge ", 12) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 12, "%31s", job_id) == 1) {
            bool purged = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    cbt_job_table[i].active = false;
                    purged = true;
                    break;
                }
            }
            if (purged) {
                printf("[CBTJESPURGE] Purging job %s from JES spool queues:\n", job_id);
                printf("  - Job %s deleted successfully. Reclaimed 14 spool tracks.\n", job_id);
            } else {
                printf("[CBTJESPURGE ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESPURGE ERROR] Job ID required.\n");
        }
        return true;
    }

    // Check for "cbtjeshold " command
    if (strncmp(cmd, "cbtjeshold ", 11) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 11, "%31s", job_id) == 1) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    strcpy(cbt_job_table[i].status, "HELD");
                    found_job = true;
                    break;
                }
            }
            if (found_job) {
                printf("[CBTJESHOLD] Holding job %s in JES spool queue.\n", job_id);
                printf("  - Job %s status set to HELD.\n", job_id);
            } else {
                printf("[CBTJESHOLD ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESHOLD ERROR] Job ID required.\n");
        }
        return true;
    }

    // Check for "cbtjesrelease " command
    if (strncmp(cmd, "cbtjesrelease ", 14) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 14, "%31s", job_id) == 1) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    strcpy(cbt_job_table[i].status, "READY");
                    found_job = true;
                    break;
                }
            }
            if (found_job) {
                printf("[CBTJESRELEASE] Releasing job %s from HELD status in JES spool queue.\n", job_id);
                printf("  - Job %s status set to READY.\n", job_id);
            } else {
                printf("[CBTJESRELEASE ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESRELEASE ERROR] Job ID required.\n");
        }
        return true;
    }

    // Check for "cbtjesclass " command
    if (strncmp(cmd, "cbtjesclass ", 12) == 0) {
        char job_id[32] = "";
        char new_class[8] = "";
        if (sscanf(cmd + 12, "%31s %7s", job_id, new_class) == 2) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    cbt_job_table[i].class_char = new_class[0];
                    found_job = true;
                    break;
                }
            }
            if (found_job) {
                printf("[CBTJESCLASS] Modifying job %s in JES spool queue.\n", job_id);
                printf("  - Job %s class updated to %s.\n", job_id, new_class);
            } else {
                printf("[CBTJESCLASS ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESCLASS ERROR] Job ID and new class parameter required.\n");
        }
        return true;
    }

    // Check for "cbtcicsinq " command
    if (strncmp(cmd, "cbtcicsinq ", 11) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 11, "%31s", job_id) == 1) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    found_job = true;
                    if (cbt_job_table[i].cics_origin) {
                        printf("[CBTCICSINQ] Querying CICS-Coupled JES Job: %s\n", job_id);
                        printf("  - CICS Transaction: SBMJ\n");
                        printf("  - Target Spool Job: %s\n", cbt_job_table[i].job_name);
                        printf("  - Current Queue Status: %s\n", cbt_job_table[i].status);
                        printf("  - CICS Execution Interface Block Status:\n");
                        printf("    * EIBRESP:   00000000 (DFHRESP(NORMAL))\n");
                        printf("    * EIBFN:     0x0C02 (INTRDR SPOOLWRITE)\n");
                        printf("    * EIBAID:    0x7D (ENTER KEY)\n");
                    } else {
                        printf("[CBTCICSINQ ERROR] Job %s was not originated/coupled via CICS.\n", job_id);
                    }
                    break;
                }
            }
            if (!found_job) {
                printf("[CBTCICSINQ ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTCICSINQ ERROR] Job ID required.\n");
        }
        return true;
    }

    // Check for "cbtsyslog" command
    if (strncmp(cmd, "cbtsyslog", 9) == 0) {
        printf("[CBTSYSLOG] Active system console log (SYSLOG):\n");
        printf("  - 12:00:00 IPL001I System IPL complete. XplOS Cooperative Kernel active.\n");
        printf("  - 12:00:05 IEE104I Active started tasks: JES2, CICS, TSO.\n");
        printf("  - 12:00:10 IEC130I DD STATEMENT MISSING FOR SYSUADS.\n");
        printf("  - 12:00:15 TSO022I USER MVSUSER LOGGED ON TO TTY001.\n");
        printf("[CBTSYSLOG] Log display completed.\n");
        return true;
    }


    return false;
}
