#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define GRID_SIZE 16

void print_sprite(const char *name, uint8_t grid[GRID_SIZE][GRID_SIZE]) {
    printf("/* Sprite: %s */\n", name);
    printf("const uint16_t sprite_%s[16] = {\n", name);
    for (int r = 0; r < GRID_SIZE; r++) {
        uint16_t row_val = 0;
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] != 0) {
                row_val |= (1 << (15 - c));
            }
        }
        printf("    0x%04x%s\n", row_val, (r == GRID_SIZE - 1) ? "" : ",");
    }
    printf("};\n\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <karateka|cannonball>\n", argv[0]);
        return 1;
    }

    uint8_t grid[GRID_SIZE][GRID_SIZE];

    if (strcmp(argv[1], "karateka") == 0) {
        printf("/* Karateka Close Combat Sprite Set */\n\n");

        // 1. Running Stance (Stance 0)
        memset(grid, 0, sizeof(grid));
        for (int r = 1; r <= 3; r++)
            for (int c = 6; c <= 8; c++) grid[r][c] = 2; // Head
        for (int r = 4; r <= 9; r++)
            for (int c = 5; c <= 9; c++) grid[r][c] = 1; // Torso
        grid[10][4] = 3; grid[11][3] = 3; grid[12][2] = 3; // Left leg
        grid[10][8] = 3; grid[11][9] = 3; grid[12][10] = 3; // Right leg
        print_sprite("karateka_running", grid);

        // 2. Fighting/Neutral Stance (Stance 1)
        memset(grid, 0, sizeof(grid));
        for (int r = 2; r <= 4; r++)
            for (int c = 6; c <= 8; c++) grid[r][c] = 2;
        for (int r = 5; r <= 10; r++)
            for (int c = 5; c <= 9; c++) grid[r][c] = 1;
        grid[5][10] = 2; grid[5][11] = 2; // Arms
        grid[6][4] = 2;
        for (int r = 11; r <= 13; r++) {
            grid[r][4] = 3;
            grid[r][10] = 3;
        }
        print_sprite("karateka_fighting", grid);

        // 3. Ducking/Low Stance (Stance 2)
        memset(grid, 0, sizeof(grid));
        for (int r = 5; r <= 7; r++)
            for (int c = 6; c <= 8; c++) grid[r][c] = 2;
        for (int r = 8; r <= 11; r++)
            for (int c = 4; c <= 10; c++) grid[r][c] = 1;
        for (int c = 3; c <= 11; c++) {
            grid[12][c] = 3;
            grid[13][c] = 3;
        }
        print_sprite("karateka_ducking", grid);

        // 4. Jumping/High Stance (Stance 3)
        memset(grid, 0, sizeof(grid));
        for (int r = 0; r <= 2; r++)
            for (int c = 6; c <= 8; c++) grid[r][c] = 2;
        for (int r = 3; r <= 8; r++)
            for (int c = 5; c <= 9; c++) grid[r][c] = 1;
        grid[9][4] = 3; grid[9][10] = 3;
        grid[10][5] = 3; grid[10][9] = 3;
        print_sprite("karateka_jumping", grid);

    } else if (strcmp(argv[1], "cannonball") == 0) {
        printf("/* Cannonball Blitz Sprite Set */\n\n");
        
        // 1. Rolling Cannonball (16x16 circular shape)
        memset(grid, 0, sizeof(grid));
        for (int r = 2; r < 14; r++) {
            for (int c = 2; c < 14; c++) {
                int dx = c - 7;
                int dy = r - 7;
                if (dx*dx + dy*dy <= 36) {
                    grid[r][c] = 1;
                }
            }
        }
        print_sprite("cannonball_obstacle", grid);

        // 2. Player Idle
        memset(grid, 0, sizeof(grid));
        for (int r = 2; r <= 4; r++)
            for (int c = 6; c <= 9; c++) grid[r][c] = 1; // Hat/Head
        for (int r = 5; r <= 11; r++)
            for (int c = 5; c <= 10; c++) grid[r][c] = 2; // Overall/Torso
        for (int r = 12; r <= 14; r++) {
            grid[r][5] = 3; // Shoes
            grid[r][10] = 3;
        }
        print_sprite("player_idle", grid);

    } else {
        fprintf(stderr, "Unknown sprite set: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
