#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_osl_bleed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_bleed_osl(double bleed_factor, double bump_scale) {
    size_t cap = 1536;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "shader HtrcInkBleed\n"
             "[\n"
             "    color inkColor = color(0.02, 0.02, 0.02),\n"
             "    float bleedFactor = %f,\n"
             "    float bumpScale = %f,\n"
             "    output color resultColor = color(0.0),\n"
             "    output vector normalOffset = vector(0.0)\n"
             "]\n"
             "{\n"
             "    // Simulates paper surface roughness and typographic ink-bleed contours\n"
             "    float noiseVal = noise(\"perlin\", u * 100.0, v * 100.0);\n"
             "    float threshold = 0.5 + noiseVal * bleedFactor;\n"
             "    resultColor = (u < threshold) ? inkColor : color(1.0);\n"
             "    normalOffset = vector(0.0, 0.0, noiseVal * bumpScale);\n"
             "}\n",
             bleed_factor, bump_scale);
             
    return res;
}
