#ifndef TSFI_S370_DAT_EXTENSIONS_H
#define TSFI_S370_DAT_EXTENSIONS_H

#include "tsfi_ramac_layout.h"
#include <stdint.h>

// 256-bit Virtual/Real Address representation
typedef struct {
    uint64_t parts[4]; // [0]=BX, [1]=PX, [2]=SX_low, [3]=SX_high
} tsfi_s370_addr_256;

// 256-bit Segment Table Entry
typedef struct {
    uint64_t page_table_origin;
    int invalid;
} tsfi_s370_segment_entry_256;

// 256-bit Page Table Entry
typedef struct {
    tsfi_s370_addr_256 real_page_frame;
    int invalid;
    int write_protect;
} tsfi_s370_page_entry_256;

// 512-bit Virtual/Real Address representation (ZMM-native scale)
typedef struct {
    uint64_t parts[8];
} tsfi_s370_addr_512;

// 512-bit Segment Table Entry
typedef struct {
    uint64_t page_table_origin;
    int invalid;
} tsfi_s370_segment_entry_512;

// 512-bit Page Table Entry
typedef struct {
    tsfi_s370_addr_512 real_page_frame;
    int invalid;
    int write_protect;
} tsfi_s370_page_entry_512;

// Extended 31-bit address translation stats
typedef struct {
    uint64_t tlb_hits;
    uint64_t tlb_misses;
    uint64_t total_translation_ns;
    uint64_t page_faults;
    uint64_t segment_faults;
    uint32_t table_crc;
    uint64_t lap_violations;
    uint64_t key_violations;
    uint64_t ccw_violations;
    int timer_interrupt_pending;
    uint64_t translation_256_count;
    uint64_t translation_512_count;
} tsfi_s370_dat_stats;

extern tsfi_s370_dat_stats g_dat_stats;

int tsfi_s370_dat_translate_extended(tsfi_s370_cpu_state *cpu, uint32_t virtual_addr,
                                     tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables,
                                     uint32_t *out_physical_addr, int *out_write_protected);

uint32_t tsfi_s370_audit_page_tables(tsfi_s370_segment_entry *seg_table, int seg_count,
                                     tsfi_s370_page_entry *page_tables);

void tsfi_s370_track_ipte(uint32_t virtual_addr);

// System/370 CPU & Channel security verification extensions
int tsfi_s370_validate_write_with_interrupt(tsfi_s370_cpu_state *cpu, uint32_t real_addr,
                                             int is_write_protected_page,
                                             tsfi_s370_storage_key *block_keys, int block_count,
                                             uint8_t *real_memory, int mem_size);

int tsfi_s370_audit_ccw_chain(const tsfi_s370_ccw *ccw_chain, int chain_len, uint32_t mem_limit);

int tsfi_s370_check_storage_key_audit(uint8_t psw_key, uint32_t real_addr, int is_write,
                                      tsfi_s370_storage_key *block_keys, int block_count);

void tsfi_s370_tick_interval_timer(tsfi_s370_cpu_state *cpu, uint8_t *real_memory, int mem_size, double elapsed_seconds);

// 256-bit Dynamic Address Translation (DAT)
int tsfi_s370_dat_translate_256(const tsfi_s370_addr_256 *virtual_addr,
                                 const tsfi_s370_segment_entry_256 *seg_table, int seg_count,
                                 const tsfi_s370_page_entry_256 *page_tables,
                                 tsfi_s370_addr_256 *out_physical_addr, int *out_write_protected);

// 512-bit Dynamic Address Translation (DAT)
int tsfi_s370_dat_translate_512(const tsfi_s370_addr_512 *virtual_addr,
                                 const tsfi_s370_segment_entry_512 *seg_table, int seg_count,
                                 const tsfi_s370_page_entry_512 *page_tables,
                                 tsfi_s370_addr_512 *out_physical_addr, int *out_write_protected);

#endif // TSFI_S370_DAT_EXTENSIONS_H
