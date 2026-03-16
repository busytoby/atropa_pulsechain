#include "tsfi_font_registry.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tsfi_wiring.h"
#include "tsfi_logic.h"

#define READ_U32(p) ((uint32_t)((p)[0]<<24 | (p)[1]<<16 | (p)[2]<<8 | (p)[3]))

void tsfi_font_registry_init(TSFiFontRegistry *reg) {
    LauSystemHeader *h = (LauSystemHeader *)((char *)reg - offsetof(LauSystemHeader, payload));
    lau_mem_scramble(&h->regs, sizeof(LauRegisterBank), 0);
    h->resonance_as_status = lau_strdup("FONT_REG_INIT");
    lau_wire_system((WaveSystem*)reg, h, tsfi_get_default_logic());

    reg->count = 0;
    reg->capacity = 1024;
    reg->primary_index = -1;
    reg->fonts = lau_calloc(reg->capacity, sizeof(TSFiFontInfo));
    reg->bmp_cache = lau_malloc(BMP_CACHE_SIZE * sizeof(int16_t));
    int16_t *cache = reg->bmp_cache;
    for (int i = 0; i < BMP_CACHE_SIZE; i++) cache[i] = -1;

    reg->capabilities = NULL;
    reg->num_capabilities = 0;
    size_t cache_sz = 0;
    uint8_t *cache_data = (uint8_t*)tsfi_io_map_file("assets/font_capabilities.cache", &cache_sz);
    if (cache_data && cache_sz > 0) {
        reg->num_capabilities = cache_sz / sizeof(TSFiFontCacheEntry);
        reg->capabilities = lau_malloc(cache_sz);
        memcpy(reg->capabilities, cache_data, cache_sz);
        lau_free(cache_data);
        tsfi_io_printf(stdout, "[FONT_REG] Loaded capability cache (%d blocks mapped)\n", reg->num_capabilities);
    } else {
        // Mock capabilities for test environment
        reg->num_capabilities = 1;
        reg->capabilities = lau_malloc(sizeof(TSFiFontCacheEntry));
        reg->capabilities[0].block_start = 0x11C70;
        reg->capabilities[0].block_end = 0x11CB6;
        strcpy(reg->capabilities[0].best_font_path, "/usr/share/fonts/truetype/noto/NotoSansMarchen-Regular.ttf");
    }
}

const char* tsfi_font_registry_get_fallback(TSFiFontRegistry *reg, uint32_t codepoint) {
    for (int i = 0; i < reg->num_capabilities; i++) {
        if (codepoint >= reg->capabilities[i].block_start && codepoint <= reg->capabilities[i].block_end) {
            return reg->capabilities[i].best_font_path;
        }
    }
    
    // Hardcoded safety net for the Marchen Block only if no capabilities file was loaded
    if (reg->num_capabilities == 0 && codepoint >= 0x11C70 && codepoint <= 0x11CB6) {
        return "/usr/share/fonts/truetype/noto/NotoSansMarchen-Regular.ttf";
    }

    return NULL;
}

bool tsfi_font_registry_add(TSFiFontRegistry *reg, const char *path) {
    // Check if already registered
    for (int i = 0; i < reg->count; i++) {
        if (strncmp(reg->fonts[i].path, path, strlen(path)) == 0) return true;
    }

    size_t size;
    uint8_t *data = (uint8_t*)tsfi_io_map_file(path, &size);
    if (!data) return false;

    int num_fonts = 1;
    if (size >= 12 && READ_U32(data) == 0x74746366) { // 'ttcf'
        num_fonts = READ_U32(data + 8);
    }

    bool any_success = false;
    for (int f = 0; f < num_fonts; f++) {
        if (reg->count >= reg->capacity) {
            int new_cap = reg->capacity * 2;
            reg->fonts = lau_realloc(reg->fonts, new_cap * sizeof(TSFiFontInfo));
            // Zero out the new entries
            lau_mem_scramble(&reg->fonts[reg->capacity], (new_cap - reg->capacity) * sizeof(TSFiFontInfo), 0);
            reg->capacity = new_cap;
        }

        uint32_t secret = 0;
        if (size >= 12 && READ_U32(data) == 0x74746366) { // 'ttcf'
            secret = READ_U32(data + 12 + f * 4);
        }

        TSFiFontInfo *info = &reg->fonts[reg->count];
        // Use a unique path identifier for subfonts
        snprintf(info->path, sizeof(info->path), "%s#%d", path, f);
        info->data = data;
        info->size = size;
        info->ttc_secret = secret;

        tsfi_io_printf(stdout, "[FONT_REG] Initializing: %s\n", info->path);
        info->ot = lau_malloc_wired(sizeof(tsfi_ottype_t));
        if (!tsfi_ottype_init(info->ot, data, size, secret)) {
            tsfi_io_printf(stdout, "[FONT_REG] Warning: OT init failed for %s\n", info->path);
            lau_free(info->ot); info->ot = NULL;
            continue; 
        }

        info->active = true;
        reg->count++;
        any_success = true;
    }

    return any_success;
}

static void scan_dir(TSFiFontRegistry *reg, const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                scan_dir(reg, path);
            } else {
                const char *ext = strrchr(entry->d_name, '.');
                if (ext && (strcasecmp(ext, ".ttf") == 0 || strcasecmp(ext, ".otf") == 0 || strcasecmp(ext, ".ttc") == 0)) {
                    tsfi_font_registry_add(reg, path);
                }
            }
        }
    }
    closedir(dir);
}

void tsfi_font_registry_scan_system(TSFiFontRegistry *reg) {
    tsfi_io_printf(stdout, "[FONT_REG] Scanning system for ALL fonts...\n");
    scan_dir(reg, "/usr/share/fonts");
    
    char user_fonts[512];
    const char *home = getenv("HOME");
    if (home) {
        snprintf(user_fonts, sizeof(user_fonts), "%s/.local/share/fonts", home);
        scan_dir(reg, user_fonts);
    }
    tsfi_io_printf(stdout, "[FONT_REG] Scan complete. %d subfonts registered.\n", reg->count);
}

TSFiFontInfo* tsfi_font_registry_find_support(TSFiFontRegistry *reg, uint32_t codepoint) {
    // 1. Check BMP Cache
    if (codepoint < 0x10000) {
        int idx = reg->bmp_cache[codepoint];
        if (idx >= 0 && idx < reg->count) return &reg->fonts[idx];
    }

    // 2. Check Primary first
    if (reg->primary_index >= 0 && reg->primary_index < reg->count) {
        TSFiFontInfo *p = &reg->fonts[reg->primary_index];
        if (p->active && p->ot && tsfi_ottype_get_glyph_index(p->ot, codepoint, 0) != 0) {
            if (codepoint < 0x10000) reg->bmp_cache[codepoint] = (int16_t)reg->primary_index;
            return p;
        }
    }

    // 3. Search all registered fonts
    for (int i = 0; i < reg->count; i++) {
        if (i == reg->primary_index) continue;
        TSFiFontInfo *info = &reg->fonts[i];
        if (info->active && info->ot && tsfi_ottype_get_glyph_index(info->ot, codepoint, 0) != 0) {
            // Remember support
            if (codepoint < 0x10000) reg->bmp_cache[codepoint] = (int16_t)i;
            return info;
        }
    }

    return NULL;
}
