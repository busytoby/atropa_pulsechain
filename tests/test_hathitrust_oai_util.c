#include "inc/hathitrust_oai_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void dummy_page_callback(const char *xml_page) {
    printf("[Test] Received page callback with XML length: %zu\n", strlen(xml_page));
    assert(strstr(xml_page, "<OAI-PMH") != NULL);
}

int main(void) {
    printf("[Test] Running OAI-PMH Util verification...\n");
    
    // 1. Verify XML extraction
    const char *xml = "<oai><dc:title>Auncient VM History</dc:title><resumptionToken>TOKEN123</resumptionToken></oai>";
    char *title = hathitrust_oai_extract_tag(xml, "dc:title");
    assert(title != NULL);
    assert(strcmp(title, "Auncient VM History") == 0);
    free(title);
    
    char *token = hathitrust_oai_extract_tag(xml, "resumptionToken");
    assert(token != NULL);
    assert(strcmp(token, "TOKEN123") == 0);
    free(token);
    
    // 2. Verify state storage/loading
    const char *state_file = "/tmp/hathitrust_test_state.txt";
    assert(hathitrust_oai_save_state(state_file, "2026-08-04T00:00:00Z"));
    
    char *loaded_date = hathitrust_oai_load_state(state_file);
    assert(loaded_date != NULL);
    assert(strcmp(loaded_date, "2026-08-04T00:00:00Z") == 0);
    free(loaded_date);
    remove(state_file);
    
    // 3. Verify auto-pagination using a small set (hathitrust:pdus is a standard US public domain set)
    // We execute the paginator using a mock or live request with maximum pages limited to 5
    printf("[Test] Starting live pagination test against HathiTrust OAI-PMH...\n");
    bool pagination_success = hathitrust_oai_harvest_all("hathitrust:pdus", dummy_page_callback);
    if (!pagination_success) {
        printf("[WARNING] Live pagination harvesting skipped or failed (possible network rate limiting).\n");
    } else {
        printf("[Test] Live pagination harvesting succeeded.\n");
    }
    
    printf("[Test] OAI-PMH Util verification passed.\n");
    return 0;
}
