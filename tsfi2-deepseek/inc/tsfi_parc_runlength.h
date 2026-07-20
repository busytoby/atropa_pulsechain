#ifndef TSFI_PARC_RUNLENGTH_H
#define TSFI_PARC_RUNLENGTH_H

#include <stdint.h>

typedef struct {
    int is_bold;
    int is_underline;
    int is_shadow;
    uint32_t color;
} tsfi_parc_style_run_t;

/* Render stylized text layout using Bravo run-length styling specs */
int tsfi_parc_render_styled_text(uint32_t *pixels, int w, int h, int x, int y, const char *text, const tsfi_parc_style_run_t *style, float size);

#endif // TSFI_PARC_RUNLENGTH_H
