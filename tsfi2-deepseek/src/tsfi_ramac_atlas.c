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
void tsfi_red_black_rails_init(tsfi_red_black_rails *rails) {
    if (!rails) return;
    
    // Initialize Black Rail: PRT Table
    for (int i = 0; i < 10; i++) {
        rails->prt[i].address = 0;
        rails->prt[i].limit = 0;
        rails->prt[i].is_present = 0;
        rails->prt[i].read_only = 0;
    }
    
    // Initialize Red Rail: MCP Scheduler
    tsfi_b5000_mcp_init(&rails->scheduler);
}

int tsfi_red_black_rails_resolve(tsfi_red_black_rails *rails, int task_idx, uint32_t offset, uint8_t *val_out) {
    if (!rails || !val_out || task_idx < 0 || task_idx >= 10) return -1;
    
    // Verify task is currently active in the Red Scheduler
    if (rails->scheduler.active_task_idx != task_idx) {
        return -2;
    }
    
    // Resolve relative offset using the Black Resource Table descriptor
    const tsfi_b5000_descriptor *desc = &rails->prt[task_idx];
    if (!desc->is_present) return -3;
    if (offset >= desc->limit) return -4;
    
    *val_out = (uint8_t)(offset * 3);
    return 0;
}

void tsfi_atlas_one_level_store_init(tsfi_atlas_one_level_store *store) {
    if (!store) return;
    store->page_fault_count = 0;
    for (int i = 0; i < 8; i++) {
        store->page_frame_core[i] = 0;
        store->presence_bits[i] = 0;
        store->dirty_bits[i] = 0;
    }
}

int tsfi_atlas_one_level_store_translate(tsfi_atlas_one_level_store *store, uint32_t virtual_page, int *frame_out) {
    if (!store || !frame_out) return -1;
    
    // Look up in core memory frames
    for (int i = 0; i < 8; i++) {
        if (store->presence_bits[i] && store->page_frame_core[i] == virtual_page) {
            *frame_out = i;
            return 0; // Hit
        }
    }
    
    // Page Fault (not present in core memory)
    store->page_fault_count++;
    return -2; // Miss / Fault
}

int tsfi_atlas_one_level_store_swap(tsfi_atlas_one_level_store *store, uint32_t virtual_page_in, uint32_t virtual_page_out) {
    if (!store) return -1;
    
    // Evict virtual_page_out if present in core frames
    int swap_slot = -1;
    for (int i = 0; i < 8; i++) {
        if (store->presence_bits[i] && store->page_frame_core[i] == virtual_page_out) {
            swap_slot = i;
            break;
        }
    }
    
    // If virtual_page_out not found, find first empty or default to slot 0
    if (swap_slot == -1) {
        for (int i = 0; i < 8; i++) {
            if (!store->presence_bits[i]) {
                swap_slot = i;
                break;
            }
        }
    }
    if (swap_slot == -1) swap_slot = 0; // Evict frame 0 by default
    
    store->page_frame_core[swap_slot] = virtual_page_in;
    store->presence_bits[swap_slot] = 1;
    store->dirty_bits[swap_slot] = 0;
    
    return swap_slot;
}

int tsfi_patrick_gap_validate(const uint8_t *bytecode, int len, tsfi_patrick_gap_report *report) {
    if (!bytecode || len <= 0 || !report) return -1;
    
    report->invalid_opcodes = 0;
    report->unaligned_descriptors = 0;
    report->bounds_violations = 0;
    
    int pc = 0;
    while (pc < len) {
        uint8_t op = bytecode[pc++];
        
        // Check for valid bytecode ranges based on TSFiStrategyVM opcodes
        if (!(op <= 0x24 || (op >= 0x30 && op <= 0x34))) {
            report->invalid_opcodes++;
            continue;
        }
        
        // Handle instructions with operands to check boundaries
        if (op == 0x10 || op == 0x11 || op == 0x1D || op == 0x1E) { // Arithmetic (dst, src)
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst >= 4 || src >= 4) {
                    report->bounds_violations++;
                }
            } else {
                report->bounds_violations++;
                break;
            }
        } else if (op == 0x30 || op == 0x31) { // Stack Push/Pop Register
            if (pc < len) {
                uint8_t reg = bytecode[pc++];
                if (reg >= 4) {
                    report->bounds_violations++;
                }
            } else {
                report->bounds_violations++;
                break;
            }
        }
    }
    return 0;
}

int tsfi_compile_decision_table(const tsfi_decision_table *table, uint8_t *bytecode_out, int max_len) {
    if (!table || !bytecode_out || max_len <= 0) return -1;
    
    int pc = 0;
    for (int i = 0; i < table->rule_count; i++) {
        if (pc + 7 > max_len) return -2;
        
        bytecode_out[pc] = 0x1B; // JNE
        bytecode_out[pc + 1] = (uint8_t)(pc + 7);
        bytecode_out[pc + 2] = (uint8_t)table->condition_reg_a[i];
        bytecode_out[pc + 3] = (uint8_t)table->condition_reg_b[i];
        
        bytecode_out[pc + 4] = 0x14; // SET_REG
        bytecode_out[pc + 5] = (uint8_t)table->action_reg[i];
        bytecode_out[pc + 6] = (uint8_t)table->action_val[i];
        
        pc += 7;
    }
    return pc;
}

void tsfi_atlas_tlb_init(tsfi_atlas_tlb_cache *tlb) {
    if (!tlb) return;
    tlb->clock_counter = 0;
    for (int i = 0; i < 4; i++) {
        tlb->virtual_pages[i] = 0;
        tlb->real_pages[i] = 0;
        tlb->valid_bits[i] = 0;
        tlb->access_timestamp[i] = 0;
    }
}

int tsfi_atlas_tlb_lookup(tsfi_atlas_tlb_cache *tlb, uint32_t virtual_page, uint32_t *real_page_out) {
    if (!tlb || !real_page_out) return -1;
    tlb->clock_counter++;
    for (int i = 0; i < 4; i++) {
        if (tlb->valid_bits[i] && tlb->virtual_pages[i] == virtual_page) {
            tlb->access_timestamp[i] = tlb->clock_counter;
            *real_page_out = tlb->real_pages[i];
            return 0; // Hit
        }
    }
    return -2; // Miss
}

void tsfi_atlas_tlb_insert(tsfi_atlas_tlb_cache *tlb, uint32_t virtual_page, uint32_t real_page) {
    if (!tlb) return;
    tlb->clock_counter++;
    
    // Find empty slot
    for (int i = 0; i < 4; i++) {
        if (!tlb->valid_bits[i]) {
            tlb->virtual_pages[i] = virtual_page;
            tlb->real_pages[i] = real_page;
            tlb->valid_bits[i] = 1;
            tlb->access_timestamp[i] = tlb->clock_counter;
            return;
        }
    }
    
    // Evict LRU slot
    int lru_idx = 0;
    uint32_t min_time = tlb->access_timestamp[0];
    for (int i = 1; i < 4; i++) {
        if (tlb->access_timestamp[i] < min_time) {
            min_time = tlb->access_timestamp[i];
            lru_idx = i;
        }
    }
    
    tlb->virtual_pages[lru_idx] = virtual_page;
    tlb->real_pages[lru_idx] = real_page;
    tlb->valid_bits[lru_idx] = 1;
    tlb->access_timestamp[lru_idx] = tlb->clock_counter;
}

void tsfi_atlas_vm_init(tsfi_atlas_vm *vm) {
    if (!vm) return;
    tsfi_atlas_one_level_store_init(&vm->paging);
    tsfi_atlas_tlb_init(&vm->tlb);
    vm->accumulators[0] = 0;
    vm->accumulators[1] = 0;
    vm->pc = 0;
    vm->extrabcode_triggered = 0;
    vm->extrabcode_val = 0;
}

int tsfi_atlas_vm_step(tsfi_atlas_vm *vm, const uint8_t *bytecode, int len, const uint8_t *backing_store) {
    if (!vm || !bytecode || len <= 0 || !backing_store) return -1;
    
    if (vm->pc >= (uint32_t)len) return -2;
    
    uint8_t op = bytecode[vm->pc++];
    
    if (op == 0x50) { // Load ACC A from virtual address: 0x50 <virtual_page>
        if (vm->pc < (uint32_t)len) {
            uint8_t vpage = bytecode[vm->pc++];
            uint32_t rpage = 0;
            int tlb_res = tsfi_atlas_tlb_lookup(&vm->tlb, vpage, &rpage);
            if (tlb_res != 0) {
                int frame = 0;
                int trans_res = tsfi_atlas_one_level_store_translate(&vm->paging, vpage, &frame);
                if (trans_res != 0) {
                    frame = tsfi_atlas_one_level_store_swap(&vm->paging, vpage, 0);
                }
                rpage = frame * 256;
                tsfi_atlas_tlb_insert(&vm->tlb, vpage, rpage);
            }
            vm->accumulators[0] = backing_store[rpage];
        }
    } else if (op == 0x51) { // Store ACC A to virtual address: 0x51 <virtual_page>
        if (vm->pc < (uint32_t)len) {
            uint8_t vpage = bytecode[vm->pc++];
            uint32_t rpage = 0;
            int tlb_res = tsfi_atlas_tlb_lookup(&vm->tlb, vpage, &rpage);
            if (tlb_res != 0) {
                int frame = 0;
                int trans_res = tsfi_atlas_one_level_store_translate(&vm->paging, vpage, &frame);
                if (trans_res != 0) {
                    frame = tsfi_atlas_one_level_store_swap(&vm->paging, vpage, 0);
                }
                rpage = frame * 256;
                tsfi_atlas_tlb_insert(&vm->tlb, vpage, rpage);
            }
        }
    } else if (op == 0x52) { // Add ACC B to A
        vm->accumulators[0] += vm->accumulators[1];
    } else if (op == 0x5F) { // Extrabcode Software Interrupt Trap: 0x5F <vector>
        if (vm->pc < (uint32_t)len) {
            vm->extrabcode_val = bytecode[vm->pc++];
            vm->extrabcode_triggered = 1;
        }
    }
    
    return 0;
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

uint64_t tsfi_cdc3600_extract_byte(uint64_t word, int bit_position, int byte_length) {
    if (byte_length <= 0 || byte_length > 64 || bit_position < 0 || bit_position >= 64) return 0;
    uint64_t mask = (byte_length == 64) ? ~0ULL : ((1ULL << byte_length) - 1);
    return (word >> bit_position) & mask;
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

