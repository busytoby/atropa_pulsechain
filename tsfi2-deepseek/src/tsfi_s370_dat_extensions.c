#include "tsfi_s370_dat_extensions.h"
#include "tsfi_types.h"
#include "lau_registry.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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
