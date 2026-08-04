#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_tsv_scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool hathitrust_tsv_verify_integrity(const char *tsv_line) {
    if (!tsv_line) return false;
    
    // Check that HTID is not empty (line does not start with tab)
    if (tsv_line[0] == '\t' || tsv_line[0] == '\n' || tsv_line[0] == '\r') {
        return false;
    }
    
    // Count tabs
    int tabs = 0;
    const char *ptr = tsv_line;
    while (*ptr != '\0') {
        if (*ptr == '\t') {
            tabs++;
        }
        ptr++;
    }
    
    // Valid Hathifile must have exactly 19 tab separators (20 columns)
    return (tabs == 19);
}
