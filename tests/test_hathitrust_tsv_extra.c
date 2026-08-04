#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_oai_identify.h"
#include "inc/hathitrust_solr_sql.h"
#include "inc/hathitrust_tsv_scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_oai_identify_parser(void) {
    printf("[Test] Running OAI-PMH Identify Parser tests...\n");
    
    const char *xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<OAI-PMH>\n"
        "  <Identify>\n"
        "    <repositoryName>HathiTrust Digital Library</repositoryName>\n"
        "    <adminEmail>feedback@issues.hathitrust.org</adminEmail>\n"
        "    <granularity>YYYY-MM-DDThh:mm:ssZ</granularity>\n"
        "    <compression>gzip</compression>\n"
        "  </Identify>\n"
        "</OAI-PMH>\n";
        
    HtrcOaiIdentity ident;
    bool ok = hathitrust_oai_parse_identify(xml, &ident);
    assert(ok);
    
    assert(strcmp(ident.repository_name, "HathiTrust Digital Library") == 0);
    assert(strcmp(ident.admin_email, "feedback@issues.hathitrust.org") == 0);
    assert(strcmp(ident.granularity, "YYYY-MM-DDThh:mm:ssZ") == 0);
    assert(strcmp(ident.compression, "gzip") == 0);
    
    hathitrust_oai_free_identity(&ident);
    printf("[Test] OAI-PMH Identify Parser tests passed.\n");
}

static void test_solr_sql_compiler(void) {
    printf("[Test] Running Solr-to-SQL Compiler tests...\n");
    
    char *sql = hathitrust_solr_to_sql(
        "uc1.31970011037748",
        "L'art mérovingien et carolingien",
        "Coutil, Léon",
        "1930"
    );
    assert(sql != NULL);
    printf("[Test] Generated SQL:\n%s\n", sql);
    
    assert(strstr(sql, "INSERT INTO hathitrust_catalog") != NULL);
    assert(strstr(sql, "VALUES ('uc1.31970011037748'") != NULL);
    assert(strstr(sql, "Coutil, Léon") != NULL);
    
    free(sql);
    printf("[Test] Solr-to-SQL Compiler tests passed.\n");
}

static void test_tsv_scanner(void) {
    printf("[Test] Running Hathifiles TSV Integrity Scanner tests...\n");
    
    // Correct 20 columns line (19 tabs)
    const char *ok_line = "mdp.39015018415946\tshare\tpd\tgoogle\tdesc1\tsource1\tstatus1\t1001\tisbn1\tissn1\tlccn1\tTitle\timprint1\treason1\t1969\tgov1\tdate1\tplace1\tSlovak\ttext\n";
    // Malformed line (too few tabs)
    const char *bad_line = "mdp.39015018415946\tshare\tpd\tgoogle\tTitle\t1969\n";
    
    assert(hathitrust_tsv_verify_integrity(ok_line) == true);
    assert(hathitrust_tsv_verify_integrity(bad_line) == false);
    
    printf("[Test] Hathifiles TSV Integrity Scanner tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust TSV extra utility tests...\n");
    test_oai_identify_parser();
    test_solr_sql_compiler();
    test_tsv_scanner();
    printf("[Test] All TSV extra utility tests completed successfully.\n");
    return 0;
}
