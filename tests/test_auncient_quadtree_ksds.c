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
        { 0, 0, 50, 50, 999, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },      // Quadrant 0 (Leaf)
        { 50, 0, 100, 50, 888, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },    // Quadrant 1 (Leaf)
        { 0, 50, 50, 100, 777, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } },    // Quadrant 2 (Leaf)
        { 50, 50, 100, 100, 666, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }   // Quadrant 3 (Leaf)
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

    // AIX Quadtree Index Test
    printf("[Test] Validating Alternate Index (AIX) Quadtree Query...\n");
    const char *primary_file = "/tmp/test_primary.dat.bin";
    const char *aix_file = "/tmp/test_aix.dat.bin";

    FILE *pf = fopen(primary_file, "wb");
    assert(pf != NULL);
    // Write padding
    uint8_t zero_pad[256] = {0};
    fwrite(zero_pad, 1, 256, pf);

    uint32_t rec_offset = (uint32_t)ftell(pf);
    const char *record_text = "12345\tallow\tpd\t1002345\tvol. 1\n";
    fwrite(record_text, 1, strlen(record_text), pf);
    fclose(pf);

    // AIX node mapping coordinate (10, 10) to the primary record offset
    InteropQuadNode aix_nodes[1] = {
        { 0, 0, 100, 100, rec_offset, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }
    };

    ok = tsfi_qt_ksds_aix_write(aix_file, "AUNCIENT_AIX\nQuadtreeCount:\t1\nRecordCount:\t1", aix_nodes, 1, &rec_offset, 1);
    assert(ok == true);

    uint8_t aix_res[256] = {0};
    int aix_res_len = 0;
    ok = tsfi_qt_ksds_aix_query(aix_file, primary_file, 10, 10, aix_res, sizeof(aix_res), &aix_res_len);
    assert(ok == true);
    aix_res[aix_res_len] = '\0';

    assert(strcmp((char *)aix_res, "12345\tallow\tpd\t1002345\tvol. 1") == 0);

    remove(primary_file);
    remove(aix_file);
    printf("   ✓ Alternate Index (AIX) Quadtree query verified successfully.\n");

    // Unicode Dual-Encoding and Normalization Test
    printf("[Test] Validating Unicode Dual-Encoding and Normalization...\n");
    const char *uni_file = "/tmp/test_unicode.dat.bin";
    const char *utf8_in = "HathiTrust Café: Auncient History of the VM (öé)";

    InteropQuadNode uni_nodes[1] = {
        { 0, 0, 100, 100, 42, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }
    };

    ok = tsfi_qt_ksds_write_dual(uni_file, "AUNCIENT_UNI\nQuadtreeCount:\t1\nRecordCount:\t1", uni_nodes, 1, utf8_in);
    assert(ok == true);

    char ebcdic_out[256] = {0};
    int ebcdic_len = 0;
    char utf8_out[256] = {0};
    int utf8_len = 0;

    ok = tsfi_qt_ksds_read_dual(
        uni_file,
        header_out,
        sizeof(header_out),
        nodes_out,
        1,
        &node_count,
        ebcdic_out,
        sizeof(ebcdic_out),
        &ebcdic_len,
        utf8_out,
        sizeof(utf8_out),
        &utf8_len
    );
    assert(ok == true);

    printf("[Test] Validating Raw UTF-8 Segment...\n");
    utf8_out[utf8_len] = '\0';
    assert(strcmp(utf8_out, utf8_in) == 0);

    printf("[Test] Validating Normalized EBCDIC Segment...\n");
    // Translate EBCDIC back to ASCII using tsfi_cw_ebcdic_to_ascii_buf to assert normalization
    extern int tsfi_cw_ebcdic_to_ascii_buf(const uint8_t *ebcdic_in, char *ascii_out, int len);
    char ascii_back[256] = {0};
    tsfi_cw_ebcdic_to_ascii_buf((const uint8_t *)ebcdic_out, ascii_back, ebcdic_len);
    ascii_back[ebcdic_len] = '\0';

    assert(strcmp(ascii_back, "HathiTrust Cafe: Auncient History of the VM (oe)") == 0);

    remove(uni_file);
    printf("   ✓ Unicode dual-encoding and normalization verified successfully.\n");

    printf("=============================================================\n");
    printf("TSV QUADTREE KSDS TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
