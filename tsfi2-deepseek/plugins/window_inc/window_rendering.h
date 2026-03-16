#ifndef WINDOW_RENDERING_H
#define WINDOW_RENDERING_H

#include <stdint.h>

void draw_image(uint32_t *pixels, int buf_w, int buf_h, int x, int y, int w, int h, unsigned char *src, int img_w, int img_h, int channels);

#endif
