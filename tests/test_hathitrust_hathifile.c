#include "inc/hathitrust_hathifile.h"
#include "tsfi2-deepseek/inc/tsfi_mainframe_computerworld.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    int rc = 0;
    (void)rc;
    printf("[Test] Running Hathifile TSV parsing tests...\n");
    
    // Construct a mock Hathifile row: 20 fields separated by tabs
    // Note empty fields (tabs next to tabs) to verify strsep logic works
    char line[] = "miua.4919149.0001.001\tallow\tpd\t1002345\tvol. 1\tMIU\t\t01234567\t9780123456789\t\t2001-12345\tAuncient History of the VM\tAnn Arbor, MI\tbib\t2026-08-04 00:00:00\t0\t2026\tmi\teng\tBK";
    
    HathifileRow row;
    bool success = hathifile_parse_line(line, &row);
    assert(success);
    
    // Assert individual field values match the mocked string inputs
    assert(strcmp(row.htid, "miua.4919149.0001.001") == 0);
    assert(strcmp(row.access, "allow") == 0);
    assert(strcmp(row.rights, "pd") == 0);
    assert(strcmp(row.ht_bib_key, "1002345") == 0);
    assert(strcmp(row.description, "vol. 1") == 0);
    assert(strcmp(row.source, "MIU") == 0);
    assert(strcmp(row.source_inst_status, "") == 0); // Empty field
    assert(strcmp(row.oclc, "01234567") == 0);
    assert(strcmp(row.isbn, "9780123456789") == 0);
    assert(strcmp(row.issn, "") == 0); // Empty field
    assert(strcmp(row.lccn, "2001-12345") == 0);
    assert(strcmp(row.title, "Auncient History of the VM") == 0);
    assert(strcmp(row.imprint, "Ann Arbor, MI") == 0);
    assert(strcmp(row.rights_reason_code, "bib") == 0);
    assert(strcmp(row.rights_timestamp, "2026-08-04 00:00:00") == 0);
    assert(strcmp(row.us_gov_doc_flag, "0") == 0);
    assert(strcmp(row.rights_date_used, "2026") == 0);
    assert(strcmp(row.pub_place, "mi") == 0);
    assert(strcmp(row.lang, "eng") == 0);
    assert(strcmp(row.bib_fmt, "BK") == 0);
    
    hathifile_free_row(&row);
    printf("[Test] Hathifile parser assertions passed.\n");

    // Test VSAM export capability
    printf("[Test] Exporting Hathifile data to VSAM KSDS database...\n");
    FILE *mf = fopen("/tmp/mock_hathifile.txt", "w");
    assert(mf != NULL);
    char line_raw[] = "12345\tallow\tpd\t1002345\tvol. 1\tMIU\t\t01234567\t9780123456789\t\t2001-12345\tAuncient History of the VM\tAnn Arbor, MI\tbib\t2026-08-04 00:00:00\t0\t2026\tmi\teng\tBK\n";
    fwrite(line_raw, 1, strlen(line_raw), mf);
    fclose(mf);

    bool export_ok = hathifile_export_to_vsam("/tmp/mock_hathifile.txt", "/tmp/hathifile_ksds.dat.bin");
    assert(export_ok == true);

    tsfi_cw_vsam_ksds ht_ksds;
    memset(&ht_ksds, 0, sizeof(ht_ksds));
    rc = tsfi_cw_vsam_open(&ht_ksds, "/tmp/hathifile_ksds.dat.bin");
    assert(rc == 0);

    uint8_t read_data[1024] = {0};
    int read_len = 0;
    rc = tsfi_cw_vsam_read(&ht_ksds, "ht.12345", read_data, sizeof(read_data) - 1, &read_len);
    assert(rc == 0);
    read_data[read_len] = '\0';

    char expected[] = "12345\tallow\tpd\t1002345\tvol. 1\tMIU\t\t01234567\t9780123456789\t\t2001-12345\tAuncient History of the VM\tAnn Arbor, MI\tbib\t2026-08-04 00:00:00\t0\t2026\tmi\teng\tBK";
    assert(strcmp((char *)read_data, expected) == 0);

    remove("/tmp/mock_hathifile.txt");
    remove("/tmp/hathifile_ksds.dat.bin");
    printf("[Test] VSAM export validated successfully.\n");

    // Test Quadtree KSDS and AIX Export
    printf("[Test] Exporting Hathifile data to Quadtree KSDS with AIX...\n");
    FILE *mqf = fopen("/tmp/mock_hathifile_qt.txt", "w");
    assert(mqf != NULL);
    char line_qt[] = "12345\tallow\tpd\t1002345\tvol. 1\tMIU\t\t01234567\t9780123456789\t\t2001-12345\tAuncient History of the VM\tAnn Arbor, MI\tbib\t2026-08-04 00:00:00\t0\t2026\tmi\teng\tBK\n";
    fwrite(line_qt, 1, strlen(line_qt), mqf);
    fclose(mqf);

    bool export_qt_ok = hathifile_export_to_quadtree_ksds(
        "/tmp/mock_hathifile_qt.txt",
        "/tmp/ht_primary.dat.bin",
        "/tmp/ht_aix_isbn.dat.bin",
        "/tmp/ht_aix_oclc.dat.bin"
    );
    assert(export_qt_ok == true);

    // Verify retrieval via ISBN AIX Quadtree path
    // Helper to calculate coordinates
    extern uint64_t hash_string(const char *str); // Declare helper
    uint64_t h_isbn = hash_string("9780123456789");
    uint32_t x_i = (h_isbn & 0xFFFFFFFF) % 100;
    uint32_t y_i = ((h_isbn >> 32) & 0xFFFFFFFF) % 100;

    uint8_t res_buf[1024] = {0};
    int res_len = 0;
    // Link query interface
    extern bool tsfi_qt_ksds_aix_query(
        const char *aix_filepath,
        const char *primary_filepath,
        uint32_t secondary_x,
        uint32_t secondary_y,
        uint8_t *record_out,
        size_t max_record,
        int *record_len_out
    );

    bool q_ok = tsfi_qt_ksds_aix_query(
        "/tmp/ht_aix_isbn.dat.bin",
        "/tmp/ht_primary.dat.bin",
        x_i,
        y_i,
        res_buf,
        sizeof(res_buf) - 1,
        &res_len
    );
    assert(q_ok == true);
    res_buf[res_len] = '\0';

    char expected_qt[] = "12345\tallow\tpd\t1002345\tvol. 1\tMIU\t\t01234567\t9780123456789\t\t2001-12345\tAuncient History of the VM\tAnn Arbor, MI\tbib\t2026-08-04 00:00:00\t0\t2026\tmi\teng\tBK";
    assert(strcmp((char *)res_buf, expected_qt) == 0);

    remove("/tmp/mock_hathifile_qt.txt");
    remove("/tmp/ht_primary.dat.bin");
    remove("/tmp/ht_primary.dat.bin.qt.bin");
    remove("/tmp/ht_aix_isbn.dat.bin");
    remove("/tmp/ht_aix_oclc.dat.bin");
    printf("[Test] Quadtree KSDS and AIX export validated successfully.\n");

    return 0;
}
