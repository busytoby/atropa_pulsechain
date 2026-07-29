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
#include "tsfi_xplos_shell_cbt_vtam.h"
extern XplosVirtualDisk g_vfs;
extern CbtSpoolJob cbt_job_table[10];
extern XplosScheduler *g_active_sched;
static void resolve_pds_name_helper(const char *member, char *out, size_t max_len) {
    snprintf(out, max_len, "%s.dat.bin", member);
}

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
static bool handle_vtamstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  VTAM LOGICAL PATHWAY NETWORK STATISTICS                       \n");
    printf("================================================================================\n");
    printf("  - Active Session Routes: TSO, CICS, USENET\n");
    printf("  - Inbound Buffer Frames: 1042 frames\n");
    printf("  - Outbound Buffer Frames: 876 frames\n");
    printf("  - Path Route Stability: 100%% (STABLE)\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_vtamroute(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  **AUNCIENT** VTAM PATH TUNNEL ROUTING DIAGNOSTICS\n");
    printf("================================================================================\n");
    printf(" CDRM PATH ROUTE STATUS:\n");
    printf("   - HOST: XPL1   -> TARGET: CDRM_LOCAL (STABLE)\n");
    printf("   - PATH INDEX: 04 (PRIMARY) via LU 1\n");
    printf("   - TRANSMISSION STABILITY: 100%%\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_vtambuf(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  VTAM NETWORK BUFFER FRAME TRACE DIAGNOSTICS                   \n");
    printf("================================================================================\n");
    printf(" FRAME ID | DIR | LEN  | SNA HEADER PAYLOAD PREVIEW\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" FRM00241 | IN  |  256 | FID2 [DAAF=02, OAAF=01, SN=1042] COMMAND_DATA\n");
    printf(" FRM00242 | OUT |  512 | FID2 [DAAF=01, OAAF=02, SN=0876] RESPONSE_ACK\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_vtamtrace(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  **AUNCIENT** VTAM PATH ROUTING PATHWAY TRACE\n");
    printf("================================================================================\n");
    printf(" CDRM PATH ROUTE HOP TRACE FOR HOST: XPL1 -> TARGET: CDRM_REMOTE\n");
    printf("   1. [FID2 PATH] DAAF=02 via LU 1 (0.142 ms)\n");
    printf("   2. [FID2 PATH] DAAF=03 via CDRM Gateway Node (0.284 ms) [STABLE]\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_vtamrefr(void) {
    printf("[VTAM] Solicit Logon Panel solicit inputs cleared.\n");
    printf("[VTAM] Redrawing solicit screen buffer and resetting active APPLID flags. RC=0000\n");
    return true;
}

static bool handle_vtamtracebuf(const char *cmd) {
    char state[16] = "";
    if (sscanf(cmd + 13, "%15s", state) < 1) {
        printf("[VTAMTRACEBUF ERROR] Syntax: vtamtracebuf [on|off]\n");
        return true;
    }
    printf("[VTAM] Real-time buffer trace tracking set to: %s\n", state);
    printf("  - Trace hooks active on FID2 routing blocks. RC=0000\n");
    return true;
}

static bool handle_vtambufdet(const char *cmd) {
    char frame_id[32] = "";
    if (sscanf(cmd + 11, "%31s", frame_id) < 1) {
        printf("[VTAMBUFDET ERROR] Syntax: vtambufdet <frame_id>\n");
        return true;
    }
    printf("\n");
    printf("================================================================================\n");
    printf("                  VTAM BUFFER HEXADECIMAL DETAILS TRACE: %s                    \n", frame_id);
    printf("================================================================================\n");
    printf(" OFFSET   | HEXADECIMAL REPRESENTATION                  | EBCDIC CHAR PREVIEW\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" 00000000 | F2 00 02 01 04 12 00 00 40 40 40 40 40 40   | .............     \n");
    printf(" 00000010 | C3 C9 C3 E2 40 C5 D5 E3 D9 E8 F2 F4 F1 F0   | CICS ENTRY2410. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_vtamussstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  **AUNCIENT** VTAM USS LOGON SOLICITS SESSION MONITOR         \n");
    printf("================================================================================\n");
    printf(" TERMINAL ID | LOGON APPLID | CONNECT STATUS | LOGON ATTEMPTS | SESSION TYPE\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" LUN00001    | TSO          | CONNECTED      |              1 | TSO LOGON\n");
    printf(" LUN00002    | CICS         | CONNECTED      |              1 | CICS ENTRY. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_vtamussfmt(const char *cmd) {
    char format[16] = "";
    if (sscanf(cmd + 11, "%15s", format) < 1) {
        printf("[VTAMUSSFMT ERROR] Syntax: vtamussfmt <format>\n");
        return true;
    }
    printf("[VTAM] USS solicits logon panel display format changed to: %s\n", format);
    printf("  - Active USS message template updated successfully. RC=0000\n");
    return true;
}

static bool handle_vtamussdelay(const char *cmd) {
    int delay_ms = 0;
    if (sscanf(cmd + 13, "%d", &delay_ms) < 1) {
        printf("[VTAMUSSDELAY ERROR] Syntax: vtamussdelay <delay_ms>\n");
        return true;
    }
    printf("[VTAM] Solicit Logon Panel logical path delay set to: %d ms\n", delay_ms);
    printf("  - Synthetic delay applied to network solicit buffers. RC=0000\n");
    return true;
}

bool tsfi_xplos_shell_cbt_vtam(const char *cmd) {
    if (strncmp(cmd, "cbtnet ", 7) == 0) return handle_cbtnet(cmd + 7);
    if (strncmp(cmd, "logon ", 6) == 0) return handle_vtam_logon(cmd);
    if (strncmp(cmd, "vtam3270 ", 9) == 0) return handle_vtam3270(cmd);
    if (strcmp(cmd, "vtamuss") == 0) return handle_vtamuss();
    if (strcmp(cmd, "vtamstat") == 0) return handle_vtamstat();
    if (strcmp(cmd, "vtamroute") == 0) return handle_vtamroute();
    if (strcmp(cmd, "vtambuf") == 0) return handle_vtambuf();
    if (strcmp(cmd, "vtamtrace") == 0) return handle_vtamtrace();
    if (strcmp(cmd, "vtamrefr") == 0) return handle_vtamrefr();
    if (strncmp(cmd, "vtamtracebuf ", 13) == 0) return handle_vtamtracebuf(cmd);
    if (strncmp(cmd, "vtambufdet ", 11) == 0) return handle_vtambufdet(cmd);
    if (strcmp(cmd, "vtamussstat") == 0) return handle_vtamussstat();
    if (strncmp(cmd, "vtamussfmt ", 11) == 0) return handle_vtamussfmt(cmd);
    if (strncmp(cmd, "vtamussdelay ", 13) == 0) return handle_vtamussdelay(cmd);
    return false;
}
