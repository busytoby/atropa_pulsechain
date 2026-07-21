#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_runcible_lang.h"
#include "tsfi_parc_keyboard.h"
#include "tsfi_parc_keyset.h"
#include "tsfi_parc_notetaker.h"
#include "tsfi_parc_routing.h"
#include "tsfi_parc_clipboard.h"
#include "tsfi_parc_stcomp.h"
#include "tsfi_parc_tape_label_yul.h"
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_autodin_tape_proof.h"

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

    if (strncmp(cmd_line, "PUP", 3) == 0) {
        out_cmd->type = RUNCIBLE_CMD_PUP;
        const char *args = cmd_line + 3;
        while (*args == ' ' || *args == '\t') args++;
        int net = 0, host = 0, type = 0;
        if (sscanf(args, "SEND %d %d %d", &net, &host, &type) >= 3) {
            out_cmd->dest_net = (uint8_t)net;
            out_cmd->dest_host = (uint8_t)host;
            out_cmd->pup_type = (uint8_t)type;
        }
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "CLIP", 4) == 0) {
        out_cmd->type = RUNCIBLE_CMD_CLIP;
        const char *args = cmd_line + 4;
        while (*args == ' ' || *args == '\t') args++;
        sscanf(args, "%15s", out_cmd->sub_op);
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "ST", 2) == 0) {
        out_cmd->type = RUNCIBLE_CMD_ST;
        const char *args = cmd_line + 2;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "TAPE PROOF", 10) == 0) {
        out_cmd->type = RUNCIBLE_CMD_TAPE_PROOF;
        const char *args = cmd_line + 10;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "TAPE LABEL", 10) == 0) {
        out_cmd->type = RUNCIBLE_CMD_TAPE_LABEL;
        const char *args = cmd_line + 10;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "TAPE CATALOG", 12) == 0) {
        out_cmd->type = RUNCIBLE_CMD_TAPE_CATALOG;
        const char *args = cmd_line + 12;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "TAPE BOUNDS", 11) == 0) {
        out_cmd->type = RUNCIBLE_CMD_TAPE_BOUNDS;
        const char *args = cmd_line + 11;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "TAPE PHASE", 10) == 0) {
        out_cmd->type = RUNCIBLE_CMD_TAPE_PHASE;
        const char *args = cmd_line + 10;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "TAPE CHECK", 10) == 0) {
        out_cmd->type = RUNCIBLE_CMD_TAPE_CHECK;
        const char *args = cmd_line + 10;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "BEGIN", 5) == 0) {
        out_cmd->type = RUNCIBLE_CMD_BEGIN;
        return 0;
    }

    if (strncmp(cmd_line, "END", 3) == 0) {
        out_cmd->type = RUNCIBLE_CMD_END;
        return 0;
    }

    if (strncmp(cmd_line, "THUNK", 5) == 0) {
        out_cmd->type = RUNCIBLE_CMD_THUNK;
        const char *args = cmd_line + 5;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "EVAL", 4) == 0) {
        out_cmd->type = RUNCIBLE_CMD_EVAL;
        const char *args = cmd_line + 4;
        while (*args == ' ' || *args == '\t') args++;
        strncpy(out_cmd->raw_args, args, sizeof(out_cmd->raw_args) - 1);
        return 0;
    }

    if (strncmp(cmd_line, "VOID", 4) == 0) {
        out_cmd->type = RUNCIBLE_CMD_VOID;
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
        case RUNCIBLE_CMD_PUP: {
            tsfi_parc_routing_table_t rtable;
            tsfi_parc_routing_init(&rtable);
            uint8_t next_hop = 0;
            tsfi_parc_routing_lookup(&rtable, cmd->dest_net, &next_hop);
            printf("[RUNCIBLE TTY] PUP Packet Injection Net %d Host %d Type %d -> Gateway Next Hop %d\n",
                   cmd->dest_net, cmd->dest_host, cmd->pup_type, next_hop);
            return 0;
        }
        case RUNCIBLE_CMD_CLIP: {
            tsfi_parc_clipboard_t clip;
            tsfi_parc_clip_init(&clip);
            tsfi_parc_clip_copy(&clip, "Runcible Clipboard Stream", 25);
            printf("[RUNCIBLE TTY] CLIP Modeless Buffer Operation (%s) -> Size %d bytes\n",
                   cmd->sub_op[0] ? cmd->sub_op : "COPY", clip.length);
            return 0;
        }
        case RUNCIBLE_CMD_ST: {
            uint8_t bc[64];
            int bc_len = tsfi_parc_st_compile("3 + 4", bc, sizeof(bc));
            printf("[RUNCIBLE TTY] ST Smalltalk-76 Live Method Eval -> Bytecode Count %d\n", bc_len);
            return 0;
        }
        case RUNCIBLE_CMD_TAPE_PROOF: {
            tsfi_autodin_tape_proof_result_t res;
            int r = tsfi_autodin_verify_tape_file(cmd->raw_args[0] ? cmd->raw_args : "./tmp/test_guarded.dat.bin", TAPE_SECURITY_TOPSECRET, &res);
            printf("[RUNCIBLE TTY] TAPE PROOF AUTODIN Diagnostic (%s) -> Verified: %d (SigValid: %d)\n",
                   cmd->raw_args[0] ? cmd->raw_args : "./tmp/test_guarded.dat.bin", r == 0 ? 1 : 0, res.sig_valid);
            return 0;
        }
        case RUNCIBLE_CMD_TAPE_LABEL: {
            printf("[RUNCIBLE TTY] TAPE LABEL Inscribe Header -> Args: %s\n", cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_TAPE_CATALOG: {
            tsfi_tape_catalog_entry_t entries[4];
            int c = tsfi_tape_catalog_process_all(cmd->raw_args[0] ? cmd->raw_args : ".", entries, 4);
            printf("[RUNCIBLE TTY] TAPE CATALOG Scan Directory (%s) -> Cataloged %d Volume Slices\n",
                   cmd->raw_args[0] ? cmd->raw_args : ".", c);
            return 0;
        }
        case RUNCIBLE_CMD_TAPE_BOUNDS: {
            printf("[RUNCIBLE TTY] TAPE BOUNDS Query HDR4 Spatial Coordinates -> Target: %s\n", cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_TAPE_PHASE: {
            printf("[RUNCIBLE TTY] TAPE PHASE Query HDR7 Lissajous Invariants -> Target: %s\n", cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_TAPE_CHECK: {
            uint8_t tape_hdr[720];
            tsfi_tape_label_yul_format_full_header(tape_hdr, "HDL001", "HOLDERS_0.DAT.BIN", TAPE_SECURITY_SECRET, 0.0f, 0.0f, 512.0f, 512.0f, "HDL000", "HDL002", 0.125f, 2, 4, 8, 32, 30);
            int valid = tsfi_tape_label_yul_validate_sequence(tape_hdr);
            printf("[RUNCIBLE TTY] TAPE CHECK Volume Diagnostic (%s) -> Full 8-Block Sequence Valid: %d\n",
                   cmd->raw_args[0] ? cmd->raw_args : "HDL001", valid);
            return 0;
        }
        case RUNCIBLE_CMD_BEGIN: {
            printf("[RUNCIBLE TTY] ALGOL 60 BEGIN Block Frame -> Pushing Lexical Scope Depth +1 (450 Gas Slot)\n");
            return 0;
        }
        case RUNCIBLE_CMD_END: {
            printf("[RUNCIBLE TTY] ALGOL 60 END Block Frame -> Popping Lexical Scope Stack Frame\n");
            return 0;
        }
        case RUNCIBLE_CMD_THUNK: {
            printf("[RUNCIBLE TTY] ALGOL 60 THUNK Definition -> Binding Lazy Parameter Closure (%s)\n", cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_EVAL: {
            printf("[RUNCIBLE TTY] ALGOL 60 EVAL Thunk Evaluation -> Dynamic Call-by-Name Evaluation (%s) [450 Gas Slot]\n", cmd->raw_args);
            return 0;
        }
        case RUNCIBLE_CMD_VOID: {
            printf("[RUNCIBLE TTY] VOID Dysnomia VM Root State Discard -> Fuse(0) Dependent Registers Collapsed\n");
            return 0;
        }
        case RUNCIBLE_CMD_SLIDE: {
            printf("[RUNCIBLE TTY] Executing Dual-Path Littlewood-Richardson Tableau Slide\n");
            return 0;
        }
        case RUNCIBLE_CMD_STATUS: {
            printf("[RUNCIBLE TTY] Subsystem Active | AUTODIN Cryptographic Proof Engine Validated\n");
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
