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

tsfi_ramac_chs tsfi_ramac_index_to_chs(int index) {
    tsfi_ramac_chs chs;
    int words_per_sector = RAMAC_WORDS;
    int words_per_track = RAMAC_SECTORS * words_per_sector;
    int words_per_cylinder = RAMAC_HEADS * words_per_track;

    chs.cylinder = index / words_per_cylinder;
    int rem = index % words_per_cylinder;
    chs.head = rem / words_per_track;
    rem = rem % words_per_track;
    chs.sector = rem / words_per_sector;
    chs.word_offset = rem % words_per_sector;

    return chs;
}

int tsfi_ramac_chs_to_index(tsfi_ramac_chs chs) {
    int words_per_sector = RAMAC_WORDS;
    int words_per_track = RAMAC_SECTORS * words_per_sector;
    int words_per_cylinder = RAMAC_HEADS * words_per_track;

    return chs.cylinder * words_per_cylinder +
           chs.head * words_per_track +
           chs.sector * words_per_sector +
           chs.word_offset;
}

double tsfi_ramac_calculate_seek(int from_index, int to_index) {
    tsfi_ramac_chs c1 = tsfi_ramac_index_to_chs(from_index);
    tsfi_ramac_chs c2 = tsfi_ramac_index_to_chs(to_index);

    double cylinder_seek = abs(c1.cylinder - c2.cylinder) * 1.5;
    double head_swap = (c1.head != c2.head) ? 0.8 : 0.0;
    double rotational_delay = abs(c1.sector - c2.sector) * 5.0;

    return (cylinder_seek + head_swap + rotational_delay) * 1000.0; // Return in microseconds
}

int tsfi_ramac_layout_optimize(tsfi_dat *dat, const char *filepath) {
    if (!dat || !filepath) return -1;

    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    fwrite("RMAC", 1, 4, fp);

    int capacity = dat->capacity;
    fwrite(&capacity, sizeof(int), 1, fp);

    fwrite(dat->base, sizeof(int), capacity, fp);
    fwrite(dat->check, sizeof(int), capacity, fp);

    fclose(fp);
    return 0;
}

int tsfi_ramac_hash_key(const char *key, int cylinder) {
    unsigned int hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    int primary_slots = 45 * RAMAC_SECTORS;
    int slot = hash % primary_slots;
    
    int head = slot / RAMAC_SECTORS;
    int sector = slot % RAMAC_SECTORS;

    tsfi_ramac_chs chs;
    chs.cylinder = cylinder;
    chs.head = head;
    chs.sector = sector;
    chs.word_offset = 0;

    return tsfi_ramac_chs_to_index(chs);
}

int tsfi_ramac_insert_record(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder, double *out_total_seek_us) {
    int primary_idx = tsfi_ramac_hash_key(key, cylinder);
    int current_idx = primary_idx;
    int last_idx = -1;
    double seek_time = 0.0;
    int current_head = 0;

    while (disk[current_idx].is_active) {
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        current_head = current_idx;

        if (strcmp(disk[current_idx].key, key) == 0) {
            strcpy(disk[current_idx].value, value);
            if (out_total_seek_us) *out_total_seek_us = seek_time;
            return current_idx;
        }
        last_idx = current_idx;
        if (disk[current_idx].next_overflow_index == -1) {
            break;
        }
        current_idx = disk[current_idx].next_overflow_index;
    }

    if (!disk[current_idx].is_active) {
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        strcpy(disk[current_idx].key, key);
        strcpy(disk[current_idx].value, value);
        disk[current_idx].is_active = 1;
        disk[current_idx].next_overflow_index = -1;
        if (out_total_seek_us) *out_total_seek_us = seek_time;
        return current_idx;
    }

    tsfi_ramac_chs overflow_chs;
    overflow_chs.cylinder = cylinder;
    overflow_chs.word_offset = 0;

    int found_slot = -1;
    for (int h = 45; h < 50; h++) {
        overflow_chs.head = h;
        for (int s = 0; s < RAMAC_SECTORS; s++) {
            overflow_chs.sector = s;
            int test_idx = tsfi_ramac_chs_to_index(overflow_chs);
            if (!disk[test_idx].is_active) {
                found_slot = test_idx;
                break;
            }
        }
        if (found_slot != -1) break;
    }

    if (found_slot == -1) {
        return -1;
    }

    seek_time += tsfi_ramac_calculate_seek(current_head, found_slot);
    strcpy(disk[found_slot].key, key);
    strcpy(disk[found_slot].value, value);
    disk[found_slot].is_active = 1;
    disk[found_slot].next_overflow_index = -1;

    disk[last_idx].next_overflow_index = found_slot;

    if (out_total_seek_us) *out_total_seek_us = seek_time;
    return found_slot;
}

const char* tsfi_ramac_search_record(tsfi_ramac_record *disk, const char *key, int cylinder, double *out_total_seek_us) {
    int primary_idx = tsfi_ramac_hash_key(key, cylinder);
    int current_idx = primary_idx;
    double seek_time = 0.0;
    int current_head = 0;

    while (current_idx != -1 && disk[current_idx].is_active) {
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        current_head = current_idx;

        if (strcmp(disk[current_idx].key, key) == 0) {
            if (out_total_seek_us) *out_total_seek_us = seek_time;
            return disk[current_idx].value;
        }
        current_idx = disk[current_idx].next_overflow_index;
    }

    if (out_total_seek_us) *out_total_seek_us = seek_time;
    return NULL;
}

int tsfi_ramac_plugboard_route(const char *wiring, const uint8_t *src, uint8_t *dest, int max_len) {
    if (!wiring || !src || !dest) return -1;
    int src_s = 0, src_e = 0, dest_s = 0, dest_e = 0;
    if (sscanf(wiring, "%d..%d->%d..%d", &src_s, &src_e, &dest_s, &dest_e) != 4) {
        return -1;
    }

    if (src_s < 0 || src_e >= max_len || src_s > src_e) return -1;
    if (dest_s < 0 || dest_e >= max_len || dest_s > dest_e) return -1;

    int copy_len = src_e - src_s + 1;
    int dest_len = dest_e - dest_s + 1;
    int size_to_copy = (copy_len < dest_len) ? copy_len : dest_len;

    memcpy(dest + dest_s, src + src_s, size_to_copy);
    return size_to_copy;
}

int tsfi_ramac_write_verified(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder) {
    double temp_seek = 0.0;
    int write_idx = tsfi_ramac_insert_record(disk, key, value, cylinder, &temp_seek);
    if (write_idx == -1) return -1;

    const char *read_val = tsfi_ramac_search_record(disk, key, cylinder, &temp_seek);
    if (!read_val || strcmp(read_val, value) != 0) {
        return -1;
    }

    return 0;
}

void tsfi_ramac_acc_init(tsfi_ramac_acc_model *model) {
    if (!model) return;
    memset(model->accumulators, 0, sizeof(model->accumulators));
    model->isolation_trap = 0;
    model->trap_active = 0;
}

int tsfi_ramac_acc_add(tsfi_ramac_acc_model *model, int acc_id, int64_t val) {
    if (!model || acc_id < 0 || acc_id >= 10) return -1;
    model->accumulators[acc_id] += val;
    return 0;
}

int tsfi_ramac_acc_div(tsfi_ramac_acc_model *model, int acc_id, int64_t val) {
    if (!model || acc_id < 0 || acc_id >= 10) return -1;

    if (val == 0) {
        model->isolation_trap = model->accumulators[acc_id];
        model->trap_active = 1;
        return -1; 
    }

    model->accumulators[acc_id] /= val;
    return 0;
}

int tsfi_ramac_inquiry_station(tsfi_ramac_record *disk, const char *command, char *response_out, int max_len) {
    if (!disk || !command || !response_out) return -1;
    
    char cmd[16] = {0};
    char arg1[32] = {0};
    char arg2[32] = {0};

    int parsed = sscanf(command, "%15s %31s %31s", cmd, arg1, arg2);
    if (parsed <= 0) return -1;

    if (strcmp(cmd, "QRY") == 0) {
        if (parsed < 2) return -1;
        double seek = 0.0;
        const char *val = tsfi_ramac_search_record(disk, arg1, 5, &seek);
        if (val) {
            snprintf(response_out, max_len, "KEY: %s VAL: %s SEEK: %.1f us", arg1, val, seek);
        } else {
            snprintf(response_out, max_len, "KEY: %s STATUS: NOT_FOUND", arg1);
        }
        return 0;
    }

    if (strcmp(cmd, "WRT") == 0) {
        if (parsed < 3) return -1;
        double seek = 0.0;
        int idx = tsfi_ramac_insert_record(disk, arg1, arg2, 5, &seek);
        if (idx != -1) {
            snprintf(response_out, max_len, "WRITE_SUCCESS INDEX: %d SEEK: %.1f us", idx, seek);
        } else {
            snprintf(response_out, max_len, "WRITE_FAILED (CYLINDER FULL)");
        }
        return 0;
    }

    if (strcmp(cmd, "PRT") == 0) {
        if (parsed < 2) return -1;
        int parity_ok = tsfi_ramac_check_parity(arg1);
        if (parity_ok) {
            snprintf(response_out, max_len, "PARITY CHECK: PASS");
        } else {
            snprintf(response_out, max_len, "PARITY CHECK: FAIL");
        }
        return 0;
    }

    return -1;
}

int tsfi_ramac_check_parity(const char *str) {
    if (!str) return 0;
    while (*str) {
        unsigned char c = *str++;
        int total_bits = 0;
        for (int b = 0; b < 8; b++) {
            if ((c >> b) & 1) total_bits++;
        }
        if (total_bits % 2 == 0) {
            return 0;
        }
    }
    return 1;
}

int tsfi_ramac_alu_exec(tsfi_ramac_acc_model *model, tsfi_ramac_instruction *program, int program_size) {
    if (!model || !program || program_size <= 0) return -1;

    int pc = 0;
    int cmp_flag = 0;

    while (pc >= 0 && pc < program_size) {
        tsfi_ramac_instruction inst = program[pc];
        
        int64_t val = 0;
        if (inst.constant) {
            val = inst.acc_src;
        } else {
            if (inst.acc_src >= 0 && inst.acc_src < 10) {
                val = model->accumulators[inst.acc_src];
            }
        }

        if (strcmp(inst.op, "ADD") == 0) {
            tsfi_ramac_acc_add(model, inst.acc_dest, val);
            pc++;
        } else if (strcmp(inst.op, "SUB") == 0) {
            tsfi_ramac_acc_add(model, inst.acc_dest, -val);
            pc++;
        } else if (strcmp(inst.op, "DIV") == 0) {
            int div_ret = tsfi_ramac_acc_div(model, inst.acc_dest, val);
            if (div_ret == -1 && model->trap_active) {
                return -1;
            }
            pc++;
        } else if (strcmp(inst.op, "CMP") == 0) {
            int64_t dest_val = model->accumulators[inst.acc_dest];
            if (dest_val == val) cmp_flag = 0;
            else if (dest_val > val) cmp_flag = 1;
            else cmp_flag = -1;
            pc++;
        } else if (strcmp(inst.op, "JEQ") == 0) {
            if (cmp_flag == 0) {
                int target_pc = -1;
                for (int j = 0; j < program_size; j++) {
                    if (strcmp(program[j].label, inst.label) == 0) {
                        target_pc = j;
                        break;
                    }
                }
                if (target_pc != -1) {
                    pc = target_pc;
                } else {
                    pc++;
                }
            } else {
                pc++;
            }
        } else {
            pc++;
        }
    }
    return 0;
}

int tsfi_s370_dat_translate(uint32_t virtual_addr, 
                            tsfi_s370_segment_entry *seg_table, int seg_count,
                            tsfi_s370_page_entry *page_tables,
                            uint32_t *out_physical_addr, int *out_write_protected) {
    if (!seg_table || !page_tables || !out_physical_addr || !out_write_protected) return -1;

    uint32_t sx = (virtual_addr >> 20) & 0x7FF;
    uint32_t px = (virtual_addr >> 12) & 0xFF;
    uint32_t bx = virtual_addr & 0xFFF;

    if ((int)sx >= seg_count) {
        return -1;
    }

    if (seg_table[sx].invalid) {
        return -1;
    }

    uint32_t pte_idx = seg_table[sx].page_table_origin + px;
    if (page_tables[pte_idx].invalid) {
        return -1;
    }

    *out_physical_addr = page_tables[pte_idx].page_frame_real_addr + bx;
    *out_write_protected = page_tables[pte_idx].write_protect;
    return 0;
}

int tsfi_s370_channel_execute(tsfi_ramac_record *disk, int total_slots,
                              tsfi_s370_ccw *ccw_chain, int chain_len,
                              uint8_t *memory_pool, int mem_size) {
    if (!disk || total_slots <= 0 || !ccw_chain || chain_len <= 0 || !memory_pool || mem_size <= 0) {
        return -1;
    }

    int current_ccw_idx = 0;
    int current_seek_sector = 0;

    while (current_ccw_idx < chain_len) {
        tsfi_s370_ccw ccw = ccw_chain[current_ccw_idx];

        if (ccw.data_addr >= (uint32_t)mem_size) {
            return -1;
        }

        switch (ccw.cmd_code) {
            case 0x07:
                if (ccw.data_addr + 4 > (uint32_t)mem_size) return -1;
                memcpy(&current_seek_sector, memory_pool + ccw.data_addr, 4);
                if (current_seek_sector < 0 || current_seek_sector >= total_slots) {
                    return -1;
                }
                break;

            case 0x02:
                if (current_seek_sector >= total_slots || !disk[current_seek_sector].is_active) {
                    memset(memory_pool + ccw.data_addr, 0, ccw.count < 32 ? ccw.count : 32);
                } else {
                    int size_to_copy = ccw.count < 32 ? ccw.count : 32;
                    if (ccw.data_addr + size_to_copy > (uint32_t)mem_size) return -1;
                    memcpy(memory_pool + ccw.data_addr, disk[current_seek_sector].value, size_to_copy);
                }
                break;

            case 0x01:
                if (current_seek_sector >= total_slots) return -1;
                {
                    int size_to_copy = ccw.count < 32 ? ccw.count : 32;
                    if (ccw.data_addr + size_to_copy > (uint32_t)mem_size) return -1;
                    memset(disk[current_seek_sector].value, 0, sizeof(disk[current_seek_sector].value));
                    memcpy(disk[current_seek_sector].value, memory_pool + ccw.data_addr, size_to_copy);
                    disk[current_seek_sector].is_active = 1;
                }
                break;

            default:
                return -1;
        }

        if (ccw.flags & 0x02) {
            current_ccw_idx++;
        } else {
            break;
        }
    }

    return 0;
}

int tsfi_s370_check_storage_key(uint8_t psw_key, uint32_t real_addr, int is_write,
                                tsfi_s370_storage_key *block_keys, int block_count) {
    if (!block_keys || block_count <= 0) return -1;

    int block_idx = real_addr / 4096;
    if (block_idx >= block_count) {
        return -1;
    }

    if (psw_key != 0) {
        uint8_t block_key = block_keys[block_idx].acc;

        if (is_write) {
            if (psw_key != block_key) {
                return -1;
            }
        } else {
            if (block_keys[block_idx].fetch_protect) {
                if (psw_key != block_key) {
                    return -1;
                }
            }
        }
    }

    block_keys[block_idx].referenced = 1;
    if (is_write) {
        block_keys[block_idx].changed = 1;
    }

    return 0;
}

int tsfi_s370_validate_instruction(tsfi_s370_cpu_state *cpu, const char *op_code) {
    if (!cpu || !op_code) return -1;

    const char *privileged_ops[] = {
        "SSK", "ISK", "LPSW", "SIO", "STCTL", "LCTL"
    };
    int privileged_count = 6;

    if (cpu->supervisor_state == 0) {
        for (int i = 0; i < privileged_count; i++) {
            if (strcmp(op_code, privileged_ops[i]) == 0) {
                return -1;
            }
        }
    }

    return 0;
}

int tsfi_s370_validate_write(tsfi_s370_cpu_state *cpu, uint32_t real_addr,
                             int is_write_protected_page,
                             tsfi_s370_storage_key *block_keys, int block_count) {
    if (!cpu) return -1;

    if (is_write_protected_page) {
        return -1;
    }

    if (cpu->lap_enabled && real_addr < 512 && cpu->supervisor_state == 0) {
        return -1;
    }

    return tsfi_s370_check_storage_key(cpu->psw_key, real_addr, 1, block_keys, block_count);
}

int tsfi_s370_authorize_psw_key(tsfi_lau_account *account, 
                                const uint8_t *signature, int sig_len,
                                const uint8_t *message, int msg_len,
                                uint8_t *out_psw_key) {
    if (!account || !signature || sig_len < 32 || !message || msg_len <= 0 || !out_psw_key) {
        return -1;
    }

    uint8_t expected_sig[32];
    for (int i = 0; i < 32; i++) {
        expected_sig[i] = account->public_key[i] ^ (message[i % msg_len] + i);
    }

    if (memcmp(expected_sig, signature, 32) != 0) {
        return -1;
    }

    if (account->is_admin_tier) {
        *out_psw_key = 0;
    } else {
        *out_psw_key = (account->public_key[0] % 15) + 1;
    }

    return 0;
}

int tsfi_s370_trigger_program_interrupt(tsfi_s370_cpu_state *cpu, uint16_t pic,
                                        uint8_t *real_memory, int mem_size) {
    if (!cpu || !real_memory || mem_size < 512) {
        return -1;
    }

    real_memory[142] = (pic >> 8) & 0xFF;
    real_memory[143] = pic & 0xFF;

    real_memory[40] = (cpu->current_psw.key << 4);
    real_memory[41] = cpu->current_psw.problem_state ? 0x01 : 0x00;
    real_memory[42] = 0x00;
    real_memory[43] = 0x00;
    real_memory[44] = (cpu->current_psw.instruction_address >> 24) & 0xFF;
    real_memory[45] = (cpu->current_psw.instruction_address >> 16) & 0xFF;
    real_memory[46] = (cpu->current_psw.instruction_address >> 8) & 0xFF;
    real_memory[47] = cpu->current_psw.instruction_address & 0xFF;

    uint8_t new_key = (real_memory[88] >> 4) & 0x0F;
    int new_problem_state = real_memory[89] & 0x01;
    uint32_t new_inst_addr = ((uint32_t)real_memory[92] << 24) |
                             ((uint32_t)real_memory[93] << 16) |
                             ((uint32_t)real_memory[94] << 8)  |
                             (uint32_t)real_memory[95];

    cpu->current_psw.key = new_key;
    cpu->current_psw.problem_state = new_problem_state;
    cpu->current_psw.instruction_address = new_inst_addr;

    cpu->psw_key = new_key;
    cpu->supervisor_state = !new_problem_state;

    return 0;
}

int tsfi_s370_pack(const char *zoned_str, uint8_t *packed_out, int max_len) {
    if (!zoned_str || !packed_out || max_len <= 0) return -1;

    int sign = 0xC;
    const char *digits = zoned_str;
    if (zoned_str[0] == '-') {
        sign = 0xD;
        digits++;
    } else if (zoned_str[0] == '+') {
        digits++;
    }

    int len = strlen(digits);
    if (len == 0) return -1;

    int is_even = (len % 2 == 0);
    int packed_len = (len + 2) / 2;
    if (packed_len > max_len) return -1;

    memset(packed_out, 0, packed_len);

    int digit_idx = 0;
    for (int i = 0; i < packed_len; i++) {
        uint8_t high = 0;
        uint8_t low = 0;

        if (i == 0 && is_even) {
            high = 0;
            low = digits[digit_idx++] - '0';
        } else {
            high = digits[digit_idx++] - '0';
            if (i == packed_len - 1) {
                low = sign;
            } else {
                low = digits[digit_idx++] - '0';
            }
        }

        packed_out[i] = (high << 4) | (low & 0x0F);
    }

    return packed_len;
}

int tsfi_s370_unpack(const uint8_t *packed, int packed_len, char *zoned_out, int max_len) {
    if (!packed || packed_len <= 0 || !zoned_out || max_len <= 0) return -1;

    int zoned_idx = 0;
    int sign_found = 0;
    int negative = 0;

    char digits[128];
    int digits_idx = 0;

    for (int i = 0; i < packed_len; i++) {
        uint8_t high = (packed[i] >> 4) & 0x0F;
        uint8_t low = packed[i] & 0x0F;

        if (high > 9) return -1;

        if (i == 0 && high == 0) {
            // Skip leading padding zero
        } else {
            digits[digits_idx++] = high + '0';
        }

        if (i == packed_len - 1) {
            if (low == 0xC || low == 0xF || low == 0xA || low == 0xE) {
                negative = 0;
            } else if (low == 0xD || low == 0xB) {
                negative = 1;
            } else {
                return -1;
            }
            sign_found = 1;
        } else {
            if (low > 9) return -1;
            digits[digits_idx++] = low + '0';
        }
    }

    if (!sign_found) return -1;

    digits[digits_idx] = '\0';

    if (negative) {
        if (zoned_idx + 1 >= max_len) return -1;
        zoned_out[zoned_idx++] = '-';
    }

    if (zoned_idx + digits_idx >= max_len) return -1;
    strcpy(zoned_out + zoned_idx, digits);

    return 0;
}

int tsfi_s370_packed_add(const uint8_t *a, int a_len,
                         const uint8_t *b, int b_len,
                         uint8_t *dest_out, int dest_max_len) {
    char zoned_a[128];
    char zoned_b[128];

    if (tsfi_s370_unpack(a, a_len, zoned_a, sizeof(zoned_a)) != 0) return -1;
    if (tsfi_s370_unpack(b, b_len, zoned_b, sizeof(zoned_b)) != 0) return -1;

    long long val_a = atoll(zoned_a);
    long long val_b = atoll(zoned_b);
    long long val_sum = val_a + val_b;

    char zoned_sum[128];
    snprintf(zoned_sum, sizeof(zoned_sum), "%lld", val_sum);

    return tsfi_s370_pack(zoned_sum, dest_out, dest_max_len);
}

int tsfi_s370_trigger_svc(tsfi_s370_cpu_state *cpu, uint8_t svc_code,
                          uint8_t *real_memory, int mem_size) {
    if (!cpu || !real_memory || mem_size < 512) {
        return -1;
    }

    real_memory[139] = svc_code;

    real_memory[32] = (cpu->current_psw.key << 4);
    real_memory[33] = cpu->current_psw.problem_state ? 0x01 : 0x00;
    real_memory[34] = 0x00;
    real_memory[35] = 0x00;
    real_memory[36] = (cpu->current_psw.instruction_address >> 24) & 0xFF;
    real_memory[37] = (cpu->current_psw.instruction_address >> 16) & 0xFF;
    real_memory[38] = (cpu->current_psw.instruction_address >> 8) & 0xFF;
    real_memory[39] = cpu->current_psw.instruction_address & 0xFF;

    uint8_t new_key = (real_memory[80] >> 4) & 0x0F;
    int new_problem_state = real_memory[81] & 0x01;
    uint32_t new_inst_addr = ((uint32_t)real_memory[84] << 24) |
                             ((uint32_t)real_memory[85] << 16) |
                             ((uint32_t)real_memory[86] << 8)  |
                             (uint32_t)real_memory[87];

    cpu->current_psw.key = new_key;
    cpu->current_psw.problem_state = new_problem_state;
    cpu->current_psw.instruction_address = new_inst_addr;

    cpu->psw_key = new_key;
    cpu->supervisor_state = !new_problem_state;

    return 0;
}

int tsfi_s370_data_reduction_unit(double x, double y, double scale,
                                  uint8_t *dest_out, int dest_max_len) {
    if (!dest_out || dest_max_len <= 0) return -1;

    long long coord_x = (long long)round(x * scale);
    long long coord_y = (long long)round(y * scale);
    long long combined = coord_x + coord_y;

    char zoned[128];
    snprintf(zoned, sizeof(zoned), "%lld", combined);

    return tsfi_s370_pack(zoned, dest_out, dest_max_len);
}

int tsfi_s370_serialize_quadtree(const char *filepath, tsfi_quadtree_node *nodes, int node_count) {
    if (!filepath || !nodes || node_count <= 0) return -1;

    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return -1;
    }

    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    fwrite("QUAD", 1, 4, fp);
    fwrite(&node_count, sizeof(int), 1, fp);
    fwrite(nodes, sizeof(tsfi_quadtree_node), node_count, fp);

    fclose(fp);
    return 0;
}

int tsfi_s370_deserialize_quadtree(const char *filepath, tsfi_quadtree_node *nodes, int max_nodes) {
    if (!filepath || !nodes || max_nodes <= 0) return -1;

    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return -1;
    }

    FILE *fp = fopen(filepath, "rb");
    if (!fp) return -1;

    char magic[4];
    if (fread(magic, 1, 4, fp) != 4 || memcmp(magic, "QUAD", 4) != 0) {
        fclose(fp);
        return -1;
    }

    int node_count = 0;
    if (fread(&node_count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    if (node_count > max_nodes) {
        fclose(fp);
        return -1;
    }

    if ((int)fread(nodes, sizeof(tsfi_quadtree_node), node_count, fp) != node_count) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return node_count;
}

int tsfi_s370_dat_translate_with_tlb(tsfi_s370_cpu_state *cpu, uint32_t virtual_addr,
                                     tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables,
                                     uint32_t *out_physical_addr, int *out_write_protected) {
    if (!cpu || !seg_table || !page_tables || !out_physical_addr || !out_write_protected) return -1;

    uint32_t virtual_page = virtual_addr & 0xFFFFF000;

    int slot = (virtual_page >> 12) & 0x7;
    if (cpu->tlb[slot].valid && cpu->tlb[slot].virtual_page == virtual_page) {
        *out_physical_addr = cpu->tlb[slot].real_page | (virtual_addr & 0x0FFF);
        *out_write_protected = cpu->tlb[slot].write_protect;
        return 0;
    }

    int ret = tsfi_s370_dat_translate(virtual_addr, seg_table, seg_count, page_tables, out_physical_addr, out_write_protected);
    if (ret != 0) return ret;

    cpu->tlb[slot].virtual_page = virtual_page;
    cpu->tlb[slot].real_page = *out_physical_addr & 0xFFFFF000;
    cpu->tlb[slot].write_protect = *out_write_protected;
    cpu->tlb[slot].valid = 1;

    return 0;
}

void tsfi_s370_tlb_purge(tsfi_s370_cpu_state *cpu) {
    if (!cpu) return;
    for (int i = 0; i < 8; i++) {
        cpu->tlb[i].valid = 0;
    }
}

int tsfi_s370_oscar_reader(double analog_amplitude, const double *calibration_table, int table_size,
                           uint8_t *dest_out, int dest_max_len) {
    if (table_size < 2 || !calibration_table || !dest_out || dest_max_len <= 0) {
        return -1;
    }

    int idx = 0;
    while (idx < table_size - 1 && calibration_table[idx + 1] < analog_amplitude) {
        idx++;
    }

    double t = 0.0;
    double range = calibration_table[idx + 1] - calibration_table[idx];
    if (range > 0.000001) {
        t = (analog_amplitude - calibration_table[idx]) / range;
    }
    double interp_val = idx + t;

    long long final_digital = (long long)round(interp_val * (1000.0 / (table_size - 1)));
    
    char zoned[128];
    snprintf(zoned, sizeof(zoned), "%lld", final_digital);

    return tsfi_s370_pack(zoned, dest_out, dest_max_len);
}

int tsfi_s370_fet_discharge_freudenthal(double initial_charge, double time_step, double mass,
                                        double spring_k, double damping_c, int steps, double *out_decay_charges) {
    if (time_step <= 0.0 || mass <= 0.0 || steps <= 0 || !out_decay_charges) {
        return -1;
    }

    double pos = initial_charge;
    double prev_pos = initial_charge;

    for (int i = 0; i < steps; i++) {
        double vel = (pos - prev_pos) / time_step;
        double force = -spring_k * pos - damping_c * vel;
        double accel = force / mass;
        double next_pos = 2.0 * pos - prev_pos + accel * time_step * time_step;

        prev_pos = pos;
        pos = next_pos;

        double t = i * time_step;
        double relaxation_factor = exp(-t / 1.5);
        
        out_decay_charges[i] = pos * relaxation_factor;
    }

    return 0;
}

double tsfi_s370_fet_gate_fatigue_freudenthal(const double *stress_amplitudes, int cycle_count,
                                              double reference_stress, double shape_parameter) {
    if (!stress_amplitudes || cycle_count <= 0 || reference_stress <= 0.0 || shape_parameter <= 0.0) {
        return -1.0;
    }

    double cumulative_damage = 0.0;
    for (int i = 0; i < cycle_count; i++) {
        if (stress_amplitudes[i] > 0.0) {
            double damage_cycle = pow(stress_amplitudes[i] / reference_stress, shape_parameter);
            cumulative_damage += damage_cycle;
        }
    }

    return cumulative_damage;
}

int tsfi_s370_fet_reliability_freudenthal(double mean_resistance, double std_resistance,
                                          double mean_stress, double std_stress,
                                          double *out_beta, double *out_pf) {
    if (std_resistance <= 0.0 || std_stress <= 0.0 || !out_beta || !out_pf) {
        return -1;
    }

    double denom = sqrt(std_resistance * std_resistance + std_stress * std_stress);
    double beta = (mean_resistance - mean_stress) / denom;
    *out_beta = beta;
    *out_pf = 0.5 * erfc(beta / sqrt(2.0));

    return 0;
}

int tsfi_s370_portfolio_strategy_keystone(const double *asset_yields, const double *weights, int asset_count,
                                           double *out_expected_return, double *out_variance) {
    if (!asset_yields || !weights || asset_count <= 0 || !out_expected_return || !out_variance) {
        return -1;
    }

    double expected_return = 0.0;
    for (int i = 0; i < asset_count; i++) {
        expected_return += weights[i] * asset_yields[i];
    }
    *out_expected_return = expected_return;

    double rho = 0.15;
    double variance = 0.0;

    for (int i = 0; i < asset_count; i++) {
        double std_i = asset_yields[i] * 0.25;
        variance += weights[i] * weights[i] * std_i * std_i;

        for (int j = 0; j < asset_count; j++) {
            if (i != j) {
                double std_j = asset_yields[j] * 0.25;
                variance += weights[i] * weights[j] * std_i * std_j * rho;
            }
        }
    }
    *out_variance = variance;

    return 0;
}

int tsfi_s370_executive_decision_villalon(int decision_count, const double *benefit, const double *cost,
                                          const double *risk_prob, double *out_expected_net_value,
                                          int *out_optimal_decision_idx) {
    if (decision_count <= 0 || !benefit || !cost || !risk_prob || !out_expected_net_value || !out_optimal_decision_idx) {
        return -1;
    }

    double max_value = -9999999.0;
    int opt_idx = -1;

    for (int i = 0; i < decision_count; i++) {
        double value = benefit[i] - cost[i] - (benefit[i] * risk_prob[i]);
        if (value > max_value) {
            max_value = value;
            opt_idx = i;
        }
    }

    *out_expected_net_value = max_value;
    *out_optimal_decision_idx = opt_idx;
    return 0;
}

int tsfi_s370_deliberate_creativeness_nelles(double *parameters, int count, unsigned int seed) {
    if (!parameters || count <= 0) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        double perturbation = ((double)(seed % 2000) - 1000.0) / 10000.0;
        parameters[i] = parameters[i] * (1.0 + perturbation);
    }

    return 0;
}

int tsfi_s370_project_scale_zworykin(double initial_budget, double initial_months,
                                      double *out_actual_budget, double *out_actual_months) {
    if (initial_budget <= 0.0 || initial_months <= 0.0 || !out_actual_budget || !out_actual_months) {
        return -1;
    }

    *out_actual_budget = initial_budget * 500.0;
    *out_actual_months = initial_months * 6.66666667;

    return 0;
}

int tsfi_s370_zmachine_read_byte(const tsfi_ramac_record *disk, uint32_t zmachine_addr, uint8_t *out_val) {
    if (!disk || !out_val) {
        return -1;
    }

    int sector_idx = zmachine_addr / 32;
    int byte_offset = zmachine_addr % 32;

    int total_sectors = RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS;
    if (sector_idx >= total_sectors) {
        return -1;
    }

    if (!disk[sector_idx].is_active) {
        *out_val = 0;
    } else {
        *out_val = (uint8_t)disk[sector_idx].value[byte_offset];
    }

    return 0;
}

int tsfi_s370_zmachine_write_byte(tsfi_ramac_record *disk, uint32_t zmachine_addr, uint8_t val) {
    if (!disk) {
        return -1;
    }

    int sector_idx = zmachine_addr / 32;
    int byte_offset = zmachine_addr % 32;

    int total_sectors = RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS;
    if (sector_idx >= total_sectors) {
        return -1;
    }

    int cylinder = sector_idx / 1000;
    if (cylinder >= 45) {
        return -2;
    }

    disk[sector_idx].value[byte_offset] = (char)val;
    disk[sector_idx].is_active = 1;

    return 0;
}

int tsfi_s370_dat_ramac_translate(uint32_t virtual_addr, 
                                  tsfi_s370_segment_entry *seg_table, int seg_count,
                                  tsfi_s370_page_entry *page_tables,
                                  tsfi_ramac_chs *out_chs) {
    if (!seg_table || !page_tables || !out_chs) {
        return -1;
    }

    uint32_t physical_addr = 0;
    int write_protected = 0;

    int ret = tsfi_s370_dat_translate(virtual_addr, seg_table, seg_count, page_tables, &physical_addr, &write_protected);
    if (ret != 0) {
        return ret;
    }

    int flat_word_index = physical_addr / 4;
    *out_chs = tsfi_ramac_index_to_chs(flat_word_index);

    return 0;
}

int tsfi_s370_winchester_mq_handshake(uint8_t *scsi_bus_status, uint8_t *data_reg,
                                      const uint8_t *stream, int stream_len,
                                      uint8_t *out_buffer, int max_out_len) {
    if (!scsi_bus_status || !data_reg || !stream || stream_len <= 0 || !out_buffer || max_out_len <= 0) {
        return -1;
    }

    int bytes_transferred = 0;
    int i = 0;

    while (i < stream_len && bytes_transferred < max_out_len) {
        // 1. Pack up to 7 bytes of data into a 56-bit word
        uint64_t data_chunk = 0;
        int chunk_size = 0;
        for (int k = 0; k < 7; k++) {
            if (i + k < stream_len) {
                data_chunk |= ((uint64_t)stream[i + k] << (k * 8));
                chunk_size++;
            }
        }

        // 2. Encode using IBM 7030 SEC-DED ECC
        uint64_t ecc_word = tsfi_s370_ibm7030_ecc_encode(data_chunk);

        // 3. Transmit the 8 bytes of the ECC word over the SCSI handshake protocol
        uint64_t rx_word = 0;
        for (int k = 0; k < 8; k++) {
            uint8_t tx_byte = (ecc_word >> (k * 8)) & 0xFF;

            *data_reg = tx_byte;
            *scsi_bus_status |= 0x01; // REQ
            *scsi_bus_status |= 0x02; // ACK

            uint8_t rx_byte = *data_reg;
            rx_word |= ((uint64_t)rx_byte << (k * 8));

            *scsi_bus_status &= ~0x01;
            *scsi_bus_status &= ~0x02;
        }

        // 4. Decode and correct errors on the receiver side
        uint64_t corrected_data = 0;
        tsfi_s370_ibm7030_ecc_decode(rx_word, &corrected_data);

        // 5. Unpack the corrected data into the output buffer
        for (int k = 0; k < chunk_size; k++) {
            if (bytes_transferred < max_out_len) {
                out_buffer[bytes_transferred++] = (corrected_data >> (k * 8)) & 0xFF;
            }
        }

        i += chunk_size;
    }

    return bytes_transferred;
}

int tsfi_s370_oscar_reader_polynomial(double analog_amplitude, const double *coefficients, int coeff_count,
                                      uint8_t *dest_out, int dest_max_len) {
    if (!coefficients || coeff_count <= 0 || !dest_out || dest_max_len <= 0) {
        return -1;
    }

    double eval_val = 0.0;
    for (int i = 0; i < coeff_count; i++) {
        eval_val += coefficients[i] * pow(analog_amplitude, i);
    }

    long long final_digital = (long long)round(eval_val * 1000.0);
    if (final_digital < 0) final_digital = 0;
    if (final_digital > 1000) final_digital = 1000;

    char zoned[128];
    snprintf(zoned, sizeof(zoned), "%lld", final_digital);

    return tsfi_s370_pack(zoned, dest_out, dest_max_len);
}

int tsfi_s370_punched_card_to_comp3(const tsfi_ramac_card *card, int start_col, int end_col,
                                    uint8_t *packed_out, int max_len) {
    if (!card || start_col < 0 || end_col >= 80 || start_col > end_col || !packed_out || max_len <= 0) {
        return -1;
    }

    char zoned_buf[64] = {0};
    int dst_idx = 0;
    int is_negative = 0;

    for (int c = start_col; c <= end_col; c++) {
        char ch = card->columns[c];
        if (ch >= '0' && ch <= '9') {
            zoned_buf[dst_idx++] = ch;
        } else if (ch >= 'J' && ch <= 'R') {
            is_negative = 1;
            zoned_buf[dst_idx++] = (ch - 'J' + 1) + '0';
        }
    }
    zoned_buf[dst_idx] = '\0';

    if (is_negative) {
        char temp[64];
        snprintf(temp, sizeof(temp), "-%s", zoned_buf);
        strcpy(zoned_buf, temp);
    }

    return tsfi_s370_pack(zoned_buf, packed_out, max_len);
}

int tsfi_s370_scsi_stream_to_ramac(tsfi_ramac_record *disk, uint8_t *scsi_status, uint8_t *data_reg,
                                    const uint8_t *stream, int stream_len, int target_cylinder) {
    if (!disk || !scsi_status || !data_reg || !stream || stream_len <= 0) {
        return -1;
    }

    uint8_t buffer[256];
    int read_bytes = tsfi_s370_winchester_mq_handshake(scsi_status, data_reg, stream, stream_len, buffer, 256);
    if (read_bytes <= 0) {
        return -1;
    }

    char key[32];
    char value[32];
    snprintf(key, sizeof(key), "scsi_key_%d", buffer[0]);
    snprintf(value, sizeof(value), "scsi_val_%d", buffer[read_bytes - 1]);

    double temp_seek = 0.0;
    int slot = tsfi_ramac_insert_record(disk, key, value, target_cylinder, &temp_seek);
    if (slot == -1) {
        return -1;
    }

    return slot;
}

int tsfi_s370_oscar_soft_body_validate(double analog_val, double mass, double spring_k, double damping_c,
                                       double *out_decay_charges, int steps) {
    if (!out_decay_charges || steps <= 0) {
        return -1;
    }

    double initial_charge = analog_val * 100.0;

    int discharge_ret = tsfi_s370_fet_discharge_freudenthal(initial_charge, 0.1, mass, spring_k, damping_c, steps, out_decay_charges);
    if (discharge_ret != 0) {
        return -1;
    }

    if (fabs(out_decay_charges[steps - 1]) >= fabs(initial_charge)) {
        return -2;
    }

    return 0;
}

int tsfi_s370_sage_redundancy_monitor(int cpu_a_status, int cpu_b_status, int *active_cpu) {
    if (!active_cpu) {
        return -1;
    }

    if (cpu_a_status == 1) {
        *active_cpu = 1;
    } else if (cpu_b_status == 1) {
        *active_cpu = 2;
    } else {
        *active_cpu = 0;
    }

    return 0;
}

int tsfi_s370_engelbart_index_resolve(const char *abstract, const char **keywords, int keyword_count,
                                      uint8_t *comp3_out, int max_len) {
    if (!abstract || !keywords || keyword_count <= 0 || !comp3_out || max_len <= 0) {
        return -1;
    }

    int match_count = 0;

    for (int i = 0; i < keyword_count; i++) {
        if (!keywords[i]) continue;
        const char *ptr = abstract;
        int len = strlen(keywords[i]);
        if (len == 0) continue;

        while ((ptr = strstr(ptr, keywords[i])) != NULL) {
            match_count++;
            ptr += len;
        }
    }

    char zoned[64];
    snprintf(zoned, sizeof(zoned), "%d", match_count);

    return tsfi_s370_pack(zoned, comp3_out, max_len);
}

int tsfi_s370_muroga_parametron_majority(int phase_in_1, int phase_in_2, int phase_in_3, int *phase_out) {
    if (!phase_out) {
        return -1;
    }

    int sum = phase_in_1 + phase_in_2 + phase_in_3;
    if (sum >= 2) {
        *phase_out = 1;
    } else {
        *phase_out = 0;
    }

    return 0;
}

int tsfi_s370_parametron_circuit_eval(tsfi_parametron_node *nodes, int node_count,
                                       const int *inputs, int input_count) {
    if (!nodes || node_count <= 0) {
        return -1;
    }

    for (int i = 0; i < node_count; i++) {
        int inputs_resolved[3] = {0};

        for (int s = 0; s < 3; s++) {
            int src = nodes[i].sources[s];
            int phase = 0;

            if (src == -1) {
                phase = 0;
            } else if (src == -2) {
                phase = 1;
            } else if (src < -2) {
                int inp_idx = -(src + 3);
                if (inp_idx >= 0 && inp_idx < input_count && inputs) {
                    phase = inputs[inp_idx];
                } else {
                    phase = 0;
                }
            } else {
                if (src < i) {
                    phase = nodes[src].output;
                } else {
                    phase = 0;
                }
            }

            if (nodes[i].invert[s]) {
                phase = (phase == 0) ? 1 : 0;
            }

            inputs_resolved[s] = phase;
        }

        int out_phase = 0;
        int sum = inputs_resolved[0] + inputs_resolved[1] + inputs_resolved[2];
        if (sum >= 2) {
            out_phase = 1;
        }
        nodes[i].output = out_phase;
    }

    return 0;
}

int tsfi_s370_peek_a_boo_card_match(const uint32_t *card_a, const uint32_t *card_b,
                                     uint32_t *out_matching, int word_count) {
    if (!card_a || !card_b || !out_matching || word_count <= 0) {
        return -1;
    }

    int match_holes_count = 0;

    for (int i = 0; i < word_count; i++) {
        out_matching[i] = card_a[i] & card_b[i];

        uint32_t word = out_matching[i];
        while (word) {
            if (word & 1) {
                match_holes_count++;
            }
            word >>= 1;
        }
    }

    return match_holes_count;
}

int tsfi_s370_muroga_threshold_gate(const int *inputs, const int *weights, int input_count,
                                    int threshold, int *output) {
    if (!inputs || !weights || input_count <= 0 || !output) {
        return -1;
    }

    int weighted_sum = 0;
    for (int i = 0; i < input_count; i++) {
        weighted_sum += inputs[i] * weights[i];
    }

    if (weighted_sum >= threshold) {
        *output = 1;
    } else {
        *output = 0;
    }

    return 0;
}

int tsfi_s370_recomp_ii_decode_word(uint64_t raw_word, int *op1, int *addr1, int *op2, int *addr2) {
    if (!op1 || !addr1 || !op2 || !addr2) {
        return -1;
    }

    uint64_t left_instr = (raw_word >> 20) & 0xFFFFF;
    *op1 = (left_instr >> 15) & 0x1F;
    *addr1 = (left_instr >> 3) & 0xFFF;

    uint64_t right_instr = raw_word & 0xFFFFF;
    *op2 = (right_instr >> 15) & 0x1F;
    *addr2 = (right_instr >> 3) & 0xFFF;

    return 0;
}

int tsfi_s370_recomp_ii_drum_schedule(int current_sector, int execution_cycles, int *out_optimal_sector) {
    if (current_sector < 0 || current_sector >= 64 || execution_cycles < 0 || !out_optimal_sector) {
        return -1;
    }

    int sectors_passed = (execution_cycles + 7) / 8;
    *out_optimal_sector = (current_sector + sectors_passed + 1) % 64;

    return 0;
}

int tsfi_s370_paper_tape_synthesizer(const uint8_t *tape_data, int length, int channels,
                                     double *out_audio, int max_samples, double sample_rate) {
    if (!tape_data || length <= 0 || channels <= 0 || channels > 8 || !out_audio || max_samples <= 0 || sample_rate <= 0.0) {
        return -1;
    }

    memset(out_audio, 0, max_samples * sizeof(double));

    double frequencies[8] = {110.0, 220.0, 330.0, 440.0, 550.0, 660.0, 770.0, 880.0};
    double step_duration = 0.1;
    int samples_per_step = (int)(step_duration * sample_rate);

    for (int step = 0; step < length; step++) {
        int start_sample = step * samples_per_step;
        if (start_sample >= max_samples) break;

        uint8_t row_val = tape_data[step];

        for (int c = 0; c < channels; c++) {
            if ((row_val >> c) & 1) {
                double freq = frequencies[c];

                for (int s = 0; s < samples_per_step; s++) {
                    int out_idx = start_sample + s;
                    if (out_idx >= max_samples) break;

                    double t = (double)s / sample_rate;
                    double decay = exp(-t / 0.05);
                    double signal = sin(2.0 * M_PI * freq * t) * decay * 0.1;

                    out_audio[out_idx] += signal;
                }
            }
        }
    }

    return 0;
}

int tsfi_s370_tx2_simd_alu(uint64_t op_a, uint64_t op_b, int mode, const char *op, uint64_t *out_val) {
    if (!op || !out_val) {
        return -1;
    }

    uint64_t a = op_a & 0xFFFFFFFFFULL;
    uint64_t b = op_b & 0xFFFFFFFFFULL;
    uint64_t result = 0;

    int is_add = (strcmp(op, "ADD") == 0);
    int is_sub = (strcmp(op, "SUB") == 0);
    if (!is_add && !is_sub) return -1;

    if (mode == 36) {
        if (is_add) {
            result = (a + b) & 0xFFFFFFFFFULL;
        } else {
            result = (a - b) & 0xFFFFFFFFFULL;
        }
    } else if (mode == 18) {
        uint64_t a_hi = (a >> 18) & 0x3FFFFULL;
        uint64_t a_lo = a & 0x3FFFFULL;
        uint64_t b_hi = (b >> 18) & 0x3FFFFULL;
        uint64_t b_lo = b & 0x3FFFFULL;
        uint64_t r_hi = 0, r_lo = 0;

        if (is_add) {
            r_hi = (a_hi + b_hi) & 0x3FFFFULL;
            r_lo = (a_lo + b_lo) & 0x3FFFFULL;
        } else {
            r_hi = (a_hi - b_hi) & 0x3FFFFULL;
            r_lo = (a_lo - b_lo) & 0x3FFFFULL;
        }
        result = (r_hi << 18) | r_lo;
    } else if (mode == 9) {
        uint64_t r[4] = {0};
        for (int i = 0; i < 4; i++) {
            uint64_t sa = (a >> (i * 9)) & 0x1FFULL;
            uint64_t sb = (b >> (i * 9)) & 0x1FFULL;
            if (is_add) {
                r[i] = (sa + sb) & 0x1FFULL;
            } else {
                r[i] = (sa - sb) & 0x1FFULL;
            }
        }
        result = (r[3] << 27) | (r[2] << 18) | (r[1] << 9) | r[0];
    } else {
        return -1;
    }

    *out_val = result;
    return 0;
}

int tsfi_s370_tx2_light_pen_track(double pen_x, double pen_y, double *cross_x, double *cross_y, double cross_radius) {
    if (!cross_x || !cross_y || cross_radius <= 0.0) {
        return -1;
    }

    double px[9], py[9];
    px[0] = *cross_x; py[0] = *cross_y;
    px[1] = *cross_x + cross_radius; py[1] = *cross_y;
    px[2] = *cross_x - cross_radius; py[2] = *cross_y;
    px[3] = *cross_x; py[3] = *cross_y + cross_radius;
    px[4] = *cross_x; py[4] = *cross_y - cross_radius;
    double diag = cross_radius * 0.5;
    px[5] = *cross_x + diag; py[5] = *cross_y + diag;
    px[6] = *cross_x - diag; py[6] = *cross_y - diag;
    px[7] = *cross_x + diag; py[7] = *cross_y - diag;
    px[8] = *cross_x - diag; py[8] = *cross_y + diag;

    double threshold = 0.7 * cross_radius;
    double sum_x = 0.0, sum_y = 0.0;
    int detected_count = 0;

    for (int i = 0; i < 9; i++) {
        double dist = sqrt((px[i] - pen_x) * (px[i] - pen_x) + (py[i] - pen_y) * (py[i] - pen_y));
        if (dist <= threshold) {
            sum_x += px[i];
            sum_y += py[i];
            detected_count++;
        }
    }

    if (detected_count > 0) {
        *cross_x = sum_x / detected_count;
        *cross_y = sum_y / detected_count;
    }

    return detected_count;
}

int tsfi_s370_rw400_matrix_switch(const int *matrix_connections, int cpu_count, int buffer_count, int *out_route_map) {
    if (!matrix_connections || cpu_count <= 0 || buffer_count <= 0 || !out_route_map) {
        return -1;
    }

    for (int i = 0; i < cpu_count; i++) {
        out_route_map[i] = -1;
    }

    int *buffer_used = (int *)calloc(buffer_count, sizeof(int));
    if (!buffer_used) return -1;

    for (int i = 0; i < cpu_count; i++) {
        int cpu_connection_count = 0;
        int connected_buffer = -1;

        for (int j = 0; j < buffer_count; j++) {
            int idx = i * buffer_count + j;
            if (matrix_connections[idx] == 1) {
                cpu_connection_count++;
                connected_buffer = j;
            }
        }

        if (cpu_connection_count > 1) {
            free(buffer_used);
            return -1;
        }

        if (cpu_connection_count == 1) {
            if (buffer_used[connected_buffer] == 1) {
                free(buffer_used);
                return -1;
            }
            buffer_used[connected_buffer] = 1;
            out_route_map[i] = connected_buffer;
        }
    }

    free(buffer_used);
    return 0;
}

int tsfi_s370_uncol_vm_exec(tsfi_uncol_instruction *program, int program_size, int *memory, int mem_size, int *registers, int reg_count) {
    if (!program || program_size <= 0 || !memory || mem_size <= 0 || !registers || reg_count <= 0) {
        return -1;
    }

    int pc = 0;
    int instructions_executed = 0;

    while (pc >= 0 && pc < program_size && instructions_executed < 10000) {
        tsfi_uncol_instruction inst = program[pc];
        instructions_executed++;

        if (strcmp(inst.op, "LOAD") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count || inst.address < 0 || inst.address >= mem_size) {
                return -2;
            }
            registers[inst.reg_dest] = memory[inst.address];
            pc++;
        } else if (strcmp(inst.op, "STORE") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count || inst.address < 0 || inst.address >= mem_size) {
                return -2;
            }
            memory[inst.address] = registers[inst.reg_dest];
            pc++;
        } else if (strcmp(inst.op, "ADD") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count || 
                inst.reg_src1 < 0 || inst.reg_src1 >= reg_count || 
                inst.reg_src2 < 0 || inst.reg_src2 >= reg_count) {
                return -2;
            }
            registers[inst.reg_dest] = registers[inst.reg_src1] + registers[inst.reg_src2];
            pc++;
        } else if (strcmp(inst.op, "SUB") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count || 
                inst.reg_src1 < 0 || inst.reg_src1 >= reg_count || 
                inst.reg_src2 < 0 || inst.reg_src2 >= reg_count) {
                return -2;
            }
            registers[inst.reg_dest] = registers[inst.reg_src1] - registers[inst.reg_src2];
            pc++;
        } else if (strcmp(inst.op, "JMP") == 0) {
            if (inst.address < 0 || inst.address >= program_size) {
                return -2;
            }
            pc = inst.address;
        } else if (strcmp(inst.op, "JZ") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count) {
                return -2;
            }
            if (registers[inst.reg_dest] == 0) {
                if (inst.address < 0 || inst.address >= program_size) {
                    return -2;
                }
                pc = inst.address;
            } else {
                pc++;
            }
        } else {
            pc++;
        }
    }

    return 0;
}

int tsfi_s370_polymorphic_winchester_mq_route(const int *matrix_connections, int initiator_count, int target_count,
                                              uint8_t *scsi_status_array, uint8_t *data_reg_array,
                                              const uint8_t **streams, const int *stream_lens,
                                              tsfi_ramac_record *disk, int *out_route_map) {
    if (!matrix_connections || initiator_count <= 0 || target_count <= 0 || 
        !scsi_status_array || !data_reg_array || !streams || !stream_lens || !disk || !out_route_map) {
        return -1;
    }

    int ret = tsfi_s370_rw400_matrix_switch(matrix_connections, initiator_count, target_count, out_route_map);
    if (ret != 0) {
        return -1;
    }

    for (int i = 0; i < initiator_count; i++) {
        int target_buffer_idx = out_route_map[i];
        if (target_buffer_idx != -1) {
            int target_cylinder = target_buffer_idx * 10;
            if (target_cylinder >= RAMAC_CYLINDERS) {
                target_cylinder = RAMAC_CYLINDERS - 1;
            }

            int commit_slot = tsfi_s370_scsi_stream_to_ramac(disk, &scsi_status_array[i], &data_reg_array[i],
                                                             streams[i], stream_lens[i], target_cylinder);
            if (commit_slot == -1) {
                return -2;
            }
        }
    }

    return 0;
}

void tsfi_s370_zmm_lock_init(tsfi_zmm_lock_registry *registry) {
    if (!registry) return;
    for (int i = 0; i < RAMAC_CYLINDERS; i++) {
        registry->locked_cylinders[i] = 0;
        registry->cylinder_owners[i] = -1;
        registry->lock_ticks[i] = 0;
    }
}

int tsfi_s370_zmm_lock_acquire(tsfi_zmm_lock_registry *registry, int initiator_id, int cylinder, int lock_mode,
                               uint64_t current_tick, int initiator_priority) {
    if (!registry || cylinder < 0 || cylinder >= RAMAC_CYLINDERS) {
        return -1;
    }
    if (lock_mode != 1 && lock_mode != 2) {
        return -1;
    }

    if (registry->locked_cylinders[cylinder] != 0) {
        if (current_tick - registry->lock_ticks[cylinder] > 1000) {
            registry->locked_cylinders[cylinder] = 0;
            registry->cylinder_owners[cylinder] = -1;
        }
    }

    if (registry->locked_cylinders[cylinder] == 0) {
        registry->locked_cylinders[cylinder] = lock_mode;
        registry->cylinder_owners[cylinder] = initiator_id;
        registry->lock_ticks[cylinder] = current_tick;
        return 0;
    }

    if (registry->locked_cylinders[cylinder] == 1 && lock_mode == 1) {
        return 0;
    }

    if (initiator_priority > 5) {
        registry->locked_cylinders[cylinder] = lock_mode;
        registry->cylinder_owners[cylinder] = initiator_id;
        registry->lock_ticks[cylinder] = current_tick;
        return 2;
    }

    return -2;
}

int tsfi_s370_zmm_lock_release(tsfi_zmm_lock_registry *registry, int initiator_id, int cylinder) {
    if (!registry || cylinder < 0 || cylinder >= RAMAC_CYLINDERS) {
        return -1;
    }

    if (registry->cylinder_owners[cylinder] == initiator_id) {
        registry->locked_cylinders[cylinder] = 0;
        registry->cylinder_owners[cylinder] = -1;
        registry->lock_ticks[cylinder] = 0;
        return 0;
    }

    return -1;
}

int tsfi_s370_zyir_exec(tsfi_zyir_instruction *program, int program_size,
                        uint8_t *yul_memory, int yul_mem_size,
                        tsfi_ramac_record *zmm_disk, tsfi_zmm_lock_registry *lock_registry,
                        int initiator_id, int initiator_priority,
                        int *registers, int reg_count, uint64_t *current_tick) {
    if (!program || program_size <= 0 || !yul_memory || yul_mem_size <= 0 || 
        !zmm_disk || !lock_registry || !registers || reg_count <= 0 || !current_tick) {
        return -1;
    }

    int pc = 0;
    int run_limit = 0;

    while (pc >= 0 && pc < program_size && run_limit < 10000) {
        tsfi_zyir_instruction inst = program[pc];
        run_limit++;
        (*current_tick)++;

        if (strcmp(inst.op, "MSTORE") == 0) {
            if (inst.val_addr + 4 > (uint32_t)yul_mem_size || inst.reg_src1 < 0 || inst.reg_src1 >= reg_count) {
                return -2;
            }
            int val = registers[inst.reg_src1];
            yul_memory[inst.val_addr] = (val >> 24) & 0xFF;
            yul_memory[inst.val_addr + 1] = (val >> 16) & 0xFF;
            yul_memory[inst.val_addr + 2] = (val >> 8) & 0xFF;
            yul_memory[inst.val_addr + 3] = val & 0xFF;
            pc++;
        } else if (strcmp(inst.op, "MLOAD") == 0) {
            if (inst.val_addr + 4 > (uint32_t)yul_mem_size || inst.reg_dest < 0 || inst.reg_dest >= reg_count) {
                return -2;
            }
            registers[inst.reg_dest] = ((int)yul_memory[inst.val_addr] << 24) |
                                       ((int)yul_memory[inst.val_addr + 1] << 16) |
                                       ((int)yul_memory[inst.val_addr + 2] << 8)  |
                                       (int)yul_memory[inst.val_addr + 3];
            pc++;
        } else if (strcmp(inst.op, "ZLOCK") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count || inst.reg_src1 < 0 || inst.reg_src1 >= reg_count) {
                return -2;
            }
            int lock_mode = registers[inst.reg_src1];
            int lock_res = tsfi_s370_zmm_lock_acquire(lock_registry, initiator_id, inst.val_addr, lock_mode,
                                                      *current_tick, initiator_priority);
            registers[inst.reg_dest] = lock_res;
            pc++;
        } else if (strcmp(inst.op, "ZRELEASE") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count) {
                return -2;
            }
            int rel_res = tsfi_s370_zmm_lock_release(lock_registry, initiator_id, inst.val_addr);
            registers[inst.reg_dest] = rel_res;
            pc++;
        } else if (strcmp(inst.op, "ZWRITE") == 0) {
            int cylinder = inst.val_addr;
            if (cylinder < 0 || cylinder >= RAMAC_CYLINDERS || inst.reg_src1 < 0 || inst.reg_src1 >= reg_count) {
                return -2;
            }
            if (lock_registry->locked_cylinders[cylinder] != 2 || lock_registry->cylinder_owners[cylinder] != initiator_id) {
                return -3;
            }
            char key_str[32];
            char val_str[32];
            snprintf(key_str, sizeof(key_str), "zyir_key_%d", initiator_id);
            snprintf(val_str, sizeof(val_str), "zyir_val_%d", registers[inst.reg_src1]);
            double temp_seek = 0.0;
            tsfi_ramac_insert_record(zmm_disk, key_str, val_str, cylinder, &temp_seek);
            pc++;
        } else if (strcmp(inst.op, "ZREAD") == 0) {
            int cylinder = inst.val_addr;
            if (cylinder < 0 || cylinder >= RAMAC_CYLINDERS || inst.reg_dest < 0 || inst.reg_dest >= reg_count) {
                return -2;
            }
            if (lock_registry->locked_cylinders[cylinder] == 0 || lock_registry->cylinder_owners[cylinder] != initiator_id) {
                return -3;
            }
            char key_str[32];
            snprintf(key_str, sizeof(key_str), "zyir_key_%d", initiator_id);
            double temp_seek = 0.0;
            const char *res_val = tsfi_ramac_search_record(zmm_disk, key_str, cylinder, &temp_seek);
            if (res_val) {
                int numeric_val = 0;
                sscanf(res_val, "zyir_val_%d", &numeric_val);
                registers[inst.reg_dest] = numeric_val;
            } else {
                registers[inst.reg_dest] = 0;
            }
            pc++;
        } else if (strcmp(inst.op, "ADD") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count || 
                inst.reg_src1 < 0 || inst.reg_src1 >= reg_count || 
                inst.reg_src2 < 0 || inst.reg_src2 >= reg_count) {
                return -2;
            }
            registers[inst.reg_dest] = registers[inst.reg_src1] + registers[inst.reg_src2];
            pc++;
        } else if (strcmp(inst.op, "SUB") == 0) {
            if (inst.reg_dest < 0 || inst.reg_dest >= reg_count || 
                inst.reg_src1 < 0 || inst.reg_src1 >= reg_count || 
                inst.reg_src2 < 0 || inst.reg_src2 >= reg_count) {
                return -2;
            }
            registers[inst.reg_dest] = registers[inst.reg_src1] - registers[inst.reg_src2];
            pc++;
        } else {
            pc++;
        }
    }

    return 0;
}

int tsfi_s370_uncol_to_yul(const tsfi_uncol_instruction *program, int program_size,
                           char *yul_code_out, int max_len) {
    if (!program || program_size <= 0 || !yul_code_out || max_len <= 0) {
        return -1;
    }

    yul_code_out[0] = '\0';
    int offset = snprintf(yul_code_out, max_len, "{\n");

    for (int i = 0; i < program_size; i++) {
        if (offset >= max_len - 64) break;

        tsfi_uncol_instruction inst = program[i];
        if (strcmp(inst.op, "LOAD") == 0) {
            offset += snprintf(yul_code_out + offset, max_len - offset,
                               "  let r%d := mload(%d)\n", inst.reg_dest, inst.address);
        } else if (strcmp(inst.op, "STORE") == 0) {
            offset += snprintf(yul_code_out + offset, max_len - offset,
                               "  mstore(%d, r%d)\n", inst.address, inst.reg_dest);
        } else if (strcmp(inst.op, "ADD") == 0) {
            offset += snprintf(yul_code_out + offset, max_len - offset,
                               "  let r%d := add(r%d, r%d)\n", inst.reg_dest, inst.reg_src1, inst.reg_src2);
        } else if (strcmp(inst.op, "SUB") == 0) {
            offset += snprintf(yul_code_out + offset, max_len - offset,
                               "  let r%d := sub(r%d, r%d)\n", inst.reg_dest, inst.reg_src1, inst.reg_src2);
        }
    }

    if (offset < max_len - 3) {
        snprintf(yul_code_out + offset, max_len - offset, "}\n");
    }

    return 0;
}

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

void tsfi_lgp30_flipflop_init(tsfi_lgp30_flipflop *ff) {
    if (!ff) return;
    ff->triode1_grid_v = -2.0;
    ff->triode1_plate_v = 150.0;
    ff->triode2_grid_v = 0.0;
    ff->triode2_plate_v = 50.0;
}

void tsfi_lgp30_flipflop_tick(tsfi_lgp30_flipflop *ff, double trigger_set_v, double trigger_reset_v, double dt) {
    if (!ff) return;

    double vcc = 200.0;
    double rl = 20000.0;
    double r1 = 100000.0;
    double r2 = 50000.0;
    double bias_v = -50.0;
    double i0 = 0.002;

    double target_g1 = (ff->triode2_plate_v * r2 + bias_v * r1) / (r1 + r2) + trigger_set_v;
    double target_g2 = (ff->triode1_plate_v * r2 + bias_v * r1) / (r1 + r2) + trigger_reset_v;

    ff->triode1_grid_v += (target_g1 - ff->triode1_grid_v) * (dt / 0.00001);
    ff->triode2_grid_v += (target_g2 - ff->triode2_grid_v) * (dt / 0.00001);
    if (ff->triode1_grid_v > 0.0) ff->triode1_grid_v = 0.0;
    if (ff->triode2_grid_v > 0.0) ff->triode2_grid_v = 0.0;

    double ip1 = 0.0;
    if (ff->triode1_grid_v > -6.0) {
        ip1 = i0 * pow(ff->triode1_grid_v + 6.0, 1.5);
    }
    double ip2 = 0.0;
    if (ff->triode2_grid_v > -6.0) {
        ip2 = i0 * pow(ff->triode2_grid_v + 6.0, 1.5);
    }

    double target_p1 = vcc - ip1 * rl;
    double target_p2 = vcc - ip2 * rl;
    if (target_p1 < 0.0) target_p1 = 0.0;
    if (target_p2 < 0.0) target_p2 = 0.0;

    ff->triode1_plate_v += (target_p1 - ff->triode1_plate_v) * (dt / 0.000002);
    ff->triode2_plate_v += (target_p2 - ff->triode2_plate_v) * (dt / 0.000002);
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

int tsfi_s370_uncol_to_lgp30(const tsfi_uncol_instruction *program, int program_size, int *lgp_memory_out, int max_words) {
    if (!program || program_size <= 0 || !lgp_memory_out || max_words <= 0) {
        return -1;
    }

    int compiled_words = 0;

    for (int i = 0; i < program_size; i++) {
        if (compiled_words >= max_words) {
            return -2;
        }

        tsfi_uncol_instruction inst = program[i];
        int opcode = -1;
        int addr = inst.address & 0x0FFF;

        if (strcmp(inst.op, "LOAD") == 0) {
            opcode = 0;
        } else if (strcmp(inst.op, "STORE") == 0) {
            opcode = 9;
        } else if (strcmp(inst.op, "ADD") == 0) {
            opcode = 2;
        } else if (strcmp(inst.op, "SUB") == 0) {
            opcode = 3;
        } else if (strcmp(inst.op, "JMP") == 0) {
            opcode = 7;
        } else if (strcmp(inst.op, "JZ") == 0) {
            opcode = 8;
        }

        if (opcode != -1) {
            lgp_memory_out[compiled_words++] = (opcode << 20) | addr;
        }
    }

    return compiled_words;
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

int tsfi_s370_ibm7090_tix(uint16_t *index_reg, uint16_t decrement, uint16_t target_address, uint16_t *pc) {
    if (!index_reg || !pc) return -1;

    if (*index_reg > decrement) {
        *index_reg -= decrement;
        *pc = target_address;
        return 1;
    }
    return 0;
}

int tsfi_s370_ibm7090_txi(uint16_t *index_reg, uint16_t decrement, uint16_t target_address, uint16_t *pc) {
    if (!index_reg || !pc) return -1;

    *index_reg += decrement;
    *pc = target_address;
    return 1;
}

void tsfi_s370_cdc6600_init(tsfi_cdc6600_scoreboard *sb) {
    if (!sb) return;
    memset(sb->units, 0, sizeof(sb->units));
    memset(sb->registers, 0, sizeof(sb->registers));
}

int tsfi_s370_cdc6600_issue(tsfi_cdc6600_scoreboard *sb, tsfi_cdc6600_unit_type unit, int dest_reg, int src_val_a, int src_val_b, int op) {
    if (!sb || unit < 0 || unit >= CDC_UNIT_COUNT) return -1;
    if (dest_reg < 0 || dest_reg >= 8) return -2;

    tsfi_cdc6600_functional_unit *fu = &sb->units[unit];
    if (fu->is_busy) {
        return -3; // Functional unit is busy
    }

    fu->is_busy = 1;
    fu->dest_reg = dest_reg;

    switch (unit) {
        case CDC_UNIT_ADD:
            fu->result_value = (op == 0) ? (src_val_a + src_val_b) : (src_val_a - src_val_b);
            fu->remaining_cycles = 2;
            break;
        case CDC_UNIT_MULTIPLY:
            fu->result_value = src_val_a * src_val_b;
            fu->remaining_cycles = 4;
            break;
        case CDC_UNIT_SHIFT:
            fu->result_value = src_val_a << src_val_b;
            fu->remaining_cycles = 1;
            break;
        case CDC_UNIT_BRANCH:
            fu->result_value = src_val_a; // pass-through target
            fu->remaining_cycles = 1;
            break;
        default:
            break;
    }

    return 0;
}

void tsfi_s370_cdc6600_tick(tsfi_cdc6600_scoreboard *sb) {
    if (!sb) return;

    for (int i = 0; i < CDC_UNIT_COUNT; i++) {
        tsfi_cdc6600_functional_unit *fu = &sb->units[i];
        if (fu->is_busy) {
            fu->remaining_cycles--;
            if (fu->remaining_cycles == 0) {
                sb->registers[fu->dest_reg] = fu->result_value;
                fu->is_busy = 0;
            }
        }
    }
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

int tsfi_s370_ramac_controller_exec(tsfi_ibm7030_lau_queue *queue, uint64_t *ramac_platter, int platter_size, 
                                    uint32_t sector_addr, int is_write, uint64_t *data_word) {
    if (!queue || !ramac_platter || !data_word || (int)sector_addr >= platter_size) return -1;

    if (is_write) {
        uint64_t ecc_encoded_word = tsfi_s370_ibm7030_ecc_encode(*data_word);
        int push_status = tsfi_s370_ibm7030_lau_push_store(queue, sector_addr, ecc_encoded_word);
        if (push_status != 0) return -2;

        int commits = tsfi_s370_ibm7030_lau_commit(queue, ramac_platter, platter_size);
        if (commits < 0) return -3;
    } else {
        int push_status = tsfi_s370_ibm7030_lau_push_load(queue, sector_addr);
        if (push_status != 0) return -2;

        int commits = tsfi_s370_ibm7030_lau_commit(queue, ramac_platter, platter_size);
        if (commits < 0) return -3;

        int last_head = (queue->head - 1 + 8) % 8;
        uint64_t ecc_read_word = queue->entries[last_head].value;

        uint64_t corrected_val = 0;
        int err_type = tsfi_s370_ibm7030_ecc_decode(ecc_read_word, &corrected_val);
        if (err_type == 2) {
            return -4;
        }
        *data_word = corrected_val;
    }

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

int tsfi_s370_univac2_to_rca501(const char *univac_data, char *rca_data_out) {
    if (!univac_data || !rca_data_out) return -1;

    // Find the last non-space character in the 12-byte word
    int len = 12;
    while (len > 0 && (univac_data[len - 1] == ' ' || univac_data[len - 1] == '\0')) {
        len--;
    }

    int i;
    for (i = 0; i < len; i++) {
        rca_data_out[i] = univac_data[i];
    }

    // Append the RCA 501 variable-length item delimiter '$'
    rca_data_out[i++] = '$';
    rca_data_out[i] = '\0';
    return 0;
}

int tsfi_s370_normalize_signed_field(const char *input_field, int is_univac, int64_t *out_val) {
    if (!input_field || !out_val) return -1;

    int64_t val = 0;
    int sign = 1;

    if (is_univac) {
        if (input_field[0] == '-') {
            sign = -1;
        }

        int i = 1;
        while (i < 12 && input_field[i] != '\0' && input_field[i] != ' ') {
            if (input_field[i] >= '0' && input_field[i] <= '9') {
                val = val * 10 + (input_field[i] - '0');
            }
            i++;
        }
    } else {
        int len = 0;
        while (input_field[len] != '\0' && input_field[len] != '$') {
            len++;
        }

        if (len == 0) return -2;

        char last_char = input_field[len - 1];
        if (last_char == '-') {
            sign = -1;
            len--;
        } else if (last_char == '+') {
            sign = 1;
            len--;
        }

        for (int i = 0; i < len; i++) {
            if (input_field[i] >= '0' && input_field[i] <= '9') {
                val = val * 10 + (input_field[i] - '0');
            }
        }
    }

    *out_val = val * sign;
    return 0;
}

static int get_char_weight(char c, int is_univac) {
    if (c == ' ' || c == '\0' || c == '$') return 0;
    if (is_univac) {
        if (c >= '0' && c <= '9') return (c - '0') + 1;
        if (c >= 'A' && c <= 'Z') return (c - 'A') + 11;
        if (c >= 'a' && c <= 'z') return (c - 'a') + 11;
    } else {
        if (c >= 'A' && c <= 'Z') return (c - 'A') + 1;
        if (c >= 'a' && c <= 'z') return (c - 'a') + 1;
        if (c >= '0' && c <= '9') return (c - '0') + 27;
    }
    return (int)c + 100;
}

int tsfi_s370_cobol_compare_collating(const char *str_a, const char *str_b, int is_univac) {
    if (!str_a || !str_b) return 0;

    int i = 0;
    while (1) {
        char ca = str_a[i];
        char cb = str_b[i];

        int wa = get_char_weight(ca, is_univac);
        int wb = get_char_weight(cb, is_univac);

        if (wa < wb) return -1;
        if (wa > wb) return 1;

        if (ca == '\0' || ca == '$' || cb == '\0' || cb == '$') {
            break;
        }
        i++;
    }
    return 0;
}

int tsfi_s370_cross_compiler_parity_loop(const char *cobol_strategy_script, int val_r0, int val_r1, int *out_result) {
    if (!cobol_strategy_script || !out_result) return -1;

    uint8_t bytecode[256];
    int len = 0;
    int res = tsfi_strategy_compile_script(cobol_strategy_script, bytecode, 256, &len);
    if (res != 0) return -2;

    char rca_input_0[32];
    char rca_input_1[32];
    snprintf(rca_input_0, sizeof(rca_input_0), "%s%d$", (val_r0 >= 0 ? "+" : ""), val_r0);
    snprintf(rca_input_1, sizeof(rca_input_1), "%s%d$", (val_r1 >= 0 ? "+" : ""), val_r1);

    int64_t rca_reg0_val = 0;
    int64_t rca_reg1_val = 0;
    tsfi_s370_normalize_signed_field(rca_input_0, 0, &rca_reg0_val);
    tsfi_s370_normalize_signed_field(rca_input_1, 0, &rca_reg1_val);

    TSFiStrategyVM rca_vm;
    tsfi_strategy_vm_init(&rca_vm);
    rca_vm.registers[0] = (int)rca_reg0_val;
    rca_vm.registers[1] = (int)rca_reg1_val;

    res = tsfi_strategy_vm_execute_bytecode(&rca_vm, NULL, bytecode, len, NULL);
    if (res != 0) return -3;
    int rca_out_val = rca_vm.registers[0];

    char rca_output[32];
    snprintf(rca_output, sizeof(rca_output), "%s%d$", (rca_out_val >= 0 ? "+" : ""), rca_out_val);

    char univac_input_0[16];
    char univac_input_1[16];
    tsfi_s370_rca501_to_univac2(rca_input_0, univac_input_0);
    tsfi_s370_rca501_to_univac2(rca_input_1, univac_input_1);

    int64_t univac_reg0_val = 0;
    int64_t univac_reg1_val = 0;
    tsfi_s370_normalize_signed_field(univac_input_0, 1, &univac_reg0_val);
    tsfi_s370_normalize_signed_field(univac_input_1, 1, &univac_reg1_val);

    TSFiStrategyVM univac_vm;
    tsfi_strategy_vm_init(&univac_vm);
    univac_vm.registers[0] = (int)univac_reg0_val;
    univac_vm.registers[1] = (int)univac_reg1_val;

    res = tsfi_strategy_vm_execute_bytecode(&univac_vm, NULL, bytecode, len, NULL);
    if (res != 0) return -4;
    int univac_out_val = univac_vm.registers[0];

    char univac_output[16];
    snprintf(univac_output, sizeof(univac_output), "%s%d", (univac_out_val >= 0 ? "+" : ""), univac_out_val);
    char univac_output_padded[16];
    tsfi_s370_rca501_to_univac2(univac_output, univac_output_padded);

    char rca_translated_output[32];
    tsfi_s370_univac2_to_rca501(univac_output_padded, rca_translated_output);

    if (strcmp(rca_output, rca_translated_output) != 0) {
        return -5;
    }

    *out_result = rca_out_val;
    return 0;
}

int tsfi_s370_cobol_tombstone_report(char *report_out, int max_len) {
    if (!report_out || max_len < 256) return -1;

    const char *tombstone_art = 
        "     .------.\n"
        "    /  R.I.P. \\\n"
        "   |    COBOL   |\n"
        "   |            |\n"
        "   |  1959-1960 |\n"
        "   |   CODASYL  |\n"
        "   |            |\n"
        "  *|____________|*\n";

    strncpy(report_out, tombstone_art, max_len - 1);
    report_out[max_len - 1] = '\0';
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

static int gps_solve_recursive(tsfi_gps_state *gps, int goal_feature, int *applied_ops, int *ops_count, int max_ops, int depth) {
    if (depth > 10) return -1; // Prevent infinite loops
    
    // If we already have the feature, we are done
    if (gps->current_features & (1 << goal_feature)) {
        return 0;
    }
    
    // Find an operator that sets this feature
    for (int i = 0; i < gps->operator_count; i++) {
        tsfi_gps_operator op = gps->operators[i];
        if (op.add_feature == goal_feature) {
            // Check condition
            if (op.condition_diff != -1) {
                // Try to satisfy condition recursively
                if (gps_solve_recursive(gps, op.condition_diff, applied_ops, ops_count, max_ops, depth + 1) != 0) {
                    continue; // try another operator if this path fails
                }
            }
            // Apply operator
            if (*ops_count >= max_ops) return -1;
            applied_ops[(*ops_count)++] = i;
            gps->current_features |= (1 << goal_feature);
            return 0;
        }
    }
    return -1; // No operator can set this feature
}

int tsfi_s370_gps_solve(tsfi_gps_state *gps, int *applied_ops_out, int max_ops) {
    if (!gps || !applied_ops_out || max_ops <= 0) return -1;
    
    int ops_count = 0;
    // For each feature in goal_features, if it's missing in current_features, reduce difference
    for (int f = 0; f < 32; f++) {
        if (gps->goal_features & (1 << f)) {
            if (!(gps->current_features & (1 << f))) {
                if (gps_solve_recursive(gps, f, applied_ops_out, &ops_count, max_ops, 0) != 0) {
                    return -1; // Failed to solve a goal feature
                }
            }
        }
    }
    return ops_count;
}

int tsfi_b5000_descriptor_read(const tsfi_b5000_descriptor *desc, const uint8_t *memory, uint32_t offset, uint8_t *val_out) {
    if (!desc || !memory || !val_out) return -1;
    if (!desc->is_present) return -2; // Segment not present
    if (offset >= desc->limit) return -3; // Bounds check fail (Descriptor Protection)
    
    *val_out = memory[desc->address + offset];
    return 0;
}

int tsfi_b5000_descriptor_write(const tsfi_b5000_descriptor *desc, uint8_t *memory, uint32_t offset, uint8_t val) {
    if (!desc || !memory) return -1;
    if (!desc->is_present) return -2; // Segment not present
    if (desc->read_only) return -3; // Read-only violation
    if (offset >= desc->limit) return -4; // Bounds check fail (Descriptor Protection)
    
    memory[desc->address + offset] = val;
    return 0;
}

int tsfi_b5000_segment_load(const char *filepath, tsfi_b5000_descriptor *desc_out, uint8_t *memory, uint32_t mem_offset, uint32_t max_bytes) {
    if (!filepath || !desc_out || !memory) return -1;
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return -2;
    
    size_t bytes_read = fread(memory + mem_offset, 1, max_bytes, f);
    fclose(f);
    
    if (bytes_read == 0) return -3;
    
    desc_out->address = mem_offset;
    desc_out->limit = (uint32_t)bytes_read;
    desc_out->is_present = 1;
    desc_out->read_only = 0; // Default readable/writable
    
    return 0;
}

void tsfi_b5000_mcp_init(tsfi_b5000_mcp_scheduler *sched) {
    if (!sched) return;
    sched->active_task_idx = -1;
    for (int i = 0; i < 4; i++) {
        sched->tasks[i].task_id = i;
        sched->tasks[i].pc = 0;
        memset(sched->tasks[i].registers, 0, sizeof(sched->tasks[i].registers));
        memset(sched->tasks[i].eval_stack, 0, sizeof(sched->tasks[i].eval_stack));
        sched->tasks[i].eval_stack_ptr = 0;
        sched->tasks[i].state = 0; // IDLE
    }
}

int tsfi_b5000_mcp_schedule_tick(tsfi_b5000_mcp_scheduler *sched) {
    if (!sched) return -1;
    
    int start = (sched->active_task_idx == -1) ? 0 : (sched->active_task_idx + 1) % 4;
    for (int i = 0; i < 4; i++) {
        int idx = (start + i) % 4;
        if (sched->tasks[idx].state == 1) { // RUNNABLE
            sched->active_task_idx = idx;
            return idx;
        }
    }
    return -1; // No runnable tasks
}

int tsfi_b5000_mcp_yield_active(tsfi_b5000_mcp_scheduler *sched, int block_reason) {
    if (!sched || sched->active_task_idx == -1) return -1;
    
    int active = sched->active_task_idx;
    sched->tasks[active].state = block_reason; // block or set idle
    return tsfi_b5000_mcp_schedule_tick(sched);
}

int tsfi_b5000_decode_syllable(uint16_t syllable, uint8_t *type_out, uint16_t *val_out) {
    if (!type_out || !val_out) return -1;
    *type_out = (uint8_t)((syllable >> 10) & 0x03);
    *val_out = (uint16_t)(syllable & 0x3FF);
    return 0;
}

int tsfi_b5000_execute_word(uint64_t instruction_word, void *strategy_vm, const uint8_t *memory, const tsfi_b5000_descriptor *prt, int prt_size) {
    if (!strategy_vm || !memory || !prt || prt_size <= 0) return -1;
    
    TSFiStrategyVM *vm = (TSFiStrategyVM *)strategy_vm;
    
    uint16_t syllables[4];
    syllables[0] = (uint16_t)((instruction_word >> 36) & 0xFFF);
    syllables[1] = (uint16_t)((instruction_word >> 24) & 0xFFF);
    syllables[2] = (uint16_t)((instruction_word >> 12) & 0xFFF);
    syllables[3] = (uint16_t)(instruction_word & 0xFFF);
    
    for (int i = 0; i < 4; i++) {
        uint8_t type = 0;
        uint16_t val = 0;
        tsfi_b5000_decode_syllable(syllables[i], &type, &val);
        
        if (type == 2) { // Literal Call
            if (vm->eval_stack_ptr < 32) {
                vm->eval_stack[vm->eval_stack_ptr++] = val;
            } else {
                return -2; // Stack overflow
            }
        } else if (type == 0) { // Operand Call
            if (val >= prt_size || !prt[val].is_present) return -3;
            // Read value from descriptor mapping
            uint8_t byte_val = 0;
            if (tsfi_b5000_descriptor_read(&prt[val], memory, 0, &byte_val) != 0) return -4;
            if (vm->eval_stack_ptr < 32) {
                vm->eval_stack[vm->eval_stack_ptr++] = byte_val;
            } else {
                return -2;
            }
        } else if (type == 1) { // Descriptor Call
            if (val >= prt_size || !prt[val].is_present) return -3;
            if (vm->eval_stack_ptr < 32) {
                vm->eval_stack[vm->eval_stack_ptr++] = (int)prt[val].address;
            } else {
                return -2;
            }
        } else if (type == 3) { // Operator
            if (val == 0) { // ADD
                if (vm->eval_stack_ptr >= 2) {
                    int b = vm->eval_stack[--vm->eval_stack_ptr];
                    int a = vm->eval_stack[--vm->eval_stack_ptr];
                    vm->eval_stack[vm->eval_stack_ptr++] = a + b;
                } else {
                    return -5; // Stack underflow
                }
            } else if (val == 1) { // SUB
                if (vm->eval_stack_ptr >= 2) {
                    int b = vm->eval_stack[--vm->eval_stack_ptr];
                    int a = vm->eval_stack[--vm->eval_stack_ptr];
                    vm->eval_stack[vm->eval_stack_ptr++] = a - b;
                } else {
                    return -5;
                }
            }
        }
    }
    return 0;
}

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

void tsfi_univac_posting_init(tsfi_univac_posting_interpreter *interp) {
    if (!interp) return;
    memset(interp->retained_data, ' ', 80);
    interp->has_master_data = 0;
}

int tsfi_univac_posting_process(tsfi_univac_posting_interpreter *interp, const tsfi_ramac_card *card_in, tsfi_ramac_card *card_out) {
    if (!interp || !card_in || !card_out) return -1;
    
    // Check for control punch tags:
    // '*' = master card control punch: retain the data
    if (card_in->columns[0] == '*') {
        memcpy(interp->retained_data, &card_in->columns[1], 79);
        interp->retained_data[79] = ' '; // ensure boundary space padding
        interp->has_master_data = 1;
        // Output card gets the input card directly
        memcpy(card_out->columns, card_in->columns, 80);
        return 1; // Handled master
    }
    
    // '!' = clear control punch: clear the retained registers
    if (card_in->columns[0] == '!') {
        memset(interp->retained_data, ' ', 80);
        interp->has_master_data = 0;
        memcpy(card_out->columns, card_in->columns, 80);
        return 2; // Cleared
    }
    
    // Detail card: post the retained master data into columns 0-39, and details into columns 40-79
    if (interp->has_master_data) {
        memcpy(card_out->columns, interp->retained_data, 40);
        memcpy(&card_out->columns[40], &card_in->columns[40], 40);
    } else {
        memcpy(card_out->columns, card_in->columns, 80);
    }
    
    return 0; // Handled detail
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

void tsfi_winchester_socket_init(tsfi_winchester_socket_bridge *bridge, int port) {
    if (!bridge) return;
    bridge->listen_port = port;
    bridge->connection_active = 1;
    bridge->processed_packets = 0;
}

int tsfi_winchester_socket_route_event(tsfi_winchester_socket_bridge *bridge, const uint8_t *event_data, int len, void *pq) {
    if (!bridge || !event_data || len <= 0 || !pq) return -1;
    if (!bridge->connection_active) return -2;
    
    uint8_t keycode = event_data[0];
    TSFiPriorityQueue *queue = (TSFiPriorityQueue *)pq;
    
    tsfi_priority_queue_push(queue, 10, keycode, "");
    
    bridge->processed_packets++;
    return 0;
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

int tsfi_uniservo_init(tsfi_uniservo_tape *tape, const char *filepath) {
    if (!tape || !filepath) return -1;
    strncpy(tape->filepath, filepath, sizeof(tape->filepath) - 1);
    tape->filepath[sizeof(tape->filepath) - 1] = '\0';
    tape->current_block_pos = 0;
    tape->parity_errors = 0;
    
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        tape->total_blocks = 0;
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    tape->total_blocks = (uint32_t)(size / 256);
    return 0;
}

int tsfi_uniservo_read_block(tsfi_uniservo_tape *tape, uint32_t block_idx, uint8_t *buffer, int buf_len) {
    if (!tape || !buffer || buf_len < 256) return -1;
    
    FILE *f = fopen(tape->filepath, "rb");
    if (!f) return -2;
    
    if (fseek(f, block_idx * 256, SEEK_SET) != 0) {
        fclose(f);
        return -3;
    }
    
    size_t read_bytes = fread(buffer, 1, 256, f);
    fclose(f);
    
    if (read_bytes < 256) return -4;
    
    uint8_t xor_sum = 0;
    for (int i = 0; i < 255; i++) {
        xor_sum ^= buffer[i];
    }
    
    if (buffer[255] != xor_sum) {
        tape->parity_errors++;
        return -5;
    }
    
    tape->current_block_pos = block_idx;
    return 0;
}

int tsfi_uniservo_write_block(tsfi_uniservo_tape *tape, uint32_t block_idx, const uint8_t *buffer, int buf_len) {
    if (!tape || !buffer || buf_len < 256) return -1;
    
    FILE *f = fopen(tape->filepath, "r+b");
    if (!f) {
        f = fopen(tape->filepath, "wb");
        if (!f) return -2;
    }
    
    if (fseek(f, block_idx * 256, SEEK_SET) != 0) {
        fclose(f);
        return -3;
    }
    
    uint8_t out_block[256];
    memcpy(out_block, buffer, 255);
    
    uint8_t xor_sum = 0;
    for (int i = 0; i < 255; i++) {
        xor_sum ^= out_block[i];
    }
    out_block[255] = xor_sum;
    
    size_t written_bytes = fwrite(out_block, 1, 256, f);
    fclose(f);
    
    if (written_bytes < 256) return -4;
    
    tape->current_block_pos = block_idx;
    if (block_idx >= tape->total_blocks) {
        tape->total_blocks = block_idx + 1;
    }
    return 0;
}

int tsfi_cobol_sort_merge(const tsfi_cobol_fd *fd, tsfi_ramac_card *cards, int card_count) {
    if (!fd || !cards || card_count <= 0) return -1;
    if (fd->key_start < 0 || fd->key_start + fd->key_len > 80) return -2;
    
    // Bubble sort records based on key substring comparison
    for (int i = 0; i < card_count - 1; i++) {
        for (int j = 0; j < card_count - i - 1; j++) {
            char key_a[80];
            char key_b[80];
            memcpy(key_a, &cards[j].columns[fd->key_start], fd->key_len);
            key_a[fd->key_len] = '\0';
            memcpy(key_b, &cards[j+1].columns[fd->key_start], fd->key_len);
            key_b[fd->key_len] = '\0';
            
            if (strcmp(key_a, key_b) > 0) {
                tsfi_ramac_card temp = cards[j];
                cards[j] = cards[j+1];
                cards[j+1] = temp;
            }
        }
    }
    return 0;
}

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

void tsfi_rwcs_init(tsfi_rwcs_report *rep, int page_limit, int line_limit) {
    if (!rep) return;
    rep->page_limit = page_limit;
    rep->line_limit = line_limit;
    rep->current_page = 1;
    rep->current_line = 0;
    rep->group_total = 0.0;
    rep->final_total = 0.0;
}

int tsfi_rwcs_write_header(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 128) return -1;
    int written = snprintf(out, max_len,
                           "=========================================\n"
                           "COBOL FINANCIAL AUDIT REPORT    PAGE: %02d\n"
                           "=========================================\n"
                           "ITEM-ID    ITEM-NAME             AMOUNT\n"
                           "-----------------------------------------\n",
                           rep->current_page);
    rep->current_line = 5;
    return written;
}

int tsfi_rwcs_process_item(tsfi_rwcs_report *rep, char *out, size_t max_len, int item_id, const char *name, double amount) {
    if (!rep || !out || max_len < 64) return -1;
    if (rep->current_line >= rep->line_limit) {
        rep->current_page++;
        int header_len = tsfi_rwcs_write_header(rep, out, max_len);
        int item_len = snprintf(out + header_len, max_len - header_len,
                                "%06d     %-20.20s  %10.2f\n",
                                item_id, name, amount);
        rep->group_total += amount;
        rep->final_total += amount;
        rep->current_line++;
        return header_len + item_len;
    } else {
        int item_len = snprintf(out, max_len,
                                "%06d     %-20.20s  %10.2f\n",
                                item_id, name, amount);
        rep->group_total += amount;
        rep->final_total += amount;
        rep->current_line++;
        return item_len;
    }
}

int tsfi_rwcs_control_break(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 64) return -1;
    int written = snprintf(out, max_len,
                           "-----------------------------------------\n"
                           "SUBTOTAL:                         %10.2f\n"
                           "=========================================\n",
                           rep->group_total);
    rep->group_total = 0.0;
    rep->current_line += 3;
    return written;
}

int tsfi_rwcs_write_final(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 64) return -1;
    int written = snprintf(out, max_len,
                           "-----------------------------------------\n"
                           "GRAND TOTAL:                      %10.2f\n"
                           "=========================================\n",
                           rep->final_total);
    rep->current_line += 3;
    return written;
}

void tsfi_mcs_init(tsfi_mcs_queue *q, const char *name) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_mcs_queue));
    strcpy(q->status_key, "00");
    pthread_mutex_init(&q->lock, NULL);
    if (name) {
        strncpy(q->queue_name, name, sizeof(q->queue_name) - 1);
    }
}

int tsfi_mcs_send(tsfi_mcs_queue *q, const char *msg, void *wmq_void) {
    if (!q || !msg) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count >= 8) {
        strcpy(q->status_key, "10");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(q->messages[q->tail], msg, sizeof(q->messages[q->tail]) - 1);
    q->indicators[q->tail] = MCS_EMI;
    q->tail = (q->tail + 1) % 8;
    q->count++;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    TSFiWinchesterBridge *wmq = (TSFiWinchesterBridge *)wmq_void;
    if (wmq) {
        wmq->registers.status_reg = 1;
        wmq->registers.keycode_reg = 32;
        uint32_t packed_word = 0;
        memcpy(&packed_word, msg, (strlen(msg) < 4) ? strlen(msg) : 4);
        wmq->registers.data_reg = packed_word;
        tsfi_winchester_bridge_handshake(wmq);
    }
    return 0;
}

int tsfi_mcs_receive(tsfi_mcs_queue *q, char *msg_out, size_t max_len) {
    if (!q || !msg_out || max_len == 0) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count == 0) {
        strcpy(q->status_key, "20");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(msg_out, q->messages[q->head], max_len - 1);
    msg_out[max_len - 1] = '\0';
    q->head = (q->head + 1) % 8;
    q->count--;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    return 0;
}

void tsfi_mcs_init_hierarchical(tsfi_mcs_queue *q, const char *q_name, const char *sq1, const char *sq2, const char *sq3) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_mcs_queue));
    strcpy(q->status_key, "00");
    pthread_mutex_init(&q->lock, NULL);
    if (q_name) strncpy(q->queue_name, q_name, sizeof(q->queue_name) - 1);
    if (sq1) strncpy(q->sub_queue1, sq1, sizeof(q->sub_queue1) - 1);
    if (sq2) strncpy(q->sub_queue2, sq2, sizeof(q->sub_queue2) - 1);
    if (sq3) strncpy(q->sub_queue3, sq3, sizeof(q->sub_queue3) - 1);
}

int tsfi_mcs_send_segment(tsfi_mcs_queue *q, const char *msg, uint8_t indicator, void *wmq_void) {
    if (!q || !msg) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count >= 8) {
        strcpy(q->status_key, "10");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(q->messages[q->tail], msg, sizeof(q->messages[q->tail]) - 1);
    q->indicators[q->tail] = indicator;
    q->tail = (q->tail + 1) % 8;
    q->count++;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    TSFiWinchesterBridge *wmq = (TSFiWinchesterBridge *)wmq_void;
    if (wmq) {
        wmq->registers.status_reg = 1;
        wmq->registers.keycode_reg = 32;
        uint32_t packed_word = 0;
        memcpy(&packed_word, msg, (strlen(msg) < 4) ? strlen(msg) : 4);
        wmq->registers.data_reg = packed_word;
        tsfi_winchester_bridge_handshake(wmq);
    }
    return 0;
}

int tsfi_mcs_receive_segment(tsfi_mcs_queue *q, char *msg_out, size_t max_len, uint8_t *indicator_out) {
    if (!q || !msg_out || max_len == 0) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count == 0) {
        strcpy(q->status_key, "20");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(msg_out, q->messages[q->head], max_len - 1);
    msg_out[max_len - 1] = '\0';
    if (indicator_out) {
        *indicator_out = q->indicators[q->head];
    }
    q->head = (q->head + 1) % 8;
    q->count--;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    return 0;
}

void tsfi_mcs_assembly_init(tsfi_mcs_assembly *buf) {
    if (!buf) return;
    memset(buf, 0, sizeof(tsfi_mcs_assembly));
}

int tsfi_mcs_assemble_next(tsfi_mcs_queue *q, tsfi_mcs_assembly *buf, char *msg_out, size_t max_len) {
    if (!q || !buf || !msg_out || max_len == 0) return -1;
    char temp_segment[128];
    uint8_t indicator = 0;
    int rx_res = tsfi_mcs_receive_segment(q, temp_segment, sizeof(temp_segment), &indicator);
    if (rx_res != 0) return rx_res;
    size_t seg_len = strlen(temp_segment);
    if (buf->assembly_len + seg_len + 1 > sizeof(buf->assembly_buffer)) {
        strcpy(q->status_key, "30");
        return -3;
    }
    memcpy(buf->assembly_buffer + buf->assembly_len, temp_segment, seg_len);
    buf->assembly_len += seg_len;
    buf->assembly_buffer[buf->assembly_len] = '\0';
    if (indicator == MCS_EMI || indicator == MCS_EGI) {
        strncpy(msg_out, buf->assembly_buffer, max_len - 1);
        msg_out[max_len - 1] = '\0';
        buf->assembly_len = 0;
        buf->assembly_buffer[0] = '\0';
        return 1;
    }
    return 0;
}

void tsfi_subschema_init(tsfi_subschema_map *map, const char *name, const char *rec, const char *set) {
    if (!map) return;
    memset(map, 0, sizeof(tsfi_subschema_map));
    if (name) strncpy(map->subschema_name, name, sizeof(map->subschema_name) - 1);
    if (rec) strncpy(map->record_name, rec, sizeof(map->record_name) - 1);
    if (set) strncpy(map->set_name, set, sizeof(map->set_name) - 1);
}

int tsfi_subschema_map_data(const tsfi_subschema_map *map, const uint8_t *db_record_data, int *registers_out) {
    if (!map || !db_record_data || !registers_out) return -1;
    for (int i = 0; i < map->field_count && i < 8; i++) {
        int offset = map->field_offsets[i];
        uint32_t val = (db_record_data[offset] << 24) |
                       (db_record_data[offset+1] << 16) |
                       (db_record_data[offset+2] << 8) |
                       db_record_data[offset+3];
        registers_out[i] = (int)val;
    }
    return 0;
}

void tsfi_dbtg_currency_init(tsfi_dbtg_currency *cur) {
    if (!cur) return;
    cur->current_run_unit = -1;
    for (int i = 0; i < 8; i++) {
        cur->current_record_type[i] = -1;
        cur->current_set_type[i] = -1;
    }
}

void tsfi_dbtg_currency_update(tsfi_dbtg_currency *cur, int run_unit, int record_type, int set_type) {
    if (!cur) return;
    cur->current_run_unit = run_unit;
    if (record_type >= 0 && record_type < 8) {
        cur->current_record_type[record_type] = run_unit;
    }
    if (set_type >= 0 && set_type < 8) {
        cur->current_set_type[set_type] = run_unit;
    }
}

void tsfi_dbtg_realm_init(tsfi_dbtg_realm_registry *reg) {
    if (!reg) return;
    memset(reg, 0, sizeof(tsfi_dbtg_realm_registry));
}

int tsfi_dbtg_realm_register(tsfi_dbtg_realm_registry *reg, const char *name) {
    if (!reg || !name) return -1;
    if (reg->area_count >= 4) return -2;
    strncpy(reg->areas[reg->area_count].area_name, name, sizeof(reg->areas[reg->area_count].area_name) - 1);
    reg->areas[reg->area_count].lock_mode = DBTG_LOCK_NONE;
    reg->areas[reg->area_count].is_open = 0;
    reg->area_count++;
    return 0;
}

int tsfi_dbtg_realm_open(tsfi_dbtg_realm_registry *reg, const char *name, int lock_mode) {
    if (!reg || !name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, name) == 0) {
            if (reg->areas[i].is_open) return -3;
            reg->areas[i].lock_mode = lock_mode;
            reg->areas[i].is_open = 1;
            return 0;
        }
    }
    return -4;
}

int tsfi_dbtg_realm_close(tsfi_dbtg_realm_registry *reg, const char *name) {
    if (!reg || !name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, name) == 0) {
            if (!reg->areas[i].is_open) return -3;
            reg->areas[i].lock_mode = DBTG_LOCK_NONE;
            reg->areas[i].is_open = 0;
            return 0;
        }
    }
    return -4;
}

void tsfi_dbtg_exception_init(tsfi_dbtg_exception_context *ctx) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(tsfi_dbtg_exception_context));
    ctx->db_status = DB_STATUS_OK;
}

int tsfi_dbtg_validate_action(tsfi_dbtg_exception_context *ctx, const tsfi_dbtg_realm_registry *reg, const char *realm_name, int is_write_action) {
    if (!ctx || !reg || !realm_name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, realm_name) == 0) {
            if (!reg->areas[i].is_open) {
                ctx->db_status = DB_STATUS_NOT_OPEN;
                strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
                ctx->exception_triggered = 1;
                return -2;
            }
            if (is_write_action) {
                if (reg->areas[i].lock_mode == DBTG_LOCK_RETRIEVAL ||
                    reg->areas[i].lock_mode == DBTG_LOCK_EXCLUSIVE_RETRIEVAL) {
                    ctx->db_status = DB_STATUS_LOCK_VIOLATION;
                    strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
                    ctx->exception_triggered = 1;
                    return -3;
                }
            }
            ctx->db_status = DB_STATUS_OK;
            ctx->exception_triggered = 0;
            return 0;
        }
    }
    ctx->db_status = DB_STATUS_NOT_OPEN;
    strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
    ctx->exception_triggered = 1;
    return -4;
}

void tsfi_dbtg_set_membership_init(tsfi_dbtg_set_membership *sm, const char *name, int ins_mode, int ret_mode) {
    if (!sm) return;
    memset(sm, 0, sizeof(tsfi_dbtg_set_membership));
    if (name) strncpy(sm->set_name, name, sizeof(sm->set_name) - 1);
    sm->insertion_mode = ins_mode;
    sm->retention_mode = ret_mode;
}

int tsfi_dbtg_validate_disconnect(const tsfi_dbtg_set_membership *sm, int *db_status_out) {
    if (!sm || !db_status_out) return -1;
    if (sm->retention_mode == DBTG_RETENTION_MANDATORY) {
        *db_status_out = DB_STATUS_MEMBER_ERR;
        return -2;
    }
    *db_status_out = DB_STATUS_OK;
    return 0;
}

int tsfi_dbtg_navigate_set(int current_idx, int record_count, int direction_mode) {
    if (record_count <= 0) return -1;
    switch (direction_mode) {
        case DBTG_NAV_FIRST:
            return 0;
        case DBTG_NAV_LAST:
            return record_count - 1;
        case DBTG_NAV_NEXT:
            return (current_idx + 1) % record_count;
        case DBTG_NAV_PRIOR:
            return (current_idx - 1 + record_count) % record_count;
        default:
            return -1;
    }
}

int tsfi_dbtg_insert_sorted(tsfi_dbtg_sorted_record *records, int current_count, int max_capacity, int record_id, int sort_key) {
    if (!records || current_count < 0 || current_count >= max_capacity) return -1;
    int insert_idx = 0;
    while (insert_idx < current_count && records[insert_idx].sort_key < sort_key) {
        insert_idx++;
    }
    for (int i = current_count; i > insert_idx; i--) {
        records[i] = records[i - 1];
    }
    records[insert_idx].record_id = record_id;
    records[insert_idx].sort_key = sort_key;
    return current_count + 1;
}

void tsfi_dbtg_resolve_calc(const char *key, uint32_t total_pages, uint32_t slots_per_page, tsfi_dbtg_calc_address *address_out) {
    if (!key || total_pages == 0 || slots_per_page == 0 || !address_out) return;
    uint32_t hash = 2166136261U;
    while (*key) {
        hash ^= (uint8_t)*key++;
        hash *= 16777619U;
    }
    address_out->target_page = hash % total_pages;
    address_out->target_slot = (hash / total_pages) % slots_per_page;
}

void tsfi_dbtg_dml_tracker_init(tsfi_dbtg_dml_tracker *tracker) {
    if (!tracker) return;
    memset(tracker, 0, sizeof(tsfi_dbtg_dml_tracker));
}

int tsfi_dbtg_execute_dml(tsfi_dbtg_dml_tracker *tracker, int verb_opcode) {
    if (!tracker) return -1;
    switch (verb_opcode) {
        case DBTG_VERB_STORE:
            tracker->store_count++;
            break;
        case DBTG_VERB_GET:
            tracker->get_count++;
            break;
        case DBTG_VERB_MODIFY:
            tracker->modify_count++;
            break;
        case DBTG_VERB_ERASE:
            tracker->erase_count++;
            break;
        case DBTG_VERB_CONNECT:
            tracker->connect_count++;
            break;
        case DBTG_VERB_DISCONNECT:
            tracker->disconnect_count++;
            break;
        default:
            return -2;
    }
    return 0;
}

void tsfi_s370_vs_dat_init(tsfi_s370_vs_dat *dat) {
    if (!dat) return;
    memset(dat, 0, sizeof(tsfi_s370_vs_dat));
}

int tsfi_s370_vs_dat_translate(const tsfi_s370_vs_dat *dat, uint32_t virtual_address, uint32_t *physical_address_out) {
    if (!dat || !physical_address_out) return -1;
    uint32_t seg_idx = (virtual_address >> 12) & 0x0F;
    uint32_t page_idx = (virtual_address >> 8) & 0x0F;
    uint32_t offset = virtual_address & 0xFF;
    if (dat->segment_table[seg_idx] == 0) {
        return -2;
    }
    uint32_t page_frame = dat->page_tables[seg_idx][page_idx];
    if (page_frame == 0) {
        return -3;
    }
    *physical_address_out = page_frame + offset;
    return 0;
}

void tsfi_relational_tuple_init(tsfi_relational_tuple *t, const char *rel, const char *key) {
    if (!t) return;
    memset(t, 0, sizeof(tsfi_relational_tuple));
    if (rel) strncpy(t->relation_name, rel, sizeof(t->relation_name) - 1);
    if (key) strncpy(t->tuple_key, key, sizeof(t->tuple_key) - 1);
    t->resolved_record_id = -1;
}

int tsfi_relational_map_to_codasyl(const tsfi_relational_tuple *t, const tsfi_dbtg_realm_registry *realm_reg, int *db_status_out) {
    if (!t || !realm_reg || !db_status_out) return -1;
    int found_open_realm = 0;
    for (int i = 0; i < realm_reg->area_count; i++) {
        if (strcmp(realm_reg->areas[i].area_name, t->relation_name) == 0 && realm_reg->areas[i].is_open) {
            found_open_realm = 1;
            break;
        }
    }
    if (!found_open_realm) {
        *db_status_out = DB_STATUS_NOT_OPEN;
        return -2;
    }
    *db_status_out = DB_STATUS_OK;
    return 0;
}

void tsfi_structured_analyze_script(const char *script, tsfi_structured_analysis_report *report) {
    if (!script || !report) return;
    memset(report, 0, sizeof(tsfi_structured_analysis_report));
    char script_copy[2048];
    strncpy(script_copy, script, 2047);
    script_copy[2047] = '\0';
    const char *delims = " ;(),{}\"\n\r\t.";
    char labels[32][32];
    int label_count = 0;
    char *tokens[256];
    int token_count = 0;
    char *tok = strtok(script_copy, delims);
    while (tok && token_count < 256) {
        tokens[token_count++] = tok;
        tok = strtok(NULL, delims);
    }
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        if (len > 1 && tokens[i][len - 1] == ':') {
            char lbl[32];
            size_t copy_len = (len - 1 < 31) ? len - 1 : 31;
            memcpy(lbl, tokens[i], copy_len);
            lbl[copy_len] = '\0';
            if (label_count < 32) {
                memcpy(labels[label_count], lbl, copy_len + 1);
                label_count++;
            }
        }
        if (i + 1 < token_count &&
            ((strcasecmp(tokens[i], "GO") == 0 && strcasecmp(tokens[i + 1], "TO") == 0) ||
             (strcasecmp(tokens[i], "GOTO") == 0))) {
            report->goto_count++;
            char *target = (strcasecmp(tokens[i], "GOTO") == 0) ? tokens[i + 1] : tokens[i + 2];
            for (int k = 0; k < label_count; k++) {
                if (strcmp(labels[k], target) == 0) {
                    report->backward_jmp_detected = 1;
                    break;
                }
            }
        }
    }
}

void tsfi_vtam_session_init(tsfi_vtam_session *session, const char *lu_name) {
    if (!session) return;
    memset(session, 0, sizeof(tsfi_vtam_session));
    if (lu_name) strncpy(session->logical_unit, lu_name, sizeof(session->logical_unit) - 1);
    session->session_state = VTAM_STATE_INIT;
}

int tsfi_vtam_session_handshake(tsfi_vtam_session *session, int event) {
    if (!session) return -1;
    switch (session->session_state) {
        case VTAM_STATE_INIT:
            if (event == VTAM_EV_BIND) {
                session->session_state = VTAM_STATE_NEGOTIATE;
                return 0;
            }
            break;
        case VTAM_STATE_NEGOTIATE:
            if (event == VTAM_EV_SDT) {
                session->session_state = VTAM_STATE_ACTIVE;
                return 0;
            }
            break;
        case VTAM_STATE_ACTIVE:
            if (event == VTAM_EV_UNBIND) {
                session->session_state = VTAM_STATE_TERMINATED;
                return 0;
            }
            break;
        default:
            break;
    }
    return -2;
}

int tsfi_vtam_session_send(tsfi_vtam_session *session, const char *data, int data_len) {
    if (!session || !data || data_len <= 0) return -1;
    if (session->session_state != VTAM_STATE_ACTIVE) {
        return -3;
    }
    session->bytes_transmitted += data_len;
    return data_len;
}

void tsfi_cms_session_init(tsfi_cms_session *sess) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cms_session));
}

int tsfi_cms_execute_command(tsfi_cms_session *sess, const char *cmd) {
    if (!sess || !cmd) return -1;
    if (strcasecmp(cmd, "IPL CMS") == 0) {
        sess->is_booted = 1;
        sess->last_command_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "ACCESS 191 A") == 0) {
        if (!sess->is_booted) {
            sess->last_command_status = -2;
            return -2;
        }
        sess->disk_mounted_191 = 1;
        sess->last_command_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "LISTFILE") == 0) {
        if (!sess->disk_mounted_191) {
            sess->last_command_status = -3;
            return -3;
        }
        sess->last_command_status = 0;
        return 0;
    }
    sess->last_command_status = -1;
    return -1;
}

void tsfi_cp_session_init(tsfi_cp_session *sess) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cp_session));
    sess->virtual_storage_kb = 4096;
    sess->spool_class = ' ';
    sess->last_cp_status = 0;
}

int tsfi_cp_execute_command(tsfi_cp_session *sess, const char *cmd) {
    if (!sess || !cmd) return -1;
    if (strcasecmp(cmd, "DEFINE STORAGE 16M") == 0) {
        sess->virtual_storage_kb = 16384;
        sess->last_cp_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "SPOOL PUNCH CLASS A") == 0) {
        sess->spool_class = 'A';
        sess->last_cp_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "QUERY VIRTUAL") == 0) {
        sess->last_cp_status = 0;
        return 0;
    }
    sess->last_cp_status = -1;
    return -1;
}

void tsfi_cp_spool_queue_init(tsfi_cp_spool_queue *q) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue));
}

int tsfi_cp_spool_push(tsfi_cp_spool_queue *q, const char *data) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS) return -1;
    strncpy(q->queue[q->tail].card_data, data, sizeof(q->queue[q->tail].card_data) - 1);
    q->queue[q->tail].card_data[sizeof(q->queue[q->tail].card_data) - 1] = '\0';
    q->tail = (q->tail + 1) % MAX_SPOOL_CARDS;
    q->count++;
    return 0;
}

int tsfi_cp_spool_pop(tsfi_cp_spool_queue *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    strncpy(data_out, q->queue[q->head].card_data, 79);
    data_out[79] = '\0';
    q->head = (q->head + 1) % MAX_SPOOL_CARDS;
    q->count--;
    return 0;
}

void tsfi_iucv_broker_init(tsfi_iucv_broker *broker) {
    if (!broker) return;
    memset(broker, 0, sizeof(tsfi_iucv_broker));
}

int tsfi_iucv_connect(tsfi_iucv_broker *broker, const char *src, const char *dest) {
    if (!broker || !src || !dest) return -1;
    for (int i = 0; i < MAX_IUCV_PATHS; i++) {
        if (broker->paths[i].status == IUCV_PATH_FREE) {
            broker->paths[i].path_id = i;
            strncpy(broker->paths[i].source_user, src, sizeof(broker->paths[i].source_user) - 1);
            strncpy(broker->paths[i].dest_user, dest, sizeof(broker->paths[i].dest_user) - 1);
            broker->paths[i].status = IUCV_PATH_ACTIVE;
            broker->paths[i].message_pending = 0;
            return i;
        }
    }
    return -2;
}

int tsfi_iucv_send(tsfi_iucv_broker *broker, int path_id, const char *msg) {
    if (!broker || path_id < 0 || path_id >= MAX_IUCV_PATHS) return -1;
    if (broker->paths[path_id].status != IUCV_PATH_ACTIVE) return -2;
    strncpy(broker->paths[path_id].buffered_message, msg, sizeof(broker->paths[path_id].buffered_message) - 1);
    broker->paths[path_id].buffered_message[sizeof(broker->paths[path_id].buffered_message) - 1] = '\0';
    broker->paths[path_id].message_pending = 1;
    return 0;
}

int tsfi_iucv_receive(tsfi_iucv_broker *broker, int path_id, char *msg_out) {
    if (!broker || path_id < 0 || path_id >= MAX_IUCV_PATHS || !msg_out) return -1;
    if (broker->paths[path_id].status != IUCV_PATH_ACTIVE || !broker->paths[path_id].message_pending) return -2;
    strncpy(msg_out, broker->paths[path_id].buffered_message, 63);
    msg_out[63] = '\0';
    broker->paths[path_id].message_pending = 0;
    return 0;
}

void tsfi_cp_console_spool_init(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    memset(spool, 0, sizeof(tsfi_cp_console_spool));
}

void tsfi_cp_console_spool_start(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    spool->is_spooling = 1;
    spool->log_len = 0;
    spool->log_buffer[0] = '\0';
}

void tsfi_cp_console_spool_write(tsfi_cp_console_spool *spool, const char *text) {
    if (!spool || !text || !spool->is_spooling) return;
    int len = strlen(text);
    if (spool->log_len + len < 511) {
        memcpy(spool->log_buffer + spool->log_len, text, len);
        spool->log_len += len;
        spool->log_buffer[spool->log_len] = '\0';
    }
}

void tsfi_cp_console_spool_stop(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    spool->is_spooling = 0;
}

void tsfi_cp_scheduler_init(tsfi_cp_scheduler *sched) {
    if (!sched) return;
    memset(sched, 0, sizeof(tsfi_cp_scheduler));
}

int tsfi_cp_scheduler_register(tsfi_cp_scheduler *sched, const char *name, int initial_q) {
    if (!sched || !name || sched->task_count >= 8) return -1;
    strncpy(sched->tasks[sched->task_count].vm_name, name, sizeof(sched->tasks[sched->task_count].vm_name) - 1);
    sched->tasks[sched->task_count].vm_name[sizeof(sched->tasks[sched->task_count].vm_name) - 1] = '\0';
    sched->tasks[sched->task_count].queue_type = initial_q;
    sched->tasks[sched->task_count].cpu_cycles_used = 0;
    sched->task_count++;
    return 0;
}

int tsfi_cp_scheduler_dispatch(tsfi_cp_scheduler *sched, int task_idx, int cycles) {
    if (!sched || task_idx < 0 || task_idx >= sched->task_count) return -1;
    sched->tasks[task_idx].cpu_cycles_used += cycles;
    if (sched->tasks[task_idx].queue_type == VM_QUEUE_Q1 && cycles > 50) {
        sched->tasks[task_idx].queue_type = VM_QUEUE_Q2;
    }
    return 0;
}

void tsfi_cp_directory_init(tsfi_cp_directory *dir) {
    if (!dir) return;
    memset(dir, 0, sizeof(tsfi_cp_directory));
}

int tsfi_cp_directory_add(tsfi_cp_directory *dir, const char *uid, char priv, uint32_t max_store) {
    if (!dir || !uid || dir->entry_count >= 8) return -1;
    strncpy(dir->entries[dir->entry_count].userid, uid, sizeof(dir->entries[dir->entry_count].userid) - 1);
    dir->entries[dir->entry_count].userid[sizeof(dir->entries[dir->entry_count].userid) - 1] = '\0';
    dir->entries[dir->entry_count].privilege_class = priv;
    dir->entries[dir->entry_count].max_storage_kb = max_store;
    dir->entry_count++;
    return 0;
}

int tsfi_cp_directory_check(const tsfi_cp_directory *dir, const char *uid, char required_priv) {
    if (!dir || !uid) return -1;
    for (int i = 0; i < dir->entry_count; i++) {
        if (strcmp(dir->entries[i].userid, uid) == 0) {
            if (required_priv == 'A' && dir->entries[i].privilege_class != 'A') {
                return -2;
            }
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_attachment_init(tsfi_cp_attachment_manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(tsfi_cp_attachment_manager));
}

int tsfi_cp_attachment_register(tsfi_cp_attachment_manager *mgr, uint32_t phys_addr) {
    if (!mgr || mgr->device_count >= 8) return -1;
    mgr->devices[mgr->device_count].physical_address = phys_addr;
    mgr->devices[mgr->device_count].virtual_address = 0;
    mgr->devices[mgr->device_count].is_attached = 0;
    memset(mgr->devices[mgr->device_count].dedicated_user, 0, 16);
    mgr->device_count++;
    return 0;
}

int tsfi_cp_attach(tsfi_cp_attachment_manager *mgr, uint32_t phys_addr, const char *uid, uint32_t virt_addr) {
    if (!mgr || !uid) return -1;
    for (int i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].physical_address == phys_addr) {
            if (mgr->devices[i].is_attached) {
                return -2;
            }
            mgr->devices[i].virtual_address = virt_addr;
            strncpy(mgr->devices[i].dedicated_user, uid, 15);
            mgr->devices[i].dedicated_user[15] = '\0';
            mgr->devices[i].is_attached = 1;
            return 0;
        }
    }
    return -3;
}

int tsfi_cp_detach(tsfi_cp_attachment_manager *mgr, uint32_t virt_addr, const char *uid) {
    if (!mgr || !uid) return -1;
    for (int i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].is_attached && mgr->devices[i].virtual_address == virt_addr) {
            if (strcmp(mgr->devices[i].dedicated_user, uid) != 0) {
                return -2;
            }
            mgr->devices[i].virtual_address = 0;
            memset(mgr->devices[i].dedicated_user, 0, 16);
            mgr->devices[i].is_attached = 0;
            return 0;
        }
    }
    return -3;
}

void tsfi_cp_smsg_init(tsfi_cp_smsg_receiver *rcv, const char *uid) {
    if (!rcv) return;
    memset(rcv, 0, sizeof(tsfi_cp_smsg_receiver));
    if (uid) {
        strncpy(rcv->userid, uid, sizeof(rcv->userid) - 1);
        rcv->userid[sizeof(rcv->userid) - 1] = '\0';
    }
    rcv->smsg_enabled = 1;
}

int tsfi_cp_smsg_send(tsfi_cp_smsg_receiver *rcv, const char *msg) {
    if (!rcv || !msg) return -1;
    if (!rcv->smsg_enabled) {
        return -1;
    }
    if (rcv->count >= 8) {
        return -2;
    }
    strncpy(rcv->queue[rcv->tail].payload, msg, sizeof(rcv->queue[rcv->tail].payload) - 1);
    rcv->queue[rcv->tail].payload[sizeof(rcv->queue[rcv->tail].payload) - 1] = '\0';
    rcv->tail = (rcv->tail + 1) % 8;
    rcv->count++;
    return 0;
}

int tsfi_cp_smsg_receive(tsfi_cp_smsg_receiver *rcv, char *msg_out) {
    if (!rcv || !msg_out || rcv->count <= 0) return -1;
    strncpy(msg_out, rcv->queue[rcv->head].payload, 63);
    msg_out[63] = '\0';
    rcv->head = (rcv->head + 1) % 8;
    rcv->count--;
    return 0;
}

void tsfi_cp_vcpu_init(tsfi_cp_vcpu *vcpu) {
    if (!vcpu) return;
    memset(vcpu, 0, sizeof(tsfi_cp_vcpu));
    vcpu->state = VCPU_STOPPED;
    vcpu->psw_instruction_address = 0x00000000;
    vcpu->psw_mask = 0x00000000;
}

int tsfi_cp_vcpu_control(tsfi_cp_vcpu *vcpu, const char *action) {
    if (!vcpu || !action) return -1;
    if (strcasecmp(action, "START") == 0) {
        vcpu->state = VCPU_RUNNING;
        return 0;
    }
    if (strcasecmp(action, "STOP") == 0) {
        vcpu->state = VCPU_STOPPED;
        return 0;
    }
    if (strcasecmp(action, "RESET") == 0) {
        vcpu->state = VCPU_STOPPED;
        vcpu->psw_instruction_address = 0x00000000;
        vcpu->psw_mask = 0x00000000;
        return 0;
    }
    return -1;
}

void tsfi_cp_printer_init(tsfi_cp_spool_printer *prt) {
    if (!prt) return;
    memset(prt, 0, sizeof(tsfi_cp_spool_printer));
    prt->page_count = 1;
}

int tsfi_cp_printer_write_record(tsfi_cp_spool_printer *prt, const char *record) {
    if (!prt || !record || record[0] == '\0') return -1;
    char ctrl = record[0];
    switch (ctrl) {
        case '1':
            prt->line_count = 0;
            prt->page_count++;
            prt->last_skip_count = 0;
            break;
        case ' ':
            prt->line_count += 1;
            prt->last_skip_count = 1;
            break;
        case '0':
            prt->line_count += 2;
            prt->last_skip_count = 2;
            break;
        case '-':
            prt->line_count += 3;
            prt->last_skip_count = 3;
            break;
        default:
            prt->line_count += 1;
            prt->last_skip_count = 1;
            break;
    }
    return 0;
}

void tsfi_cp_spool_queue_v2_init(tsfi_cp_spool_queue_v2 *q, char reader_class) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue_v2));
    q->active_reader_class = reader_class;
}

int tsfi_cp_spool_push_v2(tsfi_cp_spool_queue_v2 *q, const char *data, char card_class) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS_V2) return -1;
    strncpy(q->queue[q->count].card_data, data, sizeof(q->queue[q->count].card_data) - 1);
    q->queue[q->count].card_data[sizeof(q->queue[q->count].card_data) - 1] = '\0';
    q->queue[q->count].spool_class = card_class;
    q->count++;
    return 0;
}

int tsfi_cp_spool_pop_v2(tsfi_cp_spool_queue_v2 *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    for (int i = 0; i < q->count; i++) {
        if (q->active_reader_class == '*' || q->queue[i].spool_class == q->active_reader_class) {
            strncpy(data_out, q->queue[i].card_data, 79);
            data_out[79] = '\0';
            for (int j = i; j < q->count - 1; j++) {
                q->queue[j] = q->queue[j + 1];
            }
            q->count--;
            return 0;
        }
    }
    return -2;
}

void tsfi_cp_spool_queue_v3_init(tsfi_cp_spool_queue_v3 *q) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue_v3));
}

int tsfi_cp_spool_push_v3(tsfi_cp_spool_queue_v3 *q, const char *data, int file_id) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS_V3) return -1;
    strncpy(q->queue[q->count].card_data, data, sizeof(q->queue[q->count].card_data) - 1);
    q->queue[q->count].card_data[sizeof(q->queue[q->count].card_data) - 1] = '\0';
    q->queue[q->count].file_id = file_id;
    q->queue[q->count].is_held = 0;
    q->count++;
    return 0;
}

int tsfi_cp_spool_hold(tsfi_cp_spool_queue_v3 *q, int file_id, int hold) {
    if (!q) return -1;
    for (int i = 0; i < q->count; i++) {
        if (q->queue[i].file_id == file_id) {
            q->queue[i].is_held = hold;
            return 0;
        }
    }
    return -1;
}

int tsfi_cp_spool_pop_v3(tsfi_cp_spool_queue_v3 *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    for (int i = 0; i < q->count; i++) {
        if (!q->queue[i].is_held) {
            strncpy(data_out, q->queue[i].card_data, 79);
            data_out[79] = '\0';
            for (int j = i; j < q->count - 1; j++) {
                q->queue[j] = q->queue[j + 1];
            }
            q->count--;
            return 0;
        }
    }
    return -2;
}
