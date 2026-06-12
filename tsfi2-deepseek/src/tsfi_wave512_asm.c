#include "tsfi_wave512_asm.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

void tsfi_wave512_init(Wave512State *state) {
    memset(state, 0, sizeof(Wave512State));
    state->output_pos = 0;
    state->symbol_count = 0;
    state->label_count = 0;
    state->statement_count = 0;
    state->pc = 0;
}

int tsfi_wave512_resolve_symbol(Wave512State *state, const char *name, int allocate_new) {
    while(*name == ' ' || *name == '\t') name++;
    char clean_name[32] = {0};
    int i = 0;
    while (name[i] && name[i] != ',' && name[i] != '\n' && name[i] != ' ' && name[i] != '\t' && i < 31) {
        clean_name[i] = name[i];
        i++;
    }
    
    if (clean_name[0] == '\0') return -1;
    
    // Check if it's a register format like W3
    if ((clean_name[0] == 'W' || clean_name[0] == 'w') && isdigit((unsigned char)clean_name[1])) {
        int r = atoi(clean_name + 1);
        if (r >= 0 && r < TSFI_WAVE_REG_COUNT) return r;
    }
    
    // Check symbol table
    for (i = 0; i < state->symbol_count; i++) {
        if (strcmp(state->symbols[i].name, clean_name) == 0) {
            return state->symbols[i].reg_idx;
        }
    }
    
    if (allocate_new && state->symbol_count < TSFI_WAVE_VAR_COUNT) {
        // Allocate variable register (W8..W15 space to keep user variables separate from registers W0..W7)
        int reg = 8 + (state->symbol_count % 8);
        snprintf(state->symbols[state->symbol_count].name, 32, "%s", clean_name);
        state->symbols[state->symbol_count].reg_idx = reg;
        return state->symbols[state->symbol_count++].reg_idx;
    }
    
    return -1;
}

void tsfi_wave512_exec(Wave512State *state, const char *code_in) {
    char *code = lau_strdup(code_in);
    char *saveptr;
    char *line = strtok_r(code, "\n", &saveptr);
    
    state->statement_count = 0;
    state->label_count = 0;
    state->pc = 0;
    
    // Step 1: Parse all lines into Statement Table and build Label Table
    while (line && state->statement_count < TSFI_WAVE_STATEMENT_COUNT) {
        while(*line == ' ' || *line == '\t') line++;
        if (*line == '\0' || *line == ';') {
            line = strtok_r(NULL, "\n", &saveptr);
            continue;
        }
        
        char label_part[32] = {0};
        char cmd_part[32] = {0};
        char args_part[256] = {0};
        
        char *colon = strchr(line, ':');
        char *search_start = line;
        if (colon) {
            size_t label_len = colon - line;
            if (label_len < 31) {
                memcpy(label_part, line, label_len);
                char *lbl_ptr = label_part;
                while (*lbl_ptr == ' ' || *lbl_ptr == '\t') lbl_ptr++;
                int end_idx = strlen(lbl_ptr) - 1;
                while (end_idx >= 0 && (lbl_ptr[end_idx] == ' ' || lbl_ptr[end_idx] == '\t')) {
                    lbl_ptr[end_idx] = '\0';
                    end_idx--;
                }
                if (state->label_count < TSFI_WAVE_LABEL_COUNT) {
                    snprintf(state->labels[state->label_count].name, 32, "%s", lbl_ptr);
                    state->labels[state->label_count].statement_idx = state->statement_count;
                    state->label_count++;
                }
            }
            search_start = colon + 1;
        }
        
        int parsed = sscanf(search_start, "%s %[^\n]", cmd_part, args_part);
        if (parsed > 0 && cmd_part[0] != ';') {
            Wave512Statement *stmt = &state->statements[state->statement_count];
            snprintf(stmt->label, 32, "%s", label_part);
            snprintf(stmt->cmd, 32, "%s", cmd_part);
            snprintf(stmt->args, 256, "%s", args_part);
            state->statement_count++;
        }
        
        line = strtok_r(NULL, "\n", &saveptr);
    }
    lau_free(code);
    
    // Step 2: Execution Loop
    int safety_counter = 0;
    while (state->pc >= 0 && state->pc < state->statement_count && safety_counter++ < 10000) {
        Wave512Statement *stmt = &state->statements[state->pc];
        int next_pc = state->pc + 1;
        
        if (strcasecmp(stmt->cmd, "WLOAD") == 0) {
            char *arg_save;
            char args_copy[256];
            snprintf(args_copy, 256, "%s", stmt->args);
            char *r_str = strtok_r(args_copy, ",", &arg_save);
            char *v_str = strtok_r(NULL, ",", &arg_save);
            if (r_str && v_str) {
                int r = tsfi_wave512_resolve_symbol(state, r_str, 1);
                float val = strtof(v_str, NULL);
                if (r >= 0 && r < TSFI_WAVE_REG_COUNT) {
                    state->registers[r] = wave512_set1(val);
                }
            }
        }
        else if (strcasecmp(stmt->cmd, "WADD") == 0 || strcasecmp(stmt->cmd, "WSUB") == 0 ||
                 strcasecmp(stmt->cmd, "WMUL") == 0 || strcasecmp(stmt->cmd, "WDIV") == 0) {
            char *arg_save;
            char args_copy[256];
            snprintf(args_copy, 256, "%s", stmt->args);
            char *rd_str = strtok_r(args_copy, ",", &arg_save);
            char *rs1_str = strtok_r(NULL, ",", &arg_save);
            char *rs2_str = strtok_r(NULL, ",", &arg_save);
            if (rd_str && rs1_str && rs2_str) {
                int rd = tsfi_wave512_resolve_symbol(state, rd_str, 1);
                int rs1 = tsfi_wave512_resolve_symbol(state, rs1_str, 0);
                int rs2 = tsfi_wave512_resolve_symbol(state, rs2_str, 0);
                if (rd >= 0 && rd < TSFI_WAVE_REG_COUNT &&
                    rs1 >= 0 && rs1 < TSFI_WAVE_REG_COUNT &&
                    rs2 >= 0 && rs2 < TSFI_WAVE_REG_COUNT) {
                    if (strcasecmp(stmt->cmd, "WADD") == 0) {
                        state->registers[rd] = VADDPS(state->registers[rs1], state->registers[rs2]);
                    } else if (strcasecmp(stmt->cmd, "WSUB") == 0) {
                        state->registers[rd] = VSUBPS(state->registers[rs1], state->registers[rs2]);
                    } else if (strcasecmp(stmt->cmd, "WMUL") == 0) {
                        state->registers[rd] = VMULPS(state->registers[rs1], state->registers[rs2]);
                    } else if (strcasecmp(stmt->cmd, "WDIV") == 0) {
                        state->registers[rd] = VDIVPS(state->registers[rs1], state->registers[rs2]);
                    }
                }
            }
        }
        else if (strcasecmp(stmt->cmd, "CMPGT") == 0) {
            char *arg_save;
            char args_copy[256];
            snprintf(args_copy, 256, "%s", stmt->args);
            char *rd_str = strtok_r(args_copy, ",", &arg_save);
            char *rs1_str = strtok_r(NULL, ",", &arg_save);
            char *rs2_str = strtok_r(NULL, ",", &arg_save);
            if (rd_str && rs1_str && rs2_str) {
                int rd = tsfi_wave512_resolve_symbol(state, rd_str, 1);
                int rs1 = tsfi_wave512_resolve_symbol(state, rs1_str, 0);
                int rs2 = tsfi_wave512_resolve_symbol(state, rs2_str, 0);
                if (rd >= 0 && rd < TSFI_WAVE_REG_COUNT &&
                    rs1 >= 0 && rs1 < TSFI_WAVE_REG_COUNT &&
                    rs2 >= 0 && rs2 < TSFI_WAVE_REG_COUNT) {
                    float *f1 = (float*)&state->registers[rs1];
                    float *f2 = (float*)&state->registers[rs2];
                    float out_vals[128];
                    for (int j = 0; j < 128; j++) {
                        out_vals[j] = (f1[j] > f2[j]) ? 1.0f : 0.0f;
                    }
                    memcpy(&state->registers[rd], out_vals, 512);
                }
            }
        }
        else if (strcasecmp(stmt->cmd, "JMP") == 0) {
            char target[32] = {0};
            sscanf(stmt->args, "%s", target);
            for (int i = 0; i < state->label_count; i++) {
                if (strcasecmp(state->labels[i].name, target) == 0) {
                    next_pc = state->labels[i].statement_idx;
                    break;
                }
            }
        }
        else if (strcasecmp(stmt->cmd, "JNZ") == 0) {
            char *arg_save;
            char args_copy[256];
            snprintf(args_copy, 256, "%s", stmt->args);
            char *cond_str = strtok_r(args_copy, ",", &arg_save);
            char *lbl_str = strtok_r(NULL, ",", &arg_save);
            if (cond_str && lbl_str) {
                while (*lbl_str == ' ' || *lbl_str == '\t') lbl_str++;
                int r = tsfi_wave512_resolve_symbol(state, cond_str, 0);
                if (r >= 0 && r < TSFI_WAVE_REG_COUNT) {
                    float *f = (float*)&state->registers[r];
                    if (f[0] != 0.0f) {
                        for (int i = 0; i < state->label_count; i++) {
                            if (strcasecmp(state->labels[i].name, lbl_str) == 0) {
                                next_pc = state->labels[i].statement_idx;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (strcasecmp(stmt->cmd, "WDUMP") == 0) {
            int r = tsfi_wave512_resolve_symbol(state, stmt->args, 0);
            if (r >= 0 && r < TSFI_WAVE_REG_COUNT) {
                float *f = (float*)&state->registers[r];
                int n = snprintf(state->output_buffer + state->output_pos, 
                                 sizeof(state->output_buffer) - state->output_pos,
                                 "W%d: [%.2f, %.2f... | %.2f...]\n", 
                                 r, f[0], f[1], f[16]);
                if (n > 0) state->output_pos += n;
            }
        }
        
        state->pc = next_pc;
    }
}