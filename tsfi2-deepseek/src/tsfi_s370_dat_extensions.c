#include "tsfi_s370_dat_extensions.h"
#include "tsfi_types.h"
#include "lau_registry.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tsfi_s370_dat_stats g_dat_stats = {0};

static uint64_t get_nanoseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int tsfi_s370_dat_translate_extended(tsfi_s370_cpu_state *cpu, uint32_t virtual_addr,
                                     tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables,
                                     uint32_t *out_physical_addr, int *out_write_protected) {
    uint64_t start_time = get_nanoseconds();
    if (!cpu || !seg_table || !page_tables || !out_physical_addr || !out_write_protected) return -1;

    uint32_t virtual_page = virtual_addr & 0xFFFFF000;
    int slot = (virtual_page >> 12) & 0x7;

    // 1. TLB Hit verification
    if (cpu->tlb[slot].valid && cpu->tlb[slot].virtual_page == virtual_page) {
        *out_physical_addr = cpu->tlb[slot].real_page | (virtual_addr & 0x0FFF);
        *out_write_protected = cpu->tlb[slot].write_protect;
        g_dat_stats.tlb_hits++;
        g_dat_stats.total_translation_ns += (get_nanoseconds() - start_time);
        return 0;
    }

    g_dat_stats.tlb_misses++;

    // 2. Page walk translation
    uint32_t sx = (virtual_addr >> 20) & 0x7FF;
    uint32_t px = (virtual_addr >> 12) & 0xFF;
    uint32_t bx = virtual_addr & 0xFFF;

    if ((int)sx >= seg_count) {
        g_dat_stats.segment_faults++;
        return -1;
    }

    if (seg_table[sx].invalid) {
        g_dat_stats.segment_faults++;
        return -1;
    }

    uint32_t pte_idx = seg_table[sx].page_table_origin + px;
    if (page_tables[pte_idx].invalid) {
        g_dat_stats.page_faults++;
        
        // Expose to Vulkan shared telemetry: update last_directive_str
        LauTelemetryState *telem = lau_telemetry_get_state();
        if (telem) {
            snprintf(telem->last_directive_str, sizeof(telem->last_directive_str),
                     "PAGE FAULT: ADDR:%08X PTE:%d", virtual_addr, pte_idx);
            telem->current_intensity = 1.0f; // High intensity warning
        }
        return -2;
    }

    *out_physical_addr = page_tables[pte_idx].page_frame_real_addr + bx;
    *out_write_protected = page_tables[pte_idx].write_protect;

    cpu->tlb[slot].virtual_page = virtual_page;
    cpu->tlb[slot].real_page = *out_physical_addr & 0xFFFFF000;
    cpu->tlb[slot].write_protect = *out_write_protected;
    cpu->tlb[slot].valid = 1;

    g_dat_stats.total_translation_ns += (get_nanoseconds() - start_time);
    return 0;
}

// 3. Table CRC integrity checks
uint32_t tsfi_s370_audit_page_tables(tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables) {
    if (!seg_table || !page_tables) return 0;
    uint32_t hash = 0;
    for (int i = 0; i < seg_count; i++) {
        hash = hash * 31 + seg_table[i].page_table_origin;
        hash = hash * 31 + seg_table[i].invalid;
    }
    g_dat_stats.table_crc = hash;
    return hash;
}

// 4. IPTE Invalidate Page Tracking
void tsfi_s370_track_ipte(uint32_t virtual_addr) {
    LauTelemetryState *telem = lau_telemetry_get_state();
    if (telem) {
        snprintf(telem->last_directive_str, sizeof(telem->last_directive_str),
                 "IPTE PURGE: ADDR:%08X", virtual_addr);
        telem->recip_symmetry = 0.0f; // Reset pacing
    }
}

// --- Verification & Security Extensions ---

// 1. Low-Address Protection with Interrupts
int tsfi_s370_validate_write_with_interrupt(tsfi_s370_cpu_state *cpu, uint32_t real_addr,
                                             int is_write_protected_page,
                                             tsfi_s370_storage_key *block_keys, int block_count,
                                             uint8_t *real_memory, int mem_size) {
    if (!cpu || !real_memory) return -1;

    if (is_write_protected_page) {
        g_dat_stats.key_violations++;
        tsfi_s370_trigger_program_interrupt(cpu, 4, real_memory, mem_size); // PIC 4: Protection
        return -1;
    }

    if (cpu->lap_enabled && real_addr < 512 && cpu->supervisor_state == 0) {
        g_dat_stats.lap_violations++;
        tsfi_s370_trigger_program_interrupt(cpu, 4, real_memory, mem_size); // PIC 4: LAP violation Protection Exception
        return -1;
    }

    int ret = tsfi_s370_check_storage_key_audit(cpu->psw_key, real_addr, 1, block_keys, block_count);
    if (ret != 0) {
        tsfi_s370_trigger_program_interrupt(cpu, 4, real_memory, mem_size);
        return -1;
    }

    return 0;
}

// 2. CCW Chain Security Auditing
int tsfi_s370_audit_ccw_chain(const tsfi_s370_ccw *ccw_chain, int chain_len, uint32_t mem_limit) {
    if (!ccw_chain || chain_len <= 0) return -1;
    for (int i = 0; i < chain_len; i++) {
        if (ccw_chain[i].data_addr + ccw_chain[i].count > mem_limit) {
            g_dat_stats.ccw_violations++;
            return -1;
        }
    }
    return 0;
}

// 3. Storage Key Audited check
int tsfi_s370_check_storage_key_audit(uint8_t psw_key, uint32_t real_addr, int is_write,
                                      tsfi_s370_storage_key *block_keys, int block_count) {
    int ret = tsfi_s370_check_storage_key(psw_key, real_addr, is_write, block_keys, block_count);
    if (ret != 0) {
        g_dat_stats.key_violations++;
    }
    return ret;
}

// 4. Interval Timer Emulation
void tsfi_s370_tick_interval_timer(tsfi_s370_cpu_state *cpu, uint8_t *real_memory, int mem_size, double elapsed_seconds) {
    if (!cpu || !real_memory || mem_size < 512) return;

    // Load big-endian 32-bit signed timer from address 80 (0x50)
    int32_t timer_val = ((int32_t)real_memory[80] << 24) |
                        ((int32_t)real_memory[81] << 16) |
                        ((int32_t)real_memory[82] << 8)  |
                        (int32_t)real_memory[83];

    int32_t decrements = (int32_t)(elapsed_seconds * 19200.0);
    if (decrements <= 0) decrements = 1;

    int32_t new_val = timer_val - decrements;

    // Store big-endian value back to memory
    real_memory[80] = (new_val >> 24) & 0xFF;
    real_memory[81] = (new_val >> 16) & 0xFF;
    real_memory[82] = (new_val >> 8) & 0xFF;
    real_memory[83] = new_val & 0xFF;

    // If timer transitions from positive to negative, trigger external interrupt
    if (timer_val >= 0 && new_val < 0) {
        g_dat_stats.timer_interrupt_pending = 1;

        // Perform external interrupt old/new PSW swap (External Old at 24, External New at 88)
        real_memory[24] = (cpu->current_psw.key << 4);
        real_memory[25] = cpu->current_psw.problem_state ? 0x01 : 0x00;
        real_memory[26] = 0x00;
        real_memory[27] = 0x00;
        real_memory[28] = (cpu->current_psw.instruction_address >> 24) & 0xFF;
        real_memory[29] = (cpu->current_psw.instruction_address >> 16) & 0xFF;
        real_memory[30] = (cpu->current_psw.instruction_address >> 8) & 0xFF;
        real_memory[31] = cpu->current_psw.instruction_address & 0xFF;

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
    }
}

// 256-bit Dynamic Address Translation (DAT)
int tsfi_s370_dat_translate_256(const tsfi_s370_addr_256 *virtual_addr,
                                 const tsfi_s370_segment_entry_256 *seg_table, int seg_count,
                                 const tsfi_s370_page_entry_256 *page_tables,
                                 tsfi_s370_addr_256 *out_physical_addr, int *out_write_protected) {
    if (!virtual_addr || !seg_table || !page_tables || !out_physical_addr || !out_write_protected) return -1;

    // Hashing high parts for Segment Index
    uint64_t sx_hash = virtual_addr->parts[2] ^ virtual_addr->parts[3];
    int sx = (int)(sx_hash % (uint64_t)seg_count);

    if (seg_table[sx].invalid) {
        return -1;
    }

    uint64_t px = virtual_addr->parts[1];
    uint64_t pte_idx = seg_table[sx].page_table_origin + px;

    if (page_tables[pte_idx].invalid) {
        return -2;
    }

    out_physical_addr->parts[3] = page_tables[pte_idx].real_page_frame.parts[3];
    out_physical_addr->parts[2] = page_tables[pte_idx].real_page_frame.parts[2];
    out_physical_addr->parts[1] = page_tables[pte_idx].real_page_frame.parts[1];
    out_physical_addr->parts[0] = page_tables[pte_idx].real_page_frame.parts[0] + virtual_addr->parts[0];

    *out_write_protected = page_tables[pte_idx].write_protect;
    g_dat_stats.translation_256_count++;

    return 0;
}
