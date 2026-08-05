#ifndef HATHITRUST_RENDER_H
#define HATHITRUST_RENDER_H

#include <stdbool.h>

typedef struct {
    double x, y, z;
} HtrcVertex;

/*
 * Calculates page turn vertex bending displacement.
 * Bends a flat page coordinates based on curl parameters.
 * Bending is scaled dynamically with WinchesterMQ system register boundaries.
 */
HtrcVertex hathitrust_displacement_shader_calc(
    const HtrcVertex *orig,
    double curl_angle,
    double wm_scale_bound
);

/*
 * Translates page metadata to a USDA (ASCII USD) representation.
 */
char* hathitrust_render_to_usda(const char *htid, int page_seq, const char *img_url);

/*
 * Derives a deterministic DNA seed value from IIIF metrics.
 */
unsigned int hathitrust_render_derive_dna(const char *htid, int page_seq, int word_count);

#endif /* HATHITRUST_RENDER_H */
