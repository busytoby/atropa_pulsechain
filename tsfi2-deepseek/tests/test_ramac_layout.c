#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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
    // Index 0 and index 8 are in different sectors of track 0
    double seek_ab = tsfi_ramac_calculate_seek(0, 8);
    printf("  Seek rotational (0 -> 8): %.2f us\n", seek_ab);
    assert(seek_ab > 0.0);

    // Case C: Different head (head swap penalty)
    // Track 0 vs Track 1 (which are under different heads)
    double seek_ac = tsfi_ramac_calculate_seek(0, 160);
    printf("  Seek head swap (0 -> 160): %.2f us\n", seek_ac);
    assert(seek_ac >= 800.0); // at least 800 us

    // Case D: Different cylinder (cylinder travel penalty)
    double seek_ad = tsfi_ramac_calculate_seek(0, 8000);
    printf("  Seek cylinder travel (0 -> 8000): %.2f us\n", seek_ad);
    assert(seek_ad >= 1500.0); // at least 1.5 ms (1500 us)

    printf("  [PASS] Seek latency model calculations verified successfully.\n");

    // 3. Layout Optimization Verification
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
