#include "inc/hathitrust_mets.h"
#include "inc/hathitrust_marc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_mets_parser(void) {
    printf("[Test] Running METS XML Parser tests...\n");
    
    const char *mock_mets =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<mets OBJID=\"uc1.31970011037748\">\n"
        "  <structMap>\n"
        "    <div TYPE=\"volume\">\n"
        "      <div TYPE=\"page\" ORDER=\"1\" LABEL=\"Page [1]\">\n"
        "        <fptr FILEID=\"FID1\"/>\n"
        "      </div>\n"
        "      <div TYPE=\"page\" ORDER=\"2\" LABEL=\"Page [2]\">\n"
        "        <fptr FILEID=\"FID2\"/>\n"
        "      </div>\n"
        "    </div>\n"
        "  </structMap>\n"
        "</mets>\n";
        
    HtrcMetsStructure mets;
    bool success = hathitrust_mets_parse(mock_mets, &mets);
    assert(success);
    
    assert(strcmp(mets.volume_id, "uc1.31970011037748") == 0);
    assert(mets.page_count == 2);
    
    assert(mets.pages[0].seq == 1);
    assert(strcmp(mets.pages[0].label, "Page [1]") == 0);
    assert(strcmp(mets.pages[0].file_id, "FID1") == 0);
    
    assert(mets.pages[1].seq == 2);
    assert(strcmp(mets.pages[1].label, "Page [2]") == 0);
    assert(strcmp(mets.pages[1].file_id, "FID2") == 0);
    
    hathitrust_mets_free(&mets);
    printf("[Test] METS XML Parser tests passed.\n");
}

static void test_marc_transcoder(void) {
    printf("[Test] Running MARC-XML Transcoder tests...\n");
    
    const char *mock_marc =
        "<record>\n"
        "  <datafield tag=\"245\" ind1=\"1\" ind2=\"0\">\n"
        "    <subfield code=\"a\">L'art mérovingien et carolingien /</subfield>\n"
        "  </datafield>\n"
        "  <datafield tag=\"100\" ind1=\"1\" ind2=\" \">\n"
        "    <subfield code=\"a\">Coutil, Léon,</subfield>\n"
        "  </datafield>\n"
        "  <datafield tag=\"260\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"c\">1930.</subfield>\n"
        "  </datafield>\n"
        "  <datafield tag=\"020\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"a\">9780000000000</subfield>\n"
        "  </datafield>\n"
        "  <datafield tag=\"035\" ind1=\" \" ind2=\" \">\n"
        "    <subfield code=\"a\">(OCoLC)491914940</subfield>\n"
        "  </datafield>\n"
        "</record>\n";
        
    HtrcMarcBibliographic bib;
    bool success = hathitrust_marc_transcode(mock_marc, &bib);
    assert(success);
    
    // Check fields with cleaned punctuation
    assert(strcmp(bib.title, "L'art mérovingien et carolingien") == 0);
    assert(strcmp(bib.author, "Coutil, Léon") == 0);
    assert(strcmp(bib.pub_date, "1930") == 0);
    assert(strcmp(bib.isbn, "9780000000000") == 0);
    assert(strcmp(bib.oclc, "(OCoLC)491914940") == 0);
    
    hathitrust_marc_free(&bib);
    printf("[Test] MARC-XML Transcoder tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust XML features unit tests...\n");
    test_mets_parser();
    test_marc_transcoder();
    printf("[Test] All XML features unit tests completed successfully.\n");
    return 0;
}
