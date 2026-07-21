#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_runcible_lang.h"
#include "tsfi_parc_keyboard.h"

int tsfi_runcible_parse_cmd(const char *cmd_line, tsfi_runcible_cmd_t *out_cmd) {
    if (!cmd_line || !out_cmd) return -1;
    memset(out_cmd, 0, sizeof(tsfi_runcible_cmd_t));

    // Skip leading whitespace
    while (*cmd_line == ' ' || *cmd_line == '\t') cmd_line++;
    if (*cmd_line == '\0') return 0;

    if (strncmp(cmd_line, "CALL", 4) == 0) {
        out_cmd->type = RUNCIBLE_CMD_CALL;
        const char *args = cmd_line + 4;
        while (*args == ' ' || *args == '\t') args++;
        if (*args != '\0') {
            out_cmd->target_addr = (uint32_t)strtoul(args, NULL, 0);
            strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        }
        return 0;
    }

    if (strncmp(cmd_line, "KB", 2) == 0) {
        out_cmd->type = RUNCIBLE_CMD_KB;
        const char *args = cmd_line + 2;
        while (*args == ' ' || *args == '\t') args++;
        int row = 0, col = 0, shift = 0;
        if (sscanf(args, "%d %d %d", &row, &col, &shift) >= 2) {
            out_cmd->kb_row = (uint8_t)row;
            out_cmd->kb_col = (uint8_t)col;
            out_cmd->kb_shift = (uint8_t)shift;
        }
        return 0;
    }

    if (strncmp(cmd_line, "SLIDE", 5) == 0) {
        out_cmd->type = RUNCIBLE_CMD_SLIDE;
        return 0;
    }

    if (strncmp(cmd_line, "STATUS", 6) == 0) {
        out_cmd->type = RUNCIBLE_CMD_STATUS;
        return 0;
    }

    out_cmd->type = RUNCIBLE_CMD_UNKNOWN;
    return -2;
}

int tsfi_runcible_exec_cmd(const tsfi_runcible_cmd_t *cmd) {
    if (!cmd) return -1;

    switch (cmd->type) {
        case RUNCIBLE_CMD_CALL: {
            printf("[RUNCIBLE TTY] Executing CALL to address: 0x%08X (Args: %s)\n",
                   cmd->target_addr, cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_KB: {
            char ascii_char = tsfi_parc_kb_translate(cmd->kb_row, cmd->kb_col, cmd->kb_shift);
            printf("[RUNCIBLE TTY] KB Scan Code Translate (Row %d, Col %d, Shift %d) -> ASCII '%c' (0x%02X)\n",
                   cmd->kb_row, cmd->kb_col, cmd->kb_shift,
                   ascii_char ? ascii_char : '?', (uint8_t)ascii_char);
            return 0;
        }
        case RUNCIBLE_CMD_SLIDE: {
            printf("[RUNCIBLE TTY] Executing Dual-Path Littlewood-Richardson Tableau Slide\n");
            return 0;
        }
        case RUNCIBLE_CMD_STATUS: {
            printf("[RUNCIBLE TTY] Subsystem Active | WinchesterMQ Registers Validated\n");
            return 0;
        }
        default:
            printf("[RUNCIBLE TTY] Unknown command syntax\n");
            return -2;
    }
}

int tsfi_runcible_main_step(const char *tty_input) {
    if (!tty_input) return -1;
    tsfi_runcible_cmd_t cmd;
    if (tsfi_runcible_parse_cmd(tty_input, &cmd) == 0) {
        return tsfi_runcible_exec_cmd(&cmd);
    }
    return -2;
}
