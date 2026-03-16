#include "tsfi_ottype_internal.h"
#include "tsfi_cff.h"
#include "tsfi_io.h"

bool tsfi_ottype_get_glyph_metrics(tsfi_ottype_t *ot, uint32_t gid, int *adv, int *lsb) {
    if (!ot->hmtx_secret) return false;
    if (gid < ot->num_h_metrics) {
        *adv = READ_U16(&ot->data[ot->hmtx_secret + gid * 4]);
        *lsb = (int16_t)READ_U16(&ot->data[ot->hmtx_secret + gid * 4 + 2]);
    } else {
        *adv = READ_U16(&ot->data[ot->hmtx_secret + (ot->num_h_metrics - 1) * 4]);
        *lsb = (int16_t)READ_U16(&ot->data[ot->hmtx_secret + ot->num_h_metrics * 4 + (gid - ot->num_h_metrics) * 2]);
    }
    return true;
}

bool tsfi_ottype_get_vertical_glyph_metrics(tsfi_ottype_t *ot, uint32_t gid, int *adv, int *tsb) {
    if (!ot->vmtx_secret) return false;
    if (gid < ot->num_v_metrics) {
        *adv = READ_U16(&ot->data[ot->vmtx_secret + gid * 4]);
        *tsb = (int16_t)READ_U16(&ot->data[ot->vmtx_secret + gid * 4 + 2]);
    } else {
        *adv = READ_U16(&ot->data[ot->vmtx_secret + (ot->num_v_metrics - 1) * 4]);
        *tsb = (int16_t)READ_U16(&ot->data[ot->vmtx_secret + ot->num_v_metrics * 4 + (gid - ot->num_v_metrics) * 2]);
    }
    return true;
}

bool tsfi_ottype_get_cursive_anchors(tsfi_ottype_t *ot, uint32_t gid, float *entry_x, float *entry_y, float *exit_x, float *exit_y) {
    if (!ot->gpos_secret) return false;
    const uint8_t *gpos = &ot->data[ot->gpos_secret];
    uint16_t feat_idx = tsfi_ottype_get_feature_index(gpos, TAG('c','u','r','s'));
    if (feat_idx == 0xFFFF) return false;

    uint16_t f_secret = READ_U16(gpos + 6);
    const uint8_t *f_list = gpos + f_secret;
    const uint8_t *ftbl = f_list + READ_U16(f_list + 2 + feat_idx * 6 + 4);
    uint16_t l_secret = READ_U16(gpos + 8);
    const uint8_t *l_list = gpos + l_secret;
    
    uint16_t f_l_count = READ_U16(ftbl + 2);
    for (uint16_t i = 0; i < f_l_count; i++) {
        uint16_t l_idx = READ_U16(ftbl + 4 + i * 2);
        const uint8_t *lookup = l_list + READ_U16(l_list + 2 + l_idx * 2);
        uint16_t type = READ_U16(lookup);
        if (type != 3) continue; // Cursive Attachment is Type 3

        uint16_t sub_count = READ_U16(lookup + 4);
        for (uint16_t j = 0; j < sub_count; j++) {
            const uint8_t *subt = lookup + READ_U16(lookup + 6 + j * 2);
            uint16_t format = READ_U16(subt);
            if (format != 1) continue;

            uint16_t cov_secret = READ_U16(subt + 2);
            const uint8_t *cov = subt + cov_secret;
            uint16_t cov_fmt = READ_U16(cov);
            int cov_idx = -1;
            if (cov_fmt == 1) {
                uint16_t c_cnt = READ_U16(cov + 2);
                for (uint16_t k = 0; k < c_cnt; k++) {
                    if (READ_U16(cov + 4 + k * 2) == gid) { cov_idx = k; break; }
                }
            } else if (cov_fmt == 2) {
                uint16_t r_cnt = READ_U16(cov + 2);
                for (uint16_t k = 0; k < r_cnt; k++) {
                    uint16_t start = READ_U16(cov + 4 + k * 6);
                    uint16_t end = READ_U16(cov + 4 + k * 6 + 2);
                    if (gid >= start && gid <= end) {
                        cov_idx = READ_U16(cov + 4 + k * 6 + 4) + (gid - start);
                        break;
                    }
                }
            }

            if (cov_idx >= 0) {
                uint16_t entry_secret = READ_U16(subt + 6 + cov_idx * 4);
                uint16_t exit_secret = READ_U16(subt + 6 + cov_idx * 4 + 2);
                if (entry_secret) {
                    const uint8_t *anch = subt + entry_secret;
                    *entry_x = (int16_t)READ_U16(anch + 2);
                    *entry_y = (int16_t)READ_U16(anch + 4);
                }
                if (exit_secret) {
                    const uint8_t *anch = subt + exit_secret;
                    *exit_x = (int16_t)READ_U16(anch + 2);
                    *exit_y = (int16_t)READ_U16(anch + 4);
                }
                return true;
            }
        }
    }
    return false;
}

int tsfi_ottype_get_jstf_extenders(tsfi_ottype_t *ot, uint32_t script_tag, uint16_t *extender_gids, int max_extenders) {
    if (!ot->jstf_secret) return 0;
    const uint8_t *jstf = &ot->data[ot->jstf_secret];
    uint32_t version = READ_U32(jstf);
    if (version != 0x00010000) return 0;

    uint16_t script_count = READ_U16(jstf + 4);
    const uint8_t *script_record = jstf + 6;

    // tsfi_io_printf(stdout, "[JSTF_DEBUG] Version: 0x%08X, ScriptCount: %d, TargetTag: 0x%08X\n", version, script_count, script_tag);

    for (uint16_t i = 0; i < script_count; i++) {
        uint32_t tag = READ_U32(script_record);
        // tsfi_io_printf(stdout, "[JSTF_DEBUG] Checking script %d: Tag 0x%08X\n", i, tag);
        if (tag == script_tag) {
            uint16_t script_secret = READ_U16(script_record + 4);
            const uint8_t *script_tbl = jstf + script_secret;
            uint16_t extender_secret = READ_U16(script_tbl);
            // tsfi_io_printf(stdout, "[JSTF_DEBUG] Found script! ScriptOff: %d, ExtenderOff: %d\n", script_secret, extender_secret);
            if (extender_secret == 0) return 0;

            const uint8_t *extender_tbl = jstf + script_secret + extender_secret;
            uint16_t glyph_count = READ_U16(extender_tbl);
            // tsfi_io_printf(stdout, "[JSTF_DEBUG] ExtenderCount: %d\n", glyph_count);
            int count = (glyph_count < max_extenders) ? glyph_count : max_extenders;
            for (int j = 0; j < count; j++) {
                extender_gids[j] = READ_U16(extender_tbl + 2 + j * 2);
            }
            return count;
        }
        script_record += 6;
    }

    return 0;
}

int tsfi_ottype_justify(tsfi_ottype_t *ot, const uint32_t *cps, int count, float target_width, tsfi_shaped_glyph_t *s, int max) {
    if (count > 512) count = 512;
    tsfi_shaped_glyph_t temp[1024];
    int shaped_count = tsfi_ottype_shape(ot, cps, count, temp, 1024, NULL, 0);
    
    float current_width = 0;
    for (int i = 0; i < shaped_count; i++) current_width += temp[i].advance_dx;

    if (current_width >= target_width || shaped_count == 0) {
        int out = (shaped_count < max) ? shaped_count : max;
        memcpy(s, temp, out * sizeof(tsfi_shaped_glyph_t));
        return out;
    }

    // Identify script for JSTF
    uint32_t script_tag = TAG('D','F','L','T');
    for (int i=0; i<count; i++) {
        if (cps[i] >= 0x0600 && cps[i] <= 0x06FF) { script_tag = TAG('a','r','a','b'); break; }
        if (cps[i] >= 0x1800 && cps[i] <= 0x18AF) { script_tag = TAG('m','o','n','g'); break; }
    }

    uint16_t extenders[16];
    int ext_count = tsfi_ottype_get_jstf_extenders(ot, script_tag, extenders, 16);
    
    if (ext_count == 0 && script_tag == TAG('a','r','a','b')) {
        extenders[0] = tsfi_ottype_get_glyph_index(ot, 0x0640, 0);
        if (extenders[0] != 0) ext_count = 1;
    }

    if (ext_count == 0) {
        int out = (shaped_count < max) ? shaped_count : max;
        memcpy(s, temp, out * sizeof(tsfi_shaped_glyph_t));
        return out;
    }

    // Find insertion points using original indices stored in cluster_index
    int gap_indices[1024];
    int gap_count = 0;
    for (int i = 0; i < shaped_count - 1; i++) {
        int idx_cur = temp[i].cluster_index;
        int idx_next = temp[i+1].cluster_index;
        tsfi_joining_type_t cur = tsfi_ottype_get_joining_type(cps[idx_cur]);
        tsfi_joining_type_t next = tsfi_ottype_get_joining_type(cps[idx_next]);
        // tsfi_io_printf(stdout, "[JUSTIFY] Glyph %d (CP 0x%X, Type %d) -> Glyph %d (CP 0x%X, Type %d)\n", i, cps[idx_cur], cur, i+1, cps[idx_next], next);
        if ((cur == TSFI_JOIN_D || cur == TSFI_JOIN_C) && (next == TSFI_JOIN_D || next == TSFI_JOIN_R || next == TSFI_JOIN_C)) {
            gap_indices[gap_count++] = i;
        }
    }

    if (gap_count == 0) {
        int out = (shaped_count < max) ? shaped_count : max;
        memcpy(s, temp, out * sizeof(tsfi_shaped_glyph_t));
        return out;
    }

    float ext_width = 0;
    int ext_adv, ext_lsb;
    tsfi_ottype_get_glyph_metrics(ot, extenders[0], &ext_adv, &ext_lsb);
    ext_width = (float)ext_adv;

    float needed = target_width - current_width;
    int num_ext_total = (int)(needed / ext_width);
    if (num_ext_total <= 0) num_ext_total = 1;

    int ext_per_gap = num_ext_total / gap_count;
    int extra = num_ext_total % gap_count;

    int total_out = 0;
    for (int i = 0; i < shaped_count; i++) {
        if (total_out < max) {
            memcpy(&s[total_out], &temp[i], sizeof(tsfi_shaped_glyph_t));
            total_out++;
        }
        
        bool is_gap = false;
        for (int g = 0; g < gap_count; g++) { if (gap_indices[g] == i) { is_gap = true; break; } }
        
        if (is_gap) {
            int to_add = ext_per_gap + (extra-- > 0 ? 1 : 0);
            for (int k = 0; k < to_add && total_out < max; k++) {
                s[total_out].glyph_index = extenders[0];
                s[total_out].advance_dx = ext_width;
                s[total_out].advance_dy = 0.0f;
                s[total_out].cluster_index = temp[i].cluster_index;
                s[total_out].transform = (tsfi_transform_t){ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
                total_out++;
            }
        }
    }

    return total_out;
}

