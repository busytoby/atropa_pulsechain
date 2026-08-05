#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_specular.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* hathitrust_generate_specular_map(
    const HtrcAltoPage *page,
    int width,
    int height
) {
    if (!page || width <= 0 || height <= 0) return NULL;
    
    size_t size = (size_t)width * height;
    unsigned char *map = malloc(size);
    if (!map) return NULL;
    
    // Default paper gloss: 200 (semi-glossy)
    memset(map, 200, size);
    
    // Determine coordinate scale factors
    // We assume layout dimensions roughly span 0 to 1400 pixels internally
    double scale_x = (double)width / 1400.0;
    double scale_y = (double)height / 2000.0;
    
    for (int i = 0; i < page->token_count; i++) {
        HtrcAltoToken *tok = &page->tokens[i];
        
        int x1 = (int)(tok->hpos * scale_x);
        int y1 = (int)(tok->vpos * scale_y);
        int x2 = (int)((tok->hpos + tok->width) * scale_x);
        int y2 = (int)((tok->vpos + tok->height) * scale_y);
        
        // Clamp boundaries
        if (x1 < 0) x1 = 0;
        if (y1 < 0) y1 = 0;
        if (x2 >= width) x2 = width - 1;
        if (y2 >= height) y2 = height - 1;
        
        // Write matte ink value (25) inside word bounding box coordinates
        for (int y = y1; y <= y2; y++) {
            for (int x = x1; x <= x2; x++) {
                map[y * width + x] = 25;
            }
        }
    }
    
    return map;
}
