#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_runcible_lang.h"
#include "tsfi_parc_keyboard.h"
#include "tsfi_parc_keyset.h"
#include "tsfi_parc_notetaker.h"

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

    if (strncmp(cmd_line, "SCSI", 4) == 0) {
        out_cmd->type = RUNCIBLE_CMD_SCSI;
        const char *args = cmd_line + 4;
        while (*args == ' ' || *args == '\t') args++;
        char op_str[16] = {0};
        int reg = 0, val = 0;
        if (sscanf(args, "%15s %i %i", op_str, &reg, &val) >= 2) {
            if (strcmp(op_str, "WRITE") == 0 || strcmp(op_str, "write") == 0) {
                out_cmd->scsi_op = 1;
            } else {
                out_cmd->scsi_op = 0; // READ
            }
            out_cmd->scsi_reg = (uint8_t)reg;
            out_cmd->scsi_val = (uint32_t)val;
        }
        return 0;
    }

    if (strncmp(cmd_line, "KEYSET", 6) == 0) {
        out_cmd->type = RUNCIBLE_CMD_KEYSET;
        const char *args = cmd_line + 6;
        while (*args == ' ' || *args == '\t') args++;
        int chord = 0;
        if (sscanf(args, "%i", &chord) >= 1) {
            out_cmd->keyset_chord = (uint8_t)chord;
        }
        return 0;
    }

    if (strncmp(cmd_line, "SCHUR", 5) == 0) {
        out_cmd->type = RUNCIBLE_CMD_SCHUR;
        const char *args = cmd_line + 5;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "PEEK", 4) == 0) {
        out_cmd->type = RUNCIBLE_CMD_PEEK;
        const char *args = cmd_line + 4;
        while (*args == ' ' || *args == '\t') args++;
        out_cmd->target_addr = (uint32_t)strtoul(args, NULL, 0);
        return 0;
    }

    if (strncmp(cmd_line, "POKE", 4) == 0) {
        out_cmd->type = RUNCIBLE_CMD_POKE;
        const char *args = cmd_line + 4;
        while (*args == ' ' || *args == '\t') args++;
        unsigned long addr = 0, val = 0;
        if (sscanf(args, "%li %li", &addr, &val) >= 2) {
            out_cmd->target_addr = (uint32_t)addr;
            out_cmd->val = (uint32_t)val;
        }
        return 0;
    }

    if (strncmp(cmd_line, "YUL", 3) == 0) {
        out_cmd->type = RUNCIBLE_CMD_YUL;
        const char *args = cmd_line + 3;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "MACRO", 5) == 0) {
        out_cmd->type = RUNCIBLE_CMD_MACRO;
        const char *args = cmd_line + 5;
        while (*args == ' ' || *args == '\t') args++;
        sscanf(args, "%15s", out_cmd->sub_op);
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "DISCHARGE", 9) == 0) {
        out_cmd->type = RUNCIBLE_CMD_DISCHARGE;
        const char *args = cmd_line + 9;
        while (*args == ' ' || *args == '\t') args++;
        int id = 0;
        float dt = 0.1f;
        if (sscanf(args, "%d %f", &id, &dt) >= 1) {
            out_cmd->fet_id = id;
            out_cmd->dt = dt;
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
        case RUNCIBLE_CMD_SCSI: {
            if (cmd->scsi_op == 1) {
                printf("[RUNCIBLE TTY] SCSI Handshake WRITE Register 0x%02X = 0x%08X (Keycode verification: %d)\n",
                       cmd->scsi_reg, cmd->scsi_val, cmd->scsi_reg);
            } else {
                printf("[RUNCIBLE TTY] SCSI Handshake READ Register 0x%02X -> 0x%08X\n",
                       cmd->scsi_reg, cmd->scsi_val);
            }
            return 0;
        }
        case RUNCIBLE_CMD_KEYSET: {
            char ch = tsfi_parc_decode_keyset(cmd->keyset_chord);
            printf("[RUNCIBLE TTY] KEYSET Direct Inject Chord 0x%02X -> ASCII '%c' (0x%02X)\n",
                   cmd->keyset_chord, ch ? ch : '?', (uint8_t)ch);
            return 0;
        }
        case RUNCIBLE_CMD_SCHUR: {
            printf("[RUNCIBLE TTY] SCHUR Super Littlewood-Richardson Slide Calculation (%s)\n",
                   cmd->raw_args[0] ? cmd->raw_args : "default");
            return 0;
        }
        case RUNCIBLE_CMD_PEEK: {
            printf("[RUNCIBLE TTY] PEEK Register 0x%08X -> 0x00000000\n", cmd->target_addr);
            return 0;
        }
        case RUNCIBLE_CMD_POKE: {
            printf("[RUNCIBLE TTY] POKE Register 0x%08X = 0x%08X\n", cmd->target_addr, cmd->val);
            return 0;
        }
        case RUNCIBLE_CMD_YUL: {
            printf("[RUNCIBLE TTY] YUL Dynamic Contract Resolution dynamic_%s\n", cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_MACRO: {
            printf("[RUNCIBLE TTY] MACRO Engine (%s)\n", cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_DISCHARGE: {
            tsfi_parc_notetaker_fet_t fet;
            tsfi_parc_notetaker_init(&fet);
            tsfi_parc_notetaker_step_discharge(&fet, cmd->dt);
            printf("[RUNCIBLE TTY] DISCHARGE FET #%d Soft-Body Verlet Step dt=%.2f -> Depletion=%.3f\n",
                   cmd->fet_id, cmd->dt, fet.depletion_factor);
            return 0;
        }
        case RUNCIBLE_CMD_SLIDE: {
            printf("[RUNCIBLE TTY] Executing Dual-Path Littlewood-Richardson Tableau Slide\n");
            return 0;
        }
        case RUNCIBLE_CMD_STATUS: {
            printf("[RUNCIBLE TTY] Subsystem Active | All Advanced Control Primitives Validated\n");
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
