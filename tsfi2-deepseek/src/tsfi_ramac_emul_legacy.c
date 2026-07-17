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
int tsfi_s370_philco212_decode(uint64_t raw_word, tsfi_philco212_instruction *inst_left, tsfi_philco212_instruction *inst_right) {
    if (!inst_left || !inst_right) return -1;

    uint64_t clean_word = raw_word & 0xFFFFFFFFFFFFULL;

    uint32_t left_val = (clean_word >> 24) & 0xFFFFFF;
    inst_left->opcode = (left_val >> 16) & 0xFF;
    inst_left->index_reg = (left_val >> 13) & 0x07;
    inst_left->mod_mode = (left_val >> 11) & 0x03;
    inst_left->address = left_val & 0x7FF;

    uint32_t right_val = clean_word & 0xFFFFFF;
    inst_right->opcode = (right_val >> 16) & 0xFF;
    inst_right->index_reg = (right_val >> 13) & 0x07;
    inst_right->mod_mode = (right_val >> 11) & 0x03;
    inst_right->address = right_val & 0x7FF;

    return 0;
}

int tsfi_s370_philco212_modify_address(tsfi_philco212_instruction *inst, int *index_registers, int index_reg_count, uint16_t *out_modified_address) {
    if (!inst || !index_registers || !out_modified_address) return -1;

    if (inst->index_reg >= index_reg_count) {
        return -1;
    }

    if (inst->index_reg == 0) {
        *out_modified_address = inst->address;
        return 0;
    }

    int index_val = index_registers[inst->index_reg];

    switch (inst->mod_mode) {
        case 0:
            *out_modified_address = (inst->address + index_val) & 0x7FF;
            break;

        case 1:
            *out_modified_address = (inst->address + index_val) & 0x7FF;
            index_registers[inst->index_reg] = (index_val + 1) & 0x7FF;
            break;

        case 2:
            *out_modified_address = (inst->address + index_val) & 0x7FF;
            index_registers[inst->index_reg] = (index_val - 1) & 0x7FF;
            break;

        case 3:
            index_val = (index_val + 1) & 0x7FF;
            index_registers[inst->index_reg] = index_val;
            *out_modified_address = (inst->address + index_val) & 0x7FF;
            break;
    }

    return 0;
}

int tsfi_s370_bendixg20_decode(uint32_t raw_word, tsfi_bendixg20_instruction *inst) {
    if (!inst) return -1;

    inst->opcode = (raw_word >> 26) & 0x3F;
    inst->index_reg = (raw_word >> 20) & 0x3F;
    inst->address = (raw_word >> 5) & 0x7FFF;
    inst->flags = raw_word & 0x1F;

    return 0;
}

int tsfi_s370_bendixg20_resolve_address(const tsfi_bendixg20_instruction *inst, const int *memory_pool, int mem_size, uint32_t *out_effective_address) {
    if (!inst || !out_effective_address) return -1;

    if (inst->index_reg == 0) {
        *out_effective_address = inst->address;
        return 0;
    }

    if (inst->index_reg >= mem_size) {
        return -1;
    }

    if (!memory_pool) return -1;

    int offset_val = memory_pool[inst->index_reg];
    *out_effective_address = (inst->address + offset_val) & 0x7FFF;

    return 0;
}

int tsfi_s370_bendixg15_dda_tick(tsfi_bendixg15_dda_integrator *integrators, int count) {
    if (!integrators || count <= 0) return -1;

    int *prev_outputs = (int *)malloc(count * sizeof(int));
    if (!prev_outputs) return -1;

    for (int i = 0; i < count; i++) {
        prev_outputs[i] = integrators[i].output_dz;
    }

    for (int i = 0; i < count; i++) {
        int dx = 0;
        if (integrators[i].src_dx_integrator == -1) {
            dx = 1;
        } else {
            int src_idx = integrators[i].src_dx_integrator;
            if (src_idx >= 0 && src_idx < count) {
                dx = prev_outputs[src_idx];
            }
        }

        int dy = 0;
        if (integrators[i].src_dy_integrator != -1) {
            int src_idx = integrators[i].src_dy_integrator;
            if (src_idx >= 0 && src_idx < count) {
                dy = prev_outputs[src_idx];
            }
        }

        if (integrators[i].dy_invert) {
            dy = -dy;
        }

        integrators[i].y += dy;

        if (dx != 0) {
            integrators[i].r += integrators[i].y * dx;

            if (integrators[i].r >= integrators[i].limit) {
                integrators[i].r -= integrators[i].limit;
                integrators[i].output_dz = 1;
            } else if (integrators[i].r <= -integrators[i].limit) {
                integrators[i].r += integrators[i].limit;
                integrators[i].output_dz = -1;
            } else {
                integrators[i].output_dz = 0;
            }
        } else {
            integrators[i].output_dz = 0;
        }
    }

    free(prev_outputs);
    return 0;
}

int tsfi_s370_lgp30_interpreter(int *memory, int mem_size, int *accumulator, int *pc, int max_steps) {
    if (!memory || mem_size <= 0 || !accumulator || !pc || max_steps <= 0) {
        return -1;
    }

    int steps = 0;
    int halted = 0;

    while (*pc >= 0 && *pc < mem_size && steps < max_steps && !halted) {
        int inst = memory[*pc];
        steps++;

        int opcode = (inst >> 20) & 0x0F;
        int addr = inst & 0x0FFF;

        if (addr >= mem_size) {
            return -2;
        }

        int next_pc = *pc + 1;

        switch (opcode) {
            case 0:
                *accumulator = memory[addr];
                break;

            case 1:
                memory[addr] = *accumulator;
                *accumulator = 0;
                break;

            case 2:
                *accumulator += memory[addr];
                break;

            case 3:
                *accumulator -= memory[addr];
                break;

            case 4:
                *accumulator *= memory[addr];
                break;

            case 5:
                if (memory[addr] != 0) {
                    *accumulator /= memory[addr];
                } else {
                    return -3;
                }
                break;

            case 6:
                *accumulator &= memory[addr];
                break;

            case 7:
                next_pc = addr;
                break;

            case 8:
                if (*accumulator < 0) {
                    next_pc = addr;
                }
                break;

            case 9:
                memory[addr] = *accumulator;
                break;

            case 10:
                memory[addr] = (memory[addr] & 0xFFFFF000) | (*accumulator & 0x00000FFF);
                break;

            case 11:
                halted = 1;
                break;

            default:
                break;
        }

        *pc = next_pc;
    }

    return steps;
}

int tsfi_s370_ibm7030_read_bits(const uint64_t *memory, uint32_t bit_address, int bit_length, uint64_t *out_val) {
    if (!memory || !out_val || bit_length <= 0 || bit_length > 64) {
        return -1;
    }

    uint32_t word_idx = bit_address / 64;
    uint32_t offset = bit_address % 64;

    uint64_t mask = (bit_length == 64) ? ~0ULL : ((1ULL << bit_length) - 1);

    if (offset + bit_length <= 64) {
        *out_val = (memory[word_idx] >> offset) & mask;
    } else {
        uint32_t bits1 = 64 - offset;
        uint32_t bits2 = bit_length - bits1;

        uint64_t val1 = (memory[word_idx] >> offset) & ((1ULL << bits1) - 1);
        uint64_t val2 = memory[word_idx + 1] & ((1ULL << bits2) - 1);

        *out_val = val1 | (val2 << bits1);
    }

    return 0;
}

int tsfi_s370_ibm7030_write_bits(uint64_t *memory, uint32_t bit_address, int bit_length, uint64_t val) {
    if (!memory || bit_length <= 0 || bit_length > 64) {
        return -1;
    }

    uint32_t word_idx = bit_address / 64;
    uint32_t offset = bit_address % 64;

    uint64_t mask = (bit_length == 64) ? ~0ULL : ((1ULL << bit_length) - 1);
    val &= mask;

    if (offset + bit_length <= 64) {
        uint64_t write_mask = mask << offset;
        memory[word_idx] = (memory[word_idx] & ~write_mask) | (val << offset);
    } else {
        uint32_t bits1 = 64 - offset;
        uint32_t bits2 = bit_length - bits1;

        uint64_t mask1 = ((1ULL << bits1) - 1) << offset;
        uint64_t mask2 = (1ULL << bits2) - 1;

        uint64_t val1 = val & ((1ULL << bits1) - 1);
        uint64_t val2 = (val >> bits1) & mask2;

        memory[word_idx] = (memory[word_idx] & ~mask1) | (val1 << offset);
        memory[word_idx + 1] = (memory[word_idx + 1] & ~mask2) | val2;
    }

    return 0;
}

static void init_ecc_positions(int *data_to_pos, int *pos_to_data) {
    int data_idx = 0;
    for (int i = 1; i <= 63; i++) {
        if ((i & (i - 1)) == 0) {
            pos_to_data[i] = -1;
        } else {
            data_to_pos[data_idx] = i;
            pos_to_data[i] = data_idx;
            data_idx++;
        }
    }
}

uint64_t tsfi_s370_ibm7030_ecc_encode(uint64_t data) {
    int data_to_pos[56];
    int pos_to_data[64];
    init_ecc_positions(data_to_pos, pos_to_data);

    uint64_t code_word = 0;
    for (int i = 0; i < 56; i++) {
        if ((data >> i) & 1) {
            code_word |= (1ULL << data_to_pos[i]);
        }
    }

    for (int j = 0; j < 6; j++) {
        int parity_pos = 1 << j;
        int parity_val = 0;
        for (int i = 1; i <= 63; i++) {
            if (i & parity_pos) {
                if ((code_word >> i) & 1) {
                    parity_val ^= 1;
                }
            }
        }
        if (parity_val) {
            code_word |= (1ULL << parity_pos);
        }
    }

    int overall_parity = 0;
    for (int i = 1; i <= 63; i++) {
        if ((code_word >> i) & 1) {
            overall_parity ^= 1;
        }
    }
    if (overall_parity) {
        code_word |= (1ULL << 0);
    }

    return code_word;
}

int tsfi_s370_ibm7030_ecc_decode(uint64_t word_64, uint64_t *out_corrected_data) {
    if (!out_corrected_data) return -1;

    int data_to_pos[56];
    int pos_to_data[64];
    init_ecc_positions(data_to_pos, pos_to_data);

    int overall_parity = 0;
    for (int i = 0; i <= 63; i++) {
        if ((word_64 >> i) & 1) {
            overall_parity ^= 1;
        }
    }

    int syndrome = 0;
    for (int j = 0; j < 6; j++) {
        int parity_pos = 1 << j;
        int parity_val = 0;
        for (int i = 1; i <= 63; i++) {
            if (i & parity_pos) {
                if ((word_64 >> i) & 1) {
                    parity_val ^= 1;
                }
            }
        }
        if (parity_val) {
            syndrome |= parity_pos;
        }
    }

    if (syndrome == 0) {
        if (overall_parity == 0) {
            uint64_t data = 0;
            for (int i = 0; i < 56; i++) {
                if ((word_64 >> data_to_pos[i]) & 1) {
                    data |= (1ULL << i);
                }
            }
            *out_corrected_data = data;
            return 0;
        } else {
            uint64_t data = 0;
            for (int i = 0; i < 56; i++) {
                if ((word_64 >> data_to_pos[i]) & 1) {
                    data |= (1ULL << i);
                }
            }
            *out_corrected_data = data;
            return 1;
        }
    } else {
        if (overall_parity != 0) {
            if (syndrome <= 63) {
                word_64 ^= (1ULL << syndrome);
            }
            uint64_t data = 0;
            for (int i = 0; i < 56; i++) {
                if ((word_64 >> data_to_pos[i]) & 1) {
                    data |= (1ULL << i);
                }
            }
            *out_corrected_data = data;
            return 1;
        } else {
            return 2;
        }
    }
}

int tsfi_s370_ibm7030_index_modify(tsfi_ibm7030_index_reg *reg, int increment_step, uint64_t *indicator_register, uint32_t *out_offset) {
    if (!reg || !indicator_register || !out_offset) return -1;

    // 1. Output the current index value offset
    *out_offset = (uint32_t)(reg->value & 0x1FFFFFFULL);

    // 2. Modify value by step (wrapping to 25 bits)
    reg->value = (reg->value + increment_step) & 0x1FFFFFFULL;

    // 3. Decrement down-counter if non-zero
    if (reg->count > 0) {
        reg->count--;
    }

    // 4. Update Indicators
    if (reg->value == 0) {
        *indicator_register |= (1ULL << 0); // Value Zero indicator
    }
    if (reg->count == 0) {
        *indicator_register |= (1ULL << 1); // Count Zero indicator
    }
    if (reg->value >= reg->limit) {
        *indicator_register |= (1ULL << 2); // Limit Exceeded indicator
    }

    // Return -1 if count zero or limit exceeded indicators were tripped
    if (reg->count == 0 || reg->value >= reg->limit) {
        return 1; // Trip condition
    }

    return 0;
}

void tsfi_s370_ibm7030_lau_init(tsfi_ibm7030_lau_queue *queue) {
    if (!queue) return;
    memset(queue->entries, 0, sizeof(queue->entries));
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

int tsfi_s370_ibm7030_lau_push_load(tsfi_ibm7030_lau_queue *queue, uint32_t address) {
    if (!queue || queue->count >= 8) return -1;

    int tail = queue->tail;
    queue->entries[tail].address = address;
    queue->entries[tail].value = 0;
    queue->entries[tail].is_store = 0;
    queue->entries[tail].is_pending = 1;
    queue->entries[tail].forwarded = 0;

    queue->tail = (tail + 1) % 8;
    queue->count++;
    return 0;
}

int tsfi_s370_ibm7030_lau_push_store(tsfi_ibm7030_lau_queue *queue, uint32_t address, uint64_t val) {
    if (!queue || queue->count >= 8) return -1;

    for (int idx = 0; idx < 8; idx++) {
        if (queue->entries[idx].is_pending && queue->entries[idx].is_store == 0 && queue->entries[idx].address == address) {
            queue->entries[idx].value = val;
            queue->entries[idx].forwarded = 1;
        }
    }

    int tail = queue->tail;
    queue->entries[tail].address = address;
    queue->entries[tail].value = val;
    queue->entries[tail].is_store = 1;
    queue->entries[tail].is_pending = 1;
    queue->entries[tail].forwarded = 0;

    queue->tail = (tail + 1) % 8;
    queue->count++;
    return 0;
}

int tsfi_s370_ibm7030_lau_commit(tsfi_ibm7030_lau_queue *queue, uint64_t *memory, int mem_size) {
    if (!queue || !memory || queue->count == 0) return 0;

    int commits = 0;
    int head = queue->head;

    while (queue->count > 0) {
        tsfi_ibm7030_lau_entry *entry = &queue->entries[head];
        if (entry->is_pending) {
            uint32_t word_addr = entry->address;
            if (word_addr >= (uint32_t)mem_size) {
                return -1;
            }

            if (entry->is_store) {
                memory[word_addr] = entry->value;
            } else {
                if (!entry->forwarded) {
                    entry->value = memory[word_addr];
                }
            }
            entry->is_pending = 0;
            commits++;
        }
        head = (head + 1) % 8;
        queue->count--;
    }
    queue->head = head;
    return commits;
}

int tsfi_s370_ibm7030_vfl_add(const uint64_t *memory, 
                              uint32_t bit_addr_a, int len_a, int byte_size_a,
                              uint32_t bit_addr_b, int len_b, int byte_size_b,
                              uint64_t *out_val) {
    if (!memory || !out_val || len_a <= 0 || byte_size_a <= 0 || len_b <= 0 || byte_size_b <= 0) {
        return -1;
    }

    int bit_len_a = len_a * byte_size_a;
    int bit_len_b = len_b * byte_size_b;

    if (bit_len_a > 64 || bit_len_b > 64) {
        return -2;
    }

    uint64_t val_a = 0;
    uint64_t val_b = 0;

    if (tsfi_s370_ibm7030_read_bits(memory, bit_addr_a, bit_len_a, &val_a) != 0) return -3;
    if (tsfi_s370_ibm7030_read_bits(memory, bit_addr_b, bit_len_b, &val_b) != 0) return -3;

    *out_val = val_a + val_b;
    return 0;
}

int tsfi_s370_ibm7030_vfl_logic(const uint64_t *memory, 
                                uint32_t bit_addr_a, int len_a, int byte_size_a,
                                uint32_t bit_addr_b, int len_b, int byte_size_b,
                                const char *op, uint64_t *out_val) {
    if (!memory || !op || !out_val || len_a <= 0 || byte_size_a <= 0 || len_b <= 0 || byte_size_b <= 0) {
        return -1;
    }

    int bit_len_a = len_a * byte_size_a;
    int bit_len_b = len_b * byte_size_b;

    if (bit_len_a > 64 || bit_len_b > 64) {
        return -2;
    }

    uint64_t val_a = 0;
    uint64_t val_b = 0;

    if (tsfi_s370_ibm7030_read_bits(memory, bit_addr_a, bit_len_a, &val_a) != 0) return -3;
    if (tsfi_s370_ibm7030_read_bits(memory, bit_addr_b, bit_len_b, &val_b) != 0) return -3;

    if (strcmp(op, "AND") == 0) {
        *out_val = val_a & val_b;
    } else if (strcmp(op, "OR") == 0) {
        *out_val = val_a | val_b;
    } else if (strcmp(op, "XOR") == 0) {
        *out_val = val_a ^ val_b;
    } else {
        return -4;
    }

    return 0;
}

void tsfi_s370_honeywell800_init(tsfi_honeywell800_scheduler *sched) {
    if (!sched) return;
    memset(sched->threads, 0, sizeof(sched->threads));
    sched->current_thread_idx = 0;
}

int tsfi_s370_honeywell800_tick(tsfi_honeywell800_scheduler *sched, int *memory, int mem_size) {
    if (!sched || !memory || mem_size <= 0) return -1;

    int tid = -1;
    for (int i = 0; i < 8; i++) {
        int candidate = (sched->current_thread_idx + i) % 8;
        if (sched->threads[candidate].is_active) {
            tid = candidate;
            break;
        }
    }

    if (tid == -1) {
        return -2; // No active threads
    }

    tsfi_honeywell800_thread *thread = &sched->threads[tid];
    if (thread->pc < 0 || thread->pc >= mem_size) {
        thread->is_active = 0;
        return -3;
    }

    int instruction = memory[thread->pc];
    int opcode = (instruction >> 24) & 0xFF;
    int address = instruction & 0xFFFFFF;

    thread->pc++;

    switch (opcode) {
        case 0x01: // LOAD
            if (address >= 0 && address < mem_size) {
                thread->accumulator = memory[address];
            }
            break;
        case 0x02: // ADD
            if (address >= 0 && address < mem_size) {
                thread->accumulator += memory[address];
            }
            break;
        case 0x03: // STORE
            if (address >= 0 && address < mem_size) {
                memory[address] = (int)thread->accumulator;
            }
            break;
        case 0x04: // HALT
            thread->is_active = 0;
            break;
        default:
            break;
    }

    sched->current_thread_idx = (tid + 1) % 8;
    return tid;
}

uint64_t tsfi_s370_cdc1604_subtractive_add(uint64_t a, uint64_t b, int bit_width) {
    if (bit_width <= 0 || bit_width > 64) return 0;

    uint64_t mask = (bit_width == 64) ? ~0ULL : ((1ULL << bit_width) - 1);
    uint64_t sum = (a & mask) + (b & mask);

    // End-around carry implementation for 1's complement subtractive adder
    uint64_t carry = (bit_width == 64) ? 0 : (sum >> bit_width);
    sum = (sum & mask) + carry;

    // Handle secondary carry if triggered
    carry = (bit_width == 64) ? 0 : (sum >> bit_width);
    sum = (sum & mask) + carry;

    return sum & mask;
}

int tsfi_s370_cdc1604_resolve_address(const int *memory, int mem_size, 
                                      uint16_t base_address, int index_reg_idx, 
                                      const int *index_registers, int indirect_flag, 
                                      uint32_t *out_effective_address) {
    if (!out_effective_address) return -1;

    uint32_t addr = base_address;

    if (index_registers && index_reg_idx >= 1 && index_reg_idx <= 6) {
        addr = (addr + index_registers[index_reg_idx]) & 0x7FFF;
    }

    if (indirect_flag) {
        if (!memory || (int)addr >= mem_size || (int)addr < 0) {
            return -2;
        }
        addr = (uint32_t)(memory[addr] & 0x7FFF);
    }

    *out_effective_address = addr;
    return 0;
}

uint64_t tsfi_s370_cdc1604_subtractive_multiply(uint64_t a, uint64_t b, int bit_width) {
    if (bit_width <= 0 || bit_width > 64) return 0;

    uint64_t mask = (bit_width == 64) ? ~0ULL : ((1ULL << bit_width) - 1);
    uint64_t sign_bit = 1ULL << (bit_width - 1);

    int sign_a = (a & sign_bit) ? 1 : 0;
    int sign_b = (b & sign_bit) ? 1 : 0;

    uint64_t val_a = sign_a ? ((~a) & mask) : a;
    uint64_t val_b = sign_b ? ((~b) & mask) : b;

    uint64_t prod = val_a * val_b;

    uint64_t carry = (bit_width == 64) ? 0 : (prod >> bit_width);
    prod = (prod & mask) + carry;

    carry = (bit_width == 64) ? 0 : (prod >> bit_width);
    prod = (prod & mask) + carry;

    if (sign_a ^ sign_b) {
        prod = (~prod) & mask;
    }

    return prod & mask;
}

int tsfi_s370_honeywell800_tick_ramac(tsfi_honeywell800_scheduler *sched, int *memory, int mem_size,
                                      tsfi_ibm7030_lau_queue *queue, uint64_t *ramac_platter, int platter_size) {
    if (!sched || !memory || mem_size <= 0) return -1;

    int tid = -1;
    for (int i = 0; i < 8; i++) {
        int candidate = (sched->current_thread_idx + i) % 8;
        if (sched->threads[candidate].is_active) {
            tid = candidate;
            break;
        }
    }

    if (tid == -1) {
        return -2;
    }

    tsfi_honeywell800_thread *thread = &sched->threads[tid];
    if (thread->pc < 0 || thread->pc >= mem_size) {
        thread->is_active = 0;
        return -3;
    }

    int instruction = memory[thread->pc];
    int opcode = (instruction >> 24) & 0xFF;
    int address = instruction & 0xFFFFFF;

    thread->pc++;

    switch (opcode) {
        case 0x01: // LOAD
            if (address >= 0 && address < mem_size) {
                thread->accumulator = memory[address];
            }
            break;
        case 0x02: // ADD
            if (address >= 0 && address < mem_size) {
                thread->accumulator += memory[address];
            }
            break;
        case 0x03: // STORE
            if (address >= 0 && address < mem_size) {
                memory[address] = (int)thread->accumulator;
            }
            break;
        case 0x04: // HALT
            thread->is_active = 0;
            break;
        case 0x05: // RAMAC_READ
            {
                uint64_t data_val = 0;
                int ret = tsfi_s370_ramac_controller_exec(queue, ramac_platter, platter_size, address, 0, &data_val);
                if (ret == 0) {
                    thread->accumulator = (int64_t)data_val;
                }
            }
            break;
        case 0x06: // RAMAC_WRITE
            {
                uint64_t data_val = (uint64_t)thread->accumulator;
                tsfi_s370_ramac_controller_exec(queue, ramac_platter, platter_size, address, 1, &data_val);
            }
            break;
        default:
            break;
    }

    sched->current_thread_idx = (tid + 1) % 8;
    return tid;
}

int tsfi_s370_rca501_to_univac2(const char *rca_data, char *univac_data_out) {
    if (!rca_data || !univac_data_out) return -1;

    int i = 0;
    while (i < 12 && rca_data[i] != '\0' && rca_data[i] != '$') {
        univac_data_out[i] = rca_data[i];
        i++;
    }

    // Pad the remaining of the 12-byte UNIVAC word with space characters
    while (i < 12) {
        univac_data_out[i] = ' ';
        i++;
    }
    univac_data_out[12] = '\0';
    return 0;
}

int tsfi_s370_rca501_card_punch(const char *text, uint16_t *card_columns_out, int max_cols) {
    if (!text || !card_columns_out || max_cols <= 0) return -1;

    int len = strlen(text);
    int cols = len < max_cols ? len : max_cols;

    for (int i = 0; i < cols; i++) {
        char c = text[i];
        uint16_t mask = 0;

        if (c >= '0' && c <= '9') {
            mask = (1 << (c - '0'));
        } else if (c >= 'A' && c <= 'I') {
            mask = (1 << 10) | (1 << (c - 'A' + 1));
        } else if (c >= 'J' && c <= 'R') {
            mask = (1 << 11) | (1 << (c - 'J' + 1));
        } else if (c >= 'S' && c <= 'Z') {
            mask = (1 << 0) | (1 << (c - 'S' + 2));
        } else if (c == '$') {
            mask = (1 << 11) | (1 << 3) | (1 << 8);
        } else if (c == ' ') {
            mask = 0;
        } else {
            mask = (1 << 8) | (1 << 4);
        }
        card_columns_out[i] = mask;
    }
    return cols;
}

int tsfi_s370_rca501_card_read(const uint16_t *card_columns, int col_count, char *text_out, int max_len) {
    if (!card_columns || col_count <= 0 || !text_out || max_len <= 0) return -1;

    int cols = col_count < max_len - 1 ? col_count : max_len - 1;

    for (int i = 0; i < cols; i++) {
        uint16_t mask = card_columns[i];
        char c = ' ';

        if (mask == 0) {
            c = ' ';
        } else if (mask == ((1 << 11) | (1 << 3) | (1 << 8))) {
            c = '$';
        } else if (mask & (1 << 10)) {
            for (int r = 1; r <= 9; r++) {
                if (mask == ((1 << 10) | (1 << r))) {
                    c = 'A' + r - 1;
                    break;
                }
            }
        } else if (mask & (1 << 11)) {
            for (int r = 1; r <= 9; r++) {
                if (mask == ((1 << 11) | (1 << r))) {
                    c = 'J' + r - 1;
                    break;
                }
            }
        } else if (mask & (1 << 0)) {
            for (int r = 2; r <= 9; r++) {
                if (mask == ((1 << 0) | (1 << r))) {
                    c = 'S' + r - 2;
                    break;
                }
            }
        } else {
            for (int r = 0; r <= 9; r++) {
                if (mask == (1 << r)) {
                    c = '0' + r;
                    break;
                }
            }
        }
        text_out[i] = c;
    }
    text_out[cols] = '\0';
    return cols;
}

int tsfi_s370_charles_adams_stored_logic_report(char *report_out, int max_len) {
    if (!report_out || max_len < 256) return -1;

    const char *report_text = 
        "=== DATAMATION MARCH 1961 REPORT ===\n"
        "Design Trends: Stored Logic Computers\n"
        "Prediction: 'Not Hardware, but Logic'\n"
        "Author: Charles W. Adams\n"
        "Summary: Dynamic microprogramming allows\n"
        "reconfigurable logic instruction paths.\n";

    strncpy(report_out, report_text, max_len - 1);
    report_out[max_len - 1] = '\0';
    return 0;
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

