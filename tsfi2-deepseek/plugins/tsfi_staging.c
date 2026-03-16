#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>
#include <unistd.h>
#include <sys/mman.h>
#include "lau_memory.h"
#include "tsfi_staging.h"

// --- Allocation ---
StagingBuffer* create_staging_buffer(size_t width, size_t height) {
    size_t stride = width * 4; // RGBA
    size_t size = stride * height;
    
    size_t aligned_size = (size + 4095) & ~4095;
    void *ptr = lau_memalign_wired_loc(4096, aligned_size, __FILE__, __LINE__);
    
    if (!ptr) return NULL;
    
    StagingBuffer *sb = lau_malloc(sizeof(StagingBuffer));
    sb->magic = TSFI_STAGING_MAGIC;
    sb->data = ptr;
    sb->size = aligned_size;
    sb->width = width;
    sb->height = height;
    sb->stride = stride;
    
    memset(ptr, 0, aligned_size);
    return sb;
}

void destroy_staging_buffer(StagingBuffer *sb) {
    if (!sb) return;
    
    // Check magic before any access
    if (sb->magic != TSFI_STAGING_MAGIC) {
        printf("[STAGING] ERROR: Invalid magic 0x%08x in destroy_staging_buffer. Skipping to avoid crash.\n", sb->magic);
        return;
    }

    if (sb->data) {
        lau_free(sb->data);
        sb->data = NULL;
    }
    sb->magic = 0; // Invalidate
    lau_free(sb);
}

// --- AVX-512 Operations ---

void staging_clear_avx512(StagingBuffer *sb, uint32_t color) {
    if (!sb || sb->magic != TSFI_STAGING_MAGIC) return;
    __m512i v_col = _mm512_set1_epi32(color);
    uint32_t *px = (uint32_t *)sb->data;
    size_t count = sb->width * sb->height;
    
    size_t i = 0;
    for (; i <= count - 16; i += 16) {
        _mm512_store_si512((void*)&px[i], v_col);
    }
    
    if (i < count) {
        __mmask16 mask = (1 << (count - i)) - 1;
        _mm512_mask_storeu_epi32(&px[i], mask, v_col);
    }
}

void staging_downsample_2x_avx512(StagingBuffer *dst, void *src, size_t src_w, size_t src_h) {
    if (!dst || dst->magic != TSFI_STAGING_MAGIC) return;
    uint32_t *s = (uint32_t*)src;
    uint32_t *d = (uint32_t*)dst->data;
    __mmask16 mask = 0x5555; 
    
    for (size_t y = 0; y < src_h; y += 2) {
        size_t d_row_offset = (y / 2) * (src_w / 2);
        size_t s_row_offset = y * src_w;
        
        for (size_t x = 0; x < src_w; x += 32) {
            __m512i v0 = _mm512_loadu_si512(&s[s_row_offset + x]);
            __m512i v1 = _mm512_loadu_si512(&s[s_row_offset + x + 16]);
            _mm512_mask_compressstoreu_epi32(&d[d_row_offset + (x/2)], mask, v0);
            _mm512_mask_compressstoreu_epi32(&d[d_row_offset + (x/2) + 8], mask, v1);
        }
    }
}

void staging_write_direct_avx512(void *mapped_ptr, size_t row_pitch, size_t height, void *src, size_t width) {
    uint8_t *d_base = (uint8_t*)mapped_ptr;
    uint8_t *s_base = (uint8_t*)src;
    size_t line_bytes = width * 4;
    if (line_bytes > row_pitch) line_bytes = row_pitch;
    size_t vectors_per_line = line_bytes / 64;
    
    for (size_t y = 0; y < height; y++) {
        uint8_t *d_row = d_base + (y * row_pitch);
        uint8_t *s_row = s_base + (y * line_bytes); 
        size_t i = 0;
        for (; i + 1 < vectors_per_line; i += 2) {
            __m512i v0 = _mm512_loadu_si512((void*)(s_row + i * 64));
            __m512i v1 = _mm512_loadu_si512((void*)(s_row + (i + 1) * 64));
            _mm512_storeu_si512((void*)(d_row + i * 64), v0);
            _mm512_storeu_si512((void*)(d_row + (i + 1) * 64), v1);
        }
        for (; i < vectors_per_line; i++) {
            __m512i v = _mm512_loadu_si512((void*)(s_row + i * 64));
            _mm512_storeu_si512((void*)(d_row + i * 64), v);
        }
    }
}

void staging_copy_avx512(StagingBuffer *dst, void *src, size_t count) {
    if (!dst || dst->magic != TSFI_STAGING_MAGIC) return;
    __m512i *d = (__m512i*)dst->data;
    __m512i *s = (__m512i*)src;
    size_t num_vectors = count / 16;
    for (size_t i = 0; i < num_vectors; i++) {
        _mm_prefetch((const char*)&s[i + 2], _MM_HINT_T0);
        __m512i v = _mm512_loadu_si512(&s[i]);
        _mm512_store_si512(&d[i], v); 
    }
}

void staging_blend_over_avx512(StagingBuffer *dst, int x, int y, int w, int h, uint32_t color) {
    if (!dst || dst->magic != TSFI_STAGING_MAGIC) return;
    uint32_t *px = (uint32_t *)dst->data;
    __m512i v_col = _mm512_set1_epi32(color);
    
    for (int j = 0; j < h; j++) {
        int dst_y = y + j;
        if (dst_y < 0 || dst_y >= (int)dst->height) continue;
        int row_offset = dst_y * dst->width + x;
        int i = 0;
        for (; i <= w - 16; i += 16) {
            _mm512_storeu_si512(&px[row_offset + i], v_col);
        }
        if (i < w) {
            __mmask16 mask = (1 << (w - i)) - 1;
            _mm512_mask_storeu_epi32(&px[row_offset + i], mask, v_col);
        }
    }
}

void draw_rounded_rect(StagingBuffer *sb, int x, int y, int w, int h, int r, uint32_t color) {
    if (!sb || sb->magic != TSFI_STAGING_MAGIC) return;
    uint32_t *px = (uint32_t *)sb->data;
    int x1 = x, y1 = y;
    int x2 = x + w, y2 = y + h;

    for (int j = y1; j < y2; j++) {
        if (j < 0 || j >= (int)sb->height) continue;
        for (int i = x1; i < x2; i++) {
            if (i < 0 || i >= (int)sb->width) continue;
            bool inside = true;
            if (i < x1 + r && j < y1 + r) { 
                if ((i - (x1 + r)) * (i - (x1 + r)) + (j - (y1 + r)) * (j - (y1 + r)) > r * r) inside = false;
            } else if (i > x2 - r - 1 && j < y1 + r) { 
                if ((i - (x2 - r - 1)) * (i - (x2 - r - 1)) + (j - (y1 + r)) * (j - (y1 + r)) > r * r) inside = false;
            } else if (i < x1 + r && j > y2 - r - 1) { 
                if ((i - (x1 + r)) * (i - (x1 + r)) + (j - (y2 - r - 1)) * (j - (y2 - r - 1)) > r * r) inside = false;
            } else if (i > x2 - r - 1 && j > y2 - r - 1) { 
                if ((i - (x2 - r - 1)) * (i - (x2 - r - 1)) + (j - (y2 - r - 1)) * (j - (y2 - r - 1)) > r * r) inside = false;
            }
            if (inside) px[j * sb->width + i] = color;
        }
    }
}

void save_staging_buffer_to_ppm(StagingBuffer *sb, const char *filename) {
    if (!sb || sb->magic != TSFI_STAGING_MAGIC || !sb->data) return;
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    fprintf(f, "P6\n%zu %zu\n255\n", sb->width, sb->height);
    uint32_t *px = (uint32_t *)sb->data;
    for (size_t i = 0; i < sb->width * sb->height; i++) {
        uint32_t c = px[i];
        uint8_t rgb[3] = { (uint8_t)((c >> 16) & 0xFF), (uint8_t)((c >> 8) & 0xFF), (uint8_t)(c & 0xFF) };
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

#include <dirent.h>
#include "tsfi_c_math.h"

#define STBTT_sqrt tsfi_sqrt
#define STBTT_pow tsfi_pow
#define STBTT_acos tsfi_acos
#define STBTT_cos tsfi_cosf
#define STBTT_fmod tsfi_fmod
#define STBTT_ifloor(x) ((int)tsfi_floor(x))
#define STBTT_iceil(x) ((int)tsfi_ceil(x))
#define STBTT_fabs tsfi_fabs

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static unsigned char *g_font_buffer = NULL;
static stbtt_fontinfo g_font_info;
static bool g_font_loaded = false;

static void ensure_font_loaded() {
    if (g_font_loaded) return;
    
    const char *font_path = "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf";
    FILE *f = fopen(font_path, "rb");
    if (!f) {
        printf("[STAGING] Warning: System font not found at %s\n", font_path);
        return;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    g_font_buffer = lau_malloc(size);
    if (fread(g_font_buffer, 1, size, f) != (size_t)size) {
        printf("[STAGING] Error: fread failed.\n");
        lau_free(g_font_buffer);
        g_font_buffer = NULL;
        fclose(f);
        return;
    }
    fclose(f);

    if (!stbtt_InitFont(&g_font_info, g_font_buffer, 0)) {
        printf("[STAGING] Error: stbtt_InitFont failed.\n");
        return;
    }
    g_font_loaded = true;
    printf("[STAGING] System font loaded: %s\n", font_path);
}

// --- Font Fallback (8x8 IBM-style) ---
static const uint8_t g_font8x8[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 (SPACE)
    {0x18,0x3c,0x3c,0x18,0x18,0x00,0x18,0x00}, // 33 (!)
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, // 34 (")
    {0x36,0x36,0x7f,0x36,0x7f,0x36,0x36,0x00}, // 35 (#)
    {0x18,0x3e,0x60,0x3c,0x06,0x7c,0x18,0x00}, // 36 ($)
    {0x00,0x63,0x66,0x0c,0x18,0x33,0x63,0x00}, // 37 (%)
    {0x1c,0x36,0x1c,0x3b,0x6e,0x66,0x3b,0x00}, // 38 (&)
    {0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00}, // 39 (')
    {0x0c,0x18,0x30,0x30,0x30,0x18,0x0c,0x00}, // 40 (()
    {0x30,0x18,0x0c,0x0c,0x0c,0x18,0x30,0x00}, // 41 ())
    {0x00,0x66,0x3c,0xff,0x3c,0x66,0x00,0x00}, // 42 (*)
    {0x00,0x18,0x18,0x7e,0x18,0x18,0x00,0x00}, // 43 (+)
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // 44 (,)
    {0x00,0x00,0x00,0x7e,0x00,0x00,0x00,0x00}, // 45 (-)
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // 46 (.)
    {0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x00}, // 47 (/)
    {0x3c,0x66,0x6e,0x7e,0x76,0x66,0x3c,0x00}, // 48 (0)
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7e,0x00}, // 49 (1)
    {0x3c,0x66,0x06,0x0c,0x18,0x30,0x7e,0x00}, // 50 (2)
    {0x3c,0x66,0x06,0x1c,0x06,0x66,0x3c,0x00}, // 51 (3)
    {0x1c,0x3c,0x6c,0xcc,0xfe,0x0c,0x1e,0x00}, // 52 (4)
    {0x7e,0x60,0x7c,0x06,0x06,0x66,0x3c,0x00}, // 53 (5)
    {0x1c,0x30,0x60,0x7c,0x66,0x66,0x3c,0x00}, // 54 (6)
    {0x7e,0x66,0x06,0x0c,0x18,0x18,0x18,0x00}, // 55 (7)
    {0x3c,0x66,0x66,0x3c,0x66,0x66,0x3c,0x00}, // 56 (8)
    {0x3c,0x66,0x66,0x3e,0x06,0x0c,0x38,0x00}, // 57 (9)
    {0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00}, // 58 (:)
    {0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00}, // 59 (;)
    {0x0c,0x18,0x30,0x60,0x30,0x18,0x0c,0x00}, // 60 (<)
    {0x00,0x00,0x7e,0x00,0x7e,0x00,0x00,0x00}, // 61 (=)
    {0x30,0x18,0x0c,0x06,0x0c,0x18,0x30,0x00}, // 62 (>)
    {0x3c,0x66,0x06,0x0c,0x18,0x00,0x18,0x00}, // 63 (?)
    {0x3c,0x66,0x6e,0x6e,0x60,0x62,0x3c,0x00}, // 64 (@)
    {0x18,0x3c,0x66,0x66,0x7e,0x66,0x66,0x00}, // 65 (A)
    {0x7c,0x66,0x66,0x7c,0x66,0x66,0x7c,0x00}, // 66 (B)
    {0x3c,0x66,0x60,0x60,0x60,0x66,0x3c,0x00}, // 67 (C)
    {0x78,0x6c,0x66,0x66,0x66,0x6c,0x78,0x00}, // 68 (D)
    {0x7e,0x60,0x60,0x78,0x60,0x60,0x7e,0x00}, // 69 (E)
    {0x7e,0x60,0x60,0x78,0x60,0x60,0x60,0x00}, // 70 (F)
    {0x3c,0x66,0x60,0x6e,0x66,0x66,0x3c,0x00}, // 71 (G)
    {0x66,0x66,0x66,0x7e,0x66,0x66,0x66,0x00}, // 72 (H)
    {0x3c,0x18,0x18,0x18,0x18,0x18,0x3c,0x00}, // 73 (I)
    {0x1e,0x06,0x06,0x06,0x06,0x66,0x3c,0x00}, // 74 (J)
    {0x66,0x6c,0x78,0x70,0x78,0x6c,0x66,0x00}, // 75 (K)
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7e,0x00}, // 76 (L)
    {0x63,0x77,0x7f,0x6b,0x63,0x63,0x63,0x00}, // 77 (M)
    {0x66,0x76,0x7e,0x7e,0x6e,0x66,0x66,0x00}, // 78 (N)
    {0x3c,0x66,0x66,0x66,0x66,0x66,0x3c,0x00}, // 79 (O)
    {0x7c,0x66,0x66,0x7c,0x60,0x60,0x60,0x00}, // 80 (P)
    {0x3c,0x66,0x66,0x66,0x6e,0x6c,0x36,0x00}, // 81 (Q)
    {0x7c,0x66,0x66,0x7c,0x6c,0x66,0x66,0x00}, // 82 (R)
    {0x3c,0x66,0x30,0x18,0x0c,0x66,0x3c,0x00}, // 83 (S)
    {0x7e,0x5a,0x18,0x18,0x18,0x18,0x18,0x00}, // 84 (T)
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3c,0x00}, // 85 (U)
    {0x66,0x66,0x66,0x66,0x66,0x3c,0x18,0x00}, // 86 (V)
    {0x63,0x63,0x63,0x6b,0x7f,0x77,0x63,0x00}, // 87 (W)
    {0x66,0x66,0x3c,0x18,0x3c,0x66,0x66,0x00}, // 88 (X)
    {0x66,0x66,0x66,0x3c,0x18,0x18,0x18,0x00}, // 89 (Y)
    {0x7e,0x06,0x0c,0x18,0x30,0x60,0x7e,0x00}, // 90 (Z)
    {0x3c,0x30,0x30,0x30,0x30,0x30,0x3c,0x00}, // 91 ([)
    {0xc0,0x60,0x30,0x18,0x0c,0x06,0x03,0x00}, // 92 (\)
    {0x3c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3c,0x00}, // 93 (])
    {0x18,0x3c,0x66,0x00,0x00,0x00,0x00,0x00}, // 94 (^)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00}, // 95 (_)
    {0x30,0x18,0x0c,0x00,0x00,0x00,0x00,0x00}, // 96 (`)
    {0x00,0x00,0x3c,0x06,0x3e,0x66,0x3e,0x00}, // 97 (a)
    {0x60,0x60,0x7c,0x66,0x66,0x66,0x7c,0x00}, // 98 (b)
    {0x00,0x00,0x3c,0x60,0x60,0x66,0x3c,0x00}, // 99 (c)
    {0x06,0x06,0x3e,0x66,0x66,0x66,0x3e,0x00}, // 100 (d)
    {0x00,0x00,0x3c,0x66,0x7e,0x60,0x3c,0x00}, // 101 (e)
    {0x1c,0x30,0x78,0x30,0x30,0x30,0x30,0x00}, // 102 (f)
    {0x00,0x00,0x3e,0x66,0x66,0x3e,0x06,0x3c}, // 103 (g)
    {0x60,0x60,0x7c,0x66,0x66,0x66,0x66,0x00}, // 104 (h)
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3c,0x00}, // 105 (i)
    {0x06,0x00,0x06,0x06,0x06,0x06,0x66,0x3c}, // 106 (j)
    {0x60,0x60,0x66,0x6c,0x78,0x6c,0x66,0x00}, // 107 (k)
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3c,0x00}, // 108 (l)
    {0x00,0x00,0x66,0x7f,0x7f,0x6b,0x63,0x00}, // 109 (m)
    {0x00,0x00,0x7c,0x66,0x66,0x66,0x66,0x00}, // 110 (n)
    {0x00,0x00,0x3c,0x66,0x66,0x66,0x3c,0x00}, // 111 (o)
    {0x00,0x00,0x7c,0x66,0x66,0x7c,0x60,0x60}, // 112 (p)
    {0x00,0x00,0x3e,0x66,0x66,0x3e,0x06,0x06}, // 113 (q)
    {0x00,0x00,0x7c,0x66,0x60,0x60,0x60,0x00}, // 114 (r)
    {0x00,0x00,0x3e,0x60,0x3c,0x06,0x7c,0x00}, // 115 (s)
    {0x30,0x30,0x7c,0x30,0x30,0x30,0x1c,0x00}, // 116 (t)
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3e,0x00}, // 117 (u)
    {0x00,0x00,0x66,0x66,0x66,0x3c,0x18,0x00}, // 118 (v)
    {0x00,0x00,0x63,0x6b,0x7f,0x7f,0x36,0x00}, // 119 (w)
    {0x00,0x00,0x66,0x3c,0x18,0x3c,0x66,0x00}, // 120 (x)
    {0x00,0x00,0x66,0x66,0x66,0x3e,0x06,0x3c}, // 121 (y)
    {0x00,0x00,0x7e,0x0c,0x18,0x30,0x7e,0x00}, // 122 (z)
    {0x0c,0x18,0x18,0x70,0x18,0x18,0x0c,0x00}, // 123 ({)
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 124 (|)
    {0x30,0x18,0x18,0x0e,0x18,0x18,0x30,0x00}, // 125 (})
    {0x00,0x00,0x00,0x36,0x5c,0x00,0x00,0x00}  // 126 (~)
};

// --- Glyph Cache ---
typedef struct {
    uint32_t codepoint;
    int gw, gh, x0, y0, advance;
    unsigned char *bitmap;
} LauGlyph;

static LauGlyph *g_glyph_cache[0x10000] = {0};

void draw_debug_codepoint(StagingBuffer *sb, int x, int y, uint32_t codepoint, uint32_t color) {
    if (!sb || sb->magic != TSFI_STAGING_MAGIC) return;
    ensure_font_loaded();
    
    uint32_t *px = (uint32_t *)sb->data;
    if (g_font_loaded) {
        float scale = stbtt_ScaleForPixelHeight(&g_font_info, 14.0f);
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&g_font_info, &ascent, &descent, &lineGap);
        int baseline = (int)(ascent * scale);

        LauGlyph *lg = NULL;
        if (codepoint < 0x10000) lg = g_glyph_cache[codepoint];

        if (!lg && codepoint < 0x10000) {
            // Populate cache entry
            lg = (LauGlyph*)lau_malloc(sizeof(LauGlyph));
            lg->codepoint = codepoint;
            stbtt_GetCodepointHMetrics(&g_font_info, codepoint, &lg->advance, &lg->gh); // Reusing gh for lsb
            stbtt_GetCodepointBitmapBox(&g_font_info, codepoint, scale, scale, &lg->x0, &lg->y0, &lg->gw, &lg->gh);
            lg->gw = lg->gw - lg->x0;
            lg->gh = lg->gh - lg->y0;
            
            if (lg->gw > 0 && lg->gh > 0) {
                lg->bitmap = lau_malloc(lg->gw * lg->gh);
                stbtt_MakeCodepointBitmap(&g_font_info, lg->bitmap, lg->gw, lg->gh, lg->gw, scale, scale, codepoint);
            } else {
                lg->bitmap = NULL;
            }
            g_glyph_cache[codepoint] = lg;
        }

        if (lg && lg->bitmap) {
            for (int j = 0; j < lg->gh; j++) {
                int py = y + baseline + lg->y0 + j;
                if (py < 0 || py >= (int)sb->height) continue;
                for (int i = 0; i < lg->gw; i++) {
                    int px_idx = x + lg->x0 + i;
                    if (px_idx < 0 || px_idx >= (int)sb->width) continue;
                    float alpha = lg->bitmap[j * lg->gw + i] / 255.0f;
                    if (alpha > 0.1f) {
                        uint32_t src = px[py * sb->width + px_idx];
                        uint8_t r = (uint8_t)(((color >> 16) & 0xFF) * alpha + ((src >> 16) & 0xFF) * (1.0f - alpha));
                        uint8_t g = (uint8_t)(((color >> 8) & 0xFF) * alpha + ((src >> 8) & 0xFF) * (1.0f - alpha));
                        uint8_t b = (uint8_t)((color & 0xFF) * alpha + (src & 0xFF) * (1.0f - alpha));
                        px[py * sb->width + px_idx] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                    }
                }
            }
        }
    } else {
        // Fallback: Use existing 8x8 fallback logic if needed (not implemented for full unicode)
        if (codepoint < 128) {
            unsigned char c = (unsigned char)codepoint;
            if (c < 32 || c > 126) c = '?';
            const uint8_t *glyph = g_font8x8[c - 32];
            for (int j = 0; j < 8; j++) {
                for (int i = 0; i < 8; i++) {
                    if (glyph[j] & (1 << i)) {
                        int px_idx = x + i;
                        int py = y + j;
                        if (px_idx >= 0 && px_idx < (int)sb->width && py >= 0 && py < (int)sb->height)
                            px[py * sb->width + px_idx] = color;
                    }
                }
            }
        }
    }
}

void draw_debug_text(StagingBuffer *sb, int x, int y, const char *text, uint32_t color, bool forward) {
    if (!sb || sb->magic != TSFI_STAGING_MAGIC) return;
    ensure_font_loaded();
    
    uint32_t *px = (uint32_t *)sb->data;
    int cx = x;
    int len = strlen(text);

    if (g_font_loaded) {
        float scale = stbtt_ScaleForPixelHeight(&g_font_info, 14.0f);
        for (int t_idx = 0; t_idx < len; t_idx++) {
            char c = forward ? text[t_idx] : text[len - 1 - t_idx];
            uint32_t codepoint = (uint32_t)(unsigned char)c;
            
            draw_debug_codepoint(sb, cx, y, codepoint, color);
            
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&g_font_info, codepoint, &advance, &lsb);
            cx += (int)(advance * scale);
        }
    } else {
        // Fallback: 8x8 font
        for (int t_idx = 0; t_idx < len; t_idx++) {
            unsigned char c = forward ? (unsigned char)text[t_idx] : (unsigned char)text[len - 1 - t_idx];
            if (c < 32 || c > 126) c = '?';
            const uint8_t *glyph = g_font8x8[c - 32];
            for (int j = 0; j < 8; j++) {
                int py = y + j;
                if (py < 0 || py >= (int)sb->height) continue;
                uint8_t row = glyph[j];
                for (int i = 0; i < 8; i++) {
                    int px_idx = cx + i;
                    if (px_idx < 0 || px_idx >= (int)sb->width) continue;
                    if (row & (0x80 >> i)) px[py * sb->width + px_idx] = color;
                }
            }
            cx += 8;
        }
    }
}

void render_file_modal(StagingBuffer *sb, const char *path) {
    if (!sb || sb->magic != TSFI_STAGING_MAGIC) return;
    DIR *d; struct dirent *dir;
    d = opendir(path);
    int row = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            draw_debug_text(sb, 50, 50 + row * 12, dir->d_name, 0xFFFFFFFF, true);
            row++; if (row > 40) break;
        }
        closedir(d);
    }
}
