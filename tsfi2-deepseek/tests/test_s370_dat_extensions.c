#include "tsfi_s370_dat_extensions.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("[TEST] Running System/370 31-bit DAT Extensions Validation...\n");

    // Initialize segment and page structures
    tsfi_s370_segment_entry seg_table[2];
    memset(seg_table, 0, sizeof(seg_table));
    seg_table[0].page_table_origin = 0;
    seg_table[0].invalid = 0;
    seg_table[1].invalid = 1; // Invalid segment to trigger segment fault

    tsfi_s370_page_entry page_table[4];
    memset(page_table, 0, sizeof(page_table));
    page_table[0].page_frame_real_addr = 0x1000;
    page_table[0].invalid = 0;
    page_table[0].write_protect = 0;
    page_table[1].invalid = 1; // Invalid page to trigger page fault

    tsfi_s370_cpu_state cpu;
    memset(&cpu, 0, sizeof(cpu));

    uint32_t phys_addr = 0;
    int wp = 0;

    // 1. Test TLB Miss on first table walk
    int ret1 = tsfi_s370_dat_translate_extended(&cpu, 0x000123, seg_table, 2, page_table, &phys_addr, &wp);
    assert(ret1 == 0);
    assert(phys_addr == 0x1123);
    assert(wp == 0);
    assert(g_dat_stats.tlb_misses == 1);
    assert(g_dat_stats.tlb_hits == 0);

    // 2. Test TLB Hit on second access
    int ret2 = tsfi_s370_dat_translate_extended(&cpu, 0x000123, seg_table, 2, page_table, &phys_addr, &wp);
    assert(ret2 == 0);
    assert(phys_addr == 0x1123);
    assert(g_dat_stats.tlb_hits == 1);
    assert(g_dat_stats.tlb_misses == 1);

    // 3. Test Segment Fault execution path
    int ret3 = tsfi_s370_dat_translate_extended(&cpu, 0x100123, seg_table, 2, page_table, &phys_addr, &wp);
    assert(ret3 == -1);
    assert(g_dat_stats.segment_faults == 1);

    // 4. Test Page Fault execution path
    int ret4 = tsfi_s370_dat_translate_extended(&cpu, 0x001123, seg_table, 2, page_table, &phys_addr, &wp);
    assert(ret4 == -2);
    assert(g_dat_stats.page_faults == 1);

    // 5. Test page table auditing (CRC-32 hash)
    uint32_t crc = tsfi_s370_audit_page_tables(seg_table, 2, page_table);
    assert(crc != 0);
    assert(g_dat_stats.table_crc == crc);

    // 6. Test Invalidate Page (IPTE) tracking
    tsfi_s370_track_ipte(0x000123);

    printf("[SUCCESS] All 31-bit DAT Extensions validated successfully!\n");
    return 0;
}
