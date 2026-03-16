#include "tsfi_vision.h"
#include "lau_memory.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG(...) tsfi_io_printf(stderr, __VA_ARGS__)

static void parse_floats(char *val, float *out, int count) {
    char *copy = lau_strdup(val);
    char *saveptr;
    char *p = strtok_r(copy, ",", &saveptr);
    for (int i = 0; i < count && p; i++) {
        out[i] = (float)atof(p);
        p = strtok_r(NULL, ",", &saveptr);
    }
    lau_free(copy);
}

bool tsfi_phenotype_load(const char *filename, TSFiFlowerPhenotype *out) {
    if (!filename || !out) return false;
    FILE *f = fopen(filename, "r");
    if (!f) return false;

    memset(out, 0, sizeof(TSFiFlowerPhenotype));
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *saveptr;
        char *key = strtok_r(line, "=", &saveptr);
        char *val = strtok_r(NULL, "\n", &saveptr);
        if (!key || !val) continue;

        if (strcmp(key, "name") == 0) strncpy(out->name, val, 31);
        else if (strcmp(key, "ideal_radial_profile") == 0) parse_floats(val, out->ideal_radial_profile, 8);
        else if (strcmp(key, "target_coverage") == 0) out->target_coverage = (float)atof(val);
        else if (strcmp(key, "core_color") == 0) parse_floats(val, out->core_color, 3);
        else if (strcmp(key, "body_color") == 0) parse_floats(val, out->body_color, 3);
        else if (strcmp(key, "outer_color") == 0) parse_floats(val, out->outer_color, 3);
        else if (strcmp(key, "trident_freq") == 0) out->trident_freq = (float)atof(val);
        else if (strcmp(key, "curling_freq") == 0) out->curling_freq = (float)atof(val);
        else if (strcmp(key, "funnel_depth") == 0) out->funnel_depth = (float)atof(val);
    }

    fclose(f);
    return true;
}

bool tsfi_glyph_phenotype_load(const char *filename, TSFiGlyphPhenotype *out) {
    if (!filename || !out) return false;
    FILE *f = fopen(filename, "r");
    if (!f) return false;

    memset(out, 0, sizeof(TSFiGlyphPhenotype));
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *saveptr;
        char *key = strtok_r(line, "=", &saveptr);
        char *val = strtok_r(NULL, "\n", &saveptr);
        if (!key || !val) continue;

        if (strcmp(key, "name") == 0) strncpy(out->name, val, 31);
        else if (strcmp(key, "codepoint") == 0) out->codepoint = (uint32_t)strtoul(val, NULL, 0);
        else if (strcmp(key, "ideal_radial_profile") == 0) parse_floats(val, out->ideal_radial_profile, 8);
        else if (strcmp(key, "target_coverage") == 0) out->target_coverage = (float)atof(val);
        else if (strcmp(key, "stroke_thickness") == 0) out->stroke_thickness = (float)atof(val);
        else if (strcmp(key, "aspect_ratio") == 0) out->aspect_ratio = (float)atof(val);
        else if (strcmp(key, "slant_angle") == 0) out->slant_angle = (float)atof(val);
        else if (strcmp(key, "color") == 0) parse_floats(val, out->color, 3);
        else if (strcmp(key, "material_secret") == 0) out->material_secret = (float)atof(val);
        else if (strcmp(key, "target_holes") == 0) out->target_holes = atoi(val);
        else if (strcmp(key, "req_stem") == 0) out->req_stem = (float)atof(val);
        else if (strcmp(key, "req_bar") == 0) out->req_bar = (float)atof(val);
        else if (strcmp(key, "req_inner_circle") == 0) out->req_inner_circle = (float)atof(val);
        else if (strcmp(key, "req_outer_circle") == 0) out->req_outer_circle = (float)atof(val);
    }

    fclose(f);
    return true;
}