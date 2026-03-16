#ifndef TSFI_OTTYPE_INTERNAL_H
#define TSFI_OTTYPE_INTERNAL_H

#include "tsfi_ottype.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "lau_memory.h"
#include "tsfi_math.h"

#define TAG(a,b,c,d) ((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(c)<<8 | (uint32_t)(d))

#define READ_U16(p) (uint16_t)(((const uint8_t*)(p))[0] << 8 | ((const uint8_t*)(p))[1])
#define READ_U32(p) (uint32_t)((uint32_t)((const uint8_t*)(p))[0] << 24 | \
                               (uint32_t)((const uint8_t*)(p))[1] << 16 | \
                               (uint32_t)((const uint8_t*)(p))[2] << 8  | \
                               (uint32_t)((const uint8_t*)(p))[3])

// Internal shaping declarations
uint16_t tsfi_ottype_get_feature_index(const uint8_t *tbl, uint32_t tag);
int tsfi_ottype_check_coverage(const uint8_t *cov, uint32_t gid);
tsfi_joining_type_t tsfi_ottype_get_joining_type(uint32_t cp);
void tsfi_ottype_calculate_joining(const uint32_t *cps, int count, uint32_t *features);

// Internal geometry declarations
bool tsfi_ottype_get_glyph_geom_internal(tsfi_ottype_t *ot, uint32_t gid, tsfi_glyph_geom_t *geom, float oX, float oY, float sX, float sY);

#endif
