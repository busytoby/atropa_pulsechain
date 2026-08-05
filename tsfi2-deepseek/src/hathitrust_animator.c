#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_animator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

char* hathitrust_generate_page_curl_keyframes_rib(int start_frame, int end_frame) {
    int total_frames = end_frame - start_frame + 1;
    if (total_frames <= 0) return NULL;
    
    size_t cap = 256 + total_frames * 256;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    char *ptr = res;
    size_t remaining = cap;
    
    int n = snprintf(ptr, remaining, "## Animated Page-Turning Keyframe RIB Sequence\n");
    ptr += n;
    remaining -= n;
    
    for (int f = start_frame; f <= end_frame; f++) {
        // Calculate interpolation t from 0.0 to 1.0
        double t = (double)(f - start_frame) / (total_frames - 1);
        
        // Sinusoidal curl angle mapping: starts flat (0.0), bends to max (pi), returns flat (0.0)
        double curl_angle = sin(t * M_PI) * (M_PI / 2.0);
        // Translation x shifts from right page position (0) to left (opposite side)
        double trans_x = -t * 2.0;
        
        n = snprintf(ptr, remaining,
                     "FrameBegin %d\n"
                     "  AttributeBegin\n"
                     "    Translate %f 0 0\n"
                     "    Displacement \"HtrcPageCurlShader\" \"float curlAngle\" [%f]\n"
                     "    SubdivisionMesh \"catmull-clark\" [4] [0 1 2 3] [\"interpolateboundary\"] [0 0] [] [] \"P\" [-1 -1.5 0  1 -1.5 0  1 1.5 0  -1 1.5 0]\n"
                     "  AttributeEnd\n"
                     "FrameEnd\n",
                     f, trans_x, curl_angle);
                     
        ptr += n;
        remaining -= n;
    }
    
    return res;
}
