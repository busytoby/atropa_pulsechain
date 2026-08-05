#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_osl_gilding.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char* hathitrust_generate_gilding_osl(bool gilded) {
    size_t cap = 1536;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    if (gilded) {
        snprintf(res, cap,
                 "shader HtrcBookGilding\n"
                 "[\n"
                 "    color goldColor = color(0.9, 0.78, 0.34),\n"
                 "    output color resultColor = color(0.0),\n"
                 "    output float specularRoughness = 0.05\n"
                 "]\n"
                 "{\n"
                 "    // Gilded reflective surface color mapping\n"
                 "    resultColor = goldColor;\n"
                 "    specularRoughness = 0.08 + 0.02 * noise(\"perlin\", u * 50.0);\n"
                 "}\n");
    } else {
        snprintf(res, cap,
                 "shader HtrcBookGilding\n"
                 "[\n"
                 "    color paperColor = color(0.95, 0.94, 0.9),\n"
                 "    color stripeColor = color(0.85, 0.82, 0.76),\n"
                 "    output color resultColor = color(0.0),\n"
                 "    output float specularRoughness = 0.8\n"
                 "]\n"
                 "{\n"
                 "    // Stacked paper stripe profile simulation\n"
                 "    float stripe = sin(v * 2000.0);\n"
                 "    resultColor = mix(paperColor, stripeColor, (stripe > 0.0) ? 0.3 : 0.0);\n"
                 "    specularRoughness = 0.75 + 0.1 * noise(\"perlin\", v * 100.0);\n"
                 "}\n");
    }
    
    return res;
}
