#include "tsfi_ottype_internal.h"
#include "tsfi_cff.h"

uint32_t tsfi_ottype_get_glyph_location(tsfi_ottype_t *ot, uint32_t glyph_index) {
    if (!ot->location_secret || glyph_index > ot->num_glyphs) return (uint32_t)-1;

    if (ot->location_format == 0) { // 16-bit secrets (Short)
        uint32_t addr = ot->location_secret + glyph_index * 2;
        if (addr + 2 > ot->size) return (uint32_t)-1;
        return (uint32_t)READ_U16(&ot->data[addr]) * 2;
    } else { // 32-bit secrets (Long)
        uint32_t addr = ot->location_secret + glyph_index * 4;
        if (addr + 4 > ot->size) return (uint32_t)-1;
        return READ_U32(&ot->data[addr]);
    }
}

bool tsfi_ottype_get_glyph_geom_internal(tsfi_ottype_t *ot, uint32_t gid, tsfi_glyph_geom_t *geom, float oX, float oY, float sX, float sY);


bool tsfi_ottype_get_glyph_geom(tsfi_ottype_t *ot, uint32_t glyph_index, tsfi_glyph_geom_t *geom) {
    if (ot->is_cff) {
        return tsfi_cff_get_glyph_geom(&ot->cff, glyph_index, geom);
    }
    return tsfi_ottype_get_glyph_geom_internal(ot, glyph_index, geom, 0, 0, 1.0f, 1.0f);
}

bool tsfi_ottype_get_glyph_geom_internal(tsfi_ottype_t *ot, uint32_t gid, tsfi_glyph_geom_t *geom, float oX, float oY, float sX, float sY) {
    if (!ot->glyf_secret || !ot->location_secret) return false;

    uint32_t glyph_secret = tsfi_ottype_get_glyph_location(ot, gid);
    uint32_t next_secret = tsfi_ottype_get_glyph_location(ot, gid + 1);
    if (glyph_secret == (uint32_t)-1 || next_secret == (uint32_t)-1) return false;
    if (glyph_secret == next_secret) return true; // Valid empty glyph (e.g. space)

    const uint8_t *glyf = &ot->data[ot->glyf_secret + glyph_secret];
    int16_t num_contours = (int16_t)READ_U16(glyf);

    if (num_contours >= 0) {
        if (num_contours == 0) return true; // Valid empty simple glyph
        
        if (num_contours > (int)geom->temp_cap_contours) {
            geom->temp_cap_contours = (num_contours + 31) & ~31;
            geom->temp_ends = (uint16_t*)lau_realloc(geom->temp_ends, sizeof(uint16_t) * geom->temp_cap_contours);
        }
        uint16_t *ends = geom->temp_ends;
        for (int i = 0; i < num_contours; i++) ends[i] = READ_U16(&glyf[10 + i * 2]);

        uint16_t num_points = ends[num_contours - 1] + 1;
        uint16_t ins_len = READ_U16(&glyf[10 + num_contours * 2]);
        const uint8_t *ptr = &glyf[10 + num_contours * 2 + 2 + ins_len];

        if (num_points > geom->temp_cap_pts) {
            geom->temp_cap_pts = (num_points + 255) & ~255;
            geom->temp_flags = (uint8_t*)lau_realloc(geom->temp_flags, geom->temp_cap_pts);
            geom->temp_x = (int16_t*)lau_realloc(geom->temp_x, geom->temp_cap_pts * sizeof(int16_t));
            geom->temp_y = (int16_t*)lau_realloc(geom->temp_y, geom->temp_cap_pts * sizeof(int16_t));
        }
        uint8_t *flags = geom->temp_flags;
        int16_t *x_coords = geom->temp_x;
        int16_t *y_coords = geom->temp_y;

        for (uint16_t i = 0; i < num_points; i++) {
            flags[i] = *ptr++;
            if (flags[i] & 0x08) { 
                uint8_t repeat = *ptr++;
                while (repeat--) {
                    i++;
                    flags[i] = flags[i-1];
                }
            }
        }

        int16_t cur_x = 0, cur_y = 0;
        for (uint16_t i = 0; i < num_points; i++) {
            if (flags[i] & 0x02) {
                uint8_t val = *ptr++;
                cur_x += (flags[i] & 0x10) ? val : -val;
            } else if (!(flags[i] & 0x10)) {
                cur_x += (int16_t)READ_U16(ptr); ptr += 2;
            }
            x_coords[i] = cur_x;
        }

        for (uint16_t i = 0; i < num_points; i++) {
            if (flags[i] & 0x04) {
                uint8_t val = *ptr++;
                cur_y += (flags[i] & 0x20) ? val : -val;
            } else if (!(flags[i] & 0x20)) {
                cur_y += (int16_t)READ_U16(ptr); ptr += 2;
            }
            y_coords[i] = cur_y;
        }

        uint32_t base_idx = geom->num_points;
        uint32_t pts_written = 0;
        uint32_t start_of_contour = 0;

        for (int c = 0; c < num_contours; c++) {
            uint16_t end_in_temp = ends[c];
            uint16_t first_pt_idx = start_of_contour;
            uint16_t last_pt_idx = end_in_temp;

            // TrueType contour midpoints expansion logic
            // We walk each point and check for consecutive off-curve points.
            for (uint16_t i = first_pt_idx; i <= last_pt_idx; i++) {
                // Ensure capacity
                if (geom->num_points + pts_written + 2 > geom->capacity) {
                    geom->capacity = (geom->num_points + pts_written + 256) * 2;
                    geom->points = lau_realloc(geom->points, geom->capacity * sizeof(tsfi_point_t));
                }

                bool curr_on = (flags[i] & 0x01);
                
                // If this is not the first point, check if previous was also off-curve
                if (i > first_pt_idx) {
                    bool prev_on = (flags[i-1] & 0x01);
                    if (!prev_on && !curr_on) {
                        // Insert midpoint
                        tsfi_point_t *mp = &geom->points[geom->num_points + pts_written];
                        mp->x = oX + (float)(x_coords[i-1] + x_coords[i]) * 0.5f * sX;
                        mp->y = oY + (float)(y_coords[i-1] + y_coords[i]) * 0.5f * sY;
                        mp->type = 1; // Midpoints are always on-curve
                        pts_written++;
                    }
                }

                // Write current point
                tsfi_point_t *p = &geom->points[geom->num_points + pts_written];
                p->x = oX + (float)x_coords[i] * sX;
                p->y = oY + (float)y_coords[i] * sY;
                p->type = curr_on ? 1 : 2;
                pts_written++;
            }

            // Wrap around: check last and first
            bool first_on = (flags[first_pt_idx] & 0x01);
            bool last_on = (flags[last_pt_idx] & 0x01);
            if (!first_on && !last_on) {
                if (geom->num_points + pts_written + 1 > geom->capacity) {
                    geom->capacity *= 2;
                    geom->points = lau_realloc(geom->points, geom->capacity * sizeof(tsfi_point_t));
                }
                tsfi_point_t *mp = &geom->points[geom->num_points + pts_written];
                mp->x = oX + (float)(x_coords[last_pt_idx] + x_coords[first_pt_idx]) * 0.5f * sX;
                mp->y = oY + (float)(y_coords[last_pt_idx] + y_coords[first_pt_idx]) * 0.5f * sY;
                mp->type = 1;
                pts_written++;
            }

            geom->contour_ends = lau_realloc(geom->contour_ends, (geom->num_contours + c + 1) * sizeof(uint16_t));
            geom->contour_ends[geom->num_contours + c] = (uint16_t)(base_idx + pts_written - 1);
            start_of_contour = last_pt_idx + 1;
        }

        geom->num_points += pts_written;
        geom->num_contours += num_contours;

        return true;
    } else {
        const uint8_t *ptr = &glyf[10];
        while (1) {
            uint16_t flags = READ_U16(ptr); ptr += 2;
            uint16_t sub_gid = READ_U16(ptr); ptr += 2;
            float sX_sub = 1.0f, sY_sub = 1.0f, sXY_sub = 0.0f, sYX_sub = 0.0f;
            float oX_sub = 0, oY_sub = 0;

            if (flags & 0x01) { // ARG_1_AND_2_ARE_WORDS
                oX_sub = (int16_t)READ_U16(ptr); ptr += 2;
                oY_sub = (int16_t)READ_U16(ptr); ptr += 2;
            } else {
                oX_sub = (int8_t)*ptr++;
                oY_sub = (int8_t)*ptr++;
            }

            if (flags & 0x08) { // WE_HAVE_A_SCALE
                sX_sub = sY_sub = (float)((int16_t)READ_U16(ptr)) / 16384.0f; ptr += 2;
            } else if (flags & 0x40) { // WE_HAVE_AN_X_AND_Y_SCALE
                sX_sub = (float)((int16_t)READ_U16(ptr)) / 16384.0f; ptr += 2;
                sY_sub = (float)((int16_t)READ_U16(ptr)) / 16384.0f; ptr += 2;
            } else if (flags & 0x80) { // WE_HAVE_A_TWO_BY_TWO
                sX_sub  = (float)((int16_t)READ_U16(ptr)) / 16384.0f; ptr += 2;
                sXY_sub = (float)((int16_t)READ_U16(ptr)) / 16384.0f; ptr += 2;
                sYX_sub = (float)((int16_t)READ_U16(ptr)) / 16384.0f; ptr += 2;
                sY_sub  = (float)((int16_t)READ_U16(ptr)) / 16384.0f; ptr += 2;
            }

            (void)sXY_sub; (void)sYX_sub;

            // Note: TrueType spec says if ARGS_ARE_XY_VALUES is NOT set, 
            // the arguments are point indices to align. 
            if (!(flags & 0x02)) {
                // Point alignment not yet fully implemented, use 0-secret for now
                // but allow the glyph to be "supported" in terms of point extraction.
                oX_sub = 0; oY_sub = 0;
            }

            tsfi_ottype_get_glyph_geom_internal(ot, sub_gid, geom, oX + oX_sub * sX, oY + oY_sub * sY, sX * sX_sub, sY * sY_sub);
            if (!(flags & 0x20)) break;
        }
        return true;
    }
}

void tsfi_ottype_free_geom(tsfi_glyph_geom_t *geom) {
    if (geom->points) lau_free(geom->points);
    if (geom->contour_ends) lau_free(geom->contour_ends);
    if (geom->temp_flags) lau_free(geom->temp_flags);
    if (geom->temp_x) lau_free(geom->temp_x);
    if (geom->temp_y) lau_free(geom->temp_y);
    if (geom->temp_ends) lau_free(geom->temp_ends);
    memset(geom, 0, sizeof(*geom));
}

K0RnStream* tsfi_ottype_compile_dna(const tsfi_glyph_geom_t *geom) {
    if (geom->num_points == 0) return NULL;
    K0RnStream *stream = lau_malloc_wired(sizeof(K0RnStream));
    lau_memset(stream, 0, sizeof(K0RnStream));
    stream->op_count = geom->num_points;
    stream->stream_capacity = geom->num_points;
    stream->ops = (K0RnOp *)lau_malloc_wired(sizeof(K0RnOp) * stream->op_count);
    lau_memset(stream->ops, 0, sizeof(K0RnOp) * stream->op_count);
    for (uint32_t i = 0; i < geom->num_points; i++) {
        K0RnOp *op = &stream->ops[i];
        op->type = K0RN_OP_SPHERE;
        op->p1 = 1.0f;
        op->x = geom->points[i].x;
        op->y = geom->points[i].y;
        op->w = 1.0f;
    }
    return stream;
}

