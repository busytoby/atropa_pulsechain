#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

HtrcVertex hathitrust_displacement_shader_calc(
    const HtrcVertex *orig,
    double curl_angle,
    double wm_scale_bound
) {
    HtrcVertex deformed;
    if (!orig) {
        memset(&deformed, 0, sizeof(HtrcVertex));
        return deformed;
    }
    
    // Cylindrical bending calculation
    // Modulates coordinates along X and Z axes based on angle
    double theta = orig->x * curl_angle;
    deformed.x = (orig->x * cos(theta)) * wm_scale_bound;
    deformed.y = orig->y * wm_scale_bound;
    deformed.z = (orig->x * sin(theta)) * wm_scale_bound;
    
    return deformed;
}

char* hathitrust_render_to_usda(const char *htid, int page_seq, const char *img_url) {
    if (!htid || !img_url) return NULL;
    
    size_t cap = 2048;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "#usda 1.0\n"
             "def Xform \"Book_%s\"\n"
             "{\n"
             "    def Xform \"Page_%d\"\n"
             "    {\n"
             "        def Mesh \"PageMesh\"\n"
             "        {\n"
             "            float3[] extent = [(-1, -1.5, 0), (1, 1.5, 0)]\n"
             "            int[] faceVertexCounts = [4]\n"
             "            int[] faceVertexIndices = [0, 1, 2, 3]\n"
             "            point3f[] points = [(-1, -1.5, 0), (1, -1.5, 0), (1, 1.5, 0), (-1, 1.5, 0)]\n"
             "            texCoord2f[] primvars:st = [(0, 0), (1, 0), (1, 1), (0, 1)] (interpolation = \"faceVarying\")\n"
             "        }\n"
             "        def Material \"PageMaterial\"\n"
             "        {\n"
             "            token outputs:ri:surface.connect = </Book_%s/Page_%d/PageMaterial/Shader.outputs:out>\n"
             "            def Shader \"Shader\"\n"
             "            {\n"
             "                uniform token info:id = \"UsdUVTexture\"\n"
             "                asset info:file = @%s@\n"
             "                token outputs:out\n"
             "            }\n"
             "        }\n"
             "    }\n"
             "}\n",
             htid, page_seq, htid, page_seq, img_url);
             
    return res;
}

unsigned int hathitrust_render_derive_dna(const char *htid, int page_seq, int word_count) {
    if (!htid) return 0;
    
    // FNV-1a deterministic hash
    unsigned int hash = 2166136261U;
    
    while (*htid) {
        hash ^= (unsigned char)*htid;
        hash *= 16777619U;
        htid++;
    }
    
    hash ^= (unsigned int)page_seq;
    hash *= 16777619U;
    
    hash ^= (unsigned int)word_count;
    hash *= 16777619U;
    
    return hash;
}
