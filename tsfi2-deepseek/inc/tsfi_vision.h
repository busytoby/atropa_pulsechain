#ifndef TSFI_VISION_H
#define TSFI_VISION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// PPM Image Structure (for CLI/File I/O)
typedef struct {
    int width;
    int height;
    uint8_t *data; // RGB data (3 bytes per pixel)
    size_t size;
} TSFiImage;

// Lifecycle
TSFiImage* tsfi_vision_load_ppm(const char *filename);
void tsfi_vision_free_image(TSFiImage *img);

// --- Core Algorithms (Raw Buffer - AVX-512 Optimized) ---
uint8_t tsfi_vision_max_value_raw(const uint8_t *data, size_t size);
bool tsfi_vision_compare_raw(const uint8_t *a, const uint8_t *b, size_t size);
uint8_t tsfi_vision_probe_pixel_raw(const uint8_t *data, int x, int y, int width, int channels);

typedef struct {
    char name[32];
    float ideal_radial_profile[8];
    float target_coverage;
    float core_color[3];
    float body_color[3];
    float outer_color[3];
    float trident_freq;
    float curling_freq;
    float funnel_depth;
    float secret_resonance;
} TSFiFlowerPhenotype;

typedef struct {
    char name[32];
    uint32_t codepoint;
    float ideal_radial_profile[8];
    float target_coverage;
    float stroke_thickness;
    float aspect_ratio;
    float slant_angle;
    float color[3];
    float material_secret;
    int target_holes;
    float req_stem;
    float req_bar;
    float req_inner_circle;
    float req_outer_circle;
} TSFiGlyphPhenotype;

// Resonance & Shape Analysis 
typedef struct {
    float coverage;         // 0.0 - 1.0 (Pixel fill rate)
    float avg_intensity;    // 0.0 - 1.0 (Average brightness)
    float center_mass_x;    // 0.0 - 1.0 (Normalized coordinate)
    float center_mass_y;    // 0.0 - 1.0 (Normalized coordinate)
    float lobe_symmetry;    // 0.0 - 1.0 (Radial symmetry score)
    float glyph_symmetry;   // 0.0 - 1.0 (Vertical axis symmetry)
    float radial_score;     // 0.0 - 1.0 (How well it fits the profile)
    float center_intensity; // 0.0 - 1.0 (Average brightness at core)
    float rim_intensity;    // 0.0 - 1.0 (Average brightness at outer rim)
    float void_ratio;       // 0.0 - 1.0 (Target: 0.0, ratio of center/rim)
    float pentagonal_fit;   // 0.0 - 1.0 (5-fold rotational symmetry)
    float edge_intensity;   // 0.0 - 1.0 (Brightness at the buffer borders)
    float entropy;          // Shannon entropy of pixel distribution
    float smoothness;       // 0.0 - 1.0 (Temporal stability)
    float complexity;       // 0.0 - 1.0 (Spatial variety / rich detail)
    float visual_delta;     // 0.0 - 1.0 (Magnitude of change from previous frame)
    float frametime_ms;     // Time taken to render/process last frame
    float fps;              // Calculated frames per second (Raw loop)
    float content_fps;      // Effective frames per second (Visual change)
    float radial_variance;  // Variance in petal lengths
    float specular_contrast;// Highlight vs Shadow ratio
    float lobe_freq;        // Dominant radial frequency (target: 5.0)
    float surface_grain;    // High-frequency detail density (veins)
    float chromatic_balance;// Ratio of throat color to petal color
    float target_correlation; // 0.0 - 1.0 match vs. ideal profile
    float progression_ratio;  // 0.0 - 1.0 aggregated mission completion
    float symmetry_stability; // Consistency of 5-fold symmetry over time
    
    // Feature Scores (Haar/Gradient)
    float feature_outer_circle; 
    float feature_inner_circle;
    float feature_stem;
    float feature_bar;

    float baseline_similarity; // 0.0 - 1.0 similarity to Gen 0
    float chroma_drift;       // Detection of blue artifacts
    float topo_hole_count;    // Count of detected closed loops
    float island_count;       // Count of disconnected glyph components
    float captured_radial_profile[8]; // Real-time radial signature

    // Vertical Metrics (from Font Source)
    float ascent;
    float descent;
    float line_gap;
} TSFiResonanceAnalysis;

typedef struct {
    TSFiFlowerPhenotype phenotype;
    TSFiResonanceAnalysis last_analysis;
    uint32_t *pixels;
    int width, height;
} TSFiFlowerEntry;

typedef struct {
    TSFiFlowerEntry entries[16];
    int count;
} TSFiFloralManifest;

// Manifest Management
void tsfi_vision_manifest_register(TSFiFloralManifest *m, const TSFiFlowerPhenotype *p);
void tsfi_vision_manifest_update(TSFiFloralManifest *m, const char *name, const TSFiResonanceAnalysis *a, const uint32_t *pixels, int w, int h);
TSFiFlowerEntry* tsfi_vision_manifest_get(TSFiFloralManifest *m, const char *name);

void tsfi_vision_analyze_glyph(const uint32_t *data, int width, int height, const TSFiGlyphPhenotype *p, TSFiResonanceAnalysis *out);

// Phenotype Asset Management
bool tsfi_phenotype_load(const char *filename, TSFiFlowerPhenotype *out);
bool tsfi_glyph_phenotype_load(const char *filename, TSFiGlyphPhenotype *out);

// Visual Integrity & Checkpointing
bool tsfi_vision_verify_integrity(const TSFiResonanceAnalysis *a);
void tsfi_vision_save_checkpoint(const char *name, const TSFiResonanceAnalysis *a, const uint32_t *pixels, int w, int h);
typedef enum {
    TSFI_CLASS_UNKNOWN = 0,
    TSFI_CLASS_FLORAL  = 1,
    TSFI_CLASS_GLYPH   = 2,
    TSFI_CLASS_LATIN_A = 3,
    TSFI_CLASS_LATIN_B = 4,

    // Explicit Floral Species
    TSFI_CLASS_XENOSTEGIA = 10,
    TSFI_CLASS_IPOMOEA    = 11,
    TSFI_CLASS_CONVOLVULUS= 12,
    TSFI_CLASS_HEWITTIA   = 13,
    TSFI_CLASS_HYALOCYSTIS= 14,
    TSFI_CLASS_OPERCULINA = 15,
    TSFI_CLASS_REMIREMA   = 16,
    TSFI_CLASS_DISTIMAKE  = 17,
    TSFI_CLASS_MERREMIA   = 18,
    TSFI_CLASS_LINE_ART   = 20,
    TSFI_CLASS_TEDDY      = 100,
    TSFI_CLASS_CROW       = 101,
    TSFI_CLASS_ALLIGATOR  = 102,
    TSFI_CLASS_POPPY      = 103
} TSFiResonanceClass;

typedef struct {
    TSFiResonanceClass class_id;
    float confidence;
    float raw_score;
    float floral_score;
    float glyph_score;
} TSFiClassification;

typedef struct {
    char name[32];
    TSFiResonanceClass target_class;
    float weight_radial;
    float weight_symmetry;
    float weight_holes;
    float weight_complexity;
    float threshold_radial;
    float threshold_symmetry;
    float threshold_top_bot; // Specular contrast proxy
    int required_holes;
    float ideal_radial_profile[8]; // Specific species template
} LauClassificationAsset;

bool tsfi_classification_load(const char *name, LauClassificationAsset *out);
TSFiClassification tsfi_vision_classify_asset(const TSFiResonanceAnalysis *a, const LauClassificationAsset *c);
TSFiClassification tsfi_vision_classify(const TSFiResonanceAnalysis *a);
void tsfi_vision_save_glyph_checkpoint(const char *name, const TSFiResonanceAnalysis *a, const uint32_t *pixels, int w, int h);
bool tsfi_vision_load_checkpoint(const char *name, TSFiResonanceAnalysis *a, uint32_t **pixels, int *w, int *h);
bool tsfi_vision_load_glyph_checkpoint(const char *name, TSFiResonanceAnalysis *a, uint32_t **pixels, int *w, int *h);

// Calculates similarity (MSE proxy) between two pixel buffers
__attribute__((force_align_arg_pointer))
float tsfi_vision_calculate_similarity(const uint32_t *a, const uint32_t *b, int width, int height);
__attribute__((force_align_arg_pointer))
float tsfi_vision_calculate_similarity_ab4h(const uint16_t *a, const uint16_t *b, int width, int height);

// Calculates edge similarity using gradient analysis
__attribute__((force_align_arg_pointer))
float tsfi_vision_calculate_edge_similarity(const uint32_t *a, const uint32_t *b, int width, int height);

// Visually calculates optimal spacing (advance) between two rendered glyphs
float tsfi_vision_calculate_glyph_spacing(const uint32_t *left_glyph, const uint32_t *right_glyph, int width, int height, float target_gap);

// Analyzes a float buffer (single channel, usually from compute shader)
void tsfi_vision_analyze_resonance(const float *data, int width, int height, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out);

// Analyzes a raw RGBA buffer (standard staging buffer)
void tsfi_vision_analyze_staging(const uint32_t *data, int width, int height, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out);
void tsfi_vision_analyze_staging_ab4h(const uint16_t *data, int width, int height, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out);

// Temporal analysis
void tsfi_vision_analyze_temporal(const uint32_t *data, int width, int height, const TSFiResonanceAnalysis *prev, float dt_ms, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out);

// --- Thunk-Based Augmented Pipeline ---
#include "tsfi_k0rn_ops.h"
void tsfi_vision_ray_query_thunk(const K0RnOp *ops, int op_count, TSFiResonanceAnalysis *out, int width, int height);
int tsfi_vision_classify_thunk(void *obj);

// --- Bloom Effect Thunk ---
void __attribute__((weak)) tsfi_bloom_thunk(uint32_t* pixels, int width, int height, float threshold, float intensity);
void __attribute__((weak)) tsfi_bloom_thunk_ab4h(uint16_t* pixels, int width, int height, float threshold, float intensity);

// --- Unified Flower Render Shading Thunk ---
typedef struct {
    float light_x, light_y, light_z;
    float sss_power;      
    float fresnel_power;   
    float secret_resonance;
    float core_color[3];
    float body_color[3];
    float outer_color[3];
} TSFiFlowerRenderUniforms;

#include "tsfi_wave_any.h"
void tsfi_flower_render_shading(const WaveStream* stream, uint32_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u);
void tsfi_flower_render_shading_ab4h(const WaveStream* stream, uint16_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u);
void tsfi_flower_render_shading_threaded(const WaveStream* stream, uint32_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u);
void tsfi_flower_render_shading_ab4h_threaded(const WaveStream* stream, uint16_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u);

#endif // TSFI_VISION_H