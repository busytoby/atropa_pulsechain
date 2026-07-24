#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


#define COLS 60
#define ROWS 30
#define NUM_ANIM_POINTS 200

typedef struct {
    double x, y, z;
} point_3d_t;

// Parses the quadtree coordinates sequentially from binary file
static int load_coordinates(const char *path, point_3d_t *points, int max_points) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 0;
    }

    uint32_t header = 0;
    if (fread(&header, sizeof(uint32_t), 1, f) != 1 || header != 0x51545245) {
        fclose(f);
        return 0;
    }

    int count = 0;
    while (count < max_points) {
        uint8_t active_flag = 0;
        if (fread(&active_flag, sizeof(uint8_t), 1, f) != 1) {
            break;
        }
        if (active_flag == 0) {
            continue;
        }

        double min_x, min_y, max_x, max_y;
        if (fread(&min_x, sizeof(double), 1, f) != 1 ||
            fread(&min_y, sizeof(double), 1, f) != 1 ||
            fread(&max_x, sizeof(double), 1, f) != 1 ||
            fread(&max_y, sizeof(double), 1, f) != 1) {
            break;
        }

        uint8_t point_flag = 0;
        if (fread(&point_flag, sizeof(uint8_t), 1, f) != 1) {
            break;
        }

        if (point_flag == 1) {
            double px, py, pz;
            if (fread(&px, sizeof(double), 1, f) != 1 ||
                fread(&py, sizeof(double), 1, f) != 1 ||
                fread(&pz, sizeof(double), 1, f) != 1) {
                break;
            }
            points[count].x = px;
            points[count].y = py;
            points[count].z = pz;
            count++;
        }
    }

    fclose(f);
    return count;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGGRAPH REAL-TIME TERMINAL PRESENTER\n");
    printf("=============================================================\n");
    fflush(stdout);

    const char *in_path = "tests/siggraph_geometry.dat.bin";
    point_3d_t raw_pts[NUM_ANIM_POINTS];
    int count = load_coordinates(in_path, raw_pts, NUM_ANIM_POINTS);
    if (count == 0) {
        printf("   [ERROR] Failed to load quadtree projection points from %s\n", in_path);
        return 1;
    }

    char grid[ROWS][COLS];
    char depth_ramp[] = " .:-=+*%@";
    int ramp_len = (int)strlen(depth_ramp);

    printf("[PRESENTER] Executing 30 frames of the animation loop...\n");
    fflush(stdout);

    // Run a finite number of loops for automated test suites
    for (int frame = 0; frame < 30; frame++) {
        // Clear grid buffer
        memset(grid, ' ', sizeof(grid));

        double angle = (double)frame * 0.15;
        double cos_a = cos(angle);
        double sin_a = sin(angle);

        // Project and rasterize points to grid with depth mapping
        for (int i = 0; i < count; i++) {
            // Apply rotation around the Y-axis
            double rx = raw_pts[i].x * cos_a - raw_pts[i].z * sin_a;
            double rz = raw_pts[i].x * sin_a + raw_pts[i].z * cos_a;
            double ry = raw_pts[i].y;

            // Map coordinates to viewport dimensions
            int cx = (int)((rx + 1.2) * (COLS / 2.4));
            int cy = (int)((ry + 1.2) * (ROWS / 2.4));

            if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS) {
                // ASCII Depth Maps: closer points (rz high) use heavier glyphs
                double norm_z = (rz + 1.2) / 2.4; // Normalized range (0.0 to 1.0)
                int char_idx = (int)(norm_z * (ramp_len - 1));
                if (char_idx < 0) char_idx = 0;
                if (char_idx >= ramp_len) char_idx = ramp_len - 1;

                grid[cy][cx] = depth_ramp[char_idx];
            }
        }

        // Output grid using ANSI cursor home code
        printf("\033[H");
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                putchar(grid[r][c]);
            }
            putchar('\n');
        }
        fflush(stdout);

        // Sleep for 30ms to throttle frame-rate
        usleep(30000);
    }

    printf("=============================================================\n");
    printf("TERMINAL PRESENTER SUITE COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
