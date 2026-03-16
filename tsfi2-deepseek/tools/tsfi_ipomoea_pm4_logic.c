#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define PM4_PKT3(opcode, count) ((3U << 30) | (((count) & 0x3FFF) << 16) | (((opcode) & 0xFF) << 8))
#define PKT3_DRAW_RECT 0x2A
#define PKT3_DRAW_OVAL 0x2B

#define W 512
#define H 512

void execute_pm4_stream(uint32_t *pm4_stream, uint32_t packet_count, uint32_t *pixels, int w, int h) {
    uint32_t pc = 0;
    while(pc < packet_count) {
        uint32_t header = pm4_stream[pc++];
        uint32_t opcode = (header >> 8) & 0xFF;
        uint32_t count = (header >> 16) & 0x3FFF;
        
        if ((opcode == PKT3_DRAW_RECT || opcode == PKT3_DRAW_OVAL) && pc + count <= packet_count) {
            uint32_t x = pm4_stream[pc++];
            uint32_t y = pm4_stream[pc++];
            uint32_t rect_w = pm4_stream[pc++];
            uint32_t rect_h = pm4_stream[pc++];
            uint32_t color = pm4_stream[pc++];
            
            if (opcode == PKT3_DRAW_RECT) {
                for (uint32_t ry = y; ry < y + rect_h && ry < (uint32_t)h; ry++) {
                    for (uint32_t rx = x; rx < x + rect_w && rx < (uint32_t)w; rx++) {
                        pixels[ry * w + rx] = color;
                    }
                }
            } else {
                float cx = x + rect_w / 2.0f;
                float cy = y + rect_h / 2.0f;
                float rw = rect_w / 2.0f;
                float rh = rect_h / 2.0f;
                if (rw <= 0 || rh <= 0) continue;
                for (uint32_t ry = y; ry < y + rect_h && ry < (uint32_t)h; ry++) {
                    for (uint32_t rx = x; rx < x + rect_w && rx < (uint32_t)w; rx++) {
                        float dx = (rx - cx) / rw;
                        float dy = (ry - cy) / rh;
                        if (dx*dx + dy*dy <= 1.0f) {
                            pixels[ry * w + rx] = color;
                        }
                    }
                }
            }
        } else {
            pc += count;
        }
    }
}

uint32_t synthesize_ipomoea_pm4(uint32_t *pm4, float t) {
    uint32_t pc = 0;
    float cx = W / 2.0f;
    float cy = H / 2.0f;
    
    // Growth parameters
    float bloom_t = (t < 0.2f) ? 0.0f : ((t > 0.6f) ? 1.0f : (t - 0.2f) / 0.4f);
    float wilt_t = (t < 0.7f) ? 0.0f : (t - 0.7f) / 0.3f;
    
    float scale = (0.1f + 0.9f * sinf(bloom_t * 1.5708f)) * (1.0f - wilt_t * 0.5f);
    float drop = wilt_t * 50.0f;
    
    // Draw Stem
    pm4[pc++] = PM4_PKT3(PKT3_DRAW_RECT, 5);
    pm4[pc++] = cx - 4; pm4[pc++] = cy + drop; pm4[pc++] = 8; pm4[pc++] = H - (cy + drop);
    pm4[pc++] = 0xFF225522;
    
    if (bloom_t > 0.0f) {
        for (int i = 0; i < 5; i++) {
            float angle = i * 2.0f * 3.14159f / 5.0f - 3.14159f / 2.0f;
            angle += wilt_t * 1.0f;
            float dist = 80.0f * scale;
            float px = cx + cosf(angle) * dist;
            float py = cy + sinf(angle) * dist + drop;
            float pw = 160.0f * scale * (1.0f - wilt_t * 0.7f);
            float ph = 160.0f * scale * (1.0f - wilt_t * 0.7f);
            
            uint32_t r = 255 - (uint32_t)(wilt_t * 100);
            uint32_t g = 255 - (uint32_t)(wilt_t * 150);
            uint32_t b = 255 - (uint32_t)(wilt_t * 200);
            uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
            
            pm4[pc++] = PM4_PKT3(PKT3_DRAW_OVAL, 5);
            pm4[pc++] = (uint32_t)(px - pw/2);
            pm4[pc++] = (uint32_t)(py - ph/2);
            pm4[pc++] = (uint32_t)pw;
            pm4[pc++] = (uint32_t)ph;
            pm4[pc++] = color;
        }
        
        pm4[pc++] = PM4_PKT3(PKT3_DRAW_OVAL, 5);
        float tw = 60.0f * scale;
        pm4[pc++] = (uint32_t)(cx - tw/2);
        pm4[pc++] = (uint32_t)(cy - tw/2 + drop);
        pm4[pc++] = (uint32_t)tw;
        pm4[pc++] = (uint32_t)tw;
        uint32_t tr = 240 - (uint32_t)(wilt_t * 100);
        uint32_t tg = 255 - (uint32_t)(wilt_t * 150);
        uint32_t tb = 200 - (uint32_t)(wilt_t * 200);
        pm4[pc++] = 0xFF000000 | (tr << 16) | (tg << 8) | tb;
    }
    
    return pc;
}
