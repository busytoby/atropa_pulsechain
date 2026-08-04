#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_harvester.h"
#include "hathitrust_oai.h"
#include "hathitrust_oai_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void extract_and_process_records(const char *xml_page, void (*record_callback)(const char *xml_record)) {
    const char *ptr = xml_page;
    while ((ptr = strstr(ptr, "<record>")) != NULL) {
        const char *end = strstr(ptr, "</record>");
        if (!end) break;
        end += 9;
        size_t len = end - ptr;
        char *record = malloc(len + 1);
        if (record) {
            memcpy(record, ptr, len);
            record[len] = '\0';
            record_callback(record);
            free(record);
        }
        ptr = end;
    }
}

bool hathitrust_run_incremental_harvest(const char *state_file,
                                        const char *set,
                                        void (*record_callback)(const char *xml_record)) {
    if (!state_file || !record_callback) return false;
    
    // Load last sync date
    char *last_sync = hathitrust_oai_load_state(state_file);
    
    // Format current date
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    char current_date[64];
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", t);
    
    printf("[Harvester] Querying updates from %s to %s...\n", 
           last_sync ? last_sync : "beginning", current_date);
           
    // First page with retries
    char *xml = NULL;
    int retry_count = 0;
    while (retry_count < 3) {
        xml = hathitrust_oai_query("ListRecords", "oai_dc", set, NULL, NULL, last_sync, NULL);
        if (xml) break;
        retry_count++;
        printf("[Harvester] Initial query failed. Retrying in 5 seconds (attempt %d/3)...\n", retry_count);
        sleep(5);
    }
    free(last_sync);
    
    if (!xml) {
        fprintf(stderr, "[ERROR] Initial harvesting query failed after retries.\n");
        return false;
    }
    
    // Process first page records
    extract_and_process_records(xml, record_callback);
    
    // Pagination traversal
    char *token = hathitrust_oai_extract_tag(xml, "resumptionToken");
    free(xml);
    
    int page_count = 1;
    while (token && strlen(token) > 0 && page_count < 5) {
        sleep(2); // Friendly delay between requests to avoid rate limits
        
        char *next_xml = NULL;
        int next_retry = 0;
        while (next_retry < 3) {
            next_xml = hathitrust_oai_query("ListRecords", NULL, NULL, NULL, token, NULL, NULL);
            if (next_xml) break;
            next_retry++;
            printf("[Harvester] Pagination query failed. Retrying in 5 seconds (attempt %d/3)...\n", next_retry);
            sleep(5);
        }
        
        free(token);
        token = NULL;
        
        if (!next_xml) {
            fprintf(stderr, "[ERROR] Harvester pagination query failed after retries.\n");
            return false;
        }
        
        extract_and_process_records(next_xml, record_callback);
        
        token = hathitrust_oai_extract_tag(next_xml, "resumptionToken");
        free(next_xml);
        page_count++;
    }
    
    free(token);
    
    // Save updated state
    if (!hathitrust_oai_save_state(state_file, current_date)) {
        fprintf(stderr, "[ERROR] Failed to save updated harvest state.\n");
        return false;
    }
    
    printf("[Harvester] Harvest cycle complete. State updated to %s.\n", current_date);
    return true;
}
