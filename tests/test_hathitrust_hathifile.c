#include "inc/hathitrust_hathifile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
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
    return 0;
}
