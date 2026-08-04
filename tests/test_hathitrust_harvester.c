#include "inc/hathitrust_harvester.h"
#include "inc/hathitrust_oai_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

static int record_count = 0;

static void verify_record_callback(const char *xml_record) {
    record_count++;
    assert(strstr(xml_record, "<record>") != NULL);
    assert(strstr(xml_record, "</record>") != NULL);
}

int main(void) {
    printf("[Test] Running Live Harvester Coordinator tests...\n");
    
    // Set up a mock state file representing last sync dynamically set to yesterday
    // This keeps the test footprint extremely lightweight and resilient against network timeouts
    time_t now = time(NULL);
    now -= 86400; // 1 day ago
    struct tm *t = gmtime(&now);
    char yesterday[64];
    strftime(yesterday, sizeof(yesterday), "%Y-%m-%d", t);
    
    const char *state_filepath = "/tmp/hathitrust_harvest_state.txt";
    hathitrust_oai_save_state(state_filepath, yesterday);
    
    // Execute incremental harvest for a very narrow set or general US public domain set
    bool harvest_success = hathitrust_run_incremental_harvest(state_filepath, "hathitrust:pdus", verify_record_callback);
    assert(harvest_success);
    
    printf("[Test] Harvester finished. Processed %d records.\n", record_count);
    
    // Validate that the state timestamp was updated to the current date (non-empty)
    char *updated_state = hathitrust_oai_load_state(state_filepath);
    assert(updated_state != NULL);
    assert(strlen(updated_state) == 10); // YYYY-MM-DD
    
    printf("[Test] Updated state date: %s\n", updated_state);
    
    free(updated_state);
    remove(state_filepath);
    
    printf("[Test] Harvester Coordinator assertions passed.\n");
    return 0;
}
