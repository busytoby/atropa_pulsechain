#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_card(const char *card) {
    // The format is (1x, I2, 2X, 12(A1, I2, 2X))
    // 1x   : pos 0
    // I2   : pos 1, 2
    // 2X   : pos 3, 4
    // A1   : pos 5
    // I2   : pos 6, 7
    // 2X   : pos 8, 9
    // Repeat starting at pos 10
    
    int n = 0;
    if(card[1] != ' ') n += (card[1]-'0')*10;
    if(card[2] != ' ') n += (card[2]-'0');
    
    if (n <= 0) return;

    long long x[15], y[15];
    x[0] = 0; y[0] = 0;

    for (int i = 0; i < n; i++) {
        int base = 5 + (i * 5); // Pair i starts at index 5, 10, 15...
        char dir = card[base];
        int d = 0;
        if(card[base+1] != ' ') d += (card[base+1]-'0')*10;
        if(card[base+2] != ' ') d += (card[base+2]-'0');

        x[i+1] = x[i];
        y[i+1] = y[i];
        if (dir == 'N') y[i+1] += d;
        else if (dir == 'S') y[i+1] -= d;
        else if (dir == 'E') x[i+1] += d;
        else if (dir == 'W') x[i+1] -= d;
    }

    long long area_sum = 0;
    for (int i = 0; i < n; i++) {
        area_sum += (x[i] * y[i+1]) - (x[i+1] * y[i]);
    }
    // Implicitly close the polygon back to origin if card logic doesn't
    area_sum += (x[n] * y[0]) - (x[0] * y[n]);

    if (area_sum < 0) area_sum = -area_sum;
    printf("THE AREA IS %06lld\n", area_sum / 2);
}

int main() {
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        if (strlen(line) < 3 || line[0] == '\n') break;
        process_card(line);
    }
    return 0;
}
