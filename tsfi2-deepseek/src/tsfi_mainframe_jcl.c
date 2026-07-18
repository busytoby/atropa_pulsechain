#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int tsfi_cw_run_jcl(const char **cards, int card_count) {
    return tsfi_cw_run_jcl_ex(cards, card_count, 0);
}

int tsfi_cw_run_jcl_ex(const char **cards, int card_count, int initial_rc) {
    if (!cards || card_count <= 0) return -1;
    int steps_run = 0;
    int current_rc = initial_rc;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0) {
            const char *exec = strstr(card, "EXEC ");
            if (exec) {
                // Check if step condition specifies bypassing
                const char *cond = strstr(card, "COND=");
                if (cond) {
                    // COND=(code,operator)
                    int cond_code = 0;
                    char op[4] = "";
                    if (sscanf(cond, "COND=(%u,%3[^)])", &cond_code, op) == 2) {
                        if (tsfi_cw_jcl_eval_cond(current_rc, cond_code, op)) {
                            // Condition met, bypass this step
                            continue;
                        }
                    }
                }
                
                steps_run++;
                // simulate return code of step execution
                if (strstr(exec, "PGM=ERR")) {
                    current_rc = 12;
                } else {
                    current_rc = 0;
                }
            }
        }
    }
    return steps_run;
}

int tsfi_cw_jcl_parse_disp(const char *disp_str, tsfi_cw_jcl_disp *disp_out) {
    if (!disp_str || !disp_out) return -1;
    memset(disp_out, 0, sizeof(tsfi_cw_jcl_disp));
    
    // Parse DISP=(status,normal,abnormal)
    const char *p = strstr(disp_str, "DISP=");
    if (!p) return -2;
    p += 5;
    
    if (*p == '(') p++;
    int s_idx = 0;
    while (*p && *p != ',' && *p != ')' && s_idx < 7) {
        disp_out->status[s_idx++] = *p++;
    }
    disp_out->status[s_idx] = '\0';
    
    if (*p == ',') {
        p++;
        int n_idx = 0;
        while (*p && *p != ',' && *p != ')' && n_idx < 7) {
            disp_out->normal[n_idx++] = *p++;
        }
        disp_out->normal[n_idx] = '\0';
    }
    
    if (*p == ',') {
        p++;
        int a_idx = 0;
        while (*p && *p != ',' && *p != ')' && a_idx < 7) {
            disp_out->abnormal[a_idx++] = *p++;
        }
        disp_out->abnormal[a_idx] = '\0';
    }
    return 0;
}

int tsfi_cw_run_jcl_sysin(const char **cards, int card_count, char *sysin_out, int max_sysin_len) {
    if (!cards || card_count <= 0 || !sysin_out || max_sysin_len <= 0) return -1;
    
    int sysin_active = 0;
    int bytes_written = 0;
    sysin_out[0] = '\0';
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (sysin_active) {
            if (strncmp(card, "/*", 2) == 0 || strncmp(card, "//", 2) == 0) {
                break; // End of inline SYSIN
            }
            int len = strlen(card);
            if (bytes_written + len + 2 > max_sysin_len) break;
            strcat(sysin_out, card);
            strcat(sysin_out, "\n");
            bytes_written += len + 1;
        } else {
            if (strstr(card, "SYSIN DD *")) {
                sysin_active = 1;
            }
        }
    }
    return bytes_written;
}

int tsfi_cw_run_jcl_proc(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc) {
    return tsfi_cw_run_jcl_proc_nested(cards, card_count, proc_cards, proc_card_count, initial_rc, 1);
}

int tsfi_cw_run_jcl_restart(const char **cards, int card_count, const char *restart_step) {
    if (!cards || card_count <= 0 || !restart_step) return -1;
    int steps_run = 0;
    int start_executing = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0) {
            const char *exec = strstr(card, "EXEC ");
            if (exec) {
                // Find step name
                char step_name[32] = "";
                int s_idx = 0;
                const char *p = card + 2;
                while (*p && *p != ' ' && *p != '\t' && s_idx < 31) {
                    step_name[s_idx++] = *p++;
                }
                step_name[s_idx] = '\0';
                
                if (strcmp(step_name, restart_step) == 0) {
                    start_executing = 1;
                }
                
                if (start_executing) {
                    steps_run++;
                }
            }
        }
    }
    return steps_run;
}

int tsfi_cw_run_jcl_set(const char **cards, int card_count, char *expanded_out, int max_len) {
    if (!cards || card_count <= 0 || !expanded_out || max_len <= 0) return -1;
    
    char set_var[32] = "";
    char set_val[128] = "";
    expanded_out[0] = '\0';
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        const char *set = strstr(card, "SET ");
        if (set) {
            set += 4;
            while (*set == ' ' || *set == '\t') set++;
            int v_idx = 0;
            while (*set && *set != '=' && *set != ' ' && v_idx < 31) {
                set_var[v_idx++] = *set++;
            }
            set_var[v_idx] = '\0';
            if (*set == '=') set++;
            int val_idx = 0;
            while (*set && *set != '\n' && *set != '\r' && val_idx < 127) {
                set_val[val_idx++] = *set++;
            }
            set_val[val_idx] = '\0';
        } else if (strncmp(card, "//", 2) == 0 && strstr(card, "EXEC ")) {
            // expand variable
            const char *exec = strstr(card, "EXEC ");
            int prefix_len = exec - card;
            strncat(expanded_out, card, prefix_len);
            strcat(expanded_out, "EXEC ");
            
            const char *var_start = exec + 5;
            if (strcmp(var_start, set_var) == 0) {
                strcat(expanded_out, set_val);
            } else {
                strcat(expanded_out, var_start);
            }
        }
    }
    return strlen(expanded_out);
}

int tsfi_cw_run_jcl_proc_nested(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc, int depth) {
    if (depth > 5) return -9;
    if (!cards || card_count <= 0) return -1;
    int steps_run = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0 && strstr(card, "EXEC ")) {
            const char *exec = strstr(card, "EXEC ");
            const char *target = exec + 5;
            if (strstr(target, "PGM=") == NULL) {
                // Call PROC
                int proc_steps = tsfi_cw_run_jcl_proc_nested(proc_cards, proc_card_count, proc_cards, proc_card_count, initial_rc, depth + 1);
                if (proc_steps == -9) return -9;
                if (proc_steps > 0) steps_run += proc_steps;
            } else {
                steps_run++;
            }
        }
    }
    return steps_run;
}

int tsfi_cw_jcl_eval_cond(int step_rc, int cond_code, const char *operator) {
    if (!operator) return 0;
    if (strcmp(operator, "LT") == 0) {
        return (cond_code < step_rc) ? 1 : 0;
    } else if (strcmp(operator, "GT") == 0) {
        return (cond_code > step_rc) ? 1 : 0;
    } else if (strcmp(operator, "EQ") == 0) {
        return (cond_code == step_rc) ? 1 : 0;
    }
    return 0;
}

int tsfi_cw_run_jcl_export(const char **cards, int card_count, char *exp_name, char *exp_val) {
    if (!cards || card_count <= 0 || !exp_name || !exp_val) return -1;
    exp_name[0] = '\0';
    exp_val[0] = '\0';
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        const char *exp = strstr(card, "EXPORT ");
        if (exp) {
            exp += 7;
            while (*exp == ' ' || *exp == '\t') exp++;
            int n_idx = 0;
            while (*exp && *exp != '=' && *exp != ' ' && n_idx < 31) {
                exp_name[n_idx++] = *exp++;
            }
            exp_name[n_idx] = '\0';
            if (*exp == '=') exp++;
            int v_idx = 0;
            while (*exp && *exp != ' ' && *exp != '\n' && *exp != '\r' && v_idx < 31) {
                exp_val[v_idx++] = *exp++;
            }
            exp_val[v_idx] = '\0';
            return 0;
        }
    }
    return -4;
}

void tsfi_cw_jcl_temp_pool_init(tsfi_cw_jcl_temp_pool *pool) {
    if (pool) {
        memset(pool, 0, sizeof(tsfi_cw_jcl_temp_pool));
    }
}

int tsfi_cw_jcl_temp_pool_add(tsfi_cw_jcl_temp_pool *pool, const char *name, const char *path) {
    if (!pool || !name || !path || pool->count >= 8) return -1;
    strcpy(pool->datasets[pool->count].name, name);
    strcpy(pool->datasets[pool->count].filepath, path);
    pool->datasets[pool->count].active = 1;
    pool->count++;
    return 0;
}

const char *tsfi_cw_jcl_temp_pool_get(tsfi_cw_jcl_temp_pool *pool, const char *name) {
    if (!pool || !name) return NULL;
    for (int i = 0; i < pool->count; i++) {
        if (pool->datasets[i].active && strcmp(pool->datasets[i].name, name) == 0) {
            return pool->datasets[i].filepath;
        }
    }
    return NULL;
}

void tsfi_cw_jcl_sysout_init(tsfi_cw_jcl_sysout *sysout) {
    if (sysout) memset(sysout, 0, sizeof(tsfi_cw_jcl_sysout));
}

int tsfi_cw_jcl_sysout_write(tsfi_cw_jcl_sysout *sysout, const char *text) {
    if (!sysout || !text) return -1;
    int len = strlen(text);
    if (sysout->length + len >= 1024) len = 1024 - sysout->length - 1;
    if (len <= 0) return 0;
    strncpy(sysout->buffer + sysout->length, text, len);
    sysout->length += len;
    sysout->buffer[sysout->length] = '\0';
    return 0;
}

int tsfi_cw_run_jcl_concat(const char **cards, int card_count, char *concat_out, int max_out_len) {
    if (!cards || card_count <= 0 || !concat_out || max_out_len <= 0) return -1;
    concat_out[0] = '\0';
    
    int in_concat = 0;
    int bytes_written = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (in_concat) {
            if (strncmp(card, "//", 2) == 0) {
                const char *space = card + 2;
                while (*space == ' ' || *space == '\t') space++;
                if (strncmp(space, "DD ", 3) == 0 || (strncmp(card, "// ", 3) == 0 && strstr(card, "DD ") == NULL)) {
                    const char *dsn = strstr(card, "DSN=");
                    if (dsn) {
                        dsn += 4;
                        int d_len = 0;
                        while (dsn[d_len] && dsn[d_len] != ',' && dsn[d_len] != ' ' && d_len < 31) {
                            d_len++;
                        }
                        if (bytes_written + d_len + 2 < max_out_len) {
                            if (bytes_written > 0) concat_out[bytes_written++] = ',';
                            strncpy(concat_out + bytes_written, dsn, d_len);
                            bytes_written += d_len;
                            concat_out[bytes_written] = '\0';
                        }
                    }
                } else {
                    break;
                }
            }
        } else {
            if (strstr(card, "DD DSN=")) {
                in_concat = 1;
                const char *dsn = strstr(card, "DSN=");
                if (dsn) {
                    dsn += 4;
                    int d_len = 0;
                    while (dsn[d_len] && dsn[d_len] != ',' && dsn[d_len] != ' ' && d_len < 31) {
                        d_len++;
                    }
                    strncpy(concat_out, dsn, d_len);
                    bytes_written += d_len;
                    concat_out[bytes_written] = '\0';
                }
            }
        }
    }
    return bytes_written;
}

int tsfi_cw_run_jcl_override(const char **cards, int card_count, const char *step_name, char *override_out, int max_len) {
    if (!cards || card_count <= 0 || !step_name || !override_out || max_len <= 0) return -1;
    override_out[0] = '\0';
    char search_pattern[64];
    snprintf(search_pattern, sizeof(search_pattern), "//%s.", step_name);
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, search_pattern, strlen(search_pattern)) == 0) {
            strncpy(override_out, card, max_len - 1);
            override_out[max_len - 1] = '\0';
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_jcl_resolve_gdg(const char *dsn_str, int current_gen, char *resolved_out, int max_len) {
    if (!dsn_str || !resolved_out || max_len <= 0) return -1;
    const char *p = strstr(dsn_str, "(");
    if (p) {
        int relative_gen = atoi(p + 1);
        int target_gen = current_gen + relative_gen;
        int base_len = p - dsn_str;
        char base_dsn[256];
        if (base_len >= 255) base_len = 255;
        strncpy(base_dsn, dsn_str, base_len);
        base_dsn[base_len] = '\0';
        snprintf(resolved_out, max_len, "%s.G%04dV00", base_dsn, target_gen);
        return 0;
    }
    strncpy(resolved_out, dsn_str, max_len - 1);
    resolved_out[max_len - 1] = '\0';
    return 0;
}
