#include "tsfi_vision_internal.h"

void tsfi_vision_save_checkpoint(const char *name, const TSFiResonanceAnalysis *a, const uint32_t *pixels, int w, int h) {
    char path[256]; snprintf(path, 255, "assets/flowers/%s.checkpoint", name);
    FILE *f = fopen(path, "wb"); if (!f) return;
    fwrite(a, sizeof(TSFiResonanceAnalysis), 1, f);
    fwrite(&w, sizeof(int), 1, f); fwrite(&h, sizeof(int), 1, f);
    if (pixels) fwrite(pixels, (size_t)w * (size_t)h * sizeof(uint32_t), 1, f);
    fclose(f);
}

void tsfi_vision_save_glyph_checkpoint(const char *name, const TSFiResonanceAnalysis *a, const uint32_t *pixels, int w, int h) {
    char path[256]; snprintf(path, 255, "assets/universal_font/%s.checkpoint", name);
    FILE *f = fopen(path, "wb"); if (!f) return;
    fwrite(a, sizeof(TSFiResonanceAnalysis), 1, f);
    fwrite(&w, sizeof(int), 1, f); fwrite(&h, sizeof(int), 1, f);
    if (pixels) fwrite(pixels, (size_t)w * (size_t)h * sizeof(uint32_t), 1, f);
    fclose(f);
}

bool tsfi_vision_load_checkpoint(const char *name, TSFiResonanceAnalysis *a, uint32_t **pixels, int *w, int *h) {
    char path[256]; snprintf(path, 255, "assets/flowers/%s.checkpoint", name);
    FILE *f = fopen(path, "rb"); 
    if (!f) return false;

    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Resolution: 800x600 pixels * 4 bytes = 1,920,000 bytes
    // Current analysis struct = 184 bytes -> 1,920,192
    // Legacy analysis struct = 172 bytes -> 1,920,180
    // Legacy analysis struct = 152 bytes -> 1,920,160

    if (size >= 1920192) {
        if (fread(a, sizeof(TSFiResonanceAnalysis), 1, f) != 1) { fclose(f); return false; }
    } else if (size >= 1920180) {
        // Legacy 172-byte struct
        memset(a, 0, sizeof(TSFiResonanceAnalysis));
        if (fread(a, 172, 1, f) != 1) { fclose(f); return false; }
    } else if (size >= 1920160) {
        // Legacy 152-byte struct
        memset(a, 0, sizeof(TSFiResonanceAnalysis));
        if (fread(a, 152, 1, f) != 1) { fclose(f); return false; }
    } else {
        // Unknown format
        fclose(f); return false;
    }

    if (fread(w, sizeof(int), 1, f) != 1) { fclose(f); return false; }
    if (fread(h, sizeof(int), 1, f) != 1) { fclose(f); return false; }

    if (pixels) {
        *pixels = (uint32_t*)lau_malloc((size_t)(*w) * (size_t)(*h) * sizeof(uint32_t));
        if (!*pixels || fread(*pixels, (size_t)(*w) * (size_t)(*h) * sizeof(uint32_t), 1, f) != 1) { 
            fclose(f); return false; 
        }
    }
    fclose(f); return true;
}

bool tsfi_vision_load_glyph_checkpoint(const char *name, TSFiResonanceAnalysis *a, uint32_t **pixels, int *w, int *h) {
    char path[256]; snprintf(path, 255, "assets/universal_font/%s.checkpoint", name);
    FILE *f = fopen(path, "rb"); if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size >= 1920192) {
        if (fread(a, sizeof(TSFiResonanceAnalysis), 1, f) != 1) { fclose(f); return false; }
    } else if (size >= 1920180) {
        memset(a, 0, sizeof(TSFiResonanceAnalysis));
        if (fread(a, 172, 1, f) != 1) { fclose(f); return false; }
    } else if (size >= 1920160) {
        memset(a, 0, sizeof(TSFiResonanceAnalysis));
        if (fread(a, 152, 1, f) != 1) { fclose(f); return false; }
    } else {
        fclose(f); return false;
    }

    if (fread(w, sizeof(int), 1, f) != 1) { fclose(f); return false; }
    if (fread(h, sizeof(int), 1, f) != 1) { fclose(f); return false; }

    if (pixels) {
        *pixels = (uint32_t*)lau_malloc((size_t)(*w) * (size_t)(*h) * sizeof(uint32_t));
        if (!*pixels || fread(*pixels, (size_t)(*w) * (size_t)(*h) * sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }
    }
    fclose(f); return true;
}
bool tsfi_vision_compare_raw(const uint8_t *a, const uint8_t *b, size_t s) { return memcmp(a,b,s)==0; }
uint8_t tsfi_vision_max_value_raw(const uint8_t *d, size_t s) { uint8_t m=0; for(size_t i=0;i<s;i++) if(d[i]>m)m=d[i]; return m; }
uint8_t tsfi_vision_probe_pixel_raw(const uint8_t *d, int x, int y, int w, int c) { return d[(y*w+x)*c]; }
TSFiImage* tsfi_vision_load_ppm(const char *f) { (void)f; return NULL; }
void tsfi_vision_free_image(TSFiImage *img) { (void)img; }
