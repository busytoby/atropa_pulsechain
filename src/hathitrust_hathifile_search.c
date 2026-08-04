#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_hathifile_search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool hathitrust_hathifile_binary_search(
    const char *filepath,
    const char *htid,
    HathifileRow *record
) {
    if (!filepath || !htid || !record) return false;
    
    FILE *f = fopen(filepath, "r");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long high = ftell(f);
    long low = 0;
    
    char line[4096];
    bool found = false;
    
    while (low < high) {
        long mid = low + (high - low) / 2;
        fseek(f, mid, SEEK_SET);
        
        // If not at start of file, align to start of next line
        if (mid > 0) {
            int c;
            while ((c = fgetc(f)) != EOF && c != '\n');
        }
        
        long current_pos = ftell(f);
        if (current_pos >= high) {
            high = mid; // Narrow down
            continue;
        }
        
        if (!fgets(line, sizeof(line), f)) {
            high = mid;
            continue;
        }
        
        // Extract HTID (first column)
        char *tab = strchr(line, '\t');
        if (!tab) {
            high = mid;
            continue;
        }
        
        size_t htid_len = tab - line;
        char current_htid[256];
        if (htid_len < sizeof(current_htid)) {
            memcpy(current_htid, line, htid_len);
            current_htid[htid_len] = '\0';
        } else {
            high = mid;
            continue;
        }
        
        int cmp = strcmp(current_htid, htid);
        if (cmp == 0) {
            // Found it! Parse the full record
            fclose(f);
            return hathifile_parse_line(line, record);
        } else if (cmp < 0) {
            low = ftell(f);
        } else {
            high = mid;
        }
    }
    
    fclose(f);
    return found;
}
