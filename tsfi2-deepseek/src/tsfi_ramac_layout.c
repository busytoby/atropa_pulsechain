#include "tsfi_ramac_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    // Physical model of IBM 305 RAMAC movement:
    // 1. Horizontal seek: Cylinder-to-cylinder arm movement.
    //    Average cylinder-to-cylinder seek: ~1.5 milliseconds per cylinder.
    double cylinder_seek = abs(c1.cylinder - c2.cylinder) * 1.5;

    // 2. Vertical seek: Disk surface head swap (if head index changes).
    //    Average head swap time: ~0.8 milliseconds.
    double head_swap = (c1.head != c2.head) ? 0.8 : 0.0;

    // 3. Rotational latency: Sector-to-sector delay.
    //    Assuming 600 RPM (100 ms per full rotation of 20 sectors).
    //    Rotational delay = 5.0 ms per sector distance.
    double rotational_delay = abs(c1.sector - c2.sector) * 5.0;

    return (cylinder_seek + head_swap + rotational_delay) * 1000.0; // Return in microseconds
}

int tsfi_ramac_layout_optimize(tsfi_dat *dat, const char *filepath) {
    if (!dat || !filepath) return -1;

    // Open target file for writing the RAMAC-optimized dat layout
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    // Write magic signature
    fwrite("RMAC", 1, 4, fp);

    // Calculate total layout words and size
    int capacity = dat->capacity;
    fwrite(&capacity, sizeof(int), 1, fp);

    // Write base and check arrays clustered as aligned structures
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
    // We restrict primary tracks to heads 0..44 (total 45 tracks per cylinder)
    int primary_slots = 45 * RAMAC_SECTORS;
    int slot = hash % primary_slots;
    
    // Return flat index within cylinder:
    // slot represents head * RAMAC_SECTORS + sector
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
    int current_head = 0; // Assume start head

    // Traverse existing collision chain
    while (disk[current_idx].is_active) {
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        current_head = current_idx;

        if (strcmp(disk[current_idx].key, key) == 0) {
            // Overwrite existing key
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
        // Direct placement
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        strcpy(disk[current_idx].key, key);
        strcpy(disk[current_idx].value, value);
        disk[current_idx].is_active = 1;
        disk[current_idx].next_overflow_index = -1;
        if (out_total_seek_us) *out_total_seek_us = seek_time;
        return current_idx;
    }

    // Find free slot in overflow area (heads 45..49) of the same cylinder
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
        // Cylinder overflow area full
        return -1;
    }

    // Write to overflow slot
    seek_time += tsfi_ramac_calculate_seek(current_head, found_slot);
    strcpy(disk[found_slot].key, key);
    strcpy(disk[found_slot].value, value);
    disk[found_slot].is_active = 1;
    disk[found_slot].next_overflow_index = -1;

    // Link the last record to this overflow slot
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

    // Immediately trigger read-after-write verification
    const char *read_val = tsfi_ramac_search_record(disk, key, cylinder, &temp_seek);
    if (!read_val || strcmp(read_val, value) != 0) {
        return -1; // Parity check mismatch!
    }

    return 0; // Verified successfully
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

    // RULE 12 INTERCEPT: Mathematical continuity denied (Division by Zero or Preference error)
    if (val == 0) {
        // Intercept, redirect to non-preferential accumulator isolation trap, and isolate in structure
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
            return 0; // Parity failure: IBM 305 RAMAC requires odd parity
        }
    }
    return 1;
}

int tsfi_ramac_alu_exec(tsfi_ramac_acc_model *model, tsfi_ramac_instruction *program, int program_size) {
    if (!model || !program || program_size <= 0) return -1;

    int pc = 0;
    int cmp_flag = 0; // 0: equal, 1: dest > src, -1: dest < src

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
                // Return immediately to handle isolated math discontinuity
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
                // Find label index
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
            pc++; // NOP
        }
    }
    return 0;
}

int tsfi_s370_dat_translate(uint32_t virtual_addr, 
                            tsfi_s370_segment_entry *seg_table, int seg_count,
                            tsfi_s370_page_entry *page_tables,
                            uint32_t *out_physical_addr, int *out_write_protected) {
    if (!seg_table || !page_tables || !out_physical_addr || !out_write_protected) return -1;

    // S/370 31-bit addressing translation:
    // SX (Segment Index) = bits 1..11 (11 bits) -> (addr >> 20) & 0x7FF
    // PX (Page Index)    = bits 12..19 (8 bits) -> (addr >> 12) & 0xFF
    // BX (Byte Offset)   = bits 20..31 (12 bits) -> addr & 0xFFF
    uint32_t sx = (virtual_addr >> 20) & 0x7FF;
    uint32_t px = (virtual_addr >> 12) & 0xFF;
    uint32_t bx = virtual_addr & 0xFFF;

    if ((int)sx >= seg_count) {
        return -1; // Segment translation exception (limit exceeded)
    }

    if (seg_table[sx].invalid) {
        return -1; // Segment invalid exception
    }

    // Offset in global page table structure
    uint32_t pte_idx = seg_table[sx].page_table_origin + px;
    if (page_tables[pte_idx].invalid) {
        return -1; // Page invalid exception
    }

    *out_physical_addr = page_tables[pte_idx].page_frame_real_addr + bx;
    *out_write_protected = page_tables[pte_idx].write_protect;
    return 0; // Translation successful
}

int tsfi_s370_channel_execute(tsfi_ramac_record *disk, int total_slots,
                              tsfi_s370_ccw *ccw_chain, int chain_len,
                              uint8_t *memory_pool, int mem_size) {
    if (!disk || total_slots <= 0 || !ccw_chain || chain_len <= 0 || !memory_pool || mem_size <= 0) {
        return -1;
    }

    int current_ccw_idx = 0;
    int current_seek_sector = 0; // Default seek index pointer

    while (current_ccw_idx < chain_len) {
        tsfi_s370_ccw ccw = ccw_chain[current_ccw_idx];

        if (ccw.data_addr >= (uint32_t)mem_size) {
            return -1; // Memory out of bounds
        }

        switch (ccw.cmd_code) {
            case 0x07: // Control Command: SEEK
                // Read target sector index from memory
                if (ccw.data_addr + 4 > (uint32_t)mem_size) return -1;
                memcpy(&current_seek_sector, memory_pool + ccw.data_addr, 4);
                if (current_seek_sector < 0 || current_seek_sector >= total_slots) {
                    return -1; // Sector index out of bounds
                }
                break;

            case 0x02: // Input Command: READ
                // Read from RAMAC disk current_seek_sector to memory_pool
                if (current_seek_sector >= total_slots || !disk[current_seek_sector].is_active) {
                    // Return empty record if inactive
                    memset(memory_pool + ccw.data_addr, 0, ccw.count < 32 ? ccw.count : 32);
                } else {
                    int size_to_copy = ccw.count < 32 ? ccw.count : 32;
                    if (ccw.data_addr + size_to_copy > (uint32_t)mem_size) return -1;
                    memcpy(memory_pool + ccw.data_addr, disk[current_seek_sector].value, size_to_copy);
                }
                break;

            case 0x01: // Output Command: WRITE
                // Write from memory_pool to RAMAC disk current_seek_sector
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
                return -1; // Unsupported CCW command code
        }

        // Check command chaining flag (bit 2 represents command chaining in standard S/370: mask 0x02)
        if (ccw.flags & 0x02) {
            current_ccw_idx++;
        } else {
            break; // Terminate I/O program
        }
    }

    return 0; // I/O channel program completed successfully
}

int tsfi_s370_check_storage_key(uint8_t psw_key, uint32_t real_addr, int is_write,
                                tsfi_s370_storage_key *block_keys, int block_count) {
    if (!block_keys || block_count <= 0) return -1;

    // S/370 block protection size: 4096-byte pages
    int block_idx = real_addr / 4096;
    if (block_idx >= block_count) {
        return -1; // Out of bounds
    }

    // Access key 0 is the master key, allowing unrestricted access
    if (psw_key != 0) {
        uint8_t block_key = block_keys[block_idx].acc;

        if (is_write) {
            // Write protection requires matching access control bits
            if (psw_key != block_key) {
                return -1; // Protection Exception
            }
        } else {
            // Read protection requires matching access control bits only if Fetch Protection (F) is enabled
            if (block_keys[block_idx].fetch_protect) {
                if (psw_key != block_key) {
                    return -1; // Protection Exception
                }
            }
        }
    }

    // Update hardware Referenced (R) and Changed (C) reference bits
    block_keys[block_idx].referenced = 1;
    if (is_write) {
        block_keys[block_idx].changed = 1;
    }

    return 0; // Access Permitted
}

int tsfi_s370_validate_instruction(tsfi_s370_cpu_state *cpu, const char *op_code) {
    if (!cpu || !op_code) return -1;

    // List of privileged operations in System/370 execution states
    const char *privileged_ops[] = {
        "SSK",   // Set Storage Key
        "ISK",   // Insert Storage Key
        "LPSW",  // Load Program Status Word
        "SIO",   // Start I/O
        "STCTL", // Store Control Register
        "LCTL"   // Load Control Register
    };
    int privileged_count = 6;

    // Problem State (supervisor_state == 0) restricts privileged instructions
    if (cpu->supervisor_state == 0) {
        for (int i = 0; i < privileged_count; i++) {
            if (strcmp(op_code, privileged_ops[i]) == 0) {
                return -1; // Privileged Operation Exception
            }
        }
    }

    return 0; // Instruction validation passes
}

int tsfi_s370_validate_write(tsfi_s370_cpu_state *cpu, uint32_t real_addr,
                             int is_write_protected_page,
                             tsfi_s370_storage_key *block_keys, int block_count) {
    if (!cpu) return -1;

    // Page-Protection Exception: write to virtual page marked read-only is prohibited
    if (is_write_protected_page) {
        return -1; // Protection Exception
    }

    // Low-Address Protection (LAP): write to first 512 bytes (addresses 0..511) in Problem State is prohibited
    if (cpu->lap_enabled && real_addr < 512 && cpu->supervisor_state == 0) {
        return -1; // Protection Exception (LAP violation)
    }

    // Delegate to standard storage key protection checks
    return tsfi_s370_check_storage_key(cpu->psw_key, real_addr, 1, block_keys, block_count);
}

int tsfi_s370_authorize_psw_key(tsfi_lau_account *account, 
                                const uint8_t *signature, int sig_len,
                                const uint8_t *message, int msg_len,
                                uint8_t *out_psw_key) {
    if (!account || !signature || sig_len < 32 || !message || msg_len <= 0 || !out_psw_key) {
        return -1;
    }

    // Verify PKI signature: expect simple cryptographic verification mapping
    uint8_t expected_sig[32];
    for (int i = 0; i < 32; i++) {
        expected_sig[i] = account->public_key[i] ^ (message[i % msg_len] + i);
    }

    if (memcmp(expected_sig, signature, 32) != 0) {
        return -1; // PKI signature verification failed
    }

    // Map verified LAU account tier privileges to PSW key
    if (account->is_admin_tier) {
        *out_psw_key = 0; // Master Key (Supervisor privileges)
    } else {
        // Regular accounts mapped to an authorized key between 1 and 15
        *out_psw_key = (account->public_key[0] % 15) + 1;
    }

    return 0; // Authorized
}

int tsfi_s370_trigger_program_interrupt(tsfi_s370_cpu_state *cpu, uint16_t pic,
                                        uint8_t *real_memory, int mem_size) {
    if (!cpu || !real_memory || mem_size < 512) {
        return -1;
    }

    // 1. Store Program Interrupt Code (16-bit PIC) at real storage offset 142 (0x8E / 0x8F)
    real_memory[142] = (pic >> 8) & 0xFF;
    real_memory[143] = pic & 0xFF;

    // 2. Save current PSW to Program Old PSW vector location (real address 40 / 0x28)
    real_memory[40] = (cpu->current_psw.key << 4);
    real_memory[41] = cpu->current_psw.problem_state ? 0x01 : 0x00;
    real_memory[42] = 0x00;
    real_memory[43] = 0x00;
    real_memory[44] = (cpu->current_psw.instruction_address >> 24) & 0xFF;
    real_memory[45] = (cpu->current_psw.instruction_address >> 16) & 0xFF;
    real_memory[46] = (cpu->current_psw.instruction_address >> 8) & 0xFF;
    real_memory[47] = cpu->current_psw.instruction_address & 0xFF;

    // 3. Load new PSW from Program New PSW vector location (real address 88 / 0x58)
    uint8_t new_key = (real_memory[88] >> 4) & 0x0F;
    int new_problem_state = real_memory[89] & 0x01;
    uint32_t new_inst_addr = ((uint32_t)real_memory[92] << 24) |
                             ((uint32_t)real_memory[93] << 16) |
                             ((uint32_t)real_memory[94] << 8)  |
                             (uint32_t)real_memory[95];

    cpu->current_psw.key = new_key;
    cpu->current_psw.problem_state = new_problem_state;
    cpu->current_psw.instruction_address = new_inst_addr;

    // Sync privilege controls
    cpu->psw_key = new_key;
    cpu->supervisor_state = !new_problem_state;

    return 0; // Interruption and PSW swap completed
}

int tsfi_s370_pack(const char *zoned_str, uint8_t *packed_out, int max_len) {
    if (!zoned_str || !packed_out || max_len <= 0) return -1;

    int sign = 0xC; // Default positive sign flag (C or F)
    const char *digits = zoned_str;
    if (zoned_str[0] == '-') {
        sign = 0xD; // Negative sign flag
        digits++;
    } else if (zoned_str[0] == '+') {
        digits++;
    }

    int len = strlen(digits);
    if (len == 0) return -1;

    // COMP-3 packed decimal format has (len + 1) / 2 bytes.
    // If len is even, we pad with a leading zero to fit into whole bytes:
    // e.g. 4 digits: 01 23 4C -> 3 bytes
    int is_even = (len % 2 == 0);
    int packed_len = (len + 2) / 2;
    if (packed_len > max_len) return -1;

    memset(packed_out, 0, packed_len);

    int digit_idx = 0;
    for (int i = 0; i < packed_len; i++) {
        uint8_t high = 0;
        uint8_t low = 0;

        if (i == 0 && is_even) {
            // First byte high nibble is padded zero
            high = 0;
            low = digits[digit_idx++] - '0';
        } else {
            high = digits[digit_idx++] - '0';
            if (i == packed_len - 1) {
                low = sign; // Last low nibble is sign C/D/F
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

    // Temporary digits buffer
    char digits[128];
    int digits_idx = 0;

    for (int i = 0; i < packed_len; i++) {
        uint8_t high = (packed[i] >> 4) & 0x0F;
        uint8_t low = packed[i] & 0x0F;

        if (high > 9) return -1; // Invalid BCD digit in high nibble

        if (i == 0 && high == 0) {
            // Skip leading padding zero
        } else {
            digits[digits_idx++] = high + '0';
        }

        if (i == packed_len - 1) {
            // Last low nibble is sign
            if (low == 0xC || low == 0xF || low == 0xA || low == 0xE) {
                negative = 0;
            } else if (low == 0xD || low == 0xB) {
                negative = 1;
            } else {
                return -1; // Invalid sign nibble
            }
            sign_found = 1;
        } else {
            if (low > 9) return -1; // Invalid BCD digit in low nibble
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

    // 1. Store 8-bit SVC code at real storage offset 139 (0x8B)
    real_memory[139] = svc_code;

    // 2. Save current PSW to SVC Old PSW vector location (real address 32 / 0x20)
    real_memory[32] = (cpu->current_psw.key << 4);
    real_memory[33] = cpu->current_psw.problem_state ? 0x01 : 0x00;
    real_memory[34] = 0x00;
    real_memory[35] = 0x00;
    real_memory[36] = (cpu->current_psw.instruction_address >> 24) & 0xFF;
    real_memory[37] = (cpu->current_psw.instruction_address >> 16) & 0xFF;
    real_memory[38] = (cpu->current_psw.instruction_address >> 8) & 0xFF;
    real_memory[39] = cpu->current_psw.instruction_address & 0xFF;

    // 3. Load new PSW from SVC New PSW vector location (real address 80 / 0x50)
    uint8_t new_key = (real_memory[80] >> 4) & 0x0F;
    int new_problem_state = real_memory[81] & 0x01;
    uint32_t new_inst_addr = ((uint32_t)real_memory[84] << 24) |
                             ((uint32_t)real_memory[85] << 16) |
                             ((uint32_t)real_memory[86] << 8)  |
                             (uint32_t)real_memory[87];

    cpu->current_psw.key = new_key;
    cpu->current_psw.problem_state = new_problem_state;
    cpu->current_psw.instruction_address = new_inst_addr;

    // Sync privilege controls
    cpu->psw_key = new_key;
    cpu->supervisor_state = !new_problem_state;

    return 0; // SVC interrupt transition completed
}

int tsfi_s370_data_reduction_unit(double x, double y, double scale,
                                  uint8_t *dest_out, int dest_max_len) {
    if (!dest_out || dest_max_len <= 0) return -1;

    // 1. Emulate Benson-Lehner Electroplotter analog-to-digital coordinate conversion:
    // Scale analog voltage values to discrete integer records
    long long coord_x = (long long)round(x * scale);
    long long coord_y = (long long)round(y * scale);

    // Sum coordinates to build structured composite data packet
    long long combined = coord_x + coord_y;

    char zoned[128];
    snprintf(zoned, sizeof(zoned), "%lld", combined);

    // 2. Package directly to COMP-3 format for immediate COBOL strategy engine consumption
    return tsfi_s370_pack(zoned, dest_out, dest_max_len);
}

int tsfi_s370_serialize_quadtree(const char *filepath, tsfi_quadtree_node *nodes, int node_count) {
    if (!filepath || !nodes || node_count <= 0) return -1;

    // Rule 13: strictly check .dat.bin extension for quadtree index
    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return -1; // Extension error
    }

    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    // Write magic signature
    fwrite("QUAD", 1, 4, fp);

    // Write node count
    fwrite(&node_count, sizeof(int), 1, fp);

    // Write array of quadtree nodes
    fwrite(nodes, sizeof(tsfi_quadtree_node), node_count, fp);

    fclose(fp);
    return 0;
}

int tsfi_s370_deserialize_quadtree(const char *filepath, tsfi_quadtree_node *nodes, int max_nodes) {
    if (!filepath || !nodes || max_nodes <= 0) return -1;

    // Rule 13: strictly check .dat.bin extension
    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return -1; // Extension error
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
        return -1; // Destination buffer size exceeded
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

    // 1. Optimized TLB lookup: Direct mapped hash lookup to fulfill sub-microsecond latency (Rule 11)
    int slot = (virtual_page >> 12) & 0x7;
    if (cpu->tlb[slot].valid && cpu->tlb[slot].virtual_page == virtual_page) {
        *out_physical_addr = cpu->tlb[slot].real_page | (virtual_addr & 0x0FFF);
        *out_write_protected = cpu->tlb[slot].write_protect;
        return 0; // Direct Map TLB Hit
    }

    // 2. TLB Miss: perform full segment-page translation table walk
    int ret = tsfi_s370_dat_translate(virtual_addr, seg_table, seg_count, page_tables, out_physical_addr, out_write_protected);
    if (ret != 0) return ret;

    // 3. Cache results inside TLB slot using bitwise masking
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

    // 1. Search calibration curve to locate interval
    int idx = 0;
    while (idx < table_size - 1 && calibration_table[idx + 1] < analog_amplitude) {
        idx++;
    }

    // 2. Linearly interpolate within selected overlay segment
    double t = 0.0;
    double range = calibration_table[idx + 1] - calibration_table[idx];
    if (range > 0.000001) {
        t = (analog_amplitude - calibration_table[idx]) / range;
    }
    double interp_val = idx + t;

    // Normalize output scale to [0..1000]
    long long final_digital = (long long)round(interp_val * (1000.0 / (table_size - 1)));
    
    char zoned[128];
    snprintf(zoned, sizeof(zoned), "%lld", final_digital);

    // 3. Pack directly to COMP-3 BCD format
    return tsfi_s370_pack(zoned, dest_out, dest_max_len);
}

int tsfi_s370_fet_discharge_freudenthal(double initial_charge, double time_step, double mass,
                                        double spring_k, double damping_c, int steps, double *out_decay_charges) {
    if (time_step <= 0.0 || mass <= 0.0 || steps <= 0 || !out_decay_charges) {
        return -1;
    }

    // Rule 10 Physics Constraints: Soft body physics (Verlet solvers and mass-spring dynamics) 
    // applies strictly and exclusively to the discharge cycles of field-effect transistors (FETs)
    double pos = initial_charge;
    double prev_pos = initial_charge; // Assume started at rest

    for (int i = 0; i < steps; i++) {
        // Calculate velocity (first order approximation)
        double vel = (pos - prev_pos) / time_step;

        // Force: Spring restoring force + viscous damping
        double force = -spring_k * pos - damping_c * vel;

        // Acceleration
        double accel = force / mass;

        // Verlet integration step: x(t+dt) = 2*x(t) - x(t-dt) + a*dt^2
        double next_pos = 2.0 * pos - prev_pos + accel * time_step * time_step;

        // Propagate state
        prev_pos = pos;
        pos = next_pos;

        // Apply Freudenthal relaxation decay: charge decreases viscoelastically over time
        double t = i * time_step;
        double relaxation_factor = exp(-t / 1.5); // Freudenthal relaxation time parameter tau_F = 1.5s
        
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
            // D = sum( (S_i / S_0) ^ alpha )
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

    // Reliability safety index: beta = (mu_R - mu_S) / sqrt(sigma_R^2 + sigma_S^2)
    double denom = sqrt(std_resistance * std_resistance + std_stress * std_stress);
    double beta = (mean_resistance - mean_stress) / denom;
    *out_beta = beta;

    // Approximate failure probability using standard normal erfc: Pf = 0.5 * erfc(beta / sqrt(2))
    *out_pf = 0.5 * erfc(beta / sqrt(2.0));

    return 0;
}

int tsfi_s370_portfolio_strategy_keystone(const double *asset_yields, const double *weights, int asset_count,
                                           double *out_expected_return, double *out_variance) {
    if (!asset_yields || !weights || asset_count <= 0 || !out_expected_return || !out_variance) {
        return -1;
    }

    // Expected return: Rp = sum( w_i * Y_i )
    double expected_return = 0.0;
    for (int i = 0; i < asset_count; i++) {
        expected_return += weights[i] * asset_yields[i];
    }
    *out_expected_return = expected_return;

    // Constant correlation factor: rho = 0.15 representing standard mid-century market portfolios
    double rho = 0.15;
    double variance = 0.0;

    for (int i = 0; i < asset_count; i++) {
        double std_i = asset_yields[i] * 0.25; // standard dev proportional to yield
        
        // Single variance terms: w_i^2 * sigma_i^2
        variance += weights[i] * weights[i] * std_i * std_i;

        // Covariance terms: sum( w_i * w_j * sigma_i * sigma_j * rho )
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
        // Expected value: V_i = Benefit_i - Cost_i - (Benefit_i * RiskProb_i)
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
        // Linear congruential generator (LCG) step to yield deterministic pseudo-random sequences
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        // Scale to a perturbation range of [-10%, +10%]
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

    // RCA Sarnoff-Zworykin scale gap: actual cost is 500x initial proposal, months is 6.67x
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
        return -1; // Address out of bounds
    }

    if (!disk[sector_idx].is_active) {
        *out_val = 0; // Uninitialized memory reads as 0
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
        return -1; // Address out of bounds
    }

    // Resolve Cylinder index to check static/high memory protection boundary
    // total sectors per cylinder is heads * sectors (50 * 20 = 1000)
    int cylinder = sector_idx / 1000;
    if (cylinder >= 45) {
        return -2; // Write protected segment violation exception
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

    // 1. Perform standard S/370 segment-page DAT translation table walk
    int ret = tsfi_s370_dat_translate(virtual_addr, seg_table, seg_count, page_tables, &physical_addr, &write_protected);
    if (ret != 0) {
        return ret; // DAT translation exception
    }

    // 2. Map resulting physical byte address to RAMAC words/geometry
    // Each RAMAC word is 4 bytes
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

    for (int i = 0; i < stream_len; i++) {
        if (bytes_transferred >= max_out_len) {
            break;
        }

        // 1. Present data byte on SCSI data lines (data register)
        *data_reg = stream[i];

        // 2. Assert REQ signal (bit 0x01 on control status register)
        *scsi_bus_status |= 0x01;

        // 3. Simulated Device Loop: Device observes REQ, reads data, asserts ACK (bit 0x02)
        *scsi_bus_status |= 0x02; 

        // 4. Host observes ACK, reads byte into out_buffer and clears REQ signal
        out_buffer[bytes_transferred++] = *data_reg;
        *scsi_bus_status &= ~0x01;

        // 5. Device observes REQ deassertion, deasserts ACK signal
        *scsi_bus_status &= ~0x02;
    }

    return bytes_transferred;
}

int tsfi_s370_oscar_reader_polynomial(double analog_amplitude, const double *coefficients, int coeff_count,
                                      uint8_t *dest_out, int dest_max_len) {
    if (!coefficients || coeff_count <= 0 || !dest_out || dest_max_len <= 0) {
        return -1;
    }

    // Evaluate polynomial: y = sum( c_i * x^i )
    double eval_val = 0.0;
    for (int i = 0; i < coeff_count; i++) {
        eval_val += coefficients[i] * pow(analog_amplitude, i);
    }

    // Normalize output scale to [0..1000]
    long long final_digital = (long long)round(eval_val * 1000.0);
    if (final_digital < 0) final_digital = 0;
    if (final_digital > 1000) final_digital = 1000;

    char zoned[128];
    snprintf(zoned, sizeof(zoned), "%lld", final_digital);

    // Pack directly to COMP-3 format
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
            // Standard IBM punch card zone representations for negative digits: J-R represents -1 to -9
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

    // Ingest buffered stream bytes as key-value pairs (using simple modulo index maps)
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

    // Initial charge mapped directly from analog amplitude representation
    double initial_charge = analog_val * 100.0;

    // Rule 10: Soft body physics Verlet solver applied strictly to FET discharge cycles
    int discharge_ret = tsfi_s370_fet_discharge_freudenthal(initial_charge, 0.1, mass, spring_k, damping_c, steps, out_decay_charges);
    if (discharge_ret != 0) {
        return -1;
    }

    // Verify decay: charge should decrease in amplitude across steps (Verlet decay validation)
    if (fabs(out_decay_charges[steps - 1]) >= fabs(initial_charge)) {
        return -2; // Fail: trajectory does not decay (instability check)
    }

    return 0;
}

int tsfi_s370_sage_redundancy_monitor(int cpu_a_status, int cpu_b_status, int *active_cpu) {
    if (!active_cpu) {
        return -1;
    }

    // SAGE AN/FSQ-7 Active-Passive priority monitoring loop:
    // CPU A is designated as the primary active processor
    if (cpu_a_status == 1) {
        *active_cpu = 1; // CPU A is active
    } else if (cpu_b_status == 1) {
        *active_cpu = 2; // Failover to CPU B
    } else {
        *active_cpu = 0; // Dual CPU fault failure state
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
            ptr += len; // Shift pointer beyond match pattern
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

    // Parametron majority decision gate: outputs phase representing the majority of the three inputs
    int sum = phase_in_1 + phase_in_2 + phase_in_3;
    if (sum >= 2) {
        *phase_out = 1; // 180 degrees phase state
    } else {
        *phase_out = 0; // 0 degrees phase state
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
                phase = 0; // Constant bias 0
            } else if (src == -2) {
                phase = 1; // Constant bias 1
            } else if (src < -2) {
                int inp_idx = -(src + 3);
                if (inp_idx >= 0 && inp_idx < input_count && inputs) {
                    phase = inputs[inp_idx];
                } else {
                    phase = 0;
                }
            } else {
                // Assert topology: source must be an already evaluated node index
                if (src < i) {
                    phase = nodes[src].output;
                } else {
                    phase = 0;
                }
            }

            // Apply phase inversion if flag is set
            if (nodes[i].invert[s]) {
                phase = (phase == 0) ? 1 : 0;
            }

            inputs_resolved[s] = phase;
        }

        // Apply majority logic gate evaluation
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
        // Bitwise AND operation representing stacked cards blocking/allowing light through holes
        out_matching[i] = card_a[i] & card_b[i];

        // Count set bits (holes allowing light to pass through) in the resulting word
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

    // Decodes two 20-bit instructions from a 40-bit Recomp II word:
    // Left Instruction (bits 20..39):
    uint64_t left_instr = (raw_word >> 20) & 0xFFFFF;
    *op1 = (left_instr >> 15) & 0x1F;   // 5-bit opcode
    *addr1 = (left_instr >> 3) & 0xFFF; // 12-bit address

    // Right Instruction (bits 0..19):
    uint64_t right_instr = raw_word & 0xFFFFF;
    *op2 = (right_instr >> 15) & 0x1F;
    *addr2 = (right_instr >> 3) & 0xFFF;

    return 0;
}

int tsfi_s370_recomp_ii_drum_schedule(int current_sector, int execution_cycles, int *out_optimal_sector) {
    if (current_sector < 0 || current_sector >= 64 || execution_cycles < 0 || !out_optimal_sector) {
        return -1;
    }

    // Recomp II drum execution sector-advance timing:
    // Let's assume 8 clock cycles represent the rotation time of 1 sector.
    int sectors_passed = (execution_cycles + 7) / 8;

    // Minimum latency code scheduling inserts a safety buffer of +1 sector to prevent missing the slot
    *out_optimal_sector = (current_sector + sectors_passed + 1) % 64;

    return 0;
}

int tsfi_s370_paper_tape_synthesizer(const uint8_t *tape_data, int length, int channels,
                                     double *out_audio, int max_samples, double sample_rate) {
    if (!tape_data || length <= 0 || channels <= 0 || channels > 8 || !out_audio || max_samples <= 0 || sample_rate <= 0.0) {
        return -1;
    }

    // Clear output audio buffer
    memset(out_audio, 0, max_samples * sizeof(double));

    // Audio tone frequencies corresponding to the 8 possible tape tracks/channels
    double frequencies[8] = {110.0, 220.0, 330.0, 440.0, 550.0, 660.0, 770.0, 880.0};

    // Duration of 1 tape row (step): 0.1 seconds
    double step_duration = 0.1;
    int samples_per_step = (int)(step_duration * sample_rate);

    for (int step = 0; step < length; step++) {
        int start_sample = step * samples_per_step;
        if (start_sample >= max_samples) break;

        uint8_t row_val = tape_data[step];

        for (int c = 0; c < channels; c++) {
            if ((row_val >> c) & 1) {
                double freq = frequencies[c];

                // Generate decaying sine wave for this channel trigger
                for (int s = 0; s < samples_per_step; s++) {
                    int out_idx = start_sample + s;
                    if (out_idx >= max_samples) break;

                    double t = (double)s / sample_rate;
                    double decay = exp(-t / 0.05); // Decay time constant tau = 50ms
                    double signal = sin(2.0 * M_PI * freq * t) * decay * 0.1; // Scale amplitude

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

    // Clean to 36-bit word length limit
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
        // Two 18-bit slices
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
        // Four 9-bit slices
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

    // Generate coordinates for a 9-point tracking cross centered at (*cross_x, *cross_y)
    double px[9], py[9];
    // Center point
    px[0] = *cross_x; py[0] = *cross_y;
    // Cardinal points
    px[1] = *cross_x + cross_radius; py[1] = *cross_y;
    px[2] = *cross_x - cross_radius; py[2] = *cross_y;
    px[3] = *cross_x; py[3] = *cross_y + cross_radius;
    px[4] = *cross_x; py[4] = *cross_y - cross_radius;
    // Intermediate diagonal points
    double diag = cross_radius * 0.5;
    px[5] = *cross_x + diag; py[5] = *cross_y + diag;
    px[6] = *cross_x - diag; py[6] = *cross_y - diag;
    px[7] = *cross_x + diag; py[7] = *cross_y - diag;
    px[8] = *cross_x - diag; py[8] = *cross_y + diag;

    // Detect points falling inside the pen aperture (radius threshold: 0.7 * cross_radius)
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

    // If points are detected, relocate the tracking cross to their centroid
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

    // Initialize all route outputs to idle (-1)
    for (int i = 0; i < cpu_count; i++) {
        out_route_map[i] = -1;
    }

    // Array tracking if a Buffer module is already bound (to prevent duplicate connections)
    int *buffer_used = (int *)calloc(buffer_count, sizeof(int));
    if (!buffer_used) return -1;

    for (int i = 0; i < cpu_count; i++) {
        int cpu_connection_count = 0;
        int connected_buffer = -1;

        for (int j = 0; j < buffer_count; j++) {
            // Index in flat 1D matrix representation
            int idx = i * buffer_count + j;
            if (matrix_connections[idx] == 1) {
                cpu_connection_count++;
                connected_buffer = j;
            }
        }

        // Conflict: a single CPU cannot be dynamically bound to more than one Buffer simultaneously
        if (cpu_connection_count > 1) {
            free(buffer_used);
            return -1;
        }

        if (cpu_connection_count == 1) {
            // Conflict: a Buffer module cannot be shared by multiple CPU connections simultaneously
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
