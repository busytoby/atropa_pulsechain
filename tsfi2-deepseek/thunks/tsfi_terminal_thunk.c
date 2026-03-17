#include <stdint.h>
#include <math.h>

// Scalar Constants for normalization (Match TSFI_FONT_COORD_SCALE)
const float COORD_SCALE = 16384.0f;

struct VRAMCell {
    uint32_t data; // [Attr:8][BG:8][FG:8][Char:8]
};

struct GlyphRegistry {
    uint32_t start_index;
    uint32_t num_segments;
    int advance;
    int min_x; int min_y;
    int max_x; int max_y;
};

struct TSFiPackedSegment {
    int16_t p0_x, p0_y;
    int16_t p1_x, p1_y;
    int16_t p2_x, p2_y;
    uint16_t flags;
    uint16_t padding;
};


static float tsfi_sqrtf(float x) {
    if (x <= 0.0f) return 0.0f;
    float result = x;
    float xhalf = 0.5f * x;
    int i = *(int*)&result;
    i = 0x5f3759df - (i >> 1); // Fast inverse square root
    result = *(float*)&i;
    result = result * (1.5f - xhalf * result * result); // Newton step
    result = result * (1.5f - xhalf * result * result); // Second step for precision
    return x * result; // sqrt(x) = x * rsqrt(x)
}

static float sdLine(float px, float py, float ax, float ay, float bx, float by) {
    float pa_x = px - ax, pa_y = py - ay;
    float ba_x = bx - ax, ba_y = by - ay;
    float h = (pa_x * ba_x + pa_y * ba_y) / (ba_x * ba_x + ba_y * ba_y);
    if (h < 0.0f) h = 0.0f;
    if (h > 1.0f) h = 1.0f;
    float dx = pa_x - ba_x * h;
    float dy = pa_y - ba_y * h;
    return tsfi_sqrtf(dx * dx + dy * dy);
}

static float sdBezier(float px, float py, float ax, float ay, float bx, float by, float cx, float cy) {
    float d1 = sdLine(px, py, ax, ay, bx, by);
    float d2 = sdLine(px, py, bx, by, cx, cy);
    return (d1 < d2) ? d1 : d2;
}

static float smoothstep(float edge0, float edge1, float x) {
    float t = (x - edge0) / (edge1 - edge0);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

void tsfi_terminal_render_thunk(uint32_t *pixels, uint32_t screen_w, uint32_t screen_h, uint32_t char_w, uint32_t char_h, const struct VRAMCell *vram, const uint32_t *unicode_map, const struct GlyphRegistry *registry, const struct TSFiPackedSegment *dna) {
    for (uint32_t py = 0; py < screen_h; py++) {
        for (uint32_t px = 0; px < screen_w; px++) {
            uint32_t tx = px / char_w;
            uint32_t ty = py / char_h;
            if (tx >= 160 || ty >= 60) continue;

            uint32_t cell_idx = ty * 160 + tx;
            struct VRAMCell cell = vram[cell_idx];
            uint32_t ch = cell.data & 0xFF;

            float g_uv_x = ((float)(px % char_w) / (float)char_w) * 2.0f - 1.0f;
            float g_uv_y = (1.0f - ((float)(py % char_h) / (float)char_h)) * 2.0f - 1.0f;

            float d = 1e10f;
            uint32_t rid = unicode_map[ch];
            if (rid > 0) {
                struct GlyphRegistry reg = registry[rid];
                for (uint32_t i = 0; i < reg.num_segments; i++) {
                    struct TSFiPackedSegment seg = dna[reg.start_index + i];
                    float p0_x = (float)seg.p0_x / COORD_SCALE;
                    float p0_y = (float)seg.p0_y / COORD_SCALE;
                    float p1_x = (float)seg.p1_x / COORD_SCALE;
                    float p1_y = (float)seg.p1_y / COORD_SCALE;
                    float p2_x = (float)seg.p2_x / COORD_SCALE;
                    float p2_y = (float)seg.p2_y / COORD_SCALE;

                    float dist = 0.0f;
                    if ((seg.flags & 2) != 0) {
                        dist = sdLine(g_uv_x, g_uv_y, p0_x, p0_y, p2_x, p2_y);
                    } else {
                        dist = sdBezier(g_uv_x, g_uv_y, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y);
                    }
                    if (dist < d) d = dist;
                }
            }

            float thickness = 0.05f;
            float softness = 0.02f;
            float alpha = smoothstep(thickness + softness, thickness - softness, d);

            uint8_t fg_r = 0, fg_g = 255, fg_b = 0; // Default Terminal Green
            uint8_t bg_r = 0, bg_g = 0, bg_b = 0;   // Default Black

            uint8_t r = (uint8_t)(bg_r * (1.0f - alpha) + fg_r * alpha);
            uint8_t g = (uint8_t)(bg_g * (1.0f - alpha) + fg_g * alpha);
            uint8_t b = (uint8_t)(bg_b * (1.0f - alpha) + fg_b * alpha);

            pixels[py * screen_w + px] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
        }
    }
}