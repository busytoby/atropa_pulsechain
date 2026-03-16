#ifndef TSFI_CFF_H
#define TSFI_CFF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "tsfi_ottype.h"

typedef tsfi_cff_inner_t tsfi_cff_t;

bool tsfi_cff_init(tsfi_cff_t *cff, const uint8_t *data, size_t size);
bool tsfi_cff_get_glyph_geom(tsfi_cff_t *cff, uint32_t glyph_index, tsfi_glyph_geom_t *geom);

#endif
