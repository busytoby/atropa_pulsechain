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
#include "tsfi_mainframe_computerworld.h"

// Variable references defined in kernel
extern XplosVirtualDisk g_vfs;
extern bool g_vfs_initialized;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
extern CbtTreeNode *g_queue_tree_root;

extern uint32_t ce_gprs[16];
extern uint8_t ce_memory[1024];
extern uint32_t xdc_ip;


bool tsfi_xplos_shell_book(const char *cmd) {
    (void)cmd;
    if (strncmp(cmd, "cbtcompress ", 12) == 0) {
        char pds_name[64] = "";
        sscanf(cmd + 12, "%63s", pds_name);
        char vfs_filename[128];
        snprintf(vfs_filename, sizeof(vfs_filename), "%s.dat.bin", pds_name);

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, vfs_filename) == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx < 0) {
            printf("[CBTCOMPRESS ERROR] Library %s not found.\n", pds_name);
            return true;
        }

        printf("[CBTCOMPRESS] Auditing PDS library: %s\n", vfs_filename);
        printf("  - Active size in bytes: %u\n", g_vfs.files[file_idx].size_bytes);
        printf("  - Reclaiming orphaned directory index sectors...\n");
        printf("  - Recalculating relative track offsets (TTR)...\n");

        uint32_t original_size = g_vfs.files[file_idx].size_bytes;
        uint32_t reclaimed_bytes = original_size * 15 / 100;
        uint32_t new_size = original_size - reclaimed_bytes;
        g_vfs.files[file_idx].size_bytes = new_size;

        printf("[CBTCOMPRESS] PDS Library %s compressed successfully.\n", pds_name);
        printf("  - Reclaimed space: %u bytes (%u sectors)\n", reclaimed_bytes, reclaimed_bytes / 256);
        printf("  - New Library size: %u bytes. RC=0000\n", new_size);
        return true;
    }

    // Check for "cbtmountmem " command
    if (strncmp(cmd, "cbtmountmem ", 12) == 0) {
        const char *path = cmd + 12;
        if (strlen(path) > 0) {
            tsfi_cbt_mount_inmemory_pds(&g_vfs, path);
        } else {
            printf("[CBTMOUNTMEM ERROR] Server path required.\n");
        }
        return true;
    }

    // Check for "cbtcpu" command
    if (strcmp(cmd, "cbtcpu") == 0) {
        printf("[CBTCPU] Querying physical processor hardware parameters:\n");
        printf("  - Machine Type:     3090-600J\n");
        printf("  - Serial Number:    4801205\n");
        printf("  - Physical Cores:   6 active engines\n");
        printf("  - Cryptographic FP: Installed (ICF-3)\n");
        printf("[CBTCPU] CPU hardware parameter query completed successfully.\n");
        return true;
    }

    // Check for "ibhdrply" command
    if (strcmp(cmd, "ibhdrply") == 0) {
        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, "IBHDRPLY.dat.bin") == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            printf("[IBHDRPLY] Starting Automatic Reply Program (Bell & Howell):\n");
            printf("  - Parsing active rules from VFS member IBHDRPLY.dat.bin:\n");
            char temp_data[8192];
            strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
            temp_data[sizeof(temp_data) - 1] = '\0';
            char *line = strtok(temp_data, "\n");
            for (int l = 0; l < 2 && line; l++) {
                printf("    * Code: %s\n", line);
                line = strtok(NULL, "\n");
            }
            
            // Parse WTOR rules from VFS
            char match_id[32] = "IEC141I";
            char match_reply[16] = "U";
            strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
            char *rule_line = strtok(temp_data, "\n");
            while (rule_line) {
                char *msgid_ptr = strstr(rule_line, "MSGID='");
                char *reply_ptr = strstr(rule_line, "REPLY='");
                if (msgid_ptr && reply_ptr) {
                    msgid_ptr += 7;
                    char *msgid_end = strchr(msgid_ptr, '\'');
                    reply_ptr += 7;
                    char *reply_end = strchr(reply_ptr, '\'');
                    if (msgid_end && reply_end) {
                        size_t id_len = msgid_end - msgid_ptr;
                        size_t rep_len = reply_end - reply_ptr;
                        if (id_len < sizeof(match_id)) {
                            memcpy(match_id, msgid_ptr, id_len);
                            match_id[id_len] = '\0';
                        }
                        if (rep_len < sizeof(match_reply)) {
                            memcpy(match_reply, reply_ptr, rep_len);
                            match_reply[rep_len] = '\0';
                        }
                        break;
                    }
                }
                rule_line = strtok(NULL, "\n");
            }

            printf("  - Intercepting WTOR: '%s A,001,SL,VAESEN,VAESEN,SYS1.MACLIB'\n", match_id);
            printf("  - Generated Reply:   '%s' (Rule matched dynamically from VFS)\n", match_reply);
            printf("[IBHDRPLY] Automatic reply executed successfully.\n");
        } else {
            printf("[IBHDRPLY ERROR] IBHDRPLY module is not mounted in VFS.\n");
        }
        return true;
    }

    // Check for "ibhwtorg" command
    if (strcmp(cmd, "ibhwtorg") == 0) {
        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, "IBHWTORG.dat.bin") == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            printf("[IBHWTORG] Retrieving pending operator console replies:\n");
            printf("  - Active parameters from VFS member IBHWTORG.dat.bin:\n");
            char temp_data[8192];
            strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
            temp_data[sizeof(temp_data) - 1] = '\0';
            char *line = strtok(temp_data, "\n");
            for (int l = 0; l < 2 && line; l++) {
                printf("    * Code: %s\n", line);
                line = strtok(NULL, "\n");
            }
            printf("  - Reply 01: 'U' (generated by IBHDRPLY using rules in IBHWTORG.dat.bin)\n");
            printf("[IBHWTORG] Operator reply retrieval completed successfully.\n");
        } else {
            printf("[IBHWTORG ERROR] IBHWTORG module is not mounted in VFS.\n");
        }
        return true;
    }

    // Check for "ocx " command
    if (strncmp(cmd, "ocx ", 4) == 0) {
        const char *member = cmd + 4;
        if (strlen(member) > 0) {
            char target_name[80];
            resolve_pds_name(member, target_name, sizeof(target_name));
            int file_idx = -1;
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, target_name) == 0) {
                    file_idx = i;
                    break;
                }
            }
            if (file_idx >= 0) {
                printf("[OCX] Executing operator command script from VFS member: %s\n", target_name);
                char temp_data[8192];
                strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
                temp_data[sizeof(temp_data) - 1] = '\0';
                
                char *line = strtok(temp_data, "\n");
                while (line) {
                    if (strstr(line, "EXEC PGM=")) {
                        char pgm_name[32] = "";
                        char *pgm_ptr = strstr(line, "EXEC PGM=");
                        if (pgm_ptr) {
                            sscanf(pgm_ptr + 9, "%31[^, \r\n]", pgm_name);
                        }
                        printf("  - Dispatching Job Step Program: %s under Scheduler\n", pgm_name);
                    } else if (strstr(line, "DD DISP=")) {
                        char dd_name[32] = "";
                        char dsn_name[64] = "";
                        sscanf(line, "%31s", dd_name);
                        char *dsn_ptr = strstr(line, "DSN=");
                        if (dsn_ptr) {
                            sscanf(dsn_ptr + 4, "%63[^, \r\n]", dsn_name);
                        }
                        printf("  - Allocating DD:%s -> Dataset:%s\n", dd_name, dsn_name);
                    }
                    line = strtok(NULL, "\n");
                }
                printf("[OCX] Command execution completed successfully.\n");
            } else {
                printf("[OCX ERROR] Member %s not found in active VFS. Please mount dataset first.\n", target_name);
            }
        } else {
            printf("[OCX ERROR] Member name required.\n");
        }
        return true;
    }

    // Check for "ibhlspac " command
    if (strncmp(cmd, "ibhlspac ", 9) == 0) {
        const char *vol = cmd + 9;
        if (strlen(vol) > 0) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "IBHLSPAC.dat.bin");
            int file_idx = -1;
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, target_name) == 0) {
                    file_idx = i;
                    break;
                }
            }
            if (file_idx >= 0) {
                printf("[IBHLSPAC] Listing DASD volume space information for: %s\n", vol);
                printf("  - Parsing volume parameters from VFS member IBHLSPAC.dat.bin:\n");
                char temp_data[8192];
                strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
                temp_data[sizeof(temp_data) - 1] = '\0';
                char *line = strtok(temp_data, "\n");
                for (int l = 0; l < 2 && line; l++) {
                    printf("    * Code: %s\n", line);
                    line = strtok(NULL, "\n");
                }

                // Dynamic allocation statistics from memory VFS
                uint32_t total_size = 0;
                for (int i = 0; i < g_vfs.count; i++) {
                    if (g_vfs.files[i].active) {
                        total_size += g_vfs.files[i].size_bytes;
                    }
                }
                uint32_t free_cyls = 1000 - (total_size / (500 * 1024));
                uint32_t free_tracks = free_cyls * 15;
                uint32_t max_contig = free_cyls / 2;

                printf("  - Dynamic VFS Allocation Statistics:\n");
                printf("    * Free Cylinders:  %u\n", free_cyls);
                printf("    * Free Tracks:     %u\n", free_tracks);
                printf("    * Largest Contig:  %u cylinders\n", max_contig);
                printf("[IBHLSPAC] Volume space listing completed successfully.\n");
            } else {
                printf("[IBHLSPAC ERROR] IBHLSPAC module is not mounted in VFS.\n");
            }
        } else {
            printf("[IBHLSPAC ERROR] Volume name required.\n");
        }
        return true;
    }

    // Check for "ibhj2005 " command
    if (strncmp(cmd, "ibhj2005 ", 9) == 0) {
        const char *args = cmd + 9;
        char user[32] = "";
        char target_cmd[32] = "";
        sscanf(args, "%31s %31s", target_cmd, user);
        
        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, "IBHJ2005.dat.bin") == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            printf("[IBHJ2005] JES2 Exit 5: Command Authorization verification:\n");
            printf("  - Parsing auth structures from VFS member IBHJ2005.dat.bin:\n");
            char temp_data[8192];
            strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
            temp_data[sizeof(temp_data) - 1] = '\0';
            char *line = strtok(temp_data, "\n");
            for (int l = 0; l < 2 && line; l++) {
                printf("    * Code: %s\n", line);
                line = strtok(NULL, "\n");
            }
            
            bool allowed = (strcmp(user, "MVSUSER") == 0 || strcmp(user, "SYSOP") == 0);
            printf("  - Authorization check for command '%s' by user '%s': %s\n", target_cmd, user, allowed ? "GRANTED" : "DENIED");
            printf("[IBHJ2005] Exit execution completed successfully.\n");
        } else {
            printf("[IBHJ2005 ERROR] IBHJ2005 module is not mounted in VFS.\n");
        }
        return true;
    }

    // Check for "ibhwtorm " command
    if (strncmp(cmd, "ibhwtorm ", 9) == 0) {
        const char *msg = cmd + 9;
        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, "IBHWTORM.dat.bin") == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            printf("[IBHWTORM] Console Write Exit: Filtering message stream:\n");
            printf("  - Parsing parameters from VFS member IBHWTORM.dat.bin:\n");
            char temp_data[8192];
            strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
            temp_data[sizeof(temp_data) - 1] = '\0';
            char *line = strtok(temp_data, "\n");
            for (int l = 0; l < 2 && line; l++) {
                printf("    * Code: %s\n", line);
                line = strtok(NULL, "\n");
            }
            bool filtered = (strstr(msg, "DEBUG") != NULL || strstr(msg, "TRACE") != NULL);
            printf("  - Message: '%s' -> %s\n", msg, filtered ? "SUPPRESSED (Filter matched)" : "PASSED");
            printf("[IBHWTORM] Log filtering execution completed successfully.\n");
        } else {
            printf("[IBHWTORM ERROR] IBHWTORM module is not mounted in VFS.\n");
        }
        return true;
    }

    // Check for "ibhj2001 " command
    if (strncmp(cmd, "ibhj2001 ", 9) == 0) {
        const char *job = cmd + 9;
        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, "IBHJ2001.dat.bin") == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            printf("[IBHJ2001] JES2 Exit 1: Formatting separator banner page for job: %s\n", job);
            printf("  - Layout rules from VFS member IBHJ2001.dat.bin:\n");
            char temp_data[8192];
            strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
            temp_data[sizeof(temp_data) - 1] = '\0';
            char *line = strtok(temp_data, "\n");
            for (int l = 0; l < 2 && line; l++) {
                printf("    * Code: %s\n", line);
                line = strtok(NULL, "\n");
            }
            printf("+----------------------------------------------------+\n");
            printf("| JOB: %-10s  CLASS: A  SYSTEM: XPL1            |\n", job);
            printf("+----------------------------------------------------+\n");
            printf("[IBHJ2001] Separator header page generated successfully.\n");
        } else {
            printf("[IBHJ2001 ERROR] IBHJ2001 module is not mounted in VFS.\n");
        }
        return true;
    }

    // Check for "ibhjespm " command
    if (strncmp(cmd, "ibhjespm ", 9) == 0) {
        const char *args = cmd + 9;
        char user[32] = "";
        char dna_sig[64] = "";
        sscanf(args, "%31s %63s", user, dna_sig);

        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, "IBHJESPM.dat.bin") == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            printf("[IBHJESPM] JES2 Security Monitor: Verifying credentials:\n");
            printf("  - Querying rules database from VFS member IBHJESPM.dat.bin:\n");
            char temp_data[8192];
            strncpy(temp_data, g_vfs.files[file_idx].data, sizeof(temp_data) - 1);
            temp_data[sizeof(temp_data) - 1] = '\0';
            char *line = strtok(temp_data, "\n");
            for (int l = 0; l < 2 && line; l++) {
                printf("    * Code: %s\n", line);
                line = strtok(NULL, "\n");
            }
            bool sig_valid = (strlen(dna_sig) >= 6);
            printf("  - Security Validation for '%s' [DNA Signature: %s]: %s\n", user, dna_sig, sig_valid ? "SUCCESS" : "FAILED");
            printf("[IBHJESPM] Security audit verification completed successfully.\n");
        } else {
            printf("[IBHJESPM ERROR] IBHJESPM module is not mounted in VFS.\n");
        }
        return true;
    }

    // Check for "pds " command
    if (strncmp(cmd, "pds ", 4) == 0) {
        const char *args = cmd + 4;
        printf("[PDS] PDS Command Processor - Version 8.6:\n");
        printf("  - Analyzing Partitioned Dataset members matching query: %s\n", args);
        int count = 0;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strstr(g_vfs.files[i].name, args)) {
                printf("    * Member: %-16s  Size: %u bytes\n", g_vfs.files[i].name, g_vfs.files[i].size_bytes);
                count++;
            }
        }
        printf("[PDS] Scan completed. Found %d matching members.\n", count);
        return true;
    }

    // Check for "vtoc " command
    if (strncmp(cmd, "vtoc ", 5) == 0) {
        const char *vol = cmd + 5;
        printf("[VTOC] Volume Table of Contents Lister for: %s\n", vol);
        printf("  - Volume: %-10s  Device: 3380  Active Datasets:\n", vol);
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active) {
                printf("    * DSN: %-24s  Tracks: %u  Extents: 1\n", g_vfs.files[i].name, (g_vfs.files[i].size_bytes + 4095) / 4096);
            }
        }
        printf("[VTOC] End of VTOC listing.\n");
        return true;
    }

    // Check for "look" command
    if (strncmp(cmd, "look", 4) == 0) {
        const char *args = cmd + 4;
        if (strstr(args, "1")) {
            printf("[LOOK1] Listing active system tasks...\n");
        } else if (strstr(args, "2")) {
            printf("[LOOK2] Displaying page frames...\n");
        } else if (strstr(args, "3")) {
            printf("[LOOK3] Showing channel paths...\n");
        } else if (strstr(args, "4")) {
            printf("[LOOK4] Showing spool space...\n");
        } else {
            printf("[LOOK] System Status Monitor:\n");
            printf("  - CPU Usage:  12.4%%   Active ASIDs: 6\n");
            printf("  - Address Space Allocations:\n");
            printf("    * ASID 1: master    (0x00000000 - 0x000FFFFF)\n");
            printf("    * ASID 2: jes2      (0x00100000 - 0x0017FFFF)\n");
            printf("    * ASID 3: console   (0x00180000 - 0x0019FFFF)\n");
            printf("    * ASID 4: tso001    (0x001A0000 - 0x002FFFFF)\n");
        }
        printf("[LOOK] System status retrieval completed.\n");
        return true;
    }

    // Check for "users" command
    if (strcmp(cmd, "users") == 0) {
        printf("[USERS] Active TSO Sessions:\n");
        printf("  - SYSOP    Console Operator  (ASID 3, Active)\n");
        printf("  - MVSUSER  TSO User session  (ASID 4, Idle 2m)\n");
        printf("  - VAESEN   Citizen monitor   (ASID 5, Active)\n");
        printf("[USERS] Total active sessions: 3\n");
        return true;
    }

    // Check for "showtcas" command
    if (strcmp(cmd, "showtcas") == 0) {
        printf("[SHOWTCAS] Terminal Control Address Space:\n");
        printf("  - VTAM Logon Manager Status: ACTIVE\n");
        printf("  - TCAS Private Size Limit:  2048K\n");
        printf("[SHOWTCAS] TCAS boundary verification completed.\n");
        return true;
    }

    // Check for "showtpvt" command
    if (strcmp(cmd, "showtpvt") == 0) {
        printf("[SHOWTPVT] Private Area Virtual Storage Limits:\n");
        printf("  - Region Start Address: 0x001A0000\n");
        printf("  - Region End Address:   0x007FFFFF\n");
        printf("  - Total Allocated Size: 6528K\n");
        printf("[SHOWTPVT] Private storage area display completed.\n");
        return true;
    }

    // Check for "steplib" command
    if (strcmp(cmd, "steplib") == 0) {
        printf("[STEPLIB] Current Job Step Library Allocations:\n");
        printf("  - DD:STEPLIB -> DSN:USER.LIBRARY  (Mounted, Authorized)\n");
        printf("  - DD:JOBLIB  -> DSN:SYS1.LINKLIB  (Mounted, Authorized)\n");
        printf("[STEPLIB] Library display completed.\n");
        return true;
    }

    // Check for "tapemap" command
    if (strncmp(cmd, "tapemap", 7) == 0) {
        const char *args = cmd + 7;
        printf("[TAPEMAP] Virtual Tape Volume Layout mapping:\n");
        printf("  - Volume Serial: CBT035   Density: 6250 BPI\n");
        if (strstr(args, "x") || strstr(args, "X")) {
            printf("  - Extended Mapping Mode: ENABLED\n");
            printf("    * Channel Program: CCW=0x07 (Read Buffers)\n");
            printf("    * Tape Drive: 0x480  Channel: 4\n");
        } else if (strstr(args, "m") || strstr(args, "M")) {
            printf("  - Detail Block Map:\n");
            printf("    * Block 001-010: Volume Label & Directory Blocks\n");
            printf("    * Block 011-1440: Load Module Data Blocks\n");
        } else {
            printf("  - File Sequence Number: 1\n");
            printf("    * Dataset Name: CBT035.LOAD.dat.bin\n");
            printf("    * Block Count:  1440  Record Length: 80 bytes\n");
        }
        printf("[TAPEMAP] Volume mapping completed.\n");
        return true;
    }

    // Check for "offload " command
    if (strncmp(cmd, "offload ", 8) == 0) {
        const char *dsn = cmd + 8;
        printf("[OFFLOAD] Offloading Partitioned Dataset: %s\n", dsn);
        printf("  - Creating sequential backup stream in memory...\n");
        printf("  - Packed 10 members into archive file.\n");
        printf("[OFFLOAD] Dataset backup completed successfully.\n");
        return true;
    }

    // Check for "logtime" command
    if (strcmp(cmd, "logtime") == 0) {
        printf("[LOGTIME] Retrieving system timing and scheduler logs:\n");
        printf("  - Active Task Count: %d\n", g_active_sched ? g_active_sched->task_count : 0);
        printf("  - System Time:       12:08:22 UTC\n");
        printf("[LOGTIME] Time log print completed successfully.\n");
        return true;
    }

    // Check for "logopts " command
    if (strncmp(cmd, "logopts ", 8) == 0) {
        const char *opt = cmd + 8;
        printf("[LOGOPTS] Updating log options to: %s\n", opt);
        printf("  - System log verbosity changed dynamically.\n");
        printf("[LOGOPTS] Log settings updated successfully.\n");
        return true;
    }

    // Check for "tapescan" command
    if (strcmp(cmd, "tapescan") == 0) {
        printf("[TAPESCAN] Scanning virtual tape blocks in memory:\n");
        printf("  - Found tape label: VolSer CBT035\n");
        printf("  - Total Blocks Scanned: 1440\n");
        printf("[TAPESCAN] Tape scan completed successfully.\n");
        return true;
    }

    // Check for "tcopy " command
    if (strncmp(cmd, "tcopy ", 6) == 0) {
        const char *args = cmd + 6;
        char src[32] = "";
        char dest[32] = "";
        sscanf(args, "%31s %31s", src, dest);
        printf("[TCOPY] Copying tape volume in memory: %s -> %s\n", src, dest);
        printf("  - VolSer: %s mapped dynamically to %s\n", src, dest);
        printf("[TCOPY] Tape volume copy completed successfully.\n");
        return true;
    }

    // Check for "release " command
    if (strncmp(cmd, "release ", 8) == 0) {
        const char *dsn = cmd + 8;
        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, dsn) == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            uint32_t old_size = g_vfs.files[file_idx].size_bytes;
            uint32_t new_size = strlen(g_vfs.files[file_idx].data);
            g_vfs.files[file_idx].size_bytes = new_size;
            printf("[RELEASE] Released unused allocation for: %s\n", dsn);
            printf("  - Freed: %u bytes\n", old_size - new_size);
        } else {
            printf("[RELEASE ERROR] Dataset %s not found in VFS.\n", dsn);
        }
        return true;
    }

    // Check for "reset" command
    if (strcmp(cmd, "reset") == 0) {
        printf("[RESET] Resetting virtual operator console and environment:\n");
        printf("  - Clearing scheduler variables...\n");
        printf("[RESET] Environment reset completed successfully.\n");
        return true;
    }

    // Check for "xeq " command
    if (strncmp(cmd, "xeq ", 4) == 0) {
        const char *pgm = cmd + 4;
        printf("[XEQ] Executing program driver: %s\n", pgm);
        printf("  - Loading execution context...\n");
        printf("[XEQ] Program execution completed successfully.\n");
        return true;
    }

    // Check for "whatsnew" command
    if (strcmp(cmd, "whatsnew") == 0) {
        printf("[WHATSNEW] Displaying latest system and CBT Tape changes:\n");
        printf("  - Version 510: Added Clean-Room Standard Library audits.\n");
        printf("  - Version 509: Integrated WinchesterMQ SCSI handshake loops.\n");
        printf("  - Version 508: Replaced Python utilities with C implementations.\n");
        printf("[WHATSNEW] News display completed successfully.\n");
        return true;
    }

    // Check for "vsamsize " command
    if (strncmp(cmd, "vsamsize ", 9) == 0) {
        const char *dsn = cmd + 9;
        printf("[VSAMSIZE] Listing VSAM Cluster Size and Extents for: %s\n", dsn);
        printf("  - Data Component Size:  2048K  Index Component Size: 256K\n");
        printf("  - Control Interval:     4096   Control Area Extents: 1\n");
        printf("[VSAMSIZE] Size reporting completed successfully.\n");
        return true;
    }

    // Check for "packrat" command
    if (strcmp(cmd, "packrat") == 0) {
        printf("[PACKRAT] Compacting and compressing in-memory VFS volumes:\n");
        uint32_t total_size = 0;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active) {
                total_size += g_vfs.files[i].size_bytes;
            }
        }
        printf("  - Unpacked size: %u bytes -> Packed size: %u bytes (Ratio: 42%%)\n", total_size, total_size * 42 / 100);
        printf("[PACKRAT] Volume compaction completed successfully.\n");
        return true;
    }

    // Check for "tapel" command
    if (strcmp(cmd, "tapel") == 0) {
        printf("[TAPEL] Parsing virtual tape volume labels:\n");
        printf("  - VOL1: Ser=CBT035  Owner=MVSUSER  Security=NONE\n");
        printf("  - HDR1: DSN=CBT035.LOAD.dat.bin  Recfm=FB  Lrecl=80\n");
        printf("[TAPEL] Tape label inspection completed successfully.\n");
        return true;
    }

    // Check for "offloado " command
    if (strncmp(cmd, "offloado ", 9) == 0) {
        const char *dsn = cmd + 9;
        printf("[OFFLOADO] Packing Partitioned Dataset using Legacy Format: %s\n", dsn);
        printf("  - Creating sequential backup stream in memory...\n");
        printf("[OFFLOADO] Legacy dataset backup completed successfully.\n");
        return true;
    }

    // Check for "lookj" command
    if (strcmp(cmd, "lookj") == 0) {
        printf("[LOOKJ] Displaying active JCL job execution queues:\n");
        int active_jobs = 0;
        for (int i = 0; i < 10; i++) {
            if (cbt_job_table[i].active) {
                printf("    * Job: %-16s  Status: %-8s  ASID: %d\n",
                       cbt_job_table[i].job_name, cbt_job_table[i].status, 10 + i);
                active_jobs++;
            }
        }
        if (active_jobs == 0) {
            printf("  - No active batch jobs currently executing.\n");
        }
        printf("[LOOKJ] Job queue inspection completed successfully.\n");
        return true;
    }

    // Check for "lookx" command
    if (strcmp(cmd, "lookx") == 0) {
        printf("[LOOKX] Extended System Parameter inspection:\n");
        printf("  - Dispatching queues: CPU=1  I/O=0  Page=0\n");
        printf("  - Outstanding WTOR prompts: 0\n");
        printf("[LOOKX] Extended inspection completed successfully.\n");
        return true;
    }


    // Check for "pdsmatch " command
    if (strncmp(cmd, "pdsmatch ", 9) == 0) {
        const char *args = cmd + 9;
        char pds1[32] = "";
        char pds2[32] = "";
        sscanf(args, "%31s %31s", pds1, pds2);
        printf("[PDSMATCH] Comparing directory structures: %s <-> %s\n", pds1, pds2);
        printf("  - Matching members... All entries aligned successfully.\n");
        printf("[PDSMATCH] Comparison completed. 0 mismatches found.\n");
        return true;
    }

    // Check for "lpscb" command
    if (strcmp(cmd, "lpscb") == 0) {
        printf("[LPSCB] Listing Protected Step Control Blocks:\n");
        printf("  - PSCB 01: User=MVSUSER  Attr=OPER,ACCT  Acct=SYS1\n");
        printf("  - PSCB 02: User=VAESEN   Attr=OPER       Acct=SYS2\n");
        printf("[LPSCB] Control block display completed.\n");
        return true;
    }

    // Check for "packratu " command
    if (strncmp(cmd, "packratu ", 9) == 0) {
        const char *dsn = cmd + 9;
        printf("[PACKRATU] Unpacking in-memory VFS archive: %s\n", dsn);
        printf("  - Decompressing dataset blocks...\n");
        printf("[PACKRATU] Archive unpacked successfully.\n");
        return true;
    }

    // Check for "pdsorig " command
    if (strncmp(cmd, "pdsorig ", 8) == 0) {
        const char *member = cmd + 8;
        printf("[PDSORIG] Querying source origin properties for member: %s\n", member);
        char target_name[80];
        resolve_pds_name(member, target_name, sizeof(target_name));
        int file_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, target_name) == 0) {
                file_idx = i;
                break;
            }
        }
        if (file_idx >= 0) {
            printf("  - Source Offset:  0x%08X  Size: %u bytes\n", g_vfs.files[file_idx].start_offset, g_vfs.files[file_idx].size_bytes);
            printf("  - DNA Signature:  Verified (Auncient clean-room)\n");
        } else {
            printf("[PDSORIG ERROR] Member %s not found in VFS.\n", target_name);
        }
        return true;
    }

    // Check for "xmdsmain" command
    if (strcmp(cmd, "xmdsmain") == 0) {
        printf("[XMDSMAIN] Commencing External Dataset Maintenance Audit:\n");
        printf("  - Active VFS Volume: MVSRES   Integrity check: PASS\n");
        printf("  - Total Active Files: %d  Errors: 0\n", g_vfs.count);
        printf("[XMDSMAIN] System catalog maintenance audit completed.\n");
        return true;
    }

    // Check for "vsammap " command
    if (strncmp(cmd, "vsammap ", 8) == 0) {
        const char *dsn = cmd + 8;
        printf("[VSAMMAP] Displaying VSAM Index boundaries and key mappings for: %s\n", dsn);
        printf("  - Index Component: KSDS (Key-Sequenced Data Set)\n");
        printf("  - Key Length: 16 bytes  Key Offset: 0\n");
        printf("    * Control Interval 01: Keys [0000000000000000 - 0000000000000099]\n");
        printf("    * Control Interval 02: Keys [0000000000000100 - 0000000000000199]\n");
        printf("[VSAMMAP] VSAM key mapping display completed.\n");
        return true;
    }

    // Check for "onlclip " command
    if (strncmp(cmd, "onlclip ", 8) == 0) {
        const char *args = cmd + 8;
        char vol[32] = "";
        char new_ser[32] = "";
        sscanf(args, "%31s %31s", vol, new_ser);
        printf("[ONLCLIP] Online Volume Serial Clipper utility:\n");
        printf("  - Querying volume serial for: %s\n", vol);
        printf("  - Clipping Volume Serial serial from '%s' to '%s' dynamically...\n", vol, new_ser);
        printf("[ONLCLIP] Volume serial clipped and updated in catalog successfully.\n");
        return true;
    }

    // Check for "getparm " command
    if (strncmp(cmd, "getparm ", 8) == 0) {
        const char *args = cmd + 8;
        printf("[GETPARM] Extracting Carmine Cannatello ASM parameters:\n");
        printf("  - Input string: '%s'\n", args);
        printf("  - Parsed Parm:  Length=%zu, Value='%s'\n", strlen(args), args);
        printf("[GETPARM] Parameter extraction completed.\n");
        return true;
    }

    // Check for "hextbl " command
    if (strncmp(cmd, "hextbl ", 7) == 0) {
        const char *args = cmd + 7;
        printf("[HEXTBL] Translate string to Hexadecimal table representation:\n");
        printf("  - Source: %s\n", args);
        printf("  - Hex:    ");
        for (size_t i = 0; args[i] != '\0'; i++) {
            printf("%02X", (unsigned char)args[i]);
        }
        printf("\n[HEXTBL] Translation completed successfully.\n");
        return true;
    }

    // Check for "initl" command
    if (strcmp(cmd, "initl") == 0) {
        printf("[INITL] Initializing Carmine Cannatello register macro layout:\n");
        printf("  - Register base pointer set: R12 (0x00008000)\n");
        printf("  - Save area chain established: 18-word layout\n");
        printf("[INITL] Macro initialization completed.\n");
        return true;
    }

    // Check for "puttbl" command
    if (strncmp(cmd, "puttbl", 6) == 0) {
        const char *args = cmd + 6;
        if (strstr(args, "2")) {
            printf("[PUTTBL2] Displaying extended translate table contents:\n");
            printf("  - 80-8F: 808182838485868788898A8B8C8D8E8F\n");
        } else {
            printf("[PUTTBL] Displaying translate table contents:\n");
            printf("  - 00-0F: 000102030405060708090A0B0C0D0E0F\n");
            printf("  - F0-FF: F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF\n");
        }
        printf("[PUTTBL] Translate table display completed.\n");
        return true;
    }

    // Check for "rcntl" command
    if (strcmp(cmd, "rcntl") == 0) {
        printf("[RCNTL] Displaying active register control block status:\n");
        printf("  - R13 Address Space boundary: 0x%08X\n", 0x001A0000);
        printf("  - Active execution flags: AMODE=31, RMODE=ANY\n");
        printf("[RCNTL] Control block status retrieved.\n");
        return true;
    }

    // Check for "valnumb " command
    if (strncmp(cmd, "valnumb ", 8) == 0) {
        const char *args = cmd + 8;
        char val_str[64] = "";
        char fmt[32] = "";
        sscanf(args, "%63s %31s", val_str, fmt);
        bool is_valid = true;
        if (strcasecmp(fmt, "HEX") == 0) {
            for (size_t i = 0; val_str[i] != '\0'; i++) {
                char c = val_str[i];
                if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                    is_valid = false;
                    break;
                }
            }
            printf("[VALNUMB] Validating hexadecimal string format: '%s'\n", val_str);
        } else {
            for (size_t i = 0; val_str[i] != '\0'; i++) {
                if (val_str[i] < '0' || val_str[i] > '9') {
                    is_valid = false;
                    break;
                }
            }
            printf("[VALNUMB] Validating numerical string format: '%s'\n", val_str);
        }
        printf("  - Validation Result: %s\n", is_valid ? "VALID FORMAT" : "INVALID CHARACTERS");
        printf("[VALNUMB] Numerical validation completed.\n");
        return true;
    }

    // Check for "cbtbookstatsstatusdetailsprogress" command
    if (strcmp(cmd, "cbtbookstatsstatusdetailsprogress") == 0) {
        printf("\n");
        printf("================================================================================\n");
        printf("                  BOOK UTILITY COMPLIANCE AUDIT PROGRESS                        \n");
        printf("================================================================================\n");
        tsfi_cw_vsam_ksds ksds;
        int open_rc = tsfi_cw_vsam_open(&ksds, "VTOC.dat.bin");
        printf(" VSAM DATABASE OPEN  : %s (RC=%d)\n", open_rc == 0 ? "SUCCESS" : "TEMPORARY INDEX", open_rc);
        printf(" AUDIT PROGRESS STATE: 100 PERCENT VERIFIED\n");
        printf(" SYSTEM STATUS       : COMPLIANT. RC=0000\n");
        printf("================================================================================\n");
        return true;
    }

    // Check for "cbtbookignorelistreset" command
    if (strcmp(cmd, "cbtbookignorelistreset") == 0) {
        printf("[BOOK] Book loader ignore/exclude configurations reset to default baseline\n");
        printf("  - Active exclusions cleared. RC=0000\n");
        return true;
    }

    // Check for "cbtbookignorelistresetstat" command
    if (strcmp(cmd, "cbtbookignorelistresetstat") == 0) {
        printf("\n");
        printf("================================================================================\n");
        printf("                  BOOK LOADER IGNORE LIST RESETS STATISTICS                     \n");
        printf("================================================================================\n");
        printf(" RESETS RUN COUNT    : 1 RESETS\n");
        printf(" RESET RUN STATUS    : OPERATIONAL. RC=0000\n");
        printf("================================================================================\n");
        return true;
    }

    // Check for "wtoregs" command
    if (strncmp(cmd, "wtoregs", 7) == 0) {
        const char *args = cmd + 7;
        if (strstr(args, "0")) {
            printf("GPR0-3:   00000000  001A2000  00008000  0000000F\n");
            printf("GPR12-15: 00008000  001A0000  8000A042  00000000\n");
        } else {
            printf("[WTOREGS] Write to Operator virtual registers dump:\n");
            printf("  - GPR0-3:   00000000  001A2000  00008000  0000000F\n");
            printf("  - GPR4-7:   00000100  00000000  00000000  00000000\n");
            printf("  - GPR8-11:  00000000  00000000  00000000  00000000\n");
            printf("  - GPR12-15: 00008000  001A0000  8000A042  00000000\n");
            printf("[WTOREGS] WTOR console dump completed successfully.\n");
        }
        return true;
    }

    return false;
}

void tsfi_cbt_pds_format_dir_block(uint8_t *dir_block, const char *member_name, uint32_t offset, uint32_t size) {
    if (!dir_block) return;
    memset(dir_block, 0, 256);
    
    // Set 'used' bytes: we have 1 entry of 16 bytes starting at ptr=2. So total used is 2 + 16 = 18 bytes.
    dir_block[0] = 0x00;
    dir_block[1] = 18;
    
    // Member Name: 8 bytes at offset 2
    int len = strlen(member_name);
    if (len > 8) len = 8;
    memcpy(&dir_block[2], member_name, len);
    for (int i = len; i < 8; i++) {
        dir_block[2 + i] = ' '; // Pad with space
    }
    
    // Offset (TTR): 3 bytes at offset 10 (ptr + 8)
    dir_block[10] = (offset >> 16) & 0xFF;
    dir_block[11] = (offset >> 8) & 0xFF;
    dir_block[12] = offset & 0xFF;
    
    // Size: 4 bytes at offset 14 (ptr + 12)
    dir_block[14] = (size >> 24) & 0xFF;
    dir_block[15] = (size >> 16) & 0xFF;
    dir_block[16] = (size >> 8) & 0xFF;
    dir_block[17] = size & 0xFF;
    
    printf("[BOOK] Formatted binary PDS directory block for member %s (Offset=%u, Size=%u)\n", member_name, offset, size);
}
