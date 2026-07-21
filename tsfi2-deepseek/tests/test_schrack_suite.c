#include "tsfi_parc_schrack.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING G. F. SCHRACK QUADTREE ALGORITHM TEST SUITE   \n");
    printf("=======================================================\n");

    // 1. Test Integer Dilation & Contraction
    uint32_t orig_x = 187;
    uint32_t orig_y = 94;
    uint64_t morton_code = tsfi_schrack_integer_dilation(orig_x, orig_y);

    uint32_t out_x = 0;
    uint32_t out_y = 0;
    tsfi_schrack_integer_contraction(morton_code, &out_x, &out_y);

    assert(out_x == orig_x);
    assert(out_y == orig_y);
    printf("[PASS] Schrack Integer Dilation/Contraction roundtrip verified: (%u, %u) -> Quadcode: 0x%016lX -> (%u, %u)\n", orig_x, orig_y, morton_code, out_x, out_y);

    // 2. Test O(1) Constant Time Equal-Size Neighbor Finding
    uint32_t depth = 8; // 256x256 Quadtree Grid
    uint64_t center_code = tsfi_schrack_integer_dilation(100, 100);

    uint64_t north_code = 0;
    int north_res = tsfi_schrack_find_equal_neighbor(center_code, depth, SCHRACK_DIR_NORTH, &north_code);
    assert(north_res == 0);

    uint32_t north_x = 0, north_y = 0;
    tsfi_schrack_integer_contraction(north_code, &north_x, &north_y);
    assert(north_x == 100);
    assert(north_y == 99);
    printf("[PASS] Schrack O(1) NORTH Neighbor verified: (100, 100) -> NORTH -> (100, 99)\n");

    uint64_t east_code = 0;
    int east_res = tsfi_schrack_find_equal_neighbor(center_code, depth, SCHRACK_DIR_EAST, &east_code);
    assert(east_res == 0);

    uint32_t east_x = 0, east_y = 0;
    tsfi_schrack_integer_contraction(east_code, &east_x, &east_y);
    assert(east_x == 101);
    assert(east_y == 100);
    printf("[PASS] Schrack O(1) EAST Neighbor verified: (100, 100) -> EAST -> (101, 100)\n");

    uint64_t ne_code = 0;
    int ne_res = tsfi_schrack_find_equal_neighbor(center_code, depth, SCHRACK_DIR_NORTH_EAST, &ne_code);
    assert(ne_res == 0);

    uint32_t ne_x = 0, ne_y = 0;
    tsfi_schrack_integer_contraction(ne_code, &ne_x, &ne_y);
    assert(ne_x == 101);
    assert(ne_y == 99);
    printf("[PASS] Schrack O(1) NORTH_EAST Neighbor verified: (100, 100) -> NORTH_EAST -> (101, 99)\n");

    // 3. Test Boundary Check
    uint64_t corner_code = tsfi_schrack_integer_dilation(0, 0);
    uint64_t out_bound_code = 0;
    int bound_res = tsfi_schrack_find_equal_neighbor(corner_code, depth, SCHRACK_DIR_NORTH, &out_bound_code);
    assert(bound_res == -2); // Out of Bounds
    printf("[PASS] Schrack Boundary Protection verified (NORTH of (0,0) returns boundary collision -2)\n");

    // 4. Test Hilbert-Schrack Curve Mapping
    uint64_t hilbert_code = tsfi_schrack_morton_to_hilbert(center_code, depth);
    assert(hilbert_code > 0);
    printf("[PASS] Hilbert-Schrack Space-Filling Curve Mapping verified: Morton 0x%016lX -> Hilbert 0x%016lX\n", center_code, hilbert_code);

    // 5. Test Rule 13 Quadtree .dat.bin Asset Extension Enforcement
    assert(tsfi_schrack_verify_quadtree_slice_asset("assets/block_history.dat.bin") == 0);
    assert(tsfi_schrack_verify_quadtree_slice_asset("invalid_quadtree_slice.json") == -3);
    printf("[PASS] Rule 13 Quadtree Storage Media Layout verified (.dat.bin accepted, .json rejected)\n");

    printf("=======================================================\n");
    printf(" ALL G. F. SCHRACK ALGORITHM VERIFICATIONS PASSED 100%% \n");
    printf("=======================================================\n");

    return 0;
}
