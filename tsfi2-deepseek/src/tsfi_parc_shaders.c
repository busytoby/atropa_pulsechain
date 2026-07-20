#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_parc_shaders.h"

int tsfi_parc_load_shaders(const char *algol_path, const char *strategy_path, tsfi_parc_shader_params_t *params) {
    if (!algol_path || !strategy_path || !params) return -1;

    // Set default parameters
    params->viewport_width = 512.0f;
    params->viewport_height = 512.0f;
    params->cursor_x = 120.0f;
    params->cursor_y = 450.0f;
    params->out_color_r = 0.11f;
    params->out_color_g = 0.09f;
    params->out_color_b = 0.08f;

    // 1. Parse Algol61 Vertex Shader Domain config
    FILE *fa = fopen(algol_path, "r");
    if (fa) {
        char line[256];
        while (fgets(line, sizeof(line), fa)) {
            if (strstr(line, "viewport_width")) {
                char *val_str = strstr(line, ":=");
                if (val_str) {
                    params->viewport_width = (float)atof(val_str + 2);
                }
            } else if (strstr(line, "viewport_height")) {
                char *val_str = strstr(line, ":=");
                if (val_str) {
                    params->viewport_height = (float)atof(val_str + 2);
                }
            }
        }
        fclose(fa);
    }

    // 2. Parse COBOL Fragment Strategy Domain variables
    FILE *fs = fopen(strategy_path, "r");
    if (fs) {
        char line[256];
        while (fgets(line, sizeof(line), fs)) {
            if (strstr(line, "WS-CURSOR-REG-X")) {
                char *val_str = strstr(line, "VALUE");
                if (val_str) {
                    params->cursor_x = (float)atof(val_str + 5);
                }
            } else if (strstr(line, "WS-CURSOR-REG-Y")) {
                char *val_str = strstr(line, "VALUE");
                if (val_str) {
                    params->cursor_y = (float)atof(val_str + 5);
                }
            } else if (strstr(line, "MOVE") && strstr(line, "WS-OUT-COLOR-R")) {
                char *val_str = strstr(line, "MOVE");
                if (val_str) {
                    params->out_color_r = (float)atof(val_str + 4);
                }
            } else if (strstr(line, "MOVE") && strstr(line, "WS-OUT-COLOR-G")) {
                char *val_str = strstr(line, "MOVE");
                if (val_str) {
                    params->out_color_g = (float)atof(val_str + 4);
                }
            } else if (strstr(line, "MOVE") && strstr(line, "WS-OUT-COLOR-B")) {
                char *val_str = strstr(line, "MOVE");
                if (val_str) {
                    params->out_color_b = (float)atof(val_str + 4);
                }
            }
        }
        fclose(fs);
    }

    return 0;
}
