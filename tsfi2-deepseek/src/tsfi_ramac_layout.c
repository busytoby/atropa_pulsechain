#include "tsfi_ramac_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
                            uint32_t *out_physical_addr) {
    if (!seg_table || !page_tables || !out_physical_addr) return -1;

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
                             tsfi_s370_storage_key *block_keys, int block_count) {
    if (!cpu) return -1;

    // Low-Address Protection (LAP): write to first 512 bytes (addresses 0..511) in Problem State is prohibited
    if (cpu->lap_enabled && real_addr < 512 && cpu->supervisor_state == 0) {
        return -1; // Protection Exception (LAP violation)
    }

    // Delegate to standard storage key protection checks
    return tsfi_s370_check_storage_key(cpu->psw_key, real_addr, 1, block_keys, block_count);
}
