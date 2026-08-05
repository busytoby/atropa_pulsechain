#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_osl_impression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_impression_osl(double indent_depth) {
    size_t cap = 1536;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "shader HtrcLetterpressImpression\n"
             "[\n"
             "    string inkMap = \"specular_map.png\",\n"
             "    float indentDepth = %f,\n"
             "    output float displacementAmount = 0.0\n"
             "]\n"
             "{\n"
             "    // Sample map: black ink (0.0), white paper (1.0)\n"
             "    float inkVal = texture(inkMap, u, v);\n"
             "    if (inkVal < 0.5) {\n"
             "        displacementAmount = -indentDepth;\n"
             "    } else {\n"
             "        displacementAmount = 0.0;\n"
             "    }\n"
             "}\n",
             indent_depth);
             
    return res;
}
