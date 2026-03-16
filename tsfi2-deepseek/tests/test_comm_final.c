#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CARD_SIZE 80

// Task 2: 1974 Texas Contest Count Encoding
// 2-9 -> '2'-'9', 10-35 -> 'A'-'Z'
static char get_count_char(int k) {
    if (k >= 2 && k <= 9) return (char)('0' + k);
    if (k >= 10 && k <= 35) return (char)('A' + (k - 10));
    return '?';
}

void process_final_facilitator() {
    char card[256];
    if (!fgets(card, sizeof(card), stdin)) return;

    // 1. Parse Control Card (User Specification)
    // N (1-4), M (6-7)
    int N = 0;
    for(int i=0; i<4; i++) if(card[i]>='0' && card[i]<='9') N = N*10 + (card[i]-'0');
    int M = 0;
    for(int i=5; i<7; i++) if(card[i]>='0' && card[i]<='9') M = M*10 + (card[i]-'0');

    uint8_t map[256];
    for (int i = 0; i < 256; i++) map[i] = (uint8_t)i;
    for (int i = 0; i < M; i++) {
        map[(uint8_t)card[9 + i]] = (uint8_t)card[39 + i];
    }

    uint8_t *block = malloc(N + 1);
    int offset = 0;
    
    while (fgets(card, sizeof(card), stdin)) {
        if (strncmp(card, "****", 4) == 0) break;
        
        uint8_t data[CARD_SIZE];
        memset(data, ' ', CARD_SIZE);
        size_t slen = strlen(card);
        if (slen > 0 && card[slen-1] == '\n') card[--slen] = '\0';
        memcpy(data, card, slen > CARD_SIZE ? CARD_SIZE : slen);

        // Task 1: Translate
        for (int i = 0; i < CARD_SIZE; i++) data[i] = map[data[i]];

        // Task 2: 1974 Blank Compression (@ marker + base36 count)
        uint8_t comp[256]; // More than enough for 80-char card
        int c_idx = 0;
        for (int i = 0; i < CARD_SIZE; i++) {
            if (data[i] == ' ') {
                int k = 1;
                while (i + 1 < CARD_SIZE && data[i+1] == ' ' ) {
                    k++;
                    i++;
                }
                if (k == 1) {
                    comp[c_idx++] = ' ';
                } else {
                    // Recursive/Multi-part compression for k > 35
                    while (k > 35) {
                        comp[c_idx++] = '@';
                        comp[c_idx++] = 'Z';
                        k -= 35;
                    }
                    if (k > 1) {
                        comp[c_idx++] = '@';
                        comp[c_idx++] = get_count_char(k);
                    } else if (k == 1) {
                        comp[c_idx++] = ' ';
                    }
                }
            } else {
                comp[c_idx++] = data[i];
            }
        }

        // Task 3: Block (Fixed length N, no splitting, blank padding)
        if (offset + c_idx > N) {
            // Pad remaining with blanks and print
            memset(block + offset, ' ', N - offset);
            block[N] = '\0';
            printf("%s\n", block);
            offset = 0;
            memset(block, 0, N + 1);
        }

        memcpy(block + offset, comp, c_idx);
        offset += c_idx;
    }

    if (offset > 0) {
        memset(block + offset, ' ', N - offset);
        block[N] = '\0';
        printf("%s\n", block);
    }
    free(block);
}

int main() {
    process_final_facilitator();
    return 0;
}
