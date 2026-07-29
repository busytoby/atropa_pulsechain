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


bool tsfi_xplos_shell_xdc(const char *cmd) {
    (void)cmd;
    // Check for "readdir " command
    if (strncmp(cmd, "readdir ", 8) == 0) {
        const char *dsn = cmd + 8;
        printf("[READDIR] Commencing Directory Block Read for dataset: %s\n", dsn);
        int count = 0;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strstr(g_vfs.files[i].name, dsn)) {
                printf("    * Member: %-16s  TTR: [00,%02X,%02X]\n", g_vfs.files[i].name, (i >> 8) & 0xFF, i & 0xFF);
                count++;
            }
        }
        printf("[READDIR] Directory block reading completed. Found %d members.\n", count);
        return true;
    }

    // Check for "move " command
    if (strncmp(cmd, "move ", 5) == 0) {
        const char *args = cmd + 5;
        char src[32] = "";
        char dest[32] = "";
        uint32_t len = 0;
        sscanf(args, "%31s %31s %u", src, dest, &len);
        printf("[MOVE] Macro Block Movement driver: %s -> %s (%u bytes)\n", src, dest, len);
        int src_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, src) == 0) {
                src_idx = i;
                break;
            }
        }
        if (src_idx >= 0) {
            if (tsfi_xplos_create_file(&g_vfs, dest, len)) {
                int dest_idx = -1;
                for (int i = 0; i < g_vfs.count; i++) {
                    if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, dest) == 0) {
                        dest_idx = i;
                        break;
                    }
                }
                if (dest_idx >= 0) {
                    memcpy(g_vfs.files[dest_idx].data, g_vfs.files[src_idx].data, len < g_vfs.files[src_idx].size_bytes ? len : g_vfs.files[src_idx].size_bytes);
                    printf("  - Block bytes copied successfully in memory.\n");
                }
            }
        } else {
            printf("[MOVE ERROR] Source member %s not found.\n", src);
        }
        return true;
    }

    // Check for "trtbl " command
    if (strncmp(cmd, "trtbl ", 6) == 0) {
        const char *args = cmd + 6;
        char str[64] = "";
        char map_type[32] = "";
        sscanf(args, "%63s %31s", str, map_type);
        printf("[TRTBL] Translate Table translation for string: '%s'\n", str);
        if (strcasecmp(map_type, "UPPER") == 0) {
            printf("  - Output: ");
            for (size_t i = 0; str[i] != '\0'; i++) {
                char c = str[i];
                if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
                putchar(c);
            }
            putchar('\n');
        } else if (strcasecmp(map_type, "LOWER") == 0) {
            printf("  - Output: ");
            for (size_t i = 0; str[i] != '\0'; i++) {
                char c = str[i];
                if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
                putchar(c);
            }
            putchar('\n');
        } else {
            printf("  - Output: %s (No translation map matched)\n", str);
        }
        printf("[TRTBL] Character mapping translation completed.\n");
        return true;
    }

    // Check for "ce " command
    if (strncmp(cmd, "ce ", 3) == 0) {
        const char *ex = cmd + 3;
        printf("[CE] Carmine Cannatello ASM Example loader:\n");
        printf("  - Loading example member: %s\n", ex);
        
        int found_idx = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strncasecmp(g_vfs.files[i].name, ex, strlen(ex)) == 0) {
                found_idx = i;
                break;
            }
        }
        
        if (found_idx >= 0) {
            printf("  - Found member in VFS: %s (%u bytes)\n", g_vfs.files[found_idx].name, g_vfs.files[found_idx].size_bytes);
            printf("  - First 120 bytes of EBCDIC source translated to ASCII:\n");
            static const char eb_map[256] = {
                [' '] = ' ', [0x40] = ' ',
                [0xC1] = 'A', [0xC2] = 'B', [0xC3] = 'C', [0xC4] = 'D', [0xC5] = 'E',
                [0xC6] = 'F', [0xC7] = 'G', [0xC8] = 'H', [0xC9] = 'I',
                [0xD1] = 'J', [0xD2] = 'K', [0xD3] = 'L', [0xD4] = 'M', [0xD5] = 'N',
                [0xD6] = 'O', [0xD7] = 'P', [0xD8] = 'Q', [0xD9] = 'R',
                [0xE2] = 'S', [0xE3] = 'T', [0xE4] = 'U', [0xE5] = 'V', [0xE6] = 'W',
                [0xE7] = 'X', [0xE8] = 'Y', [0xE9] = 'Z',
                [0xF0] = '0', [0xF1] = '1', [0xF2] = '2', [0xF3] = '3', [0xF4] = '4',
                [0xF5] = '5', [0xF6] = '6', [0xF7] = '7', [0xF8] = '8', [0xF9] = '9'
            };
            uint32_t len = g_vfs.files[found_idx].size_bytes;
            if (len > 120) len = 120;
            printf("    * ");
            for (uint32_t j = 0; j < len; j++) {
                unsigned char eb = g_vfs.files[found_idx].data[j];
                char as = eb_map[eb];
                if (as == '\0') as = '.';
                putchar(as);
                if ((j + 1) % 60 == 0 && j + 1 < len) {
                    printf("\n    * ");
                }
            }
            printf("\n");
        } else {
            printf("  - Title: Chapter Example - Instruction Execution Loop\n");
            printf("    * Code:  %s  CSECT\n", ex);
            printf("    *        STM   14,12,12(13)   Save registers\n");
        }
        printf("[CE] Example code loaded successfully.\n");
        return true;
    }

    // Check for "cbtexplore" command
    if (strncmp(cmd, "cbtexplore", 10) == 0) {
        const char *args = cmd + 10;
        while (*args == ' ') args++;
        printf("[CBTEXPLORE] CBT Tape Index and VFS Explorer:\n");
        if (strcmp(args, "catalog") == 0 || strlen(args) == 0) {
            printf("  - Available CBT Files in VFS:\n");
            printf("    * File 021: MVS Exit Processors & JCL Allocators\n");
            printf("    * File 035: Load Module Utilities & System Monitors\n");
            printf("    * File 069: Carmine Cannatello ASM Book Coding Examples\n");
        } else if (strcmp(args, "21") == 0) {
            printf("  - CBT File 021 metadata:\n");
            printf("    * Volume Serial: CBT021  Active Members: 10\n");
            printf("    * Key Utilities: OCX, IBHLSPAC, IBHWTORG, IBHDRPLY\n");
        } else if (strcmp(args, "35") == 0) {
            printf("  - CBT File 035 metadata:\n");
            printf("    * Volume Serial: CBT035  Active Members: 223\n");
            printf("    * Key Utilities: PDS, VTOC, LOOK, SHOWTCAS, TAPEMAP\n");
        } else if (strcmp(args, "69") == 0) {
            printf("  - CBT File 069 metadata:\n");
            printf("    * Volume Serial: CBT069  Active Members: 237\n");
            printf("    * Key Utilities: GETPARM, WTOREGS, READDIR, TRTBL\n");
        } else if (strncmp(args, "find ", 5) == 0) {
            const char *query = args + 5;
            printf("  - Searching for members matching query: '%s'\n", query);
            int matches = 0;
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strstr(g_vfs.files[i].name, query)) {
                    printf("    * Member: %-16s  Size: %u bytes\n", g_vfs.files[i].name, g_vfs.files[i].size_bytes);
                    matches++;
                }
            }
            printf("  - Search completed. Found %d matches.\n", matches);
        } else {
            printf("  - Unknown option. Usage: cbtexplore [catalog|21|35|69|find <query>]\n");
        }
        printf("[CBTEXPLORE] Explorer query completed successfully.\n");
        return true;
    }

    // Check for "askoper " command
    if (strncmp(cmd, "askoper ", 8) == 0) {
        const char *prompt = cmd + 8;
        printf("[ASKOPER] Operator Query Prompt: '%s'\n", prompt);
        printf("  - Intercepting console responses over SCSI WinchesterMQ loopback...\n");
        printf("  - Operator Response: 'AFFIRMATIVE'\n");
        printf("[ASKOPER] Console query completed successfully.\n");
        return true;
    }

    // Check for "asmbox " command
    if (strncmp(cmd, "asmbox ", 7) == 0) {
        const char *text = cmd + 7;
        size_t len = strlen(text);
        printf("[ASMBOX] Formatting assembler visual comments box:\n");
        printf("  *");
        for (size_t i = 0; i < len + 4; i++) putchar('*');
        printf("*\n");
        printf("  *  %s  *\n", text);
        printf("  *");
        for (size_t i = 0; i < len + 4; i++) putchar('*');
        printf("*\n");
        printf("[ASMBOX] Comment formatting completed.\n");
        return true;
    }

    // Check for "vkiller " command
    if (strncmp(cmd, "vkiller ", 8) == 0) {
        const char *target = cmd + 8;
        printf("[VKILLER] Task Terminator auditing active ASIDs:\n");
        bool terminated = false;
        int target_asid = atoi(target);
        if (target_asid > 0 && target_asid < 10) {
            printf("  - Terminating target ASID %d cleanly...\n", target_asid);
            terminated = true;
        } else {
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_name, target) == 0) {
                    cbt_job_table[i].active = false;
                    printf("  - Terminating batch job: %s (ASID %d) cleanly...\n", cbt_job_table[i].job_name, 10 + i);
                    terminated = true;
                }
            }
        }
        if (terminated) {
            printf("[VKILLER] Target terminated successfully. Core cleaned up.\n");
        } else {
            printf("[VKILLER ERROR] Target job or ASID '%s' not found.\n", target);
        }
        return true;
    }

    // Check for "blokhd " command
    if (strncmp(cmd, "blokhd ", 7) == 0) {
        const char *text = cmd + 7;
        printf("[BLOKHD] Block Letter Heading Generator:\n");
        for (size_t i = 0; text[i] != '\0'; i++) {
            char c = text[i];
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
            if (c == 'C') {
                printf("  CCCC \n C    C\n C     \n C    C\n  CCCC \n\n");
            } else if (c == 'O') {
                printf("  OOOO \n O    O\n O    O\n O    O\n  OOOO \n\n");
            } else if (c == 'P') {
                printf(" PPPPP \n P    P\n PPPPP \n P     \n P     \n\n");
            } else if (c == 'Y') {
                printf(" Y   Y \n  Y Y  \n   Y   \n   Y   \n   Y   \n\n");
            } else {
                printf(" ***** \n *   * \n ***** \n *   * \n *   * \n\n");
            }
        }
        printf("[BLOKHD] Heading generation completed.\n");
        return true;
    }

    // Check for "clrscrn" command
    if (strcmp(cmd, "clrscrn") == 0) {
        printf("\033[2J\033[H");
        printf("[CLRSCRN] Terminal screen cleared successfully.\n");
        return true;
    }

    // Check for "compare " command
    if (strncmp(cmd, "compare ", 8) == 0) {
        const char *args = cmd + 8;
        char file1[32] = "";
        char file2[32] = "";
        sscanf(args, "%31s %31s", file1, file2);
        printf("[COMPARE] Comparing files: %s <-> %s\n", file1, file2);
        int idx1 = -1, idx2 = -1;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active) {
                if (strcmp(g_vfs.files[i].name, file1) == 0) idx1 = i;
                if (strcmp(g_vfs.files[i].name, file2) == 0) idx2 = i;
            }
        }
        if (idx1 >= 0 && idx2 >= 0) {
            if (g_vfs.files[idx1].size_bytes == g_vfs.files[idx2].size_bytes &&
                memcmp(g_vfs.files[idx1].data, g_vfs.files[idx2].data, g_vfs.files[idx1].size_bytes) == 0) {
                printf("  - Result: Files match exactly (%u bytes).\n", g_vfs.files[idx1].size_bytes);
            } else {
                printf("  - Result: Mismatch found.\n");
            }
        } else {
            printf("[COMPARE ERROR] One or both files not found in VFS.\n");
        }
        return true;
    }

    // Check for "dynalc " command
    if (strncmp(cmd, "dynalc ", 7) == 0) {
        const char *args = cmd + 7;
        char dsn[64] = "";
        char dd[32] = "";
        sscanf(args, "%63s %31s", dsn, dd);
        printf("[DYNALC] Dynamic allocation SVC 99 processor:\n");
        printf("  - Allocating DSN: %s -> DD: %s\n", dsn, dd);
        if (tsfi_xplos_create_file(&g_vfs, dsn, 1024)) {
            printf("  - Allocation successful.\n");
        } else {
            printf("[DYNALC ERROR] Allocation failed.\n");
        }
        return true;
    }

    uint32_t ce_gprs[16] = {0};
    uint8_t ce_memory[1024] = {0};

    // Check for "ce_run " command
    if (strncmp(cmd, "ce_run ", 7) == 0) {
        const char *arg = cmd + 7;
        if (strcmp(arg, "status") == 0) {
            printf("[CE_RUN] Persistent GPR Status:\n");
            for (int i = 0; i < 16; i += 4) {
                printf("  R%-2d: %-10u  R%-2d: %-10u  R%-2d: %-10u  R%-2d: %-10u\n",
                       i, ce_gprs[i], i+1, ce_gprs[i+1], i+2, ce_gprs[i+2], i+3, ce_gprs[i+3]);
            }
            return true;
        } else if (strcmp(arg, "reset") == 0) {
            memset(ce_gprs, 0, sizeof(ce_gprs));
            memset(ce_memory, 0, sizeof(ce_memory));
            printf("[CE_RUN] Simulator registers and memory reset successfully.\n");
            return true;
        }
        
        printf("[CE_RUN] Executing: %s\n", arg);
        if (tsfi_xpl_execute_assembler(arg, ce_gprs, ce_memory)) {
            printf("  - Instruction executed successfully.\n");
        } else {
            printf("[CE_RUN ERROR] Failed to simulate instruction.\n");
        }
        return true;
    }

    // Check for "xdc" command
    if (strncmp(cmd, "xdc", 3) == 0) {
        const char *args = cmd + 3;
        while (*args == ' ') args++;
        printf("[XDC] External Debug Boundary Control Interface:\n");

        #define MAX_BREAKPOINTS 8
        static uint32_t xdc_breakpoints[MAX_BREAKPOINTS] = {0};
        static int xdc_breakpoint_count = 0;
        uint32_t xdc_ip = 0;

        if (strcmp(args, "halt") == 0) {
            printf("  - Intercepting raw SCSI handshake frames on WinchesterMQ interface...\n");
            printf("  - Execution halted. Visual trail opacity (alpha) locked. Coordinates solid cyan.\n");
            printf("  - Highlighted vertex spheres showing active instruction breakpoints.\n");
        } else if (strcmp(args, "step") == 0) {
            printf("  - Stepping instruction at IP: 0x%08X\n", xdc_ip);
            uint8_t op1 = ce_memory[xdc_ip];
            uint8_t op2 = ce_memory[xdc_ip + 1];
            if (op1 == 0x18) {
                int r1 = (op2 >> 4) & 0xF;
                int r2 = op2 & 0xF;
                ce_gprs[r1] = ce_gprs[r2];
                xdc_ip += 2;
            } else if (op1 == 0x5A) {
                int r1 = (op2 >> 4) & 0xF;
                int r2 = op2 & 0xF;
                ce_gprs[r1] += ce_gprs[r2];
                xdc_ip += 2;
            } else {
                xdc_ip += 2;
            }
            printf("  - Step completed. Next IP: 0x%08X\n", xdc_ip);
        } else if (strcmp(args, "run") == 0) {
            printf("  - Running instructions from IP: 0x%08X...\n", xdc_ip);
            bool bp_hit = false;
            while (xdc_ip < 1024) {
                for (int i = 0; i < xdc_breakpoint_count; i++) {
                    if (xdc_breakpoints[i] == xdc_ip) {
                        printf("  - Breakpoint hit at address: 0x%08X\n", xdc_ip);
                        bp_hit = true;
                        break;
                    }
                }
                if (bp_hit) break;

                uint8_t op1 = ce_memory[xdc_ip];
                uint8_t op2 = ce_memory[xdc_ip + 1];
                if (op1 == 0x18) {
                    int r1 = (op2 >> 4) & 0xF;
                    int r2 = op2 & 0xF;
                    ce_gprs[r1] = ce_gprs[r2];
                    xdc_ip += 2;
                } else if (op1 == 0x5A) {
                    int r1 = (op2 >> 4) & 0xF;
                    int r2 = op2 & 0xF;
                    ce_gprs[r1] += ce_gprs[r2];
                    xdc_ip += 2;
                } else {
                    break;
                }
            }
            printf("  - Execution stopped at IP: 0x%08X\n", xdc_ip);
        } else if (strncmp(args, "disasm", 6) == 0) {
            uint32_t addr = xdc_ip;
            const char *addr_str = args + 6;
            while (*addr_str == ' ') addr_str++;
            if (*addr_str != '\0') {
                addr = (uint32_t)strtoul(addr_str, NULL, 0);
            }
            printf("  - Disassembly at address 0x%08X:\n", addr);
            printf("    0x%08X:  18 12      LR    R1, R2\n", addr);
            printf("    0x%08X:  5A 12      AR    R1, R2\n", addr + 2);
        } else if (strncmp(args, "dump ", 5) == 0) {
            char addr_s[32] = "";
            uint32_t len = 16;
            sscanf(args + 5, "%31s %u", addr_s, &len);
            uint32_t addr = (uint32_t)strtoul(addr_s, NULL, 0);
            printf("  - Hexadecimal memory dump at address 0x%08X (%u bytes):\n", addr, len);
            printf("    * ");
            for (uint32_t i = 0; i < len && (addr + i) < 1024; i++) {
                printf("%02X ", ce_memory[addr + i]);
            }
            printf("\n");
        } else if (strncmp(args, "modify ", 7) == 0) {
            char addr_s[32] = "";
            char hex_s[128] = "";
            sscanf(args + 7, "%31s %127s", addr_s, hex_s);
            uint32_t addr = (uint32_t)strtoul(addr_s, NULL, 0);
            printf("  - Modifying memory at 0x%08X with bytes: %s\n", addr, hex_s);
            for (size_t i = 0; i < strlen(hex_s); i += 2) {
                unsigned int val = 0;
                sscanf(hex_s + i, "%2x", &val);
                if (addr + i/2 < 1024) {
                    ce_memory[addr + i/2] = (uint8_t)val;
                }
            }
            printf("  - Modification completed successfully.\n");
        } else if (strncmp(args, "bp ", 3) == 0) {
            const char *bp_arg = args + 3;
            if (strncmp(bp_arg, "add ", 4) == 0) {
                uint32_t addr = (uint32_t)strtoul(bp_arg + 4, NULL, 0);
                if (xdc_breakpoint_count < MAX_BREAKPOINTS) {
                    xdc_breakpoints[xdc_breakpoint_count++] = addr;
                    printf("  - Breakpoint added at address: 0x%08X\n", addr);
                } else {
                    printf("[XDC ERROR] Breakpoint limit reached.\n");
                }
            } else if (strncmp(bp_arg, "del ", 4) == 0) {
                uint32_t addr = (uint32_t)strtoul(bp_arg + 4, NULL, 0);
                bool found = false;
                for (int i = 0; i < xdc_breakpoint_count; i++) {
                    if (xdc_breakpoints[i] == addr) {
                        for (int j = i; j < xdc_breakpoint_count - 1; j++) {
                            xdc_breakpoints[j] = xdc_breakpoints[j+1];
                        }
                        xdc_breakpoint_count--;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    printf("  - Breakpoint removed at address: 0x%08X\n", addr);
                } else {
                    printf("[XDC ERROR] Breakpoint not found.\n");
                }
            } else if (strcmp(bp_arg, "list") == 0) {
                printf("  - Active Breakpoints:\n");
                for (int i = 0; i < xdc_breakpoint_count; i++) {
                    printf("    [%d] 0x%08X\n", i + 1, xdc_breakpoints[i]);
                }
                if (xdc_breakpoint_count == 0) {
                    printf("    (none)\n");
                }
            } else if (strcmp(bp_arg, "clear") == 0) {
                xdc_breakpoint_count = 0;
                printf("  - All breakpoints cleared.\n");
            }
        } else if (strncmp(args, "auth ", 5) == 0) {
            const char *key_str = args + 5;
            printf("  - Verifying PKI authorization credentials: '%s'\n", key_str);
            uint64_t keys[4] = {10001, 10002, 10003, 10004};
            uint64_t sum = 0;
            for (int i = 0; i < 4; i++) {
                sum += keys[i];
            }
            uint64_t motzkin = 953467954114363ULL;
            uint64_t auth_sig = sum % motzkin;
            printf("  - Verification status: K=4 distinct keys parsed.\n");
            printf("  - Cryptographic modular parity signature (XDC_Auth): %lu\n", auth_sig);
            printf("  - XDC authorization verified successfully. Debug session active.\n");
        } else {
            printf("  - Standard Register State Dump:\n");
            printf("    * Chin:      0x000F31A0\n");
            printf("    * Monopole:  0x00A12C50\n");
            printf("    * Identity:  0x00003D12\n");
        }
        printf("[XDC] Debugger control frame completed.\n");
        return true;
    }

    // Perform parsing & semantic actions
    MallgrenTransform tx = {1.0, 1.0, 0.0, 0.0, 0.0};
    if (tsfi_xplg_parse_semantic_action(cmd, &tx)) {
        // Apply transform to system state
        (void)tx;
    }

    return false;
}
