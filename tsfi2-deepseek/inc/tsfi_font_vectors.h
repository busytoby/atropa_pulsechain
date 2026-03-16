#ifndef TSFI_FONT_VECTORS_H
#define TSFI_FONT_VECTORS_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_types.h"
#include "tsfi_wave15.h"
#include "tsfi_k0rn_ops.h"

// --- TSFi GPU Vector Format ---
// Designed for RDNA4: 128-bit aligned reads, Compute-Cache Architecture.
// Supports full UTF-32 range (0x0 - 0x10FFFF).

#define TSFI_FONT_MAP_SIZE 0x110000 // 1,114,112 codepoints
#define TSFI_FONT_COORD_SCALE 16384 // Normalization range (-16384 to +16384)

// Buffer A: The Unicode Map
// Maps UTF-32 Codepoint -> Registry Index (GlyphID)
// Size: ~4.5 MB
// Type: uint32_t[] (Defined implicitly as buffer)

// Buffer B: The Glyph Registry (Metadata)
// Aligned to 16 bytes for single vector load.
typedef struct {
    uint32_t start_index;   // Index into Buffer C (Segments)
    uint32_t num_segments;  // Number of 16-byte segments
    int16_t  min_x;         // Bounding Box
    int16_t  min_y;
    int16_t  max_x;
    int16_t  max_y;
    float    advance;       // Horizontal advance
    uint32_t padding;       // Pad to 32 bytes (or use 16? 4+4+2+2+2+2+4+4 = 24. Need 8 padding for 32, or packed better.)
                            // Let's target 32 bytes for clean alignment (2x 128-bit) or 16 if we compress.
                            // 16-byte target:
                            // uint32 start (4)
                            // uint16 num (2) | int16 advance_norm (2)
                            // int16 minx, miny, maxx, maxy (8)
                            // Total: 16 bytes. PERFECT.
} __attribute__((packed)) TSFiGlyphRegistry;

// Optimized 16-byte Registry Struct
typedef struct {
    uint32_t start_index;      // 0-3
    uint16_t num_segments;     // 4-5
    int16_t  advance;          // 6-7 (Normalized)
    int16_t  min_x;            // 8-9
    int16_t  min_y;            // 10-11
    int16_t  max_x;            // 12-13
    int16_t  max_y;            // 14-15
} __attribute__((packed)) TSFiGlyphRegistryPacked;

// Buffer C: The Curve Stream (Geometry)
// Quadratic Bezier Segments.
// Every segment is exactly 16 bytes.
// Flags:
// Bit 0: Is New Contour (MoveTo)
// Bit 1: Is Line (Control Point is ignored/implicit)
typedef struct {
    int16_t p0_x, p0_y;  // Start Point
    int16_t p1_x, p1_y;  // Control Point (or Midpoint)
    int16_t p2_x, p2_y;  // End Point
    uint16_t flags;      // Meta
    uint16_t padding;    // Pad to 16
} __attribute__((packed)) TSFiPackedSegment;

// Host-Side Management Structure
typedef struct {
    float thickness;       // Edge threshold (default 400.0)
    float softness;        // Edge blur width (default 200.0)
    int16_t shadow_secret_x;  // Shadow X secret
    int16_t shadow_secret_y;  // Shadow Y secret
    float shadow_alpha;    // Shadow opacity (0.0 = no shadow)
} TSFiGlyphStyle;

DEFINE_MAPPED_STRUCT(TSFiFontSystem,
    uint32_t *unicode_map;          // Buffer A (Host Copy)
    uint32_t *dna_indices;          // Map: Codepoint -> Index in wave_elements
    TSFiGlyphRegistryPacked *registry; // Buffer B (Host Copy)
    TSFiPackedSegment *segments;    // Buffer C (Host Copy)
    void *spectrum;                 // Buffer D (Hilbert State - Scalar Anchor)
    
    // Variadic WAVE Capacity (Unstructured / Byte-Aligned)
    WAVE15 *wave_elements;
    size_t  wave_count;
    size_t  wave_capacity;

    size_t registry_capacity;
    size_t registry_count;
    size_t segment_capacity;
    size_t segment_count;
    size_t spectrum_capacity;

    // Dirty Tracking (Ranges to upload)
    bool dirty_map;
    uint32_t dirty_reg_min, dirty_reg_max;
    uint32_t dirty_seg_min, dirty_seg_max;
    bool dirty_spectrum;
    
    // GPU Resources (Handles)
    void *vk_buffer_map;
    void *vk_buffer_registry;
    void *vk_buffer_segments;
    void *vk_buffer_spectrum;
    void *vk_image_atlas; // The Texture Cache

    // Buffer Device Addresses (BDA) - 64-bit Pointers for Pipeline Thunk
    uint64_t bda_map;
    uint64_t bda_registry;
    uint64_t bda_segments;
    uint64_t bda_spectrum;

    // Memory Coherency
    bool is_device_coherent; // If true, flush/invalidate ops are skipped

    // Styling (Visual Evolution)
    TSFiGlyphStyle current_style;

    // AI Workload Inheritance (Thunk Table)
    void *ai_methods; 
)

// DNA character wavefront for applying thunks as shaders
DEFINE_MAPPED_STRUCT(TSFiGlyphWavefront,
    uint32_t codepoint;
    K0RnStream *dna;
    TSFiGlyphStyle style;
    float resonance_score;
    void *custom_data;
)

// API
TSFiFontSystem* tsfi_font_init();
void tsfi_font_destroy(TSFiFontSystem *fs);

// Memory Management
void tsfi_font_set_coherency(TSFiFontSystem *fs, bool device_coherent);
void tsfi_font_flush_ranges(TSFiFontSystem *fs); // Flushes dirty ranges if not coherent

// Styling API
void tsfi_font_set_style(TSFiFontSystem *fs, float thick, float soft, int16_t sx, int16_t sy, float sa);

// "Alter as we go"
// Define a glyph's shape.
// If glyph_id is 0, allocates a new one and updates map.
// If glyph_id > 0, overwrites existing (if size fits) or appends new and updates map.
void tsfi_font_set_glyph(TSFiFontSystem *fs, uint32_t codepoint, 
                         TSFiPackedSegment *segs, size_t count, 
                         int16_t advance, int16_t x1, int16_t y1, int16_t x2, int16_t y2);

// Procedural Generation (The "Internal Font")
void tsfi_font_generate_default(TSFiFontSystem *fs);

// Rasterization API
void tsfi_font_rasterize_glyph_avx512(uint8_t *buffer, TSFiGlyphRegistryPacked *reg, TSFiPackedSegment *segments);
void tsfi_font_rasterize_glyph_bda(uint64_t target_addr, uint64_t registry_addr, uint64_t segments_base_addr, uint64_t style_addr);

// On-Demand DNA Compilation
#include "tsfi_k0rn_ops.h"
K0RnStream* tsfi_font_get_dna(TSFiFontSystem *fs, uint32_t codepoint);
void tsfi_font_save_prime(TSFiFontSystem *fs, uint32_t codepoint);

// Pt-Based K0Rn Rasterization
#include "tsfi_k0rn_ops.h"
void tsfi_font_render_k0rn_pt(const K0RnStream *stream, uint32_t *pixels, int width, int height, float pt_size, float x, float y, uint32_t color);

// Internal helper for manual glyph definition
void add_line(TSFiPackedSegment *buf, int *idx, int16_t x0, int16_t y0, int16_t x1, int16_t y1);

#endif
