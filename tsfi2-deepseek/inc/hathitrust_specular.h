#ifndef HATHITRUST_SPECULAR_H
#define HATHITRUST_SPECULAR_H

#include "hathitrust_alto.h"

/*
 * Compiles ALTO OCR coordinate sets into a binary specular texture map.
 * Glossy specular paper values (0.8) are contrasted with matte ink coordinates (0.1).
 * Parameters:
 *   page: Pre-parsed ALTO page layouts.
 *   width: Target texture width in pixels (e.g. 512).
 *   height: Target texture height in pixels (e.g. 512).
 * Returns:
 *   Allocated buffer of size width * height containing specular byte levels, or NULL on error.
 *   The caller is responsible for freeing the returned buffer.
 */
unsigned char* hathitrust_generate_specular_map(
    const HtrcAltoPage *page,
    int width,
    int height
);

#endif /* HATHITRUST_SPECULAR_H */
