#include "tsfi_c_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define W 1024
#define H 1536
#define FPS 60
#define DURATION 4
#define TOTAL_FRAMES (FPS * DURATION)

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Struct matching shader ZMMRegisters block
typedef struct {
    float basePhase;
    float channelFreq;
    float signalAmp;
    float poleSteer;
    float foundationRail;
    float chinRail;
    float rBeta;
    float rCutoff;
    float accNPN;
    float accPNP;
} ZMMRegs;

// Struct matching shader OutputBuffer elements
typedef struct {
    float x, y, z, w;
    float r, g, b, a;
} ShaderVertex;

// Optimized AVX-512 Procedural Noise
static inline __m512 procedural_noise_avx512(__m512 px, __m512 py, float time_offset) {
    __m512 v12_9898 = _mm512_set1_ps(12.9898f);
    __m512 v78_233 = _mm512_set1_ps(78.233f);
    __m512 v43758_5453 = _mm512_set1_ps(43758.5453f);
    __m512 vTime = _mm512_set1_ps(time_offset);
    __m512 dot = _mm512_add_ps(_mm512_add_ps(_mm512_mul_ps(px, v12_9898), _mm512_mul_ps(py, v78_233)), vTime);
    __m512 s = _mm512_mul_ps(dot, v43758_5453);
    __m512 fs = _mm512_sub_ps(s, _mm512_floor_ps(s));
    return fs;
}

// Bresenham's Line Drawing Algorithm to overlay wireframe lines on frame buffer
static void drawLine(uint8_t* pixels, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < W && y0 >= 0 && y0 < H) {
            int idx = (x0 + y0 * W) * 3;
            pixels[idx] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// 19D Lissajous LUT Mapping Function (emulated in C)
static uint32_t getLissajousIndex(uint32_t gID, float phaseOffset, const ZMMRegs* regs) {
    float fx = regs->channelFreq * 0.001f;
    float fy = regs->foundationRail * 0.0001f;
    float fz = regs->chinRail * 0.0001f;

    float t = (float)gID * 0.01f;
    float x = sinf(fx * t + regs->basePhase + phaseOffset);
    float y = sinf(fy * t + regs->poleSteer * 0.01f);
    float z = sinf(fz * t);

    uint32_t index = (uint32_t)(fabsf(x * 1000.0f + y * 10000.0f + z * 100000.0f)) % 65536;
    return index;
}

void render_frame(int frame, uint8_t *pixels) {
    float time_offset = frame * 0.1f;
    float pulse = sinf((frame / (float)TOTAL_FRAMES) * 2.0f * ((float)TSFI_SECRET_CORE));
    float pulse_radius = 0.02f + (pulse * 0.005f);
    float eye2_radius = 0.045f + (pulse * 0.01f);

    __m512 vInvW = _mm512_set1_ps(1.0f / (float)W);
    __m512 vInvH = _mm512_set1_ps(1.0f / (float)H);
    __m512 vHalf = _mm512_set1_ps(0.5f);
    
    // 1. Render AVX-512 Procedural Background Bear
    for (int y = 0; y < H; y++) {
        float py_val = (float)y / H;
        __m512 vPy = _mm512_set1_ps(py_val);
        
        for (int x = 0; x < W; x += 16) {
            __m512 vPx = _mm512_add_ps(_mm512_set1_ps((float)x), 
                         _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0));
            vPx = _mm512_mul_ps(vPx, vInvW);

            __m512 dx1 = _mm512_sub_ps(vPx, vHalf);
            __m512 dy1 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.4f));
            __m512 dist1 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx1, dx1), _mm512_mul_ps(dy1, dy1)));

            __m512 dx2 = _mm512_sub_ps(vPx, vHalf);
            __m512 dy2 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.6f));
            __m512 dist2 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx2, dx2), _mm512_mul_ps(dy2, dy2)));

            __m512 dxb = _mm512_sub_ps(vPx, vHalf);
            __m512 dyb = _mm512_sub_ps(vPy, _mm512_set1_ps(0.75f));
            __m512 distB = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dxb, dxb), _mm512_mul_ps(dyb, dyb)));

            __m512 distH = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx2, dx2), _mm512_mul_ps(dy2, dy2))); // Head maps to Eye 2 center

            __m512 dxe1 = _mm512_sub_ps(vPx, _mm512_set1_ps(0.362f));
            __m512 dye1 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.45f));
            __m512 distE1 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dxe1, dxe1), _mm512_mul_ps(dye1, dye1)));

            __m512 dxe2 = _mm512_sub_ps(vPx, _mm512_set1_ps(0.638f));
            __m512 dye2 = _mm512_sub_ps(vPy, _mm512_set1_ps(0.45f));
            __m512 distE2 = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dxe2, dxe2), _mm512_mul_ps(dye2, dye2)));

            __mmask16 mEye1 = _mm512_cmp_ps_mask(dist1, _mm512_set1_ps(pulse_radius), _CMP_LT_OS);
            __mmask16 mEye2 = _mm512_cmp_ps_mask(dist2, _mm512_set1_ps(eye2_radius), _CMP_LT_OS);
            __mmask16 mEars = _mm512_or_mask16(_mm512_cmp_ps_mask(distE1, _mm512_set1_ps(0.075f), _CMP_LT_OS),
                                               _mm512_cmp_ps_mask(distE2, _mm512_set1_ps(0.075f), _CMP_LT_OS));
            __mmask16 mHead = _mm512_cmp_ps_mask(distH, _mm512_set1_ps(0.2f), _CMP_LT_OS);
            __mmask16 mBody = _mm512_cmp_ps_mask(distB, _mm512_set1_ps(0.25f), _CMP_LT_OS);

            __m512 vNoise = procedural_noise_avx512(_mm512_mul_ps(vPx, _mm512_set1_ps(100.0f)), 
                                                     _mm512_mul_ps(vPy, _mm512_set1_ps(100.0f)), 
                                                     time_offset);
            __mmask16 mSick = _mm512_cmp_ps_mask(vNoise, _mm512_set1_ps(0.8f), _CMP_GT_OS);

            for (int i = 0; i < 16; i++) {
                int px_idx = (y * W + x + i) * 3;
                uint8_t r = 0, g = 0, b = 0;
                uint16_t bit = 1 << i;

                if (mEye1 & bit || mEye2 & bit) {
                    r = 0; g = 255; b = 0;
                } else if (mEars & bit || mHead & bit) {
                    r = 255; g = 0; b = 0;
                } else if (mBody & bit) {
                    if (mSick & bit) {
                        r = 84; g = 107; b = 45;
                    } else {
                        r = 91; g = 63; b = 51;
                    }
                }
                pixels[px_idx] = r;
                pixels[px_idx+1] = g;
                pixels[px_idx+2] = b;
            }
        }
    }

    // 2. Perform 19D weight traversal calculations inside the CPU render pass to overlay composite wireframe lines
    ZMMRegs regs = {
        .basePhase = time_offset,
        .channelFreq = 300.0f,
        .signalAmp = 1.0f,
        .poleSteer = 0.0f,
        .foundationRail = 1200.0f,
        .chinRail = 600.0f,
        .rBeta = 10.0f + sinf(time_offset) * 5.0f,
        .rCutoff = 20.0f + cosf(time_offset) * 10.0f,
        .accNPN = 50.0f + 25.0f * sinf(time_offset * 0.2f),
        .accPNP = 50.0f + 25.0f * cosf(time_offset * 0.2f)
    };

    uint32_t vertexCount = 512;
    ShaderVertex* vertices = malloc(sizeof(ShaderVertex) * vertexCount);

    float betaGain = 100.0f / fmaxf(1.0f, regs.rBeta);
    float cutoffThreshold = (regs.rCutoff / 200.0f) * 0.3f;

    for (uint32_t gID = 0; gID < vertexCount; gID++) {
        float activeAccumulator = (gID % 2 == 0) ? regs.accNPN : regs.accPNP;
        float phaseOffset = activeAccumulator * 0.0628f;

        // Generate coherent simulated weights (mapping the DeepSeek + SD inputs)
        float dsWeight = sinf((float)gID * 0.01f) * cosf((float)gID * 0.003f) * 0.5f;
        float sdWeight = cosf((float)gID * 0.02f) * sinf((float)gID * 0.005f) * 0.4f;

        float dsBiasedVal = dsWeight * regs.signalAmp + (activeAccumulator * 0.002f * ((gID % 2 == 0) ? 1.0f : -1.0f));
        float geometryAmplitude = 0.0f;
        if (fabsf(dsBiasedVal) > cutoffThreshold) {
            geometryAmplitude = fmaxf(-2.0f, fminf(2.0f, dsBiasedVal * betaGain));
        }

        float sdBiasedVal = sdWeight * regs.signalAmp + (activeAccumulator * 0.001f * ((gID % 2 == 0) ? 1.0f : -1.0f));
        float colorModulation = 0.5f;
        if (fabsf(sdBiasedVal) > cutoffThreshold) {
            colorModulation = fmaxf(0.0f, fminf(1.0f, 0.5f + sdBiasedVal * betaGain * 0.25f));
        }

        // Segmented coordinate mapping
        float x = 0.0f, y = 0.0f, z = 0.0f;
        float angle = (float)gID * 0.15f + regs.basePhase;

        if (gID < 200) {
            // Body
            float r = 2.5f + geometryAmplitude * 0.5f;
            x = r * cosf(angle) * sinf(angle * 0.5f);
            y = r * sinf(angle) * sinf(angle * 0.5f) - 1.0f;
        } else if (gID < 350) {
            // Head
            float r = 1.8f + geometryAmplitude * 0.3f;
            x = r * cosf(angle) * sinf(angle * 0.5f);
            y = r * sinf(angle) * sinf(angle * 0.5f) + 1.8f;
        } else if (gID < 420) {
            // Ears
            float r = 0.6f + geometryAmplitude * 0.1f;
            float earSide = (gID % 2 == 0) ? 1.3f : -1.3f;
            x = r * cosf(angle) * sinf(angle * 0.5f) + earSide;
            y = r * sinf(angle) * sinf(angle * 0.5f) + 3.2f;
        } else {
            // Limbs
            float r = 0.7f + geometryAmplitude * 0.2f;
            float side = (gID % 2 == 0) ? 2.0f : -2.0f;
            float limbHeight = (gID % 4 < 2) ? 0.8f : -2.8f;
            x = r * cosf(angle) + side;
            y = r * sinf(angle) + limbHeight;
        }

        vertices[gID].x = x;
        vertices[gID].y = y;
        vertices[gID].z = z;

        if (gID % 2 == 0) {
            vertices[gID].r = 0.06f * colorModulation;
            vertices[gID].g = 0.72f * colorModulation;
            vertices[gID].b = 0.50f;
        } else {
            vertices[gID].r = 0.93f;
            vertices[gID].g = 0.26f * colorModulation;
            vertices[gID].b = 0.26f * colorModulation;
        }
    }

    // Connect vertices to overlay high-aesthetic neon wireframe lines on top of background pixels
    for (uint32_t i = 0; i < vertexCount - 1; i++) {
        // Map 3D coordinate space onto the 1024x1536 output resolution
        int x0 = (int)((vertices[i].x / 10.0f + 0.5f) * (float)W);
        int y0 = (int)((-vertices[i].y / 10.0f + 0.5f) * (float)H); // Invert Y for screen space
        int x1 = (int)((vertices[i+1].x / 10.0f + 0.5f) * (float)W);
        int y1 = (int)((-vertices[i+1].y / 10.0f + 0.5f) * (float)H);

        if (abs(x0 - x1) < 150 && abs(y0 - y1) < 150) {
            uint8_t r = (uint8_t)(vertices[i].r * 255.0f);
            uint8_t g = (uint8_t)(vertices[i].g * 255.0f);
            uint8_t b = (uint8_t)(vertices[i].b * 255.0f);
            drawLine(pixels, x0, y0, x1, y1, r, g, b);
        }
    }

    free(vertices);
}

int main() {
    printf("[TSFI] Ultra-Fast C Renderer: Initiating FFmpeg Pipe...\n");
    
    char ffmpeg_cmd[512];
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd), 
             "ffmpeg -y -f rawvideo -vcodec rawvideo -s %dx%d -pix_fmt rgb24 -r %d -i - "
             "-c:v libx264 -preset fast -pix_fmt yuv420p assets/atropa_glsl_phase3_fast.mp4",
             W, H, FPS);
    
    FILE *pipe = popen(ffmpeg_cmd, "w");
    if (!pipe) {
        perror("popen");
        return 1;
    }

    uint8_t *pixels = (uint8_t*)malloc(W * H * 3);
    
    for (int f = 0; f < TOTAL_FRAMES; f++) {
        memset(pixels, 0, W * H * 3);
        render_frame(f, pixels);
        fwrite(pixels, 1, W * H * 3, pipe);
        if (f % 20 == 0) printf("  -> Frame %d/%d rendered.\n", f, TOTAL_FRAMES);
    }

    free(pixels);
    pclose(pipe);
    printf("[SUCCESS] Render Complete: assets/atropa_glsl_phase3_fast.mp4\n");
    return 0;
}
