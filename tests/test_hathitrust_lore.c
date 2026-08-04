#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_mets.h"
#include "inc/hathitrust_rib.h"
#include "inc/hathitrust_osl.h"
#include "inc/hathitrust_hathifile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

static void test_auncient_lore_pipeline(void) {
    printf("[Test] Running Auncient Lore Record Pipeline tests...\n");
    
    // Simulate generation of a custom METS structural catalog entry for Auncient lore
    const char *lore_mets_xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<mets OBJID=\"lore.auncient001\">\n"
        "  <structMap>\n"
        "    <div TYPE=\"volume\">\n"
        "      <div TYPE=\"page\" ORDER=\"1\" LABEL=\"Page [Auncient Title]\">\n"
        "        <fptr FILEID=\"FID_LORE1\"/>\n"
        "      </div>\n"
        "    </div>\n"
        "  </structMap>\n"
        "</mets>\n";
        
    HtrcMetsStructure mets;
    bool mets_ok = hathitrust_mets_parse(lore_mets_xml, &mets);
    assert(mets_ok);
    assert(strcmp(mets.volume_id, "lore.auncient001") == 0);
    assert(mets.page_count == 1);
    
    // Compile METS into RenderMan RIB Scene
    char *rib = hathitrust_mets_to_rib(&mets);
    assert(rib != NULL);
    printf("[Test] Compiled Lore RIB Manifest Length: %zu bytes\n", strlen(rib));
    assert(strstr(rib, "id=lore.auncient001") != NULL);
    
    // Generate age-appropriate OSL paper-aging shaders for a book published in 1776
    char *osl = hathitrust_generate_aging_osl(1776, 2026); // 250 years old
    assert(osl != NULL);
    assert(strstr(osl, "agingFactor = 1.000000") != NULL);
    
    // Parse simulated Hathifile row registration
    char tsv_line[1024];
    snprintf(tsv_line, sizeof(tsv_line),
             "lore.auncient001\tshare\tpd\tgoogle\tdesc\tsource\tstatus\toclc\tisbn\tissn\tlccn\tAuncient Records of Dysnomia\timprint\treason\t1776\tgov\tdate\tplace\tEnglish\ttext");
             
    HathifileRow row;
    bool parse_ok = hathifile_parse_line(tsv_line, &row);
    assert(parse_ok);
    assert(strcmp(row.htid, "lore.auncient001") == 0);
    assert(strcmp(row.title, "Auncient Records of Dysnomia") == 0);
    assert(strcmp(row.rights_timestamp, "1776") == 0);
    
    hathifile_free_row(&row);
    free(rib);
    free(osl);
    hathitrust_mets_free(&mets);
    
    printf("[Test] Auncient Lore Record Pipeline tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust Auncient Lore record to RenderMan tests...\n");
    test_auncient_lore_pipeline();
    printf("[Test] All Auncient Lore record tests completed successfully.\n");
    return 0;
}
