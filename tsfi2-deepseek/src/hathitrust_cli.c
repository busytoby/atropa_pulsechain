#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_hathifile_search.h"
#include "inc/hathitrust_cache_profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("HathiTrust Unified CLI\n");
        printf("Usage:\n");
        printf("  %s --profile          Profile cache latencies\n", argv[0]);
        printf("  %s --search [file] [htid]  Binary search local Hathifile TSV\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "--profile") == 0) {
        printf("[CLI] Profiling cache lookup speeds...\n");
        double latency = hathitrust_profile_cache_latency();
        printf("[CLI] Average Latency: %.2f ns\n", latency);
        return 0;
    }
    
    if (strcmp(argv[1], "--search") == 0) {
        if (argc < 4) {
            printf("[Error] Missing parameters for search.\n");
            return 1;
        }
        
        const char *file = argv[2];
        const char *htid = argv[3];
        
        printf("[CLI] Searching Hathifile %s for key: %s...\n", file, htid);
        HathifileRow row;
        if (hathitrust_hathifile_binary_search(file, htid, &row)) {
            printf("[CLI] Found Record:\n");
            printf("  HTID:   %s\n", row.htid ? row.htid : "");
            printf("  Title:  %s\n", row.title ? row.title : "");
            printf("  Lang:   %s\n", row.lang ? row.lang : "");
            printf("  Rights: %s\n", row.rights ? row.rights : "");
            hathifile_free_row(&row);
        } else {
            printf("[CLI] Record not found.\n");
        }
        return 0;
    }
    
    printf("[Error] Unknown parameter: %s\n", argv[1]);
    return 1;
}
