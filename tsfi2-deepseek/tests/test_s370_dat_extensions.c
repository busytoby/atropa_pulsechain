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

    // --- CPU & Channel Security verification tests ---

    uint8_t real_memory[1024];
    memset(real_memory, 0, sizeof(real_memory));
    
    // Set up New PSW at address 88: key=0, problem_state=0, address=0x8899
    real_memory[88] = 0x00; // Key 0
    real_memory[89] = 0x00; // Problem state 0 (Supervisor state)
    real_memory[92] = 0x00;
    real_memory[93] = 0x00;
    real_memory[94] = 0x88;
    real_memory[95] = 0x99;

    tsfi_s370_storage_key block_keys[2];
    memset(block_keys, 0, sizeof(block_keys));
    block_keys[0].acc = 1; // Acc key 1

    cpu.lap_enabled = 1;
    cpu.supervisor_state = 0; // Problem state
    cpu.psw_key = 1;          // Match block key

    // A. LAP Violation Test (triggers interrupt, swaps PSW using offset 88 in emulation)
    int lap_res = tsfi_s370_validate_write_with_interrupt(&cpu, 100, 0, block_keys, 2, real_memory, 1024);
    assert(lap_res == -1);
    assert(g_dat_stats.lap_violations == 1);
    assert(cpu.current_psw.instruction_address == 0x8899);

    // B. CCW Chain Auditing Test
    tsfi_s370_ccw ccw_chain[2];
    ccw_chain[0].data_addr = 0x100;
    ccw_chain[0].count = 0x200;
    ccw_chain[1].data_addr = 0x800;
    ccw_chain[1].count = 0x900; // 0x800 + 0x900 = 0x1100, exceeds limit (0x1000)

    int ccw_audit_res1 = tsfi_s370_audit_ccw_chain(ccw_chain, 2, 0x1000);
    assert(ccw_audit_res1 == -1);
    assert(g_dat_stats.ccw_violations == 1);

    int ccw_audit_res2 = tsfi_s370_audit_ccw_chain(ccw_chain, 1, 0x1000);
    assert(ccw_audit_res2 == 0);

    // C. Interval Timer Emulation Test
    // Reset New PSW at address 88 to 0x5555
    real_memory[94] = 0x55;
    real_memory[95] = 0x55;

    // Set timer value at address 80 to 2
    real_memory[80] = 0x00;
    real_memory[81] = 0x00;
    real_memory[82] = 0x00;
    real_memory[83] = 0x02;

    cpu.current_psw.instruction_address = 0x4444;

    // Tick for 0.01 seconds -> decrements by 192 (new value will be negative)
    tsfi_s370_tick_interval_timer(&cpu, real_memory, 1024, 0.01);
    assert(g_dat_stats.timer_interrupt_pending == 1);
    assert(cpu.current_psw.instruction_address == 0x5555);

    // --- 256-bit Dynamic Address Translation (DAT) validation ---
    printf("[TEST] Validating 256-bit Dynamic Address Translation (DAT)...\n");

    tsfi_s370_segment_entry_256 seg_table_256[2];
    memset(seg_table_256, 0, sizeof(seg_table_256));
    seg_table_256[0].page_table_origin = 0;
    seg_table_256[0].invalid = 0;
    seg_table_256[1].invalid = 1;

    tsfi_s370_page_entry_256 page_table_256[2];
    memset(page_table_256, 0, sizeof(page_table_256));
    
    // Set real page frame to 0xAA00BB00CC00DD00
    page_table_256[0].real_page_frame.parts[3] = 0xAA00;
    page_table_256[0].real_page_frame.parts[2] = 0xBB00;
    page_table_256[0].real_page_frame.parts[1] = 0xCC00;
    page_table_256[0].real_page_frame.parts[0] = 0xDD000000ULL;
    page_table_256[0].invalid = 0;
    page_table_256[0].write_protect = 1;

    // Set virtual address: Segment Index = 0 (parts 2 & 3), Page Index = 0 (part 1), Offset = 0xFFULL (part 0)
    tsfi_s370_addr_256 virt_addr_256;
    virt_addr_256.parts[3] = 0x00;
    virt_addr_256.parts[2] = 0x00;
    virt_addr_256.parts[1] = 0;
    virt_addr_256.parts[0] = 0xFFULL;

    tsfi_s370_addr_256 phys_addr_256;
    int wp_256 = 0;

    int ret_256 = tsfi_s370_dat_translate_256(&virt_addr_256, seg_table_256, 2, page_table_256, &phys_addr_256, &wp_256);
    assert(ret_256 == 0);
    assert(phys_addr_256.parts[3] == 0xAA00);
    assert(phys_addr_256.parts[2] == 0xBB00);
    assert(phys_addr_256.parts[1] == 0xCC00);
    assert(phys_addr_256.parts[0] == 0xDD0000FFULL);
    assert(wp_256 == 1);
    assert(g_dat_stats.translation_256_count == 1);

    printf("[SUCCESS] All 31-bit DAT Extensions, CPU security, and 256-bit DAT validated successfully!\n");
    return 0;
}
