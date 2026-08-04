#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_hathifile_diff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool hathitrust_hathifile_diff(
    const char *old_hathifile,
    const char *new_hathifile,
    const char *diff_output
) {
    if (!old_hathifile || !new_hathifile || !diff_output) return false;
    
    FILE *f_old = fopen(old_hathifile, "r");
    FILE *f_new = fopen(new_hathifile, "r");
    FILE *f_out = fopen(diff_output, "w");
    
    if (!f_old || !f_new || !f_out) {
        if (f_old) fclose(f_old);
        if (f_new) fclose(f_new);
        if (f_out) fclose(f_out);
        return false;
    }
    
    char line_old[2048];
    char line_new[2048];
    
    // Find lines added in new file that differ from old file
    while (fgets(line_new, sizeof(line_new), f_new)) {
        fseek(f_old, 0, SEEK_SET);
        bool found = false;
        while (fgets(line_old, sizeof(line_old), f_old)) {
            if (strcmp(line_old, line_new) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            fprintf(f_out, "+ %s", line_new);
        }
    }
    
    fclose(f_old);
    fclose(f_new);
    fclose(f_out);
    return true;
}
