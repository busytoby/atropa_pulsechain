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
}

static int parse_reg(const char *token) {
    while(*token == ' ' || *token == '\t') token++;
    if (token[0] == 'W' || token[0] == 'w') {
        return atoi(token + 1);
    }
    return -1;
}

void tsfi_wave512_exec(Wave512State *state, const char *code_in) {
    char *code = lau_strdup(code_in);
    char *saveptr;
    char *line = strtok_r(code, "\n", &saveptr);
    
    while (line) {
        char cmd[32] = {0};
        char args[256] = {0};
        
        sscanf(line, "%s %[^\n]", cmd, args);
        
        if (cmd[0] != 0 && cmd[0] != ';') { 
            if (strcasecmp(cmd, "WLOAD") == 0) {
                char *arg_save;
                char *r_str = strtok_r(args, ",", &arg_save);
                char *v_str = strtok_r(NULL, ",", &arg_save);
                
                if (r_str && v_str) {
                    int r = parse_reg(r_str);
                    float val = strtof(v_str, NULL);
                    if (r >= 0 && r < TSFI_WAVE_REG_COUNT) {
                        state->registers[r] = wave512_set1(val);
                    }
                }
            }
            else if (strcasecmp(cmd, "WADD") == 0) {
                 // ... omitted
            }
            else if (strcasecmp(cmd, "WDUMP") == 0) {
                int r = parse_reg(args);
                if (r >= 0 && r < TSFI_WAVE_REG_COUNT) {
                    float *f = (float*)&state->registers[r];
                    int n = snprintf(state->output_buffer + state->output_pos, 
                                     4096 - state->output_pos,
                                     "W%d: [%.2f, %.2f... | %.2f...]\n", 
                                     r, f[0], f[1], f[16]);
                    if (n > 0) state->output_pos += n;
                }
            }
        }
        
        line = strtok_r(NULL, "\n", &saveptr);
    }
    
    lau_free(code);
}