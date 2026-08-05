#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_camera_path.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_camera_path_rib(const HtrcAltoPage *page) {
    if (!page || page->token_count == 0) return NULL;
    
    size_t cap = 2048 + page->token_count * 256;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    char *ptr = res;
    size_t remaining = cap;
    
    int n = snprintf(ptr, remaining, "## RIB Camera Reading Fly-through Path\n");
    ptr += n;
    remaining -= n;
    
    int frame = 1;
    int last_vpos = -1;
    int line_min_hpos = -1;
    int line_max_hpos = -1;
    
    for (int i = 0; i < page->token_count; i++) {
        HtrcAltoToken *tok = &page->tokens[i];
        
        if (last_vpos != -1 && tok->vpos > last_vpos + 10) {
            double center_x = (line_min_hpos + (line_max_hpos - line_min_hpos) / 2.0) / 1000.0;
            double center_y = last_vpos / 1000.0;
            
            n = snprintf(ptr, remaining,
                         "FrameBegin %d\n"
                         "  # Look at line center: x=%.3f, y=%.3f\n"
                         "  Translate %.3f %.3f 2.5\n"
                         "  Rotate 0 0 1 0\n"
                         "FrameEnd\n",
                         frame++, center_x, -center_y, -center_x, center_y);
            ptr += n;
            remaining -= n;
            
            line_min_hpos = -1;
            line_max_hpos = -1;
        }
        
        if (line_min_hpos == -1 || tok->hpos < line_min_hpos) {
            line_min_hpos = tok->hpos;
        }
        if (line_max_hpos == -1 || (tok->hpos + tok->width) > line_max_hpos) {
            line_max_hpos = tok->hpos + tok->width;
        }
        last_vpos = tok->vpos;
    }
    
    if (line_min_hpos != -1) {
        double center_x = (line_min_hpos + (line_max_hpos - line_min_hpos) / 2.0) / 1000.0;
        double center_y = last_vpos / 1000.0;
        
        n = snprintf(ptr, remaining,
                     "FrameBegin %d\n"
                     "  # Look at line center: x=%.3f, y=%.3f\n"
                     "  Translate %.3f %.3f 2.5\n"
                     "  Rotate 0 0 1 0\n"
                     "FrameEnd\n",
                     frame++, center_x, -center_y, -center_x, center_y);
        ptr += n;
        remaining -= n;
    }
    
    return res;
}
