#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_winchester_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
void tsfi_cobol_report_writer(const char *title, int total_acc, char *output_report, int max_len) {
    if (!title || !output_report || max_len <= 0) return;
    
    snprintf(output_report, max_len,
             "========================================\n"
             "           COBOL REPORT WRITER          \n"
             "Title: %s\n"
             "----------------------------------------\n"
             "ACCUMULATOR GRAND TOTAL: %-10d\n"
             "========================================\n",
             title, total_acc);
}

int tsfi_codasyl_dml_find(const tsfi_codasyl_subschema *subschema, const tsfi_codasyl_dbtg_set *sets, int set_count, int relation_id) {
    if (!subschema || !sets || set_count <= 0) return -1;
    
    int authorized = 0;
    for (int i = 0; i < 4; i++) {
        if (subschema->allowed_relation_ids[i] == relation_id) {
            authorized = 1;
            break;
        }
    }
    if (!authorized) return -2;
    
    for (int i = 0; i < set_count; i++) {
        if (sets[i].relation_id == relation_id) {
            return i;
        }
    }
    return -3;
}

void tsfi_codasyl_checkpoint_save(const tsfi_codasyl_dbtg_set *sets, int count, tsfi_codasyl_checkpoint *checkpoint) {
    if (!sets || count <= 0 || !checkpoint) return;
    checkpoint->active_relations_count = count > 10 ? 10 : count;
    for (int i = 0; i < checkpoint->active_relations_count; i++) {
        checkpoint->saved_relation_ids[i] = sets[i].relation_id;
    }
}

void tsfi_codasyl_checkpoint_rollback(tsfi_codasyl_dbtg_set *sets, int *count, const tsfi_codasyl_checkpoint *checkpoint) {
    if (!sets || !count || !checkpoint) return;
    *count = checkpoint->active_relations_count;
    for (int i = 0; i < *count; i++) {
        sets[i].relation_id = checkpoint->saved_relation_ids[i];
    }
}

int tsfi_rca501_parse_items(const uint8_t *message, int len, uint8_t items_out[8][64], int max_items) {
    if (!message || len <= 0 || max_items <= 0) return -1;
    
    int item_idx = 0;
    int char_idx = 0;
    
    for (int i = 0; i < len; i++) {
        if (message[i] == RCA501_EI || message[i] == RCA501_EM) {
            items_out[item_idx][char_idx] = '\0';
            item_idx++;
            char_idx = 0;
            if (message[i] == RCA501_EM || item_idx >= max_items) {
                break;
            }
        } else {
            if (char_idx < 63) {
                items_out[item_idx][char_idx++] = message[i];
            }
        }
    }
    return item_idx;
}

void tsfi_rca501_set_channel_busy(tsfi_rca501_controller *ctrl, int channel, int busy) {
    if (!ctrl || channel < 0 || channel > 7) return;
    if (busy) {
        ctrl->channels_busy |= (1 << channel);
    } else {
        ctrl->channels_busy &= ~(1 << channel);
    }
}

int tsfi_rca501_check_channel(const tsfi_rca501_controller *ctrl, int channel) {
    if (!ctrl || channel < 0 || channel > 7) return -1;
    return (ctrl->channels_busy & (1 << channel)) ? 1 : 0;
}

int tsfi_compile_decision_table_from_text(const char *text, uint8_t *bytecode_out, int max_len) {
    if (!text || !bytecode_out || max_len <= 0) return -1;
    tsfi_decision_table table;
    memset(&table, 0, sizeof(table));
    
    int reg_a = 0, reg_b = 0, action_reg = 0, val = 0;
    if (sscanf(text, "IF R%d == R%d MOVE %d TO R%d", &reg_a, &reg_b, &val, &action_reg) == 4) {
        strcpy(table.condition_op[0], "==");
        table.condition_reg_a[0] = reg_a;
        table.condition_reg_b[0] = reg_b;
        strcpy(table.action_op[0], "MOVE");
        table.action_reg[0] = action_reg;
        table.action_val[0] = val;
        table.rule_count = 1;
        return tsfi_compile_decision_table(&table, bytecode_out, max_len);
    }
    return -2;
}

int tsfi_winchester_socket_route_to_zmm(tsfi_winchester_socket_bridge *bridge, const uint8_t *event_data, int len, TsfiZmmVmState *zmm) {
    if (!bridge || !event_data || len <= 0 || !zmm) return -1;
    if (!bridge->connection_active) return -2;
    
    uint8_t keycode = event_data[0];
    
    if (zmm->telem) {
        zmm->telem->zmm_val = keycode;
    }
    bridge->processed_packets++;
    return 0;
}

void tsfi_compool_init(tsfi_jovial_compool *cp) {
    if (!cp) return;
    cp->entry_count = 0;
    for (int i = 0; i < 16; i++) {
        cp->entries[i].var_name[0] = '\0';
        cp->entries[i].val = 0;
    }
}

int tsfi_compool_register(tsfi_jovial_compool *cp, const char *name, uint32_t val) {
    if (!cp || !name || cp->entry_count >= 16) return -1;
    
    for (int i = 0; i < cp->entry_count; i++) {
        if (strcmp(cp->entries[i].var_name, name) == 0) {
            cp->entries[i].val = val;
            return 0;
        }
    }
    
    strncpy(cp->entries[cp->entry_count].var_name, name, 31);
    cp->entries[cp->entry_count].var_name[31] = '\0';
    cp->entries[cp->entry_count].val = val;
    cp->entry_count++;
    return 0;
}

int tsfi_compool_lookup(const tsfi_jovial_compool *cp, const char *name, uint32_t *val_out) {
    if (!cp || !name || !val_out) return -1;
    for (int i = 0; i < cp->entry_count; i++) {
        if (strcmp(cp->entries[i].var_name, name) == 0) {
            *val_out = cp->entries[i].val;
            return 0;
        }
    }
    return -2;
}

uint32_t tsfi_jovial_bit_slice(uint32_t val, int start, int length) {
    if (length <= 0 || length > 32 || start < 0 || start >= 32) return 0;
    uint32_t mask = (length == 32) ? 0xFFFFFFFF : ((1U << length) - 1);
    return (val >> start) & mask;
}

uint32_t tsfi_jovial_bit_slice_assign(uint32_t orig_val, uint32_t field_val, int start, int length) {
    if (length <= 0 || length > 32 || start < 0 || start >= 32) return orig_val;
    uint32_t mask = (length == 32) ? 0xFFFFFFFF : ((1U << length) - 1);
    field_val &= mask;
    uint32_t clear_mask = ~(mask << start);
    return (orig_val & clear_mask) | (field_val << start);
}

uint32_t tsfi_jovial_overlay_read(const tsfi_jovial_overlay_pool *pool, int offset) {
    if (!pool || offset < 0 || offset >= 16) return 0;
    return pool->overlay_storage[offset];
}

void tsfi_jovial_overlay_write(tsfi_jovial_overlay_pool *pool, int offset, uint32_t val) {
    if (!pool || offset < 0 || offset >= 16) return;
    pool->overlay_storage[offset] = val;
}

uint32_t tsfi_jovial_table_read(const tsfi_jovial_table *table, int item_idx, int word_idx) {
    if (!table || item_idx < 0 || item_idx >= 4 || word_idx < 0 || word_idx >= 2) return 0;
    
    int index = 0;
    if (table->is_parallel) {
        index = word_idx * 4 + item_idx;
    } else {
        index = item_idx * 2 + word_idx;
    }
    return table->data[index];
}

void tsfi_jovial_table_write(tsfi_jovial_table *table, int item_idx, int word_idx, uint32_t val) {
    if (!table || item_idx < 0 || item_idx >= 4 || word_idx < 0 || word_idx >= 2) return;
    
    int index = 0;
    if (table->is_parallel) {
        index = word_idx * 4 + item_idx;
    } else {
        index = item_idx * 2 + word_idx;
    }
    table->data[index] = val;
}

#include <math.h>

static void *tsfi_zmm_ctss_worker(void *arg) {
    tsfi_zmm_voice_thread *voice = (tsfi_zmm_voice_thread *)arg;
    
    // Calculate the base pointer of the scheduler structure
    tsfi_zmm_ctss_scheduler *sched = (tsfi_zmm_ctss_scheduler *)((char *)voice - voice->voice_id * sizeof(tsfi_zmm_voice_thread));
    
    for (int i = 0; i < 256; i++) {
        voice->buffer[i] = (uint8_t)(128 + 30 * sin((double)i * (voice->voice_id + 1) * 0.1));
    }
    
    pthread_mutex_lock(&sched->mix_mutex);
    for (int i = 0; i < 256; i++) {
        sched->mix_buffer[i] += (int)voice->buffer[i] - 128;
    }
    pthread_mutex_unlock(&sched->mix_mutex);
    
    return NULL;
}

void tsfi_zmm_ctss_init(tsfi_zmm_ctss_scheduler *sched, TsfiZmmVmState *zmm) {
    if (!sched) return;
    pthread_mutex_init(&sched->mix_mutex, NULL);
    memset(sched->mix_buffer, 0, sizeof(sched->mix_buffer));
    for (int i = 0; i < 4; i++) {
        sched->voices[i].zmm = zmm;
        sched->voices[i].voice_id = i;
        sched->voices[i].active = 0;
        memset(sched->voices[i].buffer, 0, sizeof(sched->voices[i].buffer));
    }
}

void tsfi_zmm_ctss_start(tsfi_zmm_ctss_scheduler *sched) {
    if (!sched) return;
    for (int i = 0; i < 4; i++) {
        sched->voices[i].active = 1;
        pthread_create(&sched->voices[i].thread_id, NULL, tsfi_zmm_ctss_worker, &sched->voices[i]);
    }
}

void tsfi_zmm_ctss_stop(tsfi_zmm_ctss_scheduler *sched) {
    if (!sched) return;
    for (int i = 0; i < 4; i++) {
        if (sched->voices[i].active) {
            pthread_join(sched->voices[i].thread_id, NULL);
            sched->voices[i].active = 0;
        }
    }
}

void tsfi_zmm_ctss_mix(tsfi_zmm_ctss_scheduler *sched, int *output_mix, int max_len) {
    if (!sched || !output_mix || max_len <= 0) return;
    pthread_mutex_lock(&sched->mix_mutex);
    int len = max_len > 256 ? 256 : max_len;
    for (int i = 0; i < len; i++) {
        output_mix[i] = sched->mix_buffer[i];
    }
    pthread_mutex_unlock(&sched->mix_mutex);
}

void tsfi_algol_scope_init(tsfi_algol_scope_frame *frame, int parent) {
    if (!frame) return;
    frame->parent_offset = parent;
    for (int i = 0; i < 8; i++) {
        frame->variables[i] = 0;
    }
}

void tsfi_algol_stack_push(tsfi_algol_call_stack *stack, int pc) {
    if (!stack || stack->sp >= 8) return;
    stack->return_pcs[stack->sp++] = pc;
}

int tsfi_algol_stack_pop(tsfi_algol_call_stack *stack) {
    if (!stack || stack->sp <= 0) return -1;
    return stack->return_pcs[--stack->sp];
}

void tsfi_algol_matrix_multiply(const tsfi_algol_matrix *a, const tsfi_algol_matrix *b, tsfi_algol_matrix *result) {
    if (!a || !b || !result) return;
    if (a->cols != b->rows) return;
    result->rows = a->rows;
    result->cols = b->cols;
    memset(result->data, 0, sizeof(result->data));
    for (int r = 0; r < a->rows; r++) {
        for (int c = 0; c < b->cols; c++) {
            int sum = 0;
            for (int k = 0; k < a->cols; k++) {
                sum += a->data[r * a->cols + k] * b->data[k * b->cols + c];
            }
            result->data[r * b->cols + c] = sum;
        }
    }
}

int tsfi_cobol_compute_eval(const char *expression, const int reg_values[8]) {
    if (!expression || !reg_values) return 0;
    while (*expression == ' ' || *expression == '\t') expression++;
    int rx = -1, ry = -1, rz = -1;
    char op1 = '\0', op2 = '\0';
    
    int matched = sscanf(expression, "R%d %c R%d %c R%d", &rx, &op1, &ry, &op2, &rz);
    if (matched == 5) {
        int val1 = (rx >= 0 && rx < 8) ? reg_values[rx] : 0;
        int val2 = (ry >= 0 && ry < 8) ? reg_values[ry] : 0;
        int val3 = (rz >= 0 && rz < 8) ? reg_values[rz] : 0;
        
        if (op1 == '*' && op2 == '+') {
            return val1 * val2 + val3;
        }
    }
    
    matched = sscanf(expression, "R%d %c R%d", &rx, &op1, &ry);
    if (matched == 3) {
        int val1 = (rx >= 0 && rx < 8) ? reg_values[rx] : 0;
        int val2 = (ry >= 0 && ry < 8) ? reg_values[ry] : 0;
        if (op1 == '+') return val1 + val2;
        if (op1 == '-') return val1 - val2;
        if (op1 == '*') return val1 * val2;
    }
    
    return 0;
}

double tsfi_algol_math_sin(double x) {
    return sin(x);
}

double tsfi_algol_math_cos(double x) {
    return cos(x);
}

double tsfi_algol_math_sqrt(double x) {
    return (x < 0.0) ? 0.0 : sqrt(x);
}

int tsfi_algol_format_output(const char *format, double val, char *buf_out, int max_len) {
    if (!format || !buf_out || max_len <= 0) return -1;
    
    if (strcmp(format, "F6.2") == 0) {
        return snprintf(buf_out, max_len, "%6.2f", val);
    } else if (strcmp(format, "$99.99") == 0) {
        char temp[32];
        snprintf(temp, sizeof(temp), "%05.2f", val);
        return snprintf(buf_out, max_len, "$%s", temp);
    }
    
    return snprintf(buf_out, max_len, "%f", val);
}

int tsfi_algol_common_write(tsfi_algol_common_block *block, int offset, int val) {
    if (!block || offset < 0 || offset >= 64) return -1;
    block->data[offset] = val;
    return 0;
}

int tsfi_algol_common_read(const tsfi_algol_common_block *block, int offset) {
    if (!block || offset < 0 || offset >= 64) return 0;
    return block->data[offset];
}

int tsfi_algol_call_cobol(const char *cobol_expr, int regs[8]) {
    return tsfi_cobol_compute_eval(cobol_expr, regs);
}

int tsfi_cobol_call_algol_proc(int proc_id, int param) {
    if (proc_id == 1) {
        int res = 1;
        for (int i = 1; i <= param; i++) res *= i;
        return res;
    }
    return 0;
}

int tsfi_algol_maze_solve(const int maze[16], int curr, int end, int visited[16]) {
    if (curr < 0 || curr >= 16 || maze[curr] == 1 || visited[curr]) return 0;
    if (curr == end) return 1;
    
    visited[curr] = 1;
    
    int row = curr / 4;
    int col = curr % 4;
    
    if (row > 0 && tsfi_algol_maze_solve(maze, curr - 4, end, visited)) return 1;
    if (row < 3 && tsfi_algol_maze_solve(maze, curr + 4, end, visited)) return 1;
    if (col > 0 && tsfi_algol_maze_solve(maze, curr - 1, end, visited)) return 1;
    if (col < 3 && tsfi_algol_maze_solve(maze, curr + 1, end, visited)) return 1;
    
    return 0;
}

int tsfi_law_query(const tsfi_law_case *db, int db_size, const char *query_word, int results_out[8]) {
    if (!db || db_size <= 0 || !query_word || !results_out) return 0;
    int count = 0;
    for (int i = 0; i < db_size; i++) {
        if (strcmp(db[i].keyword, query_word) == 0) {
            results_out[count++] = db[i].case_id;
            if (count >= 8) break;
        }
    }
    return count;
}

void tsfi_command_init(tsfi_command_center *cc) {
    if (!cc) return;
    cc->sensor_count = 0;
    for (int i = 0; i < 4; i++) {
        cc->sensors[i].sensor_name[0] = '\0';
        cc->sensors[i].curr_value = 0;
        cc->sensors[i].threshold = 0;
        cc->sensors[i].alert_triggered = 0;
    }
}

int tsfi_command_poll(tsfi_command_center *cc) {
    if (!cc) return 0;
    int triggered = 0;
    for (int i = 0; i < cc->sensor_count; i++) {
        if (cc->sensors[i].curr_value > cc->sensors[i].threshold) {
            cc->sensors[i].alert_triggered = 1;
            triggered = 1;
        } else {
            cc->sensors[i].alert_triggered = 0;
        }
    }
    return triggered;
}

uint64_t tsfi_cdc3600_extract_byte(uint64_t word, int bit_position, int byte_length) {
    if (byte_length <= 0 || byte_length > 64 || bit_position < 0 || bit_position >= 64) return 0;
    uint64_t mask = (byte_length == 64) ? ~0ULL : ((1ULL << byte_length) - 1);
    return (word >> bit_position) & mask;
}

uint64_t tsfi_double_to_cdc3600_float(double val) {
    union { double d; uint64_t u; } conv;
    conv.d = val;
    uint64_t sign = (conv.u >> 63) & 1ULL;
    int64_t exp = ((conv.u >> 52) & 0x7FFULL) - 1023 + 1024;
    if (exp < 0) exp = 0;
    if (exp > 0x7FF) exp = 0x7FF;
    uint64_t frac = (conv.u >> 16) & 0xFFFFFFFFFULL;
    return (sign << 47) | ((uint64_t)exp << 36) | frac;
}

double tsfi_cdc3600_float_to_double(uint64_t word) {
    uint64_t sign = (word >> 47) & 1ULL;
    int64_t exp = ((word >> 36) & 0x7FFULL) - 1024 + 1023;
    if (exp < 0) exp = 0;
    if (exp > 0x7FF) exp = 0x7FF;
    uint64_t frac = (word & 0xFFFFFFFFFULL) << 16;
    union { double d; uint64_t u; } conv;
    conv.u = (sign << 63) | ((uint64_t)exp << 52) | frac;
    return conv.d;
}

int tsfi_detabx_compile(const char *conditions[2], const char *actions[2], const char rules[2][2], char *cobol_out, int max_len) {
    if (!conditions || !actions || !rules || !cobol_out || max_len <= 0) return -1;
    
    cobol_out[0] = '\0';
    int written = 0;
    for (int r = 0; r < 2; r++) {
        char cond_clause[256] = "";
        int first_cond = 1;
        for (int c = 0; c < 2; c++) {
            if (rules[r][c] == 'Y') {
                if (!first_cond) {
                    strcat(cond_clause, " AND ");
                }
                strcat(cond_clause, conditions[c]);
                first_cond = 0;
            }
        }
        
        if (strlen(cond_clause) > 0) {
            char action_clause[256] = "";
            for (int a = 0; a < 2; a++) {
                if (r == a || rules[r][a] == 'Y') {
                    strcat(action_clause, actions[a]);
                    strcat(action_clause, "; ");
                }
            }
            
            char line[512];
            snprintf(line, sizeof(line), "IF %s THEN %sEND-IF. ", cond_clause, action_clause);
            if (written + (int)strlen(line) < max_len) {
                strcat(cobol_out, line);
                written += strlen(line);
            }
        }
    }
    return 0;
}

int tsfi_detabx_execute(const tsfi_detabx_table *table, int regs[8]) {
    if (!table || !regs) return -1;
    
    int cond_results[4] = {0};
    for (int i = 0; i < table->num_conditions; i++) {
        int rx = -1, val = -1;
        char op[4] = "";
        if (sscanf(table->condition_stubs[i], "R%d %3s %d", &rx, op, &val) == 3) {
            int reg_val = (rx >= 0 && rx < 8) ? regs[rx] : 0;
            if (strcmp(op, ">") == 0) cond_results[i] = (reg_val > val);
            else if (strcmp(op, "==") == 0) cond_results[i] = (reg_val == val);
            else if (strcmp(op, "<") == 0) cond_results[i] = (reg_val < val);
        }
    }
    
    int matched_rule = -1;
    for (int j = 0; j < table->num_rules; j++) {
        int rule_match = 1;
        for (int i = 0; i < table->num_conditions; i++) {
            char entry = table->condition_entries[i][j];
            if (entry == 'Y' && !cond_results[i]) { rule_match = 0; break; }
            if (entry == 'N' && cond_results[i]) { rule_match = 0; break; }
        }
        if (rule_match) {
            matched_rule = j;
            break;
        }
    }
    
    if (matched_rule != -1) {
        int seq_list[4];
        for (int i = 0; i < table->num_actions; i++) {
            seq_list[i] = i;
        }
        
        for (int x = 0; x < table->num_actions - 1; x++) {
            for (int y = x + 1; y < table->num_actions; y++) {
                int seq_x = table->action_sequence[seq_list[x]][matched_rule];
                int seq_y = table->action_sequence[seq_list[y]][matched_rule];
                if (seq_x > seq_y) {
                    int temp = seq_list[x];
                    seq_list[x] = seq_list[y];
                    seq_list[y] = temp;
                }
            }
        }
        
        for (int idx = 0; idx < table->num_actions; idx++) {
            int i = seq_list[idx];
            if (table->action_entries[i][matched_rule] == 'X') {
                int rx = -1, val = -1;
                if (sscanf(table->action_stubs[i], "SET R%d %d", &rx, &val) == 2) {
                    if (rx >= 0 && rx < 8) regs[rx] = val;
                } else if (strncmp(table->action_stubs[i], "COMPUTE ", 8) == 0) {
                    const char *expr = table->action_stubs[i] + 8;
                    const char *eq = strchr(expr, '=');
                    int target_r = -1;
                    if (eq && sscanf(expr, "R%d", &target_r) == 1) {
                        int comp_val = tsfi_cobol_compute_eval(eq + 1, regs);
                        if (target_r >= 0 && target_r < 8) regs[target_r] = comp_val;
                    }
                }
            }
        }
        return matched_rule;
    } else {
        for (int i = 0; i < table->num_actions; i++) {
            if (table->else_action_entries[i] == 'X') {
                int rx = -1, val = -1;
                if (sscanf(table->action_stubs[i], "SET R%d %d", &rx, &val) == 2) {
                    if (rx >= 0 && rx < 8) regs[rx] = val;
                } else if (strncmp(table->action_stubs[i], "COMPUTE ", 8) == 0) {
                    const char *expr = table->action_stubs[i] + 8;
                    const char *eq = strchr(expr, '=');
                    int target_r = -1;
                    if (eq && sscanf(expr, "R%d", &target_r) == 1) {
                        int comp_val = tsfi_cobol_compute_eval(eq + 1, regs);
                        if (target_r >= 0 && target_r < 8) regs[target_r] = comp_val;
                    }
                }
            }
        }
        return -2;
    }
}

int tsfi_detabx_validate(const tsfi_detabx_table *table) {
    if (!table) return -3;
    
    for (int j1 = 0; j1 < table->num_rules - 1; j1++) {
        for (int j2 = j1 + 1; j2 < table->num_rules; j2++) {
            int overlap = 1;
            for (int i = 0; i < table->num_conditions; i++) {
                char e1 = table->condition_entries[i][j1];
                char e2 = table->condition_entries[i][j2];
                if (e1 != '-' && e1 != '\0' && e2 != '-' && e2 != '\0' && e1 != e2) {
                    overlap = 0;
                    break;
                }
            }
            
            if (overlap) {
                int same_actions = 1;
                for (int a = 0; a < table->num_actions; a++) {
                    if (table->action_entries[a][j1] != table->action_entries[a][j2]) {
                        same_actions = 0;
                        break;
                    }
                }
                if (same_actions) {
                    return -2;
                } else {
                    return -1;
                }
            }
        }
    }
    return 0;
}

void tsfi_scoreboard_init(cdc_scoreboard *sb) {
    if (!sb) return;
    sb->size = 0;
    for (int i = 0; i < 8; i++) {
        sb->reg_writers[i] = -1;
    }
}

int tsfi_scoreboard_step(cdc_scoreboard *sb, int regs[8]) {
    if (!sb) return 0;
    int done_count = 0;
    for (int i = 0; i < sb->size; i++) {
        cdc_instruction *inst = &sb->queue[i];
        if (inst->stage == STAGE_DONE) continue;
        
        int can_advance = 1;
        if (inst->stage == STAGE_ISSUE) {
            if (inst->dest_reg >= 0 && inst->dest_reg < 8) {
                int writer = sb->reg_writers[inst->dest_reg];
                if (writer == -1 || writer == inst->inst_id) {
                    sb->reg_writers[inst->dest_reg] = inst->inst_id;
                    inst->stage = STAGE_READ_OPERANDS;
                }
            } else {
                inst->stage = STAGE_READ_OPERANDS;
            }
        } else if (inst->stage == STAGE_READ_OPERANDS) {
            if (inst->src1_reg >= 0 && inst->src1_reg < 8) {
                int writer = sb->reg_writers[inst->src1_reg];
                if (writer != -1 && writer != inst->inst_id) can_advance = 0;
            }
            if (inst->src2_reg >= 0 && inst->src2_reg < 8) {
                int writer = sb->reg_writers[inst->src2_reg];
                if (writer != -1 && writer != inst->inst_id) can_advance = 0;
            }
            if (can_advance) {
                inst->stage = STAGE_EXECUTE;
            }
        } else if (inst->stage == STAGE_EXECUTE) {
            inst->stage = STAGE_WRITE_BACK;
            if (regs) {
                int val1 = (inst->src1_reg >= 0 && inst->src1_reg < 8) ? regs[inst->src1_reg] : 0;
                int val2 = (inst->src2_reg >= 0 && inst->src2_reg < 8) ? regs[inst->src2_reg] : 0;
                int res = 0;
                if (strcmp(inst->op, "ADD") == 0) res = val1 + val2;
                else if (strcmp(inst->op, "SUB") == 0) res = val1 - val2;
                else if (strcmp(inst->op, "MUL") == 0) res = val1 * val2;
                if (inst->dest_reg >= 0 && inst->dest_reg < 8) {
                    regs[inst->dest_reg] = res;
                }
            }
        } else if (inst->stage == STAGE_WRITE_BACK) {
            for (int k = 0; k < i; k++) {
                cdc_instruction *prev = &sb->queue[k];
                if (prev->stage == STAGE_READ_OPERANDS) {
                    if (prev->src1_reg == inst->dest_reg || prev->src2_reg == inst->dest_reg) {
                        can_advance = 0;
                    }
                }
            }
            if (can_advance) {
                inst->stage = STAGE_DONE;
                if (inst->dest_reg >= 0 && inst->dest_reg < 8) {
                    if (sb->reg_writers[inst->dest_reg] == inst->inst_id) {
                        sb->reg_writers[inst->dest_reg] = -1;
                    }
                }
                done_count++;
            }
        }
    }
    return done_count;
}

void tsfi_ppu_init(cdc_ppu_system *sys) {
    if (!sys) return;
    sys->current_slot = 0;
    for (int i = 0; i < 10; i++) {
        sys->ppus[i].ppu_id = i;
        sys->ppus[i].task_active = 0;
        sys->ppus[i].bytes_processed = 0;
        sys->ppus[i].total_bytes = 0;
        sys->ppus[i].shared_instruction = NULL;
    }
}

void tsfi_ppu_assign(cdc_ppu_system *sys, int ppu_id, int bytes) {
    if (!sys || ppu_id < 0 || ppu_id >= 10) return;
    sys->ppus[ppu_id].task_active = 1;
    sys->ppus[ppu_id].total_bytes = bytes;
    sys->ppus[ppu_id].bytes_processed = 0;
    sys->ppus[ppu_id].shared_instruction = NULL;
}

int tsfi_ppu_step(cdc_ppu_system *sys) {
    if (!sys) return 0;
    int processed = 0;
    cdc_ppu *curr = &sys->ppus[sys->current_slot];
    if (curr->task_active) {
        if (curr->shared_instruction) {
            if (curr->shared_instruction->stage == STAGE_DONE) {
                curr->task_active = 0;
                curr->shared_instruction = NULL;
            } else {
                curr->bytes_processed++;
                processed = 1;
            }
        } else if (curr->bytes_processed < curr->total_bytes) {
            curr->bytes_processed++;
            processed = 1;
            if (curr->bytes_processed >= curr->total_bytes) {
                curr->task_active = 0;
            }
        }
    }
    sys->current_slot = (sys->current_slot + 1) % 10;
    return processed;
}

int tsfi_rand_tablet_interpolate(int raw_x, int raw_y, int raw_grid[4][2], rand_tablet_point *pt_out) {
    if (!pt_out) return -1;
    (void)raw_grid;
    // Map grid coordinate points
    pt_out->x = (raw_x < 0) ? 0 : (raw_x > 1023 ? 1023 : raw_x);
    pt_out->y = (raw_y < 0) ? 0 : (raw_y > 1023 ? 1023 : raw_y);
    pt_out->pen_down = 1;
    return 0;
}

void tsfi_rand_tablet_trace_init(rand_tablet_buffer *buf) {
    if (!buf) return;
    buf->count = 0;
}

int tsfi_rand_tablet_trace(rand_tablet_buffer *buf, int raw_x, int raw_y, int raw_grid[4][2]) {
    if (!buf) return -1;
    if (buf->count >= 32) return -2;
    rand_tablet_point pt;
    int res = tsfi_rand_tablet_interpolate(raw_x, raw_y, raw_grid, &pt);
    if (res == 0) {
        buf->points[buf->count++] = pt;
    }
    return res;
}

int tsfi_ppu_scoreboard_dispatch(cdc_ppu_system *sys, cdc_scoreboard *sb, int ppu_id, const cdc_instruction *inst) {
    if (!sys || !sb || !inst || ppu_id < 0 || ppu_id >= 10) return -1;
    if (sb->size >= 8) return -2;
    
    tsfi_ppu_assign(sys, ppu_id, 4);
    sb->queue[sb->size] = *inst;
    sb->queue[sb->size].inst_id = ppu_id;
    sb->queue[sb->size].stage = STAGE_ISSUE;
    sb->size++;
    return 0;
}

void tsfi_ids_init(ids_currency_tracker *tracker) {
    if (!tracker) return;
    tracker->size = 0;
    tracker->current_run_unit = -1;
    for (int i = 0; i < 4; i++) {
        tracker->current_record_type[i] = -1;
        tracker->current_set[i] = -1;
    }
}

int tsfi_ids_insert(ids_currency_tracker *tracker, int record_id, int parent_id, const char *data) {
    if (!tracker || tracker->size >= 16) return -1;
    ids_record *r = &tracker->records[tracker->size];
    r->record_id = record_id;
    r->parent_record_id = parent_id;
    r->next_record_id = record_id;
    if (data) {
        strncpy(r->data, data, sizeof(r->data) - 1);
        r->data[sizeof(r->data) - 1] = '\0';
    }
    
    for (int i = 0; i < tracker->size; i++) {
        if (tracker->records[i].parent_record_id == parent_id) {
            int curr = i;
            while (tracker->records[curr].next_record_id != tracker->records[i].record_id) {
                int next_found = 0;
                for (int k = 0; k < tracker->size; k++) {
                    if (tracker->records[k].record_id == tracker->records[curr].next_record_id) {
                        curr = k;
                        next_found = 1;
                        break;
                    }
                }
                if (!next_found) break;
            }
            r->next_record_id = tracker->records[curr].next_record_id;
            tracker->records[curr].next_record_id = record_id;
            break;
        }
    }
    tracker->current_run_unit = record_id;
    tracker->size++;
    return 0;
}

int tsfi_ids_navigate_next(ids_currency_tracker *tracker, int set_id) {
    if (!tracker || set_id < 0 || set_id >= 4) return -1;
    int curr_id = tracker->current_set[set_id];
    if (curr_id == -1) {
        if (tracker->size > 0) {
            tracker->current_set[set_id] = tracker->records[0].record_id;
            return tracker->records[0].record_id;
        }
        return -1;
    }
    
    for (int i = 0; i < tracker->size; i++) {
        if (tracker->records[i].record_id == curr_id) {
            int next_id = tracker->records[i].next_record_id;
            tracker->current_set[set_id] = next_id;
            tracker->current_run_unit = next_id;
            return next_id;
        }
    }
    return -1;
}

void tsfi_mackenzie_init(mackenzie_storage *store) {
    if (!store) return;
    store->current_tick = 0;
    for (int i = 0; i < 8; i++) {
        store->segments[i].sector_id = i;
        store->segments[i].access_count = 0;
        store->segments[i].last_access_tick = 0;
        store->segments[i].location = 0;
    }
}

int tsfi_mackenzie_access(mackenzie_storage *store, int sector_id) {
    if (!store || sector_id < 0 || sector_id >= 8) return -1;
    store->current_tick++;
    store->segments[sector_id].access_count++;
    store->segments[sector_id].last_access_tick = store->current_tick;
    return store->segments[sector_id].location;
}

int tsfi_mackenzie_migrate(mackenzie_storage *store, int age_threshold) {
    if (!store) return 0;
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (store->segments[i].location == 0) {
            int age = store->current_tick - store->segments[i].last_access_tick;
            if (age > age_threshold && store->segments[i].access_count < 5) {
                store->segments[i].location = 1;
                count++;
            }
        }
    }
    return count;
}

void tsfi_cross_chain_init(cross_chain_tracker *tracker) {
    if (!tracker) return;
    tracker->count = 0;
    for (int i = 0; i < 4; i++) {
        tracker->current_chain_token[i] = -1;
        tracker->current_cross_token[i] = -1;
    }
}

int tsfi_cross_chain_insert(cross_chain_tracker *tracker, const char *addr, int chain_id, const char *symbol) {
    if (!tracker || tracker->count >= 16 || !addr || !symbol) return -1;
    cross_chain_token *t = &tracker->tokens[tracker->count];
    strncpy(t->token_address, addr, sizeof(t->token_address) - 1);
    t->token_address[sizeof(t->token_address) - 1] = '\0';
    t->chain_id = chain_id;
    strncpy(t->symbol, symbol, sizeof(t->symbol) - 1);
    t->symbol[sizeof(t->symbol) - 1] = '\0';
    
    t->next_token_on_chain = tracker->count;
    t->next_token_cross_chain = tracker->count;
    
    for (int i = 0; i < tracker->count; i++) {
        if (tracker->tokens[i].chain_id == chain_id) {
            int curr = i;
            while (tracker->tokens[curr].next_token_on_chain != i) {
                curr = tracker->tokens[curr].next_token_on_chain;
            }
            t->next_token_on_chain = tracker->tokens[curr].next_token_on_chain;
            tracker->tokens[curr].next_token_on_chain = tracker->count;
            break;
        }
    }
    
    for (int i = 0; i < tracker->count; i++) {
        if (strcmp(tracker->tokens[i].symbol, symbol) == 0) {
            int curr = i;
            while (tracker->tokens[curr].next_token_cross_chain != i) {
                curr = tracker->tokens[curr].next_token_cross_chain;
            }
            t->next_token_cross_chain = tracker->tokens[curr].next_token_cross_chain;
            tracker->tokens[curr].next_token_cross_chain = tracker->count;
            break;
        }
    }
    
    tracker->count++;
    return 0;
}

int tsfi_cross_chain_navigate_chain(cross_chain_tracker *tracker, int chain_idx) {
    if (!tracker || chain_idx < 0 || chain_idx >= 4) return -1;
    int curr = tracker->current_chain_token[chain_idx];
    if (curr == -1) {
        for (int i = 0; i < tracker->count; i++) {
            if (tracker->tokens[i].chain_id == chain_idx) {
                tracker->current_chain_token[chain_idx] = i;
                return i;
            }
        }
        return -1;
    }
    int next = tracker->tokens[curr].next_token_on_chain;
    tracker->current_chain_token[chain_idx] = next;
    return next;
}

int tsfi_cross_chain_navigate_symbol(cross_chain_tracker *tracker, int sym_idx) {
    if (!tracker || sym_idx < 0 || sym_idx >= 4) return -1;
    int curr = tracker->current_cross_token[sym_idx];
    if (curr == -1) {
        return -1;
    }
    int next = tracker->tokens[curr].next_token_cross_chain;
    tracker->current_cross_token[sym_idx] = next;
    return next;
}

void tsfi_interrupt_init(tsfi_cobol_interrupt_controller *ctrl) {
    if (!ctrl) return;
    ctrl->vector_count = 0;
    for (int i = 0; i < 4; i++) {
        ctrl->vectors[i].interrupt_code = -1;
        ctrl->vectors[i].cobol_handler[0] = '\0';
    }
}

int tsfi_interrupt_register(tsfi_cobol_interrupt_controller *ctrl, int code, const char *handler) {
    if (!ctrl || ctrl->vector_count >= 4 || !handler) return -1;
    ctrl->vectors[ctrl->vector_count].interrupt_code = code;
    strncpy(ctrl->vectors[ctrl->vector_count].cobol_handler, handler, sizeof(ctrl->vectors[ctrl->vector_count].cobol_handler) - 1);
    ctrl->vectors[ctrl->vector_count].cobol_handler[sizeof(ctrl->vectors[ctrl->vector_count].cobol_handler) - 1] = '\0';
    ctrl->vector_count++;
    return 0;
}

int tsfi_interrupt_dispatch(tsfi_cobol_interrupt_controller *ctrl, int code, int regs[8]) {
    if (!ctrl || !regs) return -1;
    for (int i = 0; i < ctrl->vector_count; i++) {
        if (ctrl->vectors[i].interrupt_code == code) {
            const char *handler = ctrl->vectors[i].cobol_handler;
            int rx = -1, val = -1;
            if (sscanf(handler, "SET R%d %d", &rx, &val) == 2) {
                if (rx >= 0 && rx < 8) regs[rx] = val;
                return 0;
            } else if (strncmp(handler, "COMPUTE ", 8) == 0) {
                const char *expr = handler + 8;
                const char *eq = strchr(expr, '=');
                int target_r = -1;
                if (eq && sscanf(expr, "R%d", &target_r) == 1) {
                    int comp_val = tsfi_cobol_compute_eval(eq + 1, regs);
                    if (target_r >= 0 && target_r < 8) regs[target_r] = comp_val;
                    return 0;
                }
            }
        }
    }
    return -2;
}

void tsfi_pli_exception_init(pli_exception_system *sys) {
    if (!sys) return;
    sys->count = 0;
    for (int i = 0; i < 8; i++) {
        sys->units[i].exception_type[0] = '\0';
        sys->units[i].handler_action[0] = '\0';
        sys->units[i].active = 0;
    }
}

int tsfi_pli_exception_register(pli_exception_system *sys, const char *type, const char *action) {
    if (!sys || sys->count >= 8 || !type || !action) return -1;
    pli_on_unit *unit = &sys->units[sys->count];
    strncpy(unit->exception_type, type, sizeof(unit->exception_type) - 1);
    unit->exception_type[sizeof(unit->exception_type) - 1] = '\0';
    strncpy(unit->handler_action, action, sizeof(unit->handler_action) - 1);
    unit->handler_action[sizeof(unit->handler_action) - 1] = '\0';
    unit->active = 1;
    sys->count++;
    return 0;
}

int tsfi_pli_exception_trigger(pli_exception_system *sys, const char *type, int regs[8]) {
    if (!sys || !type || !regs) return -1;
    for (int i = sys->count - 1; i >= 0; i--) {
        if (sys->units[i].active && strcmp(sys->units[i].exception_type, type) == 0) {
            const char *action = sys->units[i].handler_action;
            int rx = -1, val = -1;
            if (sscanf(action, "SET R%d %d", &rx, &val) == 2) {
                if (rx >= 0 && rx < 8) regs[rx] = val;
                return 0;
            }
        }
    }
    return -2;
}

int tsfi_zerocopy_dispatch(cdc_ppu_system *sys, cdc_scoreboard *sb, int ppu_id, int sb_index) {
    if (!sys || !sb || ppu_id < 0 || ppu_id >= 10 || sb_index < 0 || sb_index >= sb->size) return -1;
    sys->ppus[ppu_id].shared_instruction = &sb->queue[sb_index];
    sys->ppus[ppu_id].task_active = 1;
    sys->ppus[ppu_id].total_bytes = 4;
    sys->ppus[ppu_id].bytes_processed = 0;
    return 0;
}

void tsfi_rmag_init(rmag_processor *proc) {
    if (!proc) return;
    proc->count = 0;
    for (int i = 0; i < 8; i++) {
        proc->macros[i].macro_name[0] = '\0';
        proc->macros[i].macro_template[0] = '\0';
    }
    // Pre-registered VM and Language macros
    tsfi_rmag_define(proc, "COBOL_COMPUTE", "COMPUTE $1 = R0 + R1");
    tsfi_rmag_define(proc, "ALGOL_FOR", "for i := 1 step 1 until $1 do");
    tsfi_rmag_define(proc, "YUL_SSTORE", "sstore($1, 1)");
    tsfi_rmag_define(proc, "JOVIAL_TAB", "TABLE $1 STATUS PACKED");
    tsfi_rmag_define(proc, "FORMAL_PROOF_RULE", "Theorem: Correctness of $1. Invariant: memory_copies(s) == 0. Q.E.D.");
}

int tsfi_rmag_define(rmag_processor *proc, const char *name, const char *template_str) {
    if (!proc || proc->count >= 8 || !name || !template_str) return -1;
    rmag_macro *m = &proc->macros[proc->count];
    strncpy(m->macro_name, name, sizeof(m->macro_name) - 1);
    m->macro_name[sizeof(m->macro_name) - 1] = '\0';
    strncpy(m->macro_template, template_str, sizeof(m->macro_template) - 1);
    m->macro_template[sizeof(m->macro_template) - 1] = '\0';
    proc->count++;
    return 0;
}

int tsfi_rmag_expand(rmag_processor *proc, const char *input, const char *arg, char *output, size_t max_len) {
    if (!proc || !input || !output || max_len == 0) return -1;
    
    for (int i = 0; i < proc->count; i++) {
        const char *found = strstr(input, proc->macros[i].macro_name);
        if (found) {
            size_t prefix_len = found - input;
            if (prefix_len >= max_len) prefix_len = max_len - 1;
            strncpy(output, input, prefix_len);
            output[prefix_len] = '\0';
            
            const char *tpl = proc->macros[i].macro_template;
            const char *placeholder = strstr(tpl, "$1");
            if (placeholder && arg) {
                size_t sub_len = placeholder - tpl;
                if (strlen(output) + sub_len < max_len) {
                    strncat(output, tpl, sub_len);
                }
                if (strlen(output) + strlen(arg) < max_len) {
                    strcat(output, arg);
                }
                const char *suffix = placeholder + 2;
                if (strlen(output) + strlen(suffix) < max_len) {
                    strcat(output, suffix);
                }
            } else {
                if (strlen(output) + strlen(tpl) < max_len) {
                    strcat(output, tpl);
                }
            }
            
            const char *input_suffix = found + strlen(proc->macros[i].macro_name);
            if (strlen(output) + strlen(input_suffix) < max_len) {
                strcat(output, input_suffix);
            }
            return 0;
        }
    }
    
    strncpy(output, input, max_len - 1);
    output[max_len - 1] = '\0';
    return -2;
}

void tsfi_imp_format(imp_header *hdr, uint8_t src, uint8_t dest, uint8_t link, uint8_t type) {
    if (!hdr) return;
    hdr->src_imp = src;
    hdr->dest_imp = dest;
    hdr->link_num = link;
    hdr->msg_type = type;
}

int tsfi_imp_route(const imp_header *hdr, int active_nodes[4]) {
    if (!hdr || !active_nodes) return -1;
    int target_idx = hdr->dest_imp % 4;
    if (active_nodes[target_idx]) {
        return target_idx;
    }
    for (int i = 0; i < 4; i++) {
        if (active_nodes[i] && i != target_idx) {
            return i;
        }
    }
    return -1;
}

int tsfi_bgp_proxy_route(const imp_header *hdr, const char *bgp_payload, char *routed_output, size_t max_len) {
    if (!hdr || !bgp_payload || !routed_output || max_len == 0) return -1;
    int bytes = snprintf(routed_output, max_len, "[IMP_ROUTE src=%d dest=%d link=%d type=%d] %s",
                         hdr->src_imp, hdr->dest_imp, hdr->link_num, hdr->msg_type, bgp_payload);
    return (bytes > 0 && (size_t)bytes < max_len) ? 0 : -2;
}

void tsfi_multics_init(multics_segment_table *table) {
    if (!table) return;
    table->count = 0;
    for (int i = 0; i < 16; i++) {
        table->segments[i].segment_id = 0;
        table->segments[i].base_addr = 0;
        table->segments[i].size = 0;
        table->segments[i].acl_flags = 0;
    }
}

int tsfi_multics_register(multics_segment_table *table, uint32_t segment_id, uintptr_t base_addr, size_t size, uint8_t flags) {
    if (!table || table->count >= 16) return -1;
    multics_segment *seg = &table->segments[table->count++];
    seg->segment_id = segment_id;
    seg->base_addr = base_addr;
    seg->size = size;
    seg->acl_flags = flags;
    return 0;
}

int tsfi_multics_check_access(const multics_segment_table *table, uintptr_t addr, uint8_t required_flags) {
    if (!table) return -1;
    for (int i = 0; i < table->count; i++) {
        const multics_segment *seg = &table->segments[i];
        if (addr >= seg->base_addr && addr < (seg->base_addr + seg->size)) {
            if ((seg->acl_flags & required_flags) == required_flags) {
                return 0;
            } else {
                return -2;
            }
        }
    }
    return -3;
}

uint64_t tsfi_bates_hash(const char *token, uint64_t salt) {
    if (!token) return 0;
    uint64_t h = salt ^ 953467954114363ULL;
    while (*token) {
        h = (h * 33) ^ (uint8_t)(*token);
        token++;
    }
    return h;
}

int tsfi_bates_authenticate(const char *token, uint64_t salt, uint64_t expected_hash) {
    uint64_t computed = tsfi_bates_hash(token, salt);
    return (computed == expected_hash) ? 0 : -1;
}

int tsfi_dbl_convert(const uint8_t *raw_sector_data, size_t sector_size, char *db_relation_output, size_t max_len) {
    if (!raw_sector_data || sector_size == 0 || !db_relation_output || max_len == 0) return -1;
    if (sector_size < 4) return -2;
    uint32_t owner_key = ((uint32_t)raw_sector_data[0] << 24) |
                         ((uint32_t)raw_sector_data[1] << 16) |
                         ((uint32_t)raw_sector_data[2] << 8)  |
                         (uint32_t)raw_sector_data[3];
    int bytes = snprintf(db_relation_output, max_len, "OWNER_KEY=0x%08X; MEMBERS=%zu_BYTES", owner_key, sector_size - 4);
    return (bytes > 0 && (size_t)bytes < max_len) ? 0 : -3;
}

void tsfi_mis_init(mis_database *db) {
    if (!db) return;
    db->count = 0;
    for (int i = 0; i < 32; i++) {
        db->records[i].resource_name[0] = '\0';
        db->records[i].allocation_val = 0;
        db->records[i].parent_node_id = 0;
    }
}

int tsfi_mis_insert(mis_database *db, const char *name, uint32_t allocation, uint32_t parent_id) {
    if (!db || db->count >= 32 || !name) return -1;
    mis_record *r = &db->records[db->count++];
    strncpy(r->resource_name, name, sizeof(r->resource_name) - 1);
    r->resource_name[sizeof(r->resource_name) - 1] = '\0';
    r->allocation_val = allocation;
    r->parent_node_id = parent_id;
    return 0;
}

int tsfi_mis_query(const mis_database *db, uint32_t parent_id, uint32_t min_alloc, char *result_out, size_t max_len) {
    if (!db || !result_out || max_len == 0) return -1;
    result_out[0] = '\0';
    int match_count = 0;
    for (int i = 0; i < db->count; i++) {
        const mis_record *r = &db->records[i];
        if (r->parent_node_id == parent_id && r->allocation_val >= min_alloc) {
            if (match_count > 0) {
                strncat(result_out, ",", max_len - strlen(result_out) - 1);
            }
            strncat(result_out, r->resource_name, max_len - strlen(result_out) - 1);
            match_count++;
        }
    }
    return match_count;
}

void tsfi_dbtg_selection_init(dbtg_selection_table *table) {
    if (!table) return;
    table->count = 0;
    for (int i = 0; i < 16; i++) {
        table->rules[i].owner_id = 0;
        table->rules[i].match_criteria[0] = '\0';
    }
}

int tsfi_dbtg_selection_register(dbtg_selection_table *table, uint32_t owner_id, const char *criteria) {
    if (!table || table->count >= 16 || !criteria) return -1;
    dbtg_selection_rule *rule = &table->rules[table->count++];
    rule->owner_id = owner_id;
    strncpy(rule->match_criteria, criteria, sizeof(rule->match_criteria) - 1);
    rule->match_criteria[sizeof(rule->match_criteria) - 1] = '\0';
    return 0;
}

int tsfi_dbtg_selection_resolve(const dbtg_selection_table *table, const char *member_field) {
    if (!table || !member_field) return -1;
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->rules[i].match_criteria, member_field) == 0) {
            return (int)table->rules[i].owner_id;
        }
    }
    return -1;
}

void tsfi_dsdl_init(dsdl_mapping_table *table) {
    if (!table) return;
    table->count = 0;
    for (int i = 0; i < 16; i++) {
        table->rules[i].logical_record_id = 0;
        table->rules[i].physical_cylinder = 0;
        table->rules[i].page_offset = 0;
    }
}

int tsfi_dsdl_register(dsdl_mapping_table *table, uint32_t record_id, uint32_t cylinder, uint32_t page) {
    if (!table || table->count >= 16) return -1;
    dsdl_mapping_rule *rule = &table->rules[table->count++];
    rule->logical_record_id = record_id;
    rule->physical_cylinder = cylinder;
    rule->page_offset = page;
    return 0;
}

int tsfi_dsdl_resolve(const dsdl_mapping_table *table, uint32_t record_id, uint32_t *out_cylinder, uint32_t *out_page) {
    if (!table || !out_cylinder || !out_page) return -1;
    for (int i = 0; i < table->count; i++) {
        if (table->rules[i].logical_record_id == record_id) {
            *out_cylinder = table->rules[i].physical_cylinder;
            *out_page = table->rules[i].page_offset;
            return 0;
        }
    }
    return -1;
}

void tsfi_cobol_ledger_init(cobol_ledger *ledger) {
    if (!ledger) return;
    ledger->count = 0;
    for (int i = 0; i < 16; i++) {
        ledger->accounts[i].account_id = 0;
        ledger->accounts[i].balance = 0.0;
        ledger->accounts[i].account_holder[0] = '\0';
    }
}

int tsfi_cobol_ledger_add(cobol_ledger *ledger, uint32_t acc_id, const char *holder, double initial_balance) {
    if (!ledger || ledger->count >= 16 || !holder) return -1;
    cobol_account *acc = &ledger->accounts[ledger->count++];
    acc->account_id = acc_id;
    acc->balance = initial_balance;
    strncpy(acc->account_holder, holder, sizeof(acc->account_holder) - 1);
    acc->account_holder[sizeof(acc->account_holder) - 1] = '\0';
    return 0;
}

int tsfi_cobol_ledger_transaction(cobol_ledger *ledger, uint32_t acc_id, double amount, char *audit_out, size_t max_len) {
    if (!ledger || !audit_out || max_len == 0) return -1;
    for (int i = 0; i < ledger->count; i++) {
        cobol_account *acc = &ledger->accounts[i];
        if (acc->account_id == acc_id) {
            double old_bal = acc->balance;
            acc->balance += amount;
            int bytes = snprintf(audit_out, max_len, "ACC=%u; HOLDER=%s; OLD=%.2f; NEW=%.2f; DELTA=%.2f",
                                 acc->account_id, acc->account_holder, old_bal, acc->balance, amount);
            return (bytes > 0 && (size_t)bytes < max_len) ? 0 : -2;
        }
    }
    return -3;
}

double tsfi_cobol_round(double value, int decimals, int mode) {
    double factor = pow(10.0, decimals);
    double scaled = value * factor;
    if (mode == COBOL_ROUND_TRUNC) {
        return trunc(scaled) / factor;
    } else if (mode == COBOL_ROUND_STANDARD) {
        return round(scaled) / factor;
    } else if (mode == COBOL_ROUND_BANKERS) {
        double r = round(scaled);
        double diff = scaled - r;
        if (fabs(diff) == 0.5) {
            double integer_part;
            modf(scaled, &integer_part);
            if (((int)integer_part % 2) == 0) {
                return integer_part / factor;
            } else {
                return (integer_part + (scaled > 0 ? 1.0 : -1.0)) / factor;
            }
        }
        return r / factor;
    }
    return value;
}

double tsfi_cobol_calc_interest(double principal, double rate, int periods, int mode) {
    double interest = principal * pow(1.0 + rate, periods) - principal;
    return tsfi_cobol_round(interest, 2, mode);
}

void tsfi_ach_init(ach_batch *batch) {
    if (!batch) return;
    batch->count = 0;
    for (int i = 0; i < 16; i++) {
        batch->entries[i].routing_number[0] = '\0';
        batch->entries[i].amount = 0.0;
    }
}

int tsfi_ach_verify_routing(const char *routing) {
    if (!routing || strlen(routing) != 9) return -1;
    int d[9];
    for (int i = 0; i < 9; i++) {
        if (routing[i] < '0' || routing[i] > '9') return -2;
        d[i] = routing[i] - '0';
    }
    int sum = 3 * (d[0] + d[3] + d[6]) + 7 * (d[1] + d[4] + d[7]) + (d[2] + d[5] + d[8]);
    return (sum % 10 == 0) ? 0 : -3;
}

int tsfi_ach_add(ach_batch *batch, const char *routing, double amount) {
    if (!batch || batch->count >= 16 || !routing) return -1;
    if (tsfi_ach_verify_routing(routing) != 0) return -2;
    ach_entry *entry = &batch->entries[batch->count++];
    strncpy(entry->routing_number, routing, sizeof(entry->routing_number) - 1);
    entry->routing_number[sizeof(entry->routing_number) - 1] = '\0';
    entry->amount = amount;
    return 0;
}

uint64_t tsfi_ach_calc_hash_total(const ach_batch *batch) {
    if (!batch) return 0;
    uint64_t total = 0;
    for (int i = 0; i < batch->count; i++) {
        char first_eight[9];
        strncpy(first_eight, batch->entries[i].routing_number, 8);
        first_eight[8] = '\0';
        total += strtoull(first_eight, NULL, 10);
    }
    return total;
}

int tsfi_nacha_generate_entry(char *record_out, size_t max_len, uint8_t tx_code, const char *routing, const char *account, double amount) {
    if (!record_out || max_len < 95 || !routing || !account) return -1;
    if (tsfi_ach_verify_routing(routing) != 0) return -2;
    if (tx_code != 22 && tx_code != 27 && tx_code != 32 && tx_code != 37) return -3;
    
    char routing_first8[9];
    strncpy(routing_first8, routing, 8);
    routing_first8[8] = '\0';
    char check_digit = routing[8];
    
    char acc_padded[18];
    snprintf(acc_padded, sizeof(acc_padded), "%-17.17s", account);
    
    uint64_t amt_cents = (uint64_t)round(amount * 100.0);
    char amt_padded[11];
    snprintf(amt_padded, sizeof(amt_padded), "%010llu", (unsigned long long)amt_cents);
    
    int written = snprintf(record_out, max_len, "6%02u%8.8s%c%-17.17s%10.10s%-15.15s%-22.22s%-2.2s0%015d",
                           tx_code, routing_first8, check_digit, acc_padded, amt_padded,
                           "", "Individual Name", "", 12345);
    return (written == 94) ? 0 : -4;
}

int tsfi_nacha_validate_record(const char *record) {
    if (!record || strlen(record) != 94) return -1;
    if (record[0] == '6') {
        char routing[10];
        strncpy(routing, record + 3, 9);
        routing[9] = '\0';
        return tsfi_ach_verify_routing(routing);
    }
    return 0;
}

int tsfi_nacha_generate_file(char *file_out, size_t max_len, const ach_batch *batch, uint8_t tx_code, const char *origin_routing, const char *dest_routing) {
    if (!file_out || max_len < 1000 || !batch || !origin_routing || !dest_routing) return -1;
    if (tsfi_ach_verify_routing(origin_routing) != 0 || tsfi_ach_verify_routing(dest_routing) != 0) return -2;
    file_out[0] = '\0';
    size_t offset = 0;
    int line_count = 0;
    int written = snprintf(file_out + offset, max_len - offset,
                           "101 %-10.10s%-10.10s2607161944A094101%9.9s%-26.26s\n",
                           dest_routing, origin_routing, "FedReserve", "Origin Bank");
    offset += written;
    line_count++;
    written = snprintf(file_out + offset, max_len - offset,
                       "5220%-16.16s%-20.20s%-10.10sPPDDescription   260716260716   1%8.8s0000001\n",
                       "Company Name", "Discretionary Data", "CompanyID", "FedReserve");
    offset += written;
    line_count++;
    double total_amount = 0.0;
    for (int i = 0; i < batch->count; i++) {
        char entry_buf[96];
        char acc_num[18];
        snprintf(acc_num, sizeof(acc_num), "ACC%d", i);
        int entry_res = tsfi_nacha_generate_entry(entry_buf, sizeof(entry_buf), tx_code,
                                                   batch->entries[i].routing_number, acc_num, batch->entries[i].amount);
        if (entry_res != 0) return -3;
        total_amount += batch->entries[i].amount;
        written = snprintf(file_out + offset, max_len - offset, "%s\n", entry_buf);
        offset += written;
        line_count++;
    }
    uint64_t hash_total = tsfi_ach_calc_hash_total(batch);
    uint64_t total_cents = (uint64_t)round(total_amount * 100.0);
    written = snprintf(file_out + offset, max_len - offset,
                       "8220%06d%010llu%012llu%012llu%-10.10s%-25.25s%8.8s0000001\n",
                       batch->count, (unsigned long long)(hash_total % 10000000000ULL),
                       0ULL, (unsigned long long)total_cents, "CompanyID", "", "FedReserve");
    offset += written;
    line_count++;
    int total_records = line_count + 1;
    int block_count = (total_records + 9) / 10;
    written = snprintf(file_out + offset, max_len - offset,
                       "9000001%06d%08d%010llu%012llu%012llu%-39.39s\n",
                       block_count, total_records, (unsigned long long)(hash_total % 10000000000ULL),
                       0ULL, (unsigned long long)total_cents, "");
    offset += written;
    line_count++;
    int target_lines = block_count * 10;
    while (line_count < target_lines) {
        written = snprintf(file_out + offset, max_len - offset,
                           "9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\n");
        offset += written;
        line_count++;
    }
    return line_count;
}

int tsfi_cobol_pack_hex(const char *hex_in, uint8_t *comp3_out, size_t max_len) {
    if (!hex_in || !comp3_out) return -1;
    if (hex_in[0] == '0' && (hex_in[1] == 'x' || hex_in[1] == 'X')) {
        hex_in += 2;
    }
    size_t len = strlen(hex_in);
    size_t total_nibbles = len + 1;
    if (total_nibbles % 2 != 0) {
        total_nibbles++;
    }
    size_t needed_bytes = total_nibbles / 2;
    if (needed_bytes > max_len) return -2;
    
    size_t in_idx = 0;
    size_t out_idx = 0;
    
    if (len % 2 == 0) {
        char ch = hex_in[in_idx++];
        uint8_t val = 0;
        if (ch >= '0' && ch <= '9') val = ch - '0';
        else if (ch >= 'a' && ch <= 'f') val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') val = ch - 'A' + 10;
        comp3_out[out_idx++] = val;
    }
    
    while (in_idx < len) {
        char ch1 = hex_in[in_idx++];
        uint8_t val1 = 0;
        if (ch1 >= '0' && ch1 <= '9') val1 = ch1 - '0';
        else if (ch1 >= 'a' && ch1 <= 'f') val1 = ch1 - 'a' + 10;
        else if (ch1 >= 'A' && ch1 <= 'F') val1 = ch1 - 'A' + 10;
        
        if (in_idx < len) {
            char ch2 = hex_in[in_idx++];
            uint8_t val2 = 0;
            if (ch2 >= '0' && ch2 <= '9') val2 = ch2 - '0';
            else if (ch2 >= 'a' && ch2 <= 'f') val2 = ch2 - 'a' + 10;
            else if (ch2 >= 'A' && ch2 <= 'F') val2 = ch2 - 'A' + 10;
            comp3_out[out_idx++] = (val1 << 4) | val2;
        } else {
            comp3_out[out_idx++] = (val1 << 4) | 0x0F;
            return (int)out_idx;
        }
    }
    return -3;
}

int tsfi_cobol_unpack_hex(const uint8_t *comp3_in, size_t comp3_len, char *hex_out, size_t max_len) {
    if (!comp3_in || comp3_len == 0 || !hex_out) return -1;
    uint8_t last_byte = comp3_in[comp3_len - 1];
    if ((last_byte & 0x0F) != 0x0F) return -2;
    
    size_t out_idx = 0;
    const char hex_chars[] = "0123456789abcdef";
    size_t start_idx = 0;
    
    if ((comp3_in[0] >> 4) == 0) {
        if (out_idx + 1 >= max_len) return -3;
        hex_out[out_idx++] = hex_chars[comp3_in[0] & 0x0F];
        start_idx = 1;
    }
    
    for (size_t i = start_idx; i < comp3_len; i++) {
        uint8_t b = comp3_in[i];
        uint8_t n1 = b >> 4;
        uint8_t n2 = b & 0x0F;
        
        if (i == comp3_len - 1) {
            if (out_idx + 1 >= max_len) return -3;
            hex_out[out_idx++] = hex_chars[n1];
        } else {
            if (out_idx + 2 >= max_len) return -3;
            hex_out[out_idx++] = hex_chars[n1];
            hex_out[out_idx++] = hex_chars[n2];
        }
    }
    hex_out[out_idx] = '\0';
    return 0;
}

void tsfi_ac_filter_init(tsfi_ac_filter *filter) {
    if (!filter) return;
    memset(filter, 0, sizeof(tsfi_ac_filter));
    for (int i = 0; i < 10; i++) {
        filter->nodes[0].next_states[i] = -1;
    }
    filter->nodes[0].fail_state = 0;
    filter->nodes[0].match_pattern_idx = -1;
    filter->node_count = 1;
}

int tsfi_ac_filter_add_pattern(tsfi_ac_filter *filter, const char *pattern, int pattern_idx) {
    if (!filter || !pattern || filter->node_count >= 128) return -1;
    int current = 0;
    for (int i = 0; pattern[i] != '\0'; i++) {
        if (pattern[i] < '0' || pattern[i] > '9') continue;
        int digit = pattern[i] - '0';
        if (filter->nodes[current].next_states[digit] == -1) {
            if (filter->node_count >= 128) return -2;
            int next_node = filter->node_count++;
            for (int k = 0; k < 10; k++) {
                filter->nodes[next_node].next_states[k] = -1;
            }
            filter->nodes[next_node].fail_state = 0;
            filter->nodes[next_node].match_pattern_idx = -1;
            filter->nodes[current].next_states[digit] = next_node;
        }
        current = filter->nodes[current].next_states[digit];
    }
    filter->nodes[current].match_pattern_idx = pattern_idx;
    return 0;
}

void tsfi_ac_filter_build(tsfi_ac_filter *filter) {
    if (!filter) return;
    int queue[128];
    int head = 0, tail = 0;
    for (int i = 0; i < 10; i++) {
        int next = filter->nodes[0].next_states[i];
        if (next != -1) {
            filter->nodes[next].fail_state = 0;
            queue[tail++] = next;
        } else {
            filter->nodes[0].next_states[i] = 0;
        }
    }
    while (head < tail) {
        int r = queue[head++];
        for (int i = 0; i < 10; i++) {
            int u = filter->nodes[r].next_states[i];
            if (u != -1 && u != 0) {
                queue[tail++] = u;
                int fail = filter->nodes[r].fail_state;
                while (filter->nodes[fail].next_states[i] == -1) {
                    fail = filter->nodes[fail].fail_state;
                }
                filter->nodes[u].fail_state = filter->nodes[fail].next_states[i];
                if (filter->nodes[filter->nodes[u].fail_state].match_pattern_idx != -1) {
                    filter->nodes[u].match_pattern_idx = filter->nodes[filter->nodes[u].fail_state].match_pattern_idx;
                }
            } else {
                int fail = filter->nodes[r].fail_state;
                filter->nodes[r].next_states[i] = filter->nodes[fail].next_states[i];
            }
        }
    }
}

int tsfi_ac_filter_search(const tsfi_ac_filter *filter, const char *text) {
    if (!filter || !text) return -1;
    int current = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] < '0' || text[i] > '9') continue;
        int digit = text[i] - '0';
        current = filter->nodes[current].next_states[digit];
        if (filter->nodes[current].match_pattern_idx != -1) {
            return filter->nodes[current].match_pattern_idx;
        }
    }
    return -1;
}

