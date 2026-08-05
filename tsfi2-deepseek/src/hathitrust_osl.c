#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_osl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_aging_osl(int pub_year, int target_year) {
    int age = target_year - pub_year;
    if (age < 0) age = 0;
    
    // Scale paper yellowing factor: 0.0 (new) to 1.0 (century old)
    double yellowing_factor = age / 100.0;
    if (yellowing_factor > 1.0) yellowing_factor = 1.0;
    
    size_t cap = 1536;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "shader HtrcPaperAging\n"
             "[\n"
             "    color baseColor = color(1.0, 0.98, 0.94),\n"
             "    float agingFactor = %f,\n"
             "    output color resultColor = color(0.0)\n"
             "]\n"
             "{\n"
             "    // Yellowing displacement calculated relative to book age (%d years)\n"
             "    color agedColor = color(0.85, 0.76, 0.58);\n"
             "    resultColor = mix(baseColor, agedColor, agingFactor);\n"
             "}\n",
             yellowing_factor, age);
             
    return res;
}
