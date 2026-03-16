#include "tsfi_ottype_internal.h"
#include "tsfi_cff.h"

tsfi_joining_type_t tsfi_ottype_get_joining_type(uint32_t cp) {
    // Basic Arabic Joining Types (U+0600 - U+06FF)
    if (cp >= 0x0622 && cp <= 0x0625) return TSFI_JOIN_R; // Alef with ...
    if (cp == 0x0627) return TSFI_JOIN_R; // Alef
    if (cp == 0x0628) return TSFI_JOIN_D; // Beh
    if (cp == 0x062A) return TSFI_JOIN_D; // Teh
    if (cp == 0x062B) return TSFI_JOIN_D; // Theh
    if (cp == 0x062C) return TSFI_JOIN_D; // Jeem
    if (cp == 0x062D) return TSFI_JOIN_D; // Hah
    if (cp == 0x062E) return TSFI_JOIN_D; // Khah
    if (cp == 0x062F) return TSFI_JOIN_R; // Dal
    if (cp == 0x0630) return TSFI_JOIN_R; // Thal
    if (cp == 0x0631) return TSFI_JOIN_R; // Reh
    if (cp == 0x0632) return TSFI_JOIN_R; // Zain
    if (cp >= 0x0633 && cp <= 0x063A) return TSFI_JOIN_D; // Seen to Ghain
    if (cp >= 0x0641 && cp <= 0x0647) return TSFI_JOIN_D; // Feh to Heh
    if (cp == 0x0648) return TSFI_JOIN_R; // Waw
    if (cp == 0x0649 || cp == 0x064A) return TSFI_JOIN_D; // Alef Maksura, Yeh
    if (cp == 0x0640) return TSFI_JOIN_C; // Tatweel
    
    // Mongolian Joining Types (U+1800 - U+18AF) - Simplified
    if (cp >= 0x1820 && cp <= 0x1842) return TSFI_JOIN_D; 
    
    return TSFI_JOIN_U;
}

void tsfi_ottype_calculate_joining(const uint32_t *cps, int count, uint32_t *features) {
    for (int i = 0; i < count; i++) {
        tsfi_joining_type_t cur = tsfi_ottype_get_joining_type(cps[i]);
        if (cur == TSFI_JOIN_U) {
            features[i] = TAG('i','s','o','l');
            continue;
        }

        bool join_prev = false;
        if (i > 0) {
            tsfi_joining_type_t prev = tsfi_ottype_get_joining_type(cps[i-1]);
            if (prev == TSFI_JOIN_D || prev == TSFI_JOIN_C) join_prev = true;
        }

        bool join_next = false;
        if (i < count - 1) {
            tsfi_joining_type_t next = tsfi_ottype_get_joining_type(cps[i+1]);
            if (next == TSFI_JOIN_D || next == TSFI_JOIN_R || next == TSFI_JOIN_C) join_next = true;
        }

        if (join_prev && join_next) features[i] = TAG('m','e','d','i');
        else if (join_prev) features[i] = TAG('f','i','n','a');
        else if (join_next && cur != TSFI_JOIN_R) features[i] = TAG('i','n','i','t');
        else features[i] = TAG('i','s','o','l');
    }
}

uint16_t tsfi_ottype_get_feature_index(const uint8_t *tbl, uint32_t tag) {
    uint16_t f_secret = READ_U16(tbl + 6);
    if (!f_secret) return 0xFFFF;
    const uint8_t *f_list = tbl + f_secret;
    uint16_t count = READ_U16(f_list);
    for (uint16_t i = 0; i < count; i++) {
        if (READ_U32(f_list + 2 + i * 6) == tag) return i;
    }
    return 0xFFFF;
}

int tsfi_ottype_check_coverage(const uint8_t *cov, uint32_t gid) {
    uint16_t cov_fmt = READ_U16(cov);
    if (cov_fmt == 1) {
        uint16_t c_cnt = READ_U16(cov + 2);
        for (uint16_t k = 0; k < c_cnt; k++) {
            uint16_t target = READ_U16(cov + 4 + k * 2);
            if (target == gid) return k;
        }
    } else if (cov_fmt == 2) {
        uint16_t r_cnt = READ_U16(cov + 2);
        for (uint16_t k = 0; k < r_cnt; k++) {
            uint16_t start = READ_U16(cov + 4 + k * 6);
            uint16_t end = READ_U16(cov + 4 + k * 6 + 2);
            if (gid >= start && gid <= end) {
                return READ_U16(cov + 4 + k * 6 + 4) + (gid - start);
            }
        }
    }
    return -1;
}

uint16_t tsfi_ottype_get_feature_index(const uint8_t *tbl, uint32_t tag);
static int tsfi_ottype_apply_lookup(tsfi_ottype_t *ot, const uint8_t *gsub, uint16_t l_idx, uint32_t *gids, int *count, int pos, int *indices);

uint16_t tsfi_ottype_get_class(const uint8_t *class_def, uint32_t gid) {
    uint16_t format = READ_U16(class_def);
    if (format == 1) {
        uint16_t start = READ_U16(class_def + 2);
        uint16_t count = READ_U16(class_def + 4);
        if (gid >= start && gid < start + (uint32_t)count) {
            return READ_U16(class_def + 6 + (gid - start) * 2);
        }
    } else if (format == 2) {
        uint16_t count = READ_U16(class_def + 2);
        for (uint16_t i = 0; i < count; i++) {
            uint16_t start = READ_U16(class_def + 4 + i * 6);
            uint16_t end = READ_U16(class_def + 4 + i * 6 + 2);
            if (gid >= start && gid <= end) {
                return READ_U16(class_def + 4 + i * 6 + 4);
            }
        }
    }
    return 0;
}

int tsfi_ottype_apply_chain_context_format2(tsfi_ottype_t *ot, const uint8_t *gsub, const uint8_t *subt, uint32_t *gids, int *count, int pos, int *indices) {
    uint16_t cov_secret = READ_U16(subt + 2);
    if (tsfi_ottype_check_coverage(subt + cov_secret, gids[pos]) < 0) return 0;

    uint16_t back_class_secret = READ_U16(subt + 4);
    uint16_t input_class_secret = READ_U16(subt + 6);
    uint16_t look_class_secret = READ_U16(subt + 8);
    uint16_t chain_count = READ_U16(subt + 10);

    for (uint16_t i = 0; i < chain_count; i++) {
        uint16_t rule_set_secret = READ_U16(subt + 12 + i * 2);
        if (!rule_set_secret) continue;
        const uint8_t *rule_set = subt + rule_set_secret;
        uint16_t rule_count = READ_U16(rule_set);
        for (uint16_t j = 0; j < rule_count; j++) {
            uint16_t rule_secret = READ_U16(rule_set + 2 + j * 2);
            const uint8_t *rule = rule_set + rule_secret;
            
            uint16_t backtrack_count = READ_U16(rule);
            if (pos < backtrack_count) continue;
            const uint8_t *ptr = rule + 2;
            bool match = true;
            for (uint16_t k = 0; k < backtrack_count; k++) {
                if (tsfi_ottype_get_class(subt + back_class_secret, gids[pos - backtrack_count + k]) != READ_U16(ptr)) { match = false; break; }
                ptr += 2;
            }
            if (!match) continue;

            uint16_t input_count = READ_U16(ptr); ptr += 2;
            if (pos + input_count > *count) continue;
            for (uint16_t k = 1; k < input_count; k++) { // Input[0] is matched by coverage
                if (tsfi_ottype_get_class(subt + input_class_secret, gids[pos + k]) != READ_U16(ptr)) { match = false; break; }
                ptr += 2;
            }
            if (!match) continue;

            uint16_t lookahead_count = READ_U16(ptr); ptr += 2;
            if (pos + input_count + lookahead_count > *count) continue;
            for (uint16_t k = 0; k < lookahead_count; k++) {
                if (tsfi_ottype_get_class(subt + look_class_secret, gids[pos + input_count + k]) != READ_U16(ptr)) { match = false; break; }
                ptr += 2;
            }
            if (!match) continue;

            uint16_t subst_count = READ_U16(ptr); ptr += 2;
            for (uint16_t k = 0; k < subst_count; k++) {
                uint16_t seq_idx = READ_U16(ptr + k * 4);
                uint16_t lookup_idx = READ_U16(ptr + k * 4 + 2);
                tsfi_ottype_apply_lookup(ot, gsub, lookup_idx, gids, count, pos + seq_idx, indices);
            }
            return 1;
        }
    }
    return 0;
}

int tsfi_ottype_apply_ligature(tsfi_ottype_t *ot, const uint8_t *subt, uint32_t *gids, int count, int pos, int *indices) {
    (void)ot;
    uint16_t cov_secret = READ_U16(subt + 2);
    int cov_idx = tsfi_ottype_check_coverage(subt + cov_secret, gids[pos]);
    if (cov_idx < 0) return 0;

    uint16_t lig_set_count = READ_U16(subt + 4);
    if (cov_idx >= lig_set_count) return 0;

    uint16_t lig_set_secret = READ_U16(subt + 6 + cov_idx * 2);
    const uint8_t *lig_set = subt + lig_set_secret;
    uint16_t lig_count = READ_U16(lig_set);

    for (uint16_t i = 0; i < lig_count; i++) {
        uint16_t lig_secret = READ_U16(lig_set + 2 + i * 2);
        const uint8_t *lig = lig_set + lig_secret;
        uint16_t lig_glyph = READ_U16(lig);
        uint16_t comp_count = READ_U16(lig + 2);

        if (pos + comp_count <= count) {
            bool match = true;
            for (uint16_t j = 1; j < comp_count; j++) {
                if (gids[pos + j] != READ_U16(lig + 4 + (j - 1) * 2)) {
                    match = false;
                    break;
                }
            }
            if (match) {
                gids[pos] = lig_glyph;
                // Shift remaining glyphs and indices
                for (int j = pos + 1; j < count - (comp_count - 1); j++) {
                    gids[j] = gids[j + comp_count - 1];
                    if (indices) indices[j] = indices[j + comp_count - 1];
                }
                return comp_count;
            }
        }
    }
    return 0;
}

int tsfi_ottype_apply_chain_context_format3(tsfi_ottype_t *ot, const uint8_t *gsub, const uint8_t *subt, uint32_t *gids, int *count, int pos, int *indices) {
    uint16_t backtrack_count = READ_U16(subt + 2);
    if (pos < backtrack_count) return 0;
    for (uint16_t i = 0; i < backtrack_count; i++) {
        uint16_t cov_secret = READ_U16(subt + 4 + i * 2);
        if (tsfi_ottype_check_coverage(subt + cov_secret, gids[pos - backtrack_count + i]) < 0) return 0;
    }

    const uint8_t *ptr = subt + 4 + backtrack_count * 2;
    uint16_t input_count = READ_U16(ptr); ptr += 2;
    if (pos + input_count > *count) return 0;
    for (uint16_t i = 0; i < input_count; i++) {
        uint16_t cov_secret = READ_U16(ptr + i * 2);
        if (tsfi_ottype_check_coverage(subt + cov_secret, gids[pos + i]) < 0) return 0;
    }
    ptr += input_count * 2;

    uint16_t lookahead_count = READ_U16(ptr); ptr += 2;
    if (pos + input_count + lookahead_count > *count) return 0;
    for (uint16_t i = 0; i < lookahead_count; i++) {
        uint16_t cov_secret = READ_U16(ptr + i * 2);
        if (tsfi_ottype_check_coverage(subt + cov_secret, gids[pos + input_count + i]) < 0) return 0;
    }
    ptr += lookahead_count * 2;

    uint16_t subst_count = READ_U16(ptr); ptr += 2;
    for (uint16_t i = 0; i < subst_count; i++) {
        uint16_t seq_idx = READ_U16(ptr + i * 4);
        uint16_t lookup_idx = READ_U16(ptr + i * 4 + 2);
        tsfi_ottype_apply_lookup(ot, gsub, lookup_idx, gids, count, pos + seq_idx, indices);
    }
    return 1;
}

int tsfi_ottype_apply_lookup(tsfi_ottype_t *ot, const uint8_t *gsub, uint16_t l_idx, uint32_t *gids, int *count, int pos, int *indices) {
    uint16_t l_secret_base = READ_U16(gsub + 8);
    const uint8_t *l_list = gsub + l_secret_base;
    const uint8_t *lookup = l_list + READ_U16(l_list + 2 + l_idx * 2);
    uint16_t type = READ_U16(lookup);
    uint16_t sub_count = READ_U16(lookup + 4);

    for (uint16_t j = 0; j < sub_count; j++) {
        const uint8_t *subt = lookup + READ_U16(lookup + 6 + j * 2);
        uint16_t real_type = type;
        const uint8_t *real_subt = subt;
        
        if (type == 7) { // Extension
            real_type = READ_U16(subt + 2);
            real_subt = subt + READ_U32(subt + 4);
        }

        if (real_type == 1) { // Single
            uint16_t format = READ_U16(real_subt);
            int cov_idx = tsfi_ottype_check_coverage(real_subt + READ_U16(real_subt + 2), gids[pos]);
            if (cov_idx >= 0) {
                if (format == 1) gids[pos] = (gids[pos] + (int16_t)READ_U16(real_subt + 4)) & 0xFFFF;
                else if (format == 2) gids[pos] = READ_U16(real_subt + 6 + cov_idx * 2);
                return 1;
            }
        } else if (real_type == 4) { // Ligature
            int matched = tsfi_ottype_apply_ligature(ot, real_subt, gids, *count, pos, indices);
            if (matched > 0) {
                *count -= (matched - 1);
                return 1;
            }
        } else if (real_type == 6) { // Chain Context
            uint16_t format = READ_U16(real_subt);
            if (format == 2) {
                if (tsfi_ottype_apply_chain_context_format2(ot, gsub, real_subt, gids, count, pos, indices)) return 1;
            } else if (format == 3) {
                if (tsfi_ottype_apply_chain_context_format3(ot, gsub, real_subt, gids, count, pos, indices)) return 1;
            }
        }
    }
    return 0;
}

int tsfi_ottype_shape(tsfi_ottype_t *ot, const uint32_t *cps, int count, tsfi_shaped_glyph_t *s, int max, const uint32_t *opt_features, int num_opt_features) {
    if (count > 1024) count = 1024;
    uint32_t gids[1024];
    uint32_t join_features[1024] = {0};
    int indices[1024];
    for (int i=0; i<count; i++) indices[i] = i;
    
    // Identify script
    bool is_arabic = false;
    for (int i=0; i<count; i++) { if (cps[i] >= 0x0600 && cps[i] <= 0x06FF) { is_arabic = true; break; } }

    if (is_arabic) {
        tsfi_ottype_calculate_joining(cps, count, join_features);
    }

    for (int i = 0; i < count; i++) gids[i] = tsfi_ottype_get_glyph_index(ot, cps[i], 0);

    if (ot->gsub_secret) {
        const uint8_t *gsub = &ot->data[ot->gsub_secret];
        uint32_t tags[64];
        int num_tags = 0;

        if (is_arabic) {
            tags[num_tags++] = TAG('c','c','m','p');
            tags[num_tags++] = TAG('l','o','c','l');
            tags[num_tags++] = TAG('i','n','i','t');
            tags[num_tags++] = TAG('m','e','d','i');
            tags[num_tags++] = TAG('f','i','n','a');
            tags[num_tags++] = TAG('i','s','o','l');
            tags[num_tags++] = TAG('r','l','i','g');
            tags[num_tags++] = TAG('r','t','l','m');
            tags[num_tags++] = TAG('l','i','g','a');
            tags[num_tags++] = TAG('m','s','e','t');
        } else {
            tags[num_tags++] = TAG('c','c','m','p');
            tags[num_tags++] = TAG('l','o','c','l');
            tags[num_tags++] = TAG('l','i','g','a');
            tags[num_tags++] = TAG('c','l','i','g');
            tags[num_tags++] = TAG('d','l','i','g');
        }

        // Add common numeric and math features
        tags[num_tags++] = TAG('f','r','a','c');
        tags[num_tags++] = TAG('n','u','m','r');
        tags[num_tags++] = TAG('d','n','o','m');
        tags[num_tags++] = TAG('s','u','p','s');
        tags[num_tags++] = TAG('s','u','b','s');
        tags[num_tags++] = TAG('o','r','d','n');
        tags[num_tags++] = TAG('l','n','u','m');
        tags[num_tags++] = TAG('o','n','u','m');
        tags[num_tags++] = TAG('p','n','u','m');
        tags[num_tags++] = TAG('t','n','u','m');
        tags[num_tags++] = TAG('z','e','r','o');
        tags[num_tags++] = TAG('v','e','r','t');
        tags[num_tags++] = TAG('v','r','t','2');
        tags[num_tags++] = TAG('m','e','d','2');
        tags[num_tags++] = TAG('f','i','n','2');
        tags[num_tags++] = TAG('f','i','n','3');
        tags[num_tags++] = TAG('l','t','r','m');
        tags[num_tags++] = TAG('f','w','i','d');

        if (opt_features && num_opt_features > 0) {
            for (int i = 0; i < num_opt_features && num_tags < 64; i++) {
                tags[num_tags++] = opt_features[i];
            }
        }
        
        for (int t = 0; t < num_tags; t++) {
            uint16_t f_idx = tsfi_ottype_get_feature_index(gsub, tags[t]);
            if (f_idx == 0xFFFF) continue;

            uint16_t f_secret = READ_U16(gsub + 6);
            const uint8_t *f_list = gsub + f_secret;
            const uint8_t *ftbl = f_list + READ_U16(f_list + 2 + f_idx * 6 + 4);
            uint16_t f_l_count = READ_U16(ftbl + 2);

            for (uint16_t i = 0; i < f_l_count; i++) {
                uint16_t l_idx = READ_U16(ftbl + 4 + i * 2);
                for (int p = 0; p < count; p++) {
                    if (is_arabic && (tags[t] == TAG('i','n','i','t') || tags[t] == TAG('m','e','d','i') || 
                                      tags[t] == TAG('f','i','n','a') || tags[t] == TAG('i','s','o','l'))) {
                        if (join_features[indices[p]] != tags[t]) continue;
                    }
                    tsfi_ottype_apply_lookup(ot, gsub, l_idx, gids, &count, p, indices);
                }
            }
        }
    }

    int total = 0;
    for (int i = 0; i < count && total < max; i++) {
        s[total].glyph_index = gids[i];
        int a, l;
        
        s[total].transform = (tsfi_transform_t){ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
        
        if (tsfi_ottype_get_glyph_metrics(ot, gids[i], &a, &l)) s[total].advance_dx = (float)a;
        else s[total].advance_dx = 0;
        
        s[total].advance_dy = 0.0f;
        s[total].cluster_index = (uint32_t)indices[i]; 
        
        total++;
    }
    return total;
}

