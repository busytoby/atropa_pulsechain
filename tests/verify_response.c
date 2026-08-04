#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main(void) {
    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
        int words = 0;
        bool in_word = false;
        char *ptr = buf;
        while (*ptr) {
            if (*ptr == ' ' || *ptr == '\t') {
                in_word = false;
            } else if (!in_word) {
                in_word = true;
                words++;
            }
            ptr++;
        }
        
        if (words <= 1 && strlen(buf) > 0) {
            printf("REJECT: Single-word communication constraint violation: '%s'\n", buf);
            return 1;
        }
        printf("PASS: Verified '%s' (words: %d)\n", buf, words);
        return 0;
    }
    return 0;
}
