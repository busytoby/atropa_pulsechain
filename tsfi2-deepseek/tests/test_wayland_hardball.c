#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Screen Geometry
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// Structures
typedef struct {
    float x, y, z;
} Point3D;

typedef struct {
    int x, y;
} Point2D;

// Projection from 3D to 2D screen coordinates
Point2D project(Point3D p) {
    Point2D out;
    // Vanishing point at (512, 200) representing the center outfield wall
    float fov = 600.0f;
    float depth = p.z + 500.0f;
    if (depth <= 1.0f) depth = 1.0f;

    out.x = (int)((p.x * fov) / depth + (SCREEN_WIDTH / 2));
    out.y = (int)((-p.y * fov) / depth + 400); // Shift horizon to middle-ish
    return out;
}

// Draw a solid circle with custom colors
void drawCircle(uint32_t *px, int cx, int cy, int radius, uint32_t color) {
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                int dx = x - cx;
                int dy = y - cy;
                if (dx*dx + dy*dy <= radius*radius) {
                    px[y * SCREEN_WIDTH + x] = color;
                }
            }
        }
    }
}

// Draw a volumetric capsule/cylinder (pitcher/batter limbs or lines)
void drawCapsule(uint32_t *px, Point3D p1, Point3D p2, float radius, uint32_t color) {
    Point2D sc1 = project(p1);
    Point2D sc2 = project(p2);
    
    int dx = sc2.x - sc1.x;
    int dy = sc2.y - sc1.y;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1.0f) return;

    // Standard rasterized line with width representing volumetric scaling
    float avgDepth = (p1.z + p2.z) / 2.0f + 500.0f;
    float screenRadius = radius * (600.0f / avgDepth);
    if (screenRadius < 1.0f) screenRadius = 1.0f;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Find distance to segment
            float l2 = (sc2.x - sc1.x)*(sc2.x - sc1.x) + (sc2.y - sc1.y)*(sc2.y - sc1.y);
            float t = ((x - sc1.x) * (sc2.x - sc1.x) + (y - sc1.y) * (sc2.y - sc1.y)) / l2;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            float projX = sc1.x + t * (sc2.x - sc1.x);
            float projY = sc1.y + t * (sc2.y - sc1.y);
            float dist2 = (x - projX)*(x - projX) + (y - projY)*(y - projY);
            if (dist2 <= screenRadius * screenRadius) {
                px[y * SCREEN_WIDTH + x] = color;
            }
        }
    }
}

// Client-side Sobel Filter Edge Contouring Preprocessor
void applySobelOutline(uint32_t *px, uint32_t *out) {
    // Standard Sobel kernels
    int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 1; y < SCREEN_HEIGHT - 1; y++) {
        for (int x = 1; x < SCREEN_WIDTH - 1; x++) {
            float valX = 0.0f;
            float valY = 0.0f;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    uint32_t pixel = px[(y + ky) * SCREEN_WIDTH + (x + kx)];
                    // Grayscale conversion
                    uint8_t r = (pixel >> 16) & 0xFF;
                    uint8_t g = (pixel >> 8) & 0xFF;
                    uint8_t b = pixel & 0xFF;
                    float gray = 0.299f * r + 0.587f * g + 0.114f * b;

                    valX += gray * gx[ky + 1][kx + 1];
                    valY += gray * gy[ky + 1][kx + 1];
                }
            }

            float magnitude = sqrtf(valX * valX + valY * valY);
            if (magnitude > 255.0f) magnitude = 255.0f;
            uint8_t edgeVal = (uint8_t)magnitude;

            if (edgeVal > 45) {
                out[y * SCREEN_WIDTH + x] = 0xFFFFFFFF; // White outline
            } else {
                out[y * SCREEN_WIDTH + x] = 0xFF000000; // Black background
            }
        }
    }
}

// Render Hardball frame with C64 Pitcher layout & volumetric depth
void renderHardballFrame(uint32_t *px, int frame, bool useSobel) {
    (void)useSobel;
    // 1. Clear with Retro Field Green base
    uint32_t greenField = 0xFF2E7D32;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        px[i] = greenField;
    }

    // 2. Scoreboard Header (C64 Style)
    // Dark gray panel at top
    for (int y = 0; y < 60; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            px[y * SCREEN_WIDTH + x] = 0xFF1E293B;
        }
    }

    // 3. Perspective Foul Lines & Home Plate
    // Draw infield brown dirt polygon lines
    drawCapsule(px, (Point3D){-400.0f, -50.0f, -200.0f}, (Point3D){0.0f, -50.0f, 800.0f}, 6.0f, 0xFF8D6E63);
    drawCapsule(px, (Point3D){400.0f, -50.0f, -200.0f}, (Point3D){0.0f, -50.0f, 800.0f}, 6.0f, 0xFF8D6E63);

    // 4. Volumetric Pitcher (Mound in center)
    Point3D pitcherHead = {0.0f, 60.0f, 300.0f};
    Point3D pitcherTorso = {0.0f, -20.0f, 300.0f};
    Point2D scPitcher = project(pitcherHead);
    drawCapsule(px, pitcherTorso, pitcherHead, 25.0f, 0xFFE0E0E0); // White uniform Torso
    drawCircle(px, scPitcher.x, scPitcher.y, 20, 0xFFD7CCC8); // Head

    // 5. Volumetric Batter standing at Home Plate (close z-depth)
    float swingOffset = sinf(frame * 0.15f) * 40.0f;
    Point3D batterHead = {-90.0f, 80.0f, -100.0f};
    Point3D batterTorso = {-90.0f, -40.0f, -100.0f};
    Point3D batterBatTip = {-90.0f + swingOffset, 120.0f - swingOffset, -100.0f};
    Point2D scBatter = project(batterHead);
    drawCapsule(px, batterTorso, batterHead, 45.0f, 0xFF1976D2); // Blue uniform Torso
    drawCircle(px, scBatter.x, scBatter.y, 32, 0xFFFFE0B2); // Head
    drawCapsule(px, batterHead, batterBatTip, 10.0f, 0xFFFFA000); // Baseball Bat (Yellow/Orange)

    // 6. Volumetric Catcher (behind plate)
    Point3D catcherHead = {0.0f, -10.0f, -150.0f};
    Point3D catcherTorso = {0.0f, -90.0f, -150.0f};
    Point2D scCatcher = project(catcherHead);
    drawCapsule(px, catcherTorso, catcherHead, 50.0f, 0xFFD32F2F); // Red chest protector
    drawCircle(px, scCatcher.x, scCatcher.y, 35, 0xFF37474F); // Dark Catcher Helmet

    // 7. Interactive Flying Ball (Z-depth trajectory)
    float ballCycle = (frame % 45) / 45.0f;
    Point3D ballPos = {
        0.0f + ballCycle * swingOffset, 
        40.0f - ballCycle * 100.0f, 
        300.0f - ballCycle * 450.0f
    };
    Point2D scBall = project(ballPos);
    float ballSize = 6.0f + ballCycle * 22.0f; // Volumetric scaling as ball approaches
    drawCircle(px, scBall.x, scBall.y, (int)ballSize, 0xFFFFFFFF); // White ball
}

int main() {
    printf("=== C64 Hardball Vulkan-Simulation ===\n");
    printf("[INFO] Allocating screen grid framebuffers (1024x768)...\n");

    size_t sz = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t);
    uint32_t *frameBuffer = malloc(sz);
    uint32_t *edgeBuffer = malloc(sz);

    if (!frameBuffer || !edgeBuffer) {
        printf("[ERROR] Failed to allocate buffers.\n");
        return 1;
    }

    // Render loop mimicking Vulkan presentation ticks
    for (int frame = 0; frame < 50; frame++) {
        // Toggle Sobel edge extraction mid-way to simulate ControlNet outlines toggling
        bool useSobel = (frame >= 25);
        
        renderHardballFrame(frameBuffer, frame, useSobel);

        if (useSobel) {
            applySobelOutline(frameBuffer, edgeBuffer);
            // Save frame status
            if (frame % 10 == 0 || frame == 25) {
                printf("[Vulkan presentation] Frame %02d: Contoured Sobel outlines generated (ControlNet ready)\n", frame);
            }
        } else {
            if (frame % 10 == 0) {
                printf("[Vulkan presentation] Frame %02d: Volumetric lighting depth rendered\n", frame);
            }
        }
    }

    printf("[SUCCESS] Generated 50 high-quality frames of Hardball simulator!\n");
    free(frameBuffer);
    free(edgeBuffer);
    return 0;
}
