#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif

typedef struct {
    int old_num;
    int new_num;
    char text[256];
} BasicLine;

static void renumber_code(BasicLine *lines, int count) {
    // 1. Sort the lines by old line numbers
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (lines[j].old_num > lines[j+1].old_num) {
                BasicLine temp = lines[j];
                lines[j] = lines[j+1];
                lines[j+1] = temp;
            }
        }
    }
    
    // 2. Assign new line numbers starting at 10 incrementing by 10
    for (int i = 0; i < count; i++) {
        lines[i].new_num = (i + 1) * 10;
    }
    
    // 3. Update references (GOTO, GOSUB, THEN)
    for (int i = 0; i < count; i++) {
        char new_text[256] = "";
        char *src = lines[i].text;
        char *dest = new_text;
        size_t dest_avail = sizeof(new_text) - 1;
        
        while (*src) {
            bool match = false;
            int offset = 0;
            if (strncasecmp(src, "GOTO", 4) == 0) { match = true; offset = 4; }
            else if (strncasecmp(src, "GOSUB", 5) == 0) { match = true; offset = 5; }
            else if (strncasecmp(src, "THEN", 4) == 0) { match = true; offset = 4; }
            
            if (match) {
                int copy_len = snprintf(dest, dest_avail, "%.*s", offset, src);
                if (copy_len > 0) {
                    dest += copy_len;
                    dest_avail -= (size_t)copy_len;
                }
                src += offset;
                
                while (*src == ' ' || *src == '\t') {
                    if (dest_avail > 0) {
                        *dest++ = *src;
                        dest_avail--;
                    }
                    src++;
                }
                
                if (isdigit((unsigned char)*src)) {
                    int ref_num = atoi(src);
                    while (isdigit((unsigned char)*src)) src++;
                    
                    int new_ref = -1;
                    for (int k = 0; k < count; k++) {
                        if (lines[k].old_num == ref_num) {
                            new_ref = lines[k].new_num;
                            break;
                        }
                    }
                    
                    if (new_ref != -1) {
                        int written = snprintf(dest, dest_avail, "%d", new_ref);
                        if (written > 0) {
                            dest += written;
                            dest_avail -= (size_t)written;
                        }
                    } else {
                        int written = snprintf(dest, dest_avail, "%d", ref_num);
                        if (written > 0) {
                            dest += written;
                            dest_avail -= (size_t)written;
                        }
                    }
                }
            } else {
                if (dest_avail > 0) {
                    *dest++ = *src++;
                    dest_avail--;
                } else {
                    break;
                }
            }
        }
        *dest = '\0';
        snprintf(lines[i].text, sizeof(lines[i].text), "%.255s", new_text);
    }
}

int main(void) {
    printf("[UNIT TEST] Testing BASIC Renumbering Utility logic...\n");
    
    BasicLine lines[4];
    lines[0].old_num = 40;
    snprintf(lines[0].text, sizeof(lines[0].text), "GOTO 10");
    
    lines[1].old_num = 10;
    snprintf(lines[1].text, sizeof(lines[1].text), "PRINT \"START\"");
    
    lines[2].old_num = 30;
    snprintf(lines[2].text, sizeof(lines[2].text), "PRINT \"LOOP\"");
    
    lines[3].old_num = 20;
    snprintf(lines[3].text, sizeof(lines[3].text), "GOSUB 30");
    
    renumber_code(lines, 4);
    
    // Check sorting and number assignment
    assert(lines[0].old_num == 10 && lines[0].new_num == 10);
    assert(lines[1].old_num == 20 && lines[1].new_num == 20);
    assert(lines[2].old_num == 30 && lines[2].new_num == 30);
    assert(lines[3].old_num == 40 && lines[3].new_num == 40);
    
    // Check updated GOSUB and GOTO references
    assert(strcmp(lines[1].text, "GOSUB 30") == 0); // old 30 -> new 30
    assert(strcmp(lines[3].text, "GOTO 10") == 0);  // old 10 -> new 10
    
    printf("[UNIT TEST] All assertions passed successfully!\n");
    return 0;
}
