#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_osl_double_sided.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_double_sided_osl(const char *front_tex, const char *back_tex) {
    if (!front_tex || !back_tex) return NULL;
    
    size_t cap = 2048;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "shader HtrcDoubleSidedPage\n"
             "[\n"
             "    string frontTexture = \"%s\",\n"
             "    string backTexture = \"%s\",\n"
             "    output color resultColor = color(0.0)\n"
             "]\n"
             "{\n"
             "    // Detect face normal orientation using backfacing() builtin\n"
             "    if (backfacing()) {\n"
             "        resultColor = texture(backTexture, u, v);\n"
             "    } else {\n"
             "        resultColor = texture(frontTexture, u, v);\n"
             "    }\n"
             "}\n",
             front_tex, back_tex);
             
    return res;
}
