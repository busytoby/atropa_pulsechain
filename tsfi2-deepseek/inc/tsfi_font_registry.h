#ifndef TSFI_FONT_REGISTRY_H
#define TSFI_FONT_REGISTRY_H

#include <stdint.h>
#include <stdbool.h>
#include "tsfi_ottype.h"

typedef struct {
    char path[256];
    uint8_t *data;
    size_t size;
    tsfi_ottype_t *ot;
    uint32_t ttc_secret;
    bool active;
} TSFiFontInfo;

typedef struct {
    uint32_t block_start;
    uint32_t block_end;
    char block_name[64];
    char best_font_path[256];
    
    // Metrics used to determine the Optimal Fallback
    uint32_t highest_coverage_count;
    uint32_t lowest_penalty_score;
    uint32_t highest_layout_score;
} TSFiFontCacheEntry;

#include "tsfi_types.h"

#define MAX_FONTS 1024
#define BMP_CACHE_SIZE 0x10000

DEFINE_MAPPED_STRUCT(TSFiFontRegistry,
    int count;
    int capacity;
    int primary_index;
    TSFiFontInfo *fonts;
    
    // Cache: Codepoint -> Index in fonts array
    // We use a simple 1:1 map for the Basic Multilingual Plane (0-0xFFFF)
    // and a sparse approach or direct search for higher planes if needed.
    int16_t *bmp_cache; 
    
    TSFiFontCacheEntry *capabilities;
    int num_capabilities;
)

void tsfi_font_registry_init(TSFiFontRegistry *reg);
bool tsfi_font_registry_add(TSFiFontRegistry *reg, const char *path);
void tsfi_font_registry_scan_system(TSFiFontRegistry *reg);
TSFiFontInfo* tsfi_font_registry_find_support(TSFiFontRegistry *reg, uint32_t codepoint);
const char* tsfi_font_registry_get_fallback(TSFiFontRegistry *reg, uint32_t codepoint);

#endif
