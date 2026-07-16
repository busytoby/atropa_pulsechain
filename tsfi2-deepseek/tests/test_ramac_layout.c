#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_ramac_layout.h"

int main(void) {
    printf("=============================================================\n");
    printf("RAMAC LAYOUT GEOMETRY & SEEK SIMULATION VERIFICATION\n");
    printf("=============================================================\n");

    // 1. CHS Translation Verification
    printf("[Test] Verifying Index <-> CHS translation...\n");
    int test_indices[] = {0, 8, 160, 8000, 123456, 799999};
    for (size_t i = 0; i < sizeof(test_indices)/sizeof(test_indices[0]); i++) {
        int idx = test_indices[i];
        tsfi_ramac_chs chs = tsfi_ramac_index_to_chs(idx);
        int roundtrip = tsfi_ramac_chs_to_index(chs);
        printf("  Index %d -> Cyl %d, Head %d, Sec %d, Word %d -> Roundtrip %d\n",
               idx, chs.cylinder, chs.head, chs.sector, chs.word_offset, roundtrip);
        assert(idx == roundtrip);
    }
    printf("  [PASS] CHS translation roundtrip verified successfully.\n");

    // 2. Seek Latency Verification
    printf("[Test] Verifying RAMAC seek simulation latency...\n");
    // Case A: Same sector (0 seek latency)
    double seek_aa = tsfi_ramac_calculate_seek(10, 10);
    printf("  Seek same index (10 -> 10): %.2f us\n", seek_aa);
    assert(seek_aa == 0.0);

    // Case B: Different sector, same head/cyl (rotational delay only)
    double seek_ab = tsfi_ramac_calculate_seek(0, 8);
    printf("  Seek rotational (0 -> 8): %.2f us\n", seek_ab);
    assert(seek_ab > 0.0);

    // Case C: Different head (head swap penalty)
    double seek_ac = tsfi_ramac_calculate_seek(0, 160);
    printf("  Seek head swap (0 -> 160): %.2f us\n", seek_ac);
    assert(seek_ac >= 800.0); 

    // Case D: Different cylinder (cylinder travel penalty)
    double seek_ad = tsfi_ramac_calculate_seek(0, 8000);
    printf("  Seek cylinder travel (0 -> 8000): %.2f us\n", seek_ad);
    assert(seek_ad >= 1500.0); 

    printf("  [PASS] Seek latency model calculations verified successfully.\n");

    // 3. Cylinder Overflow Hash Verification
    printf("[Test] Verifying RAMAC Cylinder Overflow Hash table...\n");
    int total_slots = RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS;
    tsfi_ramac_record *disk = calloc(total_slots, sizeof(tsfi_ramac_record));
    assert(disk != NULL);

    double total_seek = 0.0;
    // Insert first key in cylinder 5
    int idx1 = tsfi_ramac_insert_record(disk, "customer_101", "active_record", 5, &total_seek);
    printf("  Inserted 'customer_101' at slot index %d (seek: %.2f us)\n", idx1, total_seek);
    assert(idx1 != -1);
    assert(disk[idx1].is_active);

    // Force a collision on the primary slot of "customer_999" by manually pre-occupying it
    int primary_idx_999 = tsfi_ramac_hash_key("customer_999", 5);
    disk[primary_idx_999].is_active = 1;
    strcpy(disk[primary_idx_999].key, "colliding_dummy");
    strcpy(disk[primary_idx_999].value, "dummy_val");
    disk[primary_idx_999].next_overflow_index = -1;

    double collision_seek = 0.0;
    int idx2 = tsfi_ramac_insert_record(disk, "customer_999", "colliding_record", 5, &collision_seek);
    printf("  Inserted colliding 'customer_999' at slot index %d (seek: %.2f us)\n", idx2, collision_seek);
    assert(idx2 != -1);
    assert(idx2 != primary_idx_999);
    
    // Verify that the overflow record is located in heads 45..49 (same cylinder)
    tsfi_ramac_chs chs2 = tsfi_ramac_index_to_chs(idx2);
    printf("  Collision resolved to Cylinder %d, Head %d, Sector %d\n", chs2.cylinder, chs2.head, chs2.sector);
    assert(chs2.cylinder == 5);
    assert(chs2.head >= 45 && chs2.head < 50);

    // Search validation
    double search_seek = 0.0;
    const char *val1 = tsfi_ramac_search_record(disk, "customer_101", 5, &search_seek);
    assert(val1 != NULL && strcmp(val1, "active_record") == 0);
    printf("  Search 'customer_101' val: %s (seek: %.2f us)\n", val1, search_seek);

    const char *val2 = tsfi_ramac_search_record(disk, "customer_999", 5, &search_seek);
    assert(val2 != NULL && strcmp(val2, "colliding_record") == 0);
    printf("  Search 'customer_999' val: %s (seek: %.2f us)\n", val2, search_seek);

    printf("  [PASS] Cylinder Overflow Hash verified successfully.\n");

    // 3.5. Read-After-Write Verification & Plugboard Emulator
    printf("[Test] Verifying Read-After-Write verification...\n");
    int verify_status = tsfi_ramac_write_verified(disk, "customer_333", "verified_val", 5);
    assert(verify_status == 0);
    printf("  [PASS] Read-After-Write verification check passed.\n");

    printf("[Test] Verifying RAMAC Plugboard panel router...\n");
    uint8_t src_buf[64] = "RAMAC_CONTROL_PANEL_INPUT_DATA_1956";
    uint8_t dest_buf[64] = {0};
    // Wire bytes 6..18 ("CONTROL_PANEL") to bytes 10..22 of dest
    int routed = tsfi_ramac_plugboard_route("6..18->10..22", src_buf, dest_buf, 64);
    printf("  Plugboard routed %d bytes: Dest = '%s'\n", routed, dest_buf + 10);
    assert(routed == 13);
    assert(strcmp((char*)(dest_buf + 10), "CONTROL_PANEL") == 0);
    printf("  [PASS] Plugboard router layout updates verified successfully.\n");

    free(disk);

    // 4. Layout Optimization Verification
    printf("[Test] Verifying layout serialization...\n");
    tsfi_dat mock_dat;
    mock_dat.capacity = 1000;
    mock_dat.base = (int*)calloc(mock_dat.capacity, sizeof(int));
    mock_dat.check = (int*)calloc(mock_dat.capacity, sizeof(int));
    mock_dat.values = NULL;
    mock_dat.tail = NULL;
    mock_dat.tail_size = 0;

    for (int i = 0; i < mock_dat.capacity; i++) {
        mock_dat.base[i] = i * 2;
        mock_dat.check[i] = i;
    }

    int ret = tsfi_ramac_layout_optimize(&mock_dat, "tmp/test_ramac_layout.dat.bin");
    assert(ret == 0);
    printf("  [PASS] Saved RAMAC layout file to tmp/test_ramac_layout.dat.bin\n");

    free(mock_dat.base);
    free(mock_dat.check);

    printf("[PASS] All RAMAC layout and seek simulation components verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
