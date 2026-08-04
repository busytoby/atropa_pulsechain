#include "tsfi_quadtree_ksds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TSV-HEADER QUADTREE KSDS INTEGRATION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    const char *filepath = "/tmp/test_quadtree_ksds.dat.bin";

    // Initialize 5 quadtree nodes representing spatial divisions
    InteropQuadNode nodes[5] = {
        { 0, 0, 100, 100, 12345, { 1, 2, 3, 4 } },  // Root node pointing to 4 quadrants
        { 0, 0, 50, 50, 999, { 0, 0, 0, 0 } },      // Quadrant 0 (Leaf)
        { 50, 0, 100, 50, 888, { 0, 0, 0, 0 } },    // Quadrant 1 (Leaf)
        { 0, 50, 50, 100, 777, { 0, 0, 0, 0 } },    // Quadrant 2 (Leaf)
        { 50, 50, 100, 100, 666, { 0, 0, 0, 0 } }   // Quadrant 3 (Leaf)
    };

    // TSV Header defining the database index configuration
    const char *tsv_header = "AUNCIENT_KSDS_QT\nQuadtreeCount:\t5\nRecordCount:\t100";
    const uint8_t mock_records[] = "DATABASE_PAYLOAD_EBCDIC_BLOCK_DATA";
    size_t rec_len = strlen((const char *)mock_records);

    // Write to the KSDS database file (.dat.bin format)
    bool ok = tsfi_qt_ksds_write(filepath, tsv_header, nodes, 5, mock_records, rec_len);
    assert(ok == true);

    // Read back and verify the header and index
    char header_out[256] = {0};
    InteropQuadNode nodes_out[5];
    memset(nodes_out, 0, sizeof(nodes_out));
    int node_count = 0;
    uint8_t records_out[256] = {0};
    int records_len = 0;

    ok = tsfi_qt_ksds_read(
        filepath,
        header_out,
        sizeof(header_out),
        nodes_out,
        5,
        &node_count,
        records_out,
        sizeof(records_out),
        &records_len
    );
    assert(ok == true);

    printf("[Test] Validating TSV Header...\n");
    assert(strstr(header_out, "AUNCIENT_KSDS_QT") != NULL);
    assert(strstr(header_out, "QuadtreeCount:\t5") != NULL);

    printf("[Test] Validating Quadtree Index Nodes...\n");
    assert(node_count == 5);
    assert(nodes_out[0].value == 12345);
    assert(nodes_out[4].value == 666);

    printf("[Test] Validating Quadtree Coordinate Search...\n");
    // Query coordinate (25, 25) which should hit quadrant 0 (value 999)
    uint32_t found_val = interop_quadtree_query(nodes_out, 0, 25, 25);
    assert(found_val == 999);

    // Query coordinate (75, 75) which should hit quadrant 3 (value 666)
    found_val = interop_quadtree_query(nodes_out, 0, 75, 75);
    assert(found_val == 666);

    printf("[Test] Validating EBCDIC Record Payload...\n");
    records_out[records_len] = '\0';
    assert(strcmp((char *)records_out, (const char *)mock_records) == 0);

    remove(filepath);
    printf("   ✓ TSV-header Quadtree KSDS format validated successfully.\n");
    printf("=============================================================\n");
    printf("TSV QUADTREE KSDS TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
