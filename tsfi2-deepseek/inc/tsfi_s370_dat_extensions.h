#ifndef TSFI_S370_DAT_EXTENSIONS_H
#define TSFI_S370_DAT_EXTENSIONS_H

#include "tsfi_ramac_layout.h"
#include <stdint.h>

// Extended 31-bit address translation stats
typedef struct {
    uint64_t tlb_hits;
    uint64_t tlb_misses;
    uint64_t total_translation_ns;
    uint64_t page_faults;
    uint64_t segment_faults;
    uint32_t table_crc;
} tsfi_s370_dat_stats;

extern tsfi_s370_dat_stats g_dat_stats;

int tsfi_s370_dat_translate_extended(tsfi_s370_cpu_state *cpu, uint32_t virtual_addr,
                                     tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables,
                                     uint32_t *out_physical_addr, int *out_write_protected);

uint32_t tsfi_s370_audit_page_tables(tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables);

void tsfi_s370_track_ipte(uint32_t virtual_addr);

#endif // TSFI_S370_DAT_EXTENSIONS_H
