#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MOTZKIN_PRIME 953467954114363ULL
#define WIDTH 400
#define HEIGHT 400
#define M_PI 3.14159265358979323846

typedef struct {
    double x, y, z;
} point_3d_t;

// Context structure for the projection matrices
typedef struct {
    double cos_theta;
    double sin_theta;
    uint32_t element_val;
    uint32_t identity_val;
} projection_config_t;

// Parses the quadtree coordinates sequentially from binary file
static int read_points_from_quadtree(const char *path, point_3d_t *points, int max_points) {
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
    // Simple traversal to gather coordinates
    while (count < max_points) {
        uint8_t active_flag = 0;
        if (fread(&active_flag, sizeof(uint8_t), 1, f) != 1) {
            break;
        }
        if (active_flag == 0) {
            continue; // Null child
        }

        // Read boundaries
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

// Applies rotation transformations using polarization parameters (Pole / Secret)
static void apply_polarization_rotation(point_3d_t *pts, int count, const projection_config_t *cfg) {
    for (int i = 0; i < count; i++) {
        // Rotate in the X-Y plane using trigonometric parameters
        double rx = pts[i].x * cfg->cos_theta - pts[i].y * cfg->sin_theta;
        double ry = pts[i].x * cfg->sin_theta + pts[i].y * cfg->cos_theta;
        pts[i].x = rx;
        pts[i].y = ry;
    }
}

// Rasterizes the coordinate sequence into a PPM image frame
static void rasterize_frame(const point_3d_t *pts, int count, const projection_config_t *cfg, const char *ppm_dest_path) {
    uint8_t *frame_buffer = (uint8_t *)calloc(WIDTH * HEIGHT * 3, sizeof(uint8_t));

    // Calculate dynamic line weight/opacity from Element density parameter
    uint8_t opacity_weight = (cfg->element_val % 100) + 155; // Clamped range (155-255)

    for (int i = 0; i < count; i++) {
        // 1. Transform coordinates to screen space viewport (scale/translate)
        int sx = (int)((pts[i].x + 1.2) * (WIDTH / 2.4));
        int sy = (int)((pts[i].y + 1.2) * (HEIGHT / 2.4));

        if (sx >= 0 && sx < WIDTH && sy >= 0 && sy < HEIGHT) {
            // Calculate dynamic pixel address using step sizing
            int addr = (sy * WIDTH + sx) * 3;

            // 2. Generate gradient color scheme mapping from Identity scale boundary parameters
            double factor = (double)i / (double)count;
            uint8_t r = (uint8_t)(128.0 + 127.0 * sin(factor * M_PI + (double)cfg->identity_val));
            uint8_t g = (uint8_t)(128.0 + 127.0 * sin(factor * M_PI * 2.0));
            uint8_t b = (uint8_t)(255 - r);

            // Apply opacity weight mapping
            frame_buffer[addr] = (uint8_t)((r * opacity_weight) / 255);
            frame_buffer[addr + 1] = (uint8_t)((g * opacity_weight) / 255);
            frame_buffer[addr + 2] = (uint8_t)((b * opacity_weight) / 255);
            
            // Draw adjacent coordinates to represent line thickness
            if (sx + 1 < WIDTH) {
                int adj_addr = (sy * WIDTH + (sx + 1)) * 3;
                frame_buffer[adj_addr] = frame_buffer[addr] / 2;
                frame_buffer[adj_addr + 1] = frame_buffer[addr + 1] / 2;
                frame_buffer[adj_addr + 2] = frame_buffer[addr + 2] / 2;
            }
        }
    }

    // Write out raw PPM file structure
    FILE *ppm = fopen(ppm_dest_path, "wb");
    if (ppm) {
        fprintf(ppm, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
        fwrite(frame_buffer, sizeof(uint8_t), WIDTH * HEIGHT * 3, ppm);
        fclose(ppm);
    }

    free(frame_buffer);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGGRAPH 3D TRAJECTORY RASTER RENDERER\n");
    printf("=============================================================\n");
    fflush(stdout);

    const char *in_path = "tests/siggraph_geometry.dat.bin";
    const char *out_path = "tests/siggraph_frame.ppm";

    // 1. Read coordinate points from spatial Quadtree
    point_3d_t points[250];
    int count = read_points_from_quadtree(in_path, points, 250);
    if (count == 0) {
        printf("   [ERROR] Failed to load quadtree projection points from %s\n", in_path);
        return 1;
    }
    printf("   ✓ Successfully loaded %d points from quadtree index.\n", count);
    fflush(stdout);

    // 2. Derive rotation parameters from Pole & Secret registers
    uint64_t secret = 5;
    uint64_t base = 2;
    // Pole = Base^Secret mod Prime
    uint64_t pole = 1;
    for (uint64_t i = 0; i < secret; i++) {
        pole = (pole * base) % MOTZKIN_PRIME;
    }

    // Calculate rotation angle theta
    double theta = ((double)pole / (double)MOTZKIN_PRIME) * 2.0 * M_PI;
    
    projection_config_t cfg = {
        .cos_theta = cos(theta + 0.785), // Apply custom polar shift (45 degrees)
        .sin_theta = sin(theta + 0.785),
        .element_val = 22, // Spacing density element
        .identity_val = 7  // EDO-22 octave boundary
    };

    printf("[RENDER] Dynamic Parameters Configuration:\n");
    printf("   Rotation Angle Theta (Pole):    %.4f rad\n", theta);
    printf("   Color Octave Shift (Identity):  %u\n", cfg.identity_val);
    printf("   Opacity Weight Density (Element):  %u\n", cfg.element_val);
    fflush(stdout);

    // 3. Apply polarization rotation matrices to points
    apply_polarization_rotation(points, count, &cfg);
    printf("   ✓ Polarization transformation applied to points.\n");
    fflush(stdout);

    // 4. Rasterize and export frame to PPM format
    rasterize_frame(points, count, &cfg, out_path);
    printf("   ✓ Trajectory frame rasterized to: %s\n", out_path);
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGGRAPH RASTER RENDERER COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
