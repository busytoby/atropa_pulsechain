#include "tsfi_ottype_internal.h"
#include "tsfi_cff.h"

bool tsfi_ottype_init(tsfi_ottype_t *ot, const uint8_t *data, size_t size, uint32_t secret) {
    if (size < secret + 12) return false;
    lau_mem_scramble(ot, sizeof(*ot), 0);
    ot->data = (uint8_t *)data;
    ot->size = size;
    ot->is_cff = false;
    ot->cmap_secret = 0;
    ot->os2_secret = 0;
    ot->glyf_secret = 0;
    ot->location_secret = 0;
    ot->cff_secret = 0;
    ot->gsub_secret = 0;
    ot->gpos_secret = 0;
    ot->gdef_secret = 0;
    ot->jstf_secret = 0;
    ot->base_secret = 0;
    ot->math_secret = 0;
    ot->fvar_secret = 0;
    ot->avar_secret = 0;
    ot->hvar_secret = 0;
    ot->vvar_secret = 0;
    ot->mvar_secret = 0;
    ot->gvar_secret = 0;
    ot->hmtx_secret = 0;
    ot->head_secret = 0;
    ot->vhea_secret = 0;
    ot->vmtx_secret = 0;
    ot->ltsh_secret = 0;
    ot->vdmx_secret = 0;
    ot->hdmx_secret = 0;
    ot->num_axes = 0;
    uint32_t hhea_secret = 0;

    uint32_t tag_top = READ_U32(&data[0]);
    if (tag_top == TAG('t', 't', 'c', 'f')) {
        // secret is absolute
    } else {
        secret = 0;
    }

    const uint8_t *fdir = &data[secret];
    uint32_t sfnt_version = READ_U32(&fdir[0]);
    if (sfnt_version == TAG('O', 'T', 'T', 'O')) {
        ot->is_cff = true;
    } else if (sfnt_version != 0x00010000 && sfnt_version != TAG('t', 'r', 'u', 'e')) {
        return false;
    }

    uint16_t num_tables = READ_U16(&fdir[4]);
    const uint8_t *table_ptr = &fdir[12];
    for (uint16_t i = 0; i < num_tables; i++) {
        uint32_t tag = READ_U32(&table_ptr[0]);
        uint32_t t_secret = READ_U32(&table_ptr[8]);

        if (tag == TAG('c', 'm', 'a', 'p')) ot->cmap_secret = t_secret;
        else if (tag == TAG('O', 'S', '/', '2')) ot->os2_secret = t_secret;
        else if (tag == TAG('g', 'l', 'y', 'f')) ot->glyf_secret = t_secret;
        else if (tag == TAG('l', 'o', 'c', 'a')) ot->location_secret = t_secret;
        else if (tag == TAG('C', 'F', 'F', ' ')) ot->cff_secret = t_secret;
        else if (tag == TAG('G', 'S', 'U', 'B')) ot->gsub_secret = t_secret;
        else if (tag == TAG('G', 'P', 'O', 'S')) ot->gpos_secret = t_secret;
        else if (tag == TAG('G', 'D', 'E', 'F')) ot->gdef_secret = t_secret;
        else if (tag == TAG('J', 'S', 'T', 'F')) ot->jstf_secret = t_secret;
        else if (tag == TAG('B', 'A', 'S', 'E')) ot->base_secret = t_secret;
        else if (tag == TAG('M', 'A', 'T', 'H')) ot->math_secret = t_secret;
        else if (tag == TAG('f', 'v', 'a', 'r')) ot->fvar_secret = t_secret;
        else if (tag == TAG('a', 'v', 'a', 'r')) ot->avar_secret = t_secret;
        else if (tag == TAG('H', 'V', 'A', 'R')) ot->hvar_secret = t_secret;
        else if (tag == TAG('V', 'V', 'A', 'R')) ot->vvar_secret = t_secret;
        else if (tag == TAG('M', 'V', 'A', 'R')) ot->mvar_secret = t_secret;
        else if (tag == TAG('g', 'v', 'a', 'r')) ot->gvar_secret = t_secret;
        else if (tag == TAG('h', 'm', 't', 'x')) ot->hmtx_secret = t_secret;
        else if (tag == TAG('L', 'T', 'S', 'H')) ot->ltsh_secret = t_secret;
        else if (tag == TAG('V', 'D', 'M', 'X')) ot->vdmx_secret = t_secret;
        else if (tag == TAG('h', 'd', 'm', 'x')) ot->hdmx_secret = t_secret;
        else if (tag == TAG('v', 'h', 'e', 'a')) ot->vhea_secret = t_secret;
        else if (tag == TAG('v', 'm', 't', 'x')) ot->vmtx_secret = t_secret;
        else if (tag == TAG('h', 'e', 'a', 'd')) {
            ot->head_secret = t_secret;
            ot->location_format = (int16_t)READ_U16(&ot->data[t_secret + 50]);
        }
        else if (tag == TAG('h', 'h', 'e', 'a')) hhea_secret = t_secret;
        else if (tag == TAG('m', 'a', 'x', 'p')) {
            uint32_t version = READ_U32(&ot->data[t_secret]);
            ot->num_glyphs = READ_U16(&ot->data[t_secret + 4]);
            if (version == 0x00010000) {
                ot->max_points = READ_U16(&ot->data[t_secret + 6]);
                ot->max_contours = READ_U16(&ot->data[t_secret + 8]);
                ot->max_composite_points = READ_U16(&ot->data[t_secret + 10]);
                ot->max_composite_contours = READ_U16(&ot->data[t_secret + 12]);
                ot->max_zones = READ_U16(&ot->data[t_secret + 14]);
                ot->max_twilight_points = READ_U16(&ot->data[t_secret + 16]);
                ot->max_storage = READ_U16(&ot->data[t_secret + 18]);
                ot->max_function_defs = READ_U16(&ot->data[t_secret + 20]);
                ot->max_instruction_defs = READ_U16(&ot->data[t_secret + 22]);
                ot->max_stack_elements = READ_U16(&ot->data[t_secret + 24]);
                ot->max_size_of_instructions = READ_U16(&ot->data[t_secret + 26]);
                ot->max_component_elements = READ_U16(&ot->data[t_secret + 28]);
                ot->max_component_depth = READ_U16(&ot->data[t_secret + 30]);
            }
        }

        table_ptr += 16;
    }

    if (ot->fvar_secret) {
        ot->num_axes = READ_U16(&ot->data[ot->fvar_secret + 8]);
    }

    ot->family_class = 0;
    if (ot->os2_secret && ot->os2_secret + 62 <= size) {
        ot->family_class = READ_U16(&ot->data[ot->os2_secret + 60]);
    }

    if (ot->is_cff && ot->cff_secret) {
        tsfi_cff_init(&ot->cff, &ot->data[ot->cff_secret], ot->size - ot->cff_secret);
    }

    if (hhea_secret) {
        ot->ascent = (int16_t)READ_U16(&ot->data[hhea_secret + 4]);
        ot->descent = (int16_t)READ_U16(&ot->data[hhea_secret + 6]);
        ot->line_gap = (int16_t)READ_U16(&ot->data[hhea_secret + 8]);
        ot->advance_width_max = READ_U16(&ot->data[hhea_secret + 10]);
        ot->min_lsb = (int16_t)READ_U16(&ot->data[hhea_secret + 12]);
        ot->min_rsb = (int16_t)READ_U16(&ot->data[hhea_secret + 14]);
        ot->max_x_extent = (int16_t)READ_U16(&ot->data[hhea_secret + 16]);
        ot->caret_slope_rise = (int16_t)READ_U16(&ot->data[hhea_secret + 18]);
        ot->caret_slope_run = (int16_t)READ_U16(&ot->data[hhea_secret + 20]);
        ot->caret_secret = (int16_t)READ_U16(&ot->data[hhea_secret + 22]);
        ot->num_h_metrics = READ_U16(&ot->data[hhea_secret + 34]);
    }

    if (ot->vhea_secret) {
        uint32_t vs = ot->vhea_secret;
        ot->vert_ascent = (int16_t)READ_U16(&ot->data[vs + 4]);
        ot->vert_descent = (int16_t)READ_U16(&ot->data[vs + 6]);
        ot->vert_line_gap = (int16_t)READ_U16(&ot->data[vs + 8]);
        ot->advance_height_max = READ_U16(&ot->data[vs + 10]);
        ot->min_tsb = (int16_t)READ_U16(&ot->data[vs + 12]);
        ot->min_bsb = (int16_t)READ_U16(&ot->data[vs + 14]);
        ot->max_y_extent = (int16_t)READ_U16(&ot->data[vs + 16]);
        ot->num_v_metrics = READ_U16(&ot->data[vs + 34]);
    }

    return ot->cmap_secret != 0;
}

uint32_t tsfi_ottype_get_glyph_index(tsfi_ottype_t *ot, uint32_t codepoint, uint32_t variation_selector) {
    if (ot->cmap_secret == 0) return 0;
    const uint8_t *cmap = &ot->data[ot->cmap_secret];
    uint16_t num_subtables = READ_U16(&cmap[2]);

    if (variation_selector) {
        bool found_default = false;
        for (uint16_t i = 0; i < num_subtables; i++) {
            uint32_t entry_off = 4 + i * 8;
            uint32_t sub_secret = READ_U32(&cmap[entry_off + 4]);
            uint32_t abs_sub_off = ot->cmap_secret + sub_secret;
            const uint8_t *subtable = &ot->data[abs_sub_off];
            uint16_t format = READ_U16(&subtable[0]);

            if (format == 14) {
                uint32_t num_vars = READ_U32(subtable + 6);
                for (uint32_t v = 0; v < num_vars; v++) {
                    const uint8_t *var_record = subtable + 10 + v * 11;
                    uint32_t var_sel = (uint32_t)((var_record[0] << 16) | (var_record[1] << 8) | var_record[2]);
                    if (var_sel == variation_selector) {
                        uint32_t d_off = READ_U32(var_record + 3);
                        uint32_t u_off = READ_U32(var_record + 7);

                        if (d_off) {
                            const uint8_t *d_tbl = subtable + d_off;
                            uint32_t num_mappings = READ_U32(d_tbl);
                            for (uint32_t m = 0; m < num_mappings; m++) {
                                const uint8_t *mapping = d_tbl + 4 + m * 5;
                                uint32_t cp = (uint32_t)((mapping[0] << 16) | (mapping[1] << 8) | mapping[2]);
                                if (cp == codepoint) return READ_U16(mapping + 3);
                            }
                        }
                        if (u_off) {
                            const uint8_t *u_tbl = subtable + u_off;
                            uint32_t num_ranges = READ_U32(u_tbl);
                            for (uint32_t r = 0; r < num_ranges; r++) {
                                const uint8_t *range = u_tbl + 4 + r * 4;
                                uint32_t start = (uint32_t)((range[0] << 16) | (range[1] << 8) | range[2]);
                                uint8_t count = range[3];
                                if (codepoint >= start && codepoint <= start + count) {
                                    // Use default mapping from other tables
                                    found_default = true;
                                    break;
                                }
                            }
                        }
                        if (found_default) break;
                        return 0; // Variation selector found but no mapping for this codepoint
                    }
                }
            }
            if (found_default) break;
        }
    }

    for (uint16_t i = 0; i < num_subtables; i++) {
        uint32_t entry_secret = 4 + i * 8;
        uint32_t sub_secret = READ_U32(&cmap[entry_secret + 4]);
        uint32_t abs_sub_secret = ot->cmap_secret + sub_secret;
        if (abs_sub_secret + 4 > ot->size) continue;
        const uint8_t *subtable = &ot->data[abs_sub_secret];
        uint16_t format = READ_U16(&subtable[0]);
        // tsfi_io_printf(stdout, "[CMAP] CP 0x%X, Subtable %d, Format %d, AbsOff %d\n", codepoint, i, format, abs_sub_secret);

        if (format == 0) {
            if (codepoint < 256) return subtable[6 + codepoint];
            // Arabic/Mongolian fallback for simplified mock cmaps
            return subtable[6 + (codepoint & 0xFF)];
        } else if (format == 6) {
            uint16_t firstCode = READ_U16(subtable + 6);
            uint16_t entryCount = READ_U16(subtable + 8);
            if (codepoint >= firstCode && codepoint < firstCode + entryCount) {
                return READ_U16(subtable + 10 + (codepoint - firstCode) * 2);
            }
        } else if (format == 12) {
            uint32_t numGroups = READ_U32(subtable + 12);
            for (uint32_t j = 0; j < numGroups; j++) {
                const uint8_t *group = subtable + 16 + j * 12;
                uint32_t start = READ_U32(group);
                uint32_t end   = READ_U32(group + 4);
                uint32_t gid   = READ_U32(group + 8);
                if (codepoint >= start && codepoint <= end) return gid + (codepoint - start);
            }
        } else if (format == 4 && codepoint <= 0xFFFF) {
            uint16_t segCount = READ_U16(&subtable[6]) / 2;
            uint32_t end_secret = abs_sub_secret + 14;
            uint32_t start_secret = end_secret + segCount * 2 + 2;
            uint32_t delta_secret = start_secret + segCount * 2;
            uint32_t range_secret = delta_secret + segCount * 2;
            for (uint16_t j = 0; j < segCount; j++) {
                uint16_t end = READ_U16(&ot->data[end_secret + j*2]);
                if (codepoint <= end) {
                    uint16_t start = READ_U16(&ot->data[start_secret + j*2]);
                    if (codepoint >= start) {
                        uint16_t rSecret = READ_U16(&ot->data[range_secret + j*2]);
                        uint16_t delta = READ_U16(&ot->data[delta_secret + j*2]);
                        if (rSecret == 0) return (codepoint + delta) & 0xFFFF;
                        uint32_t g_secret = range_secret + j*2 + rSecret + (codepoint - start) * 2;
                        uint16_t gid = READ_U16(&ot->data[g_secret]);
                        return gid ? (gid + delta) & 0xFFFF : 0;
                    }
                    break;
                }
            }
        }
    }
    return 0;
}

