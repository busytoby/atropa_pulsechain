#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int tsfi_cw_pack_comp3(const char *ascii_num, uint8_t *comp3_out, int max_out_len, int *out_len) {
    if (!ascii_num || !comp3_out || !out_len) return -1;
    int len = strlen(ascii_num);
    if (len == 0) return -2;

    int is_negative = 0;
    int start_idx = 0;
    if (ascii_num[0] == '-') {
        is_negative = 1;
        start_idx = 1;
    } else if (ascii_num[0] == '+') {
        start_idx = 1;
    }

    int digits_len = len - start_idx;
    int total_nibbles = digits_len + 1; // digits + sign nibble
    int required_bytes = (total_nibbles + 1) / 2;
    if (required_bytes > max_out_len) return -3;

    memset(comp3_out, 0, required_bytes);
    *out_len = required_bytes;

    int digit_pos = 0;
    if (digits_len % 2 == 0) {
        digit_pos = 1; // leading zero pad
    }

    for (int i = start_idx; i < len; i++) {
        char c = ascii_num[i];
        if (c < '0' || c > '9') return -4;
        uint8_t digit = c - '0';
        int byte_idx = digit_pos / 2;
        int nibble_idx = digit_pos % 2;
        if (nibble_idx == 0) {
            comp3_out[byte_idx] |= (digit << 4);
        } else {
            comp3_out[byte_idx] |= digit;
        }
        digit_pos++;
    }

    // Set sign nibble: positive standard C, negative standard D
    int sign_byte = required_bytes - 1;
    if (is_negative) {
        comp3_out[sign_byte] |= 0x0D;
    } else {
        comp3_out[sign_byte] |= 0x0C;
    }
    return 0;
}

int tsfi_cw_unpack_comp3(const uint8_t *comp3_in, int comp3_len, char *ascii_out, int max_ascii_len) {
    if (!comp3_in || comp3_len <= 0 || !ascii_out || max_ascii_len <= 0) return -1;

    int total_nibbles = comp3_len * 2;
    int digits_len = total_nibbles - 1;
    if (digits_len + 2 > max_ascii_len) return -2; // sign + digits + null

    int out_idx = 0;
    int is_signed = 0;
    int is_negative = 0;

    // Check last nibble for sign
    uint8_t sign_nibble = comp3_in[comp3_len - 1] & 0x0F;
    if (sign_nibble == 0x0B || sign_nibble == 0x0D) {
        is_signed = 1;
        is_negative = 1;
    } else if (sign_nibble == 0x0A || sign_nibble == 0x0C || sign_nibble == 0x0F) {
        is_signed = 1;
    }

    if (!is_signed) return -3;

    if (is_negative) {
        ascii_out[out_idx++] = '-';
    }

    int skip_leading_zero = 1;
    for (int i = 0; i < digits_len; i++) {
        int byte_idx = i / 2;
        int nibble_idx = i % 2;
        uint8_t val = comp3_in[byte_idx];
        uint8_t digit = (nibble_idx == 0) ? (val >> 4) : (val & 0x0F);
        
        if (skip_leading_zero && digit == 0 && i < digits_len - 1) {
            continue; // strip leading zero
        }
        skip_leading_zero = 0;
        
        if (digit > 9) return -4;
        ascii_out[out_idx++] = '0' + digit;
    }

    ascii_out[out_idx] = '\0';
    return 0;
}

int tsfi_cw_parse_copybook_line(const char *copybook_line, tsfi_cw_copybook *cb) {
    if (!copybook_line || !cb) return -1;
    if (cb->field_count >= 32) return -2;

    const char *p = copybook_line;
    while (*p == ' ' || *p == '\t') p++;
    
    if (*p < '0' || *p > '9') return -3;
    int level = 0;
    while (*p >= '0' && *p <= '9') {
        level = level * 10 + (*p - '0');
        p++;
    }
    while (*p == ' ' || *p == '\t') p++;

    char name[32];
    int name_idx = 0;
    while (*p && *p != ' ' && *p != '\t' && *p != '.') {
        if (name_idx < 31) name[name_idx++] = *p;
        p++;
    }
    name[name_idx] = '\0';
    if (name_idx == 0) return -4;
    while (*p == ' ' || *p == '\t') p++;

    if (level == 66) {
        char ren_start[32] = "";
        char ren_end[32] = "";
        const char *ren_ptr = strstr(p, "RENAMES");
        if (ren_ptr) {
            ren_ptr += 7;
            while (*ren_ptr == ' ' || *ren_ptr == '\t') ren_ptr++;
            int s_idx = 0;
            while (*ren_ptr && *ren_ptr != ' ' && *ren_ptr != '\t' && *ren_ptr != '.' && s_idx < 31) {
                ren_start[s_idx++] = *ren_ptr++;
            }
            ren_start[s_idx] = '\0';
            
            while (*ren_ptr == ' ' || *ren_ptr == '\t') ren_ptr++;
            if (strncmp(ren_ptr, "THRU", 4) == 0 || strncmp(ren_ptr, "THROUGH", 7) == 0) {
                if (strncmp(ren_ptr, "THRU", 4) == 0) ren_ptr += 4;
                else ren_ptr += 7;
                while (*ren_ptr == ' ' || *ren_ptr == '\t') ren_ptr++;
                int e_idx = 0;
                while (*ren_ptr && *ren_ptr != ' ' && *ren_ptr != '\t' && *ren_ptr != '.' && e_idx < 31) {
                    ren_end[e_idx++] = *ren_ptr++;
                }
                ren_end[e_idx] = '\0';
            }
        }
        
        tsfi_cw_cobol_field *f = &cb->fields[cb->field_count];
        f->level = 66;
        snprintf(f->name, sizeof(f->name), "%s", name);
        f->type = COBOL_TYPE_ALPHA;
        f->usage = COBOL_USAGE_DISPLAY;
        f->length = 0;
        f->offset = 0;
        f->occurs = 1;
        f->value[0] = '\0';
        f->redefines[0] = '\0';
        f->depending_on[0] = '\0';
        f->indexed_by[0] = '\0';
        strcpy(f->renames_start, ren_start);
        strcpy(f->renames_end, ren_end);
        
        int start_offset = -1, end_offset = -1;
        for (int i = 0; i < cb->field_count; i++) {
            if (strcmp(cb->fields[i].name, ren_start) == 0) {
                start_offset = cb->fields[i].offset;
            }
            if (strlen(ren_end) > 0 && strcmp(cb->fields[i].name, ren_end) == 0) {
                end_offset = cb->fields[i].offset + cb->fields[i].length;
            }
        }
        if (start_offset != -1) {
            f->offset = start_offset;
            if (end_offset != -1) {
                f->length = end_offset - start_offset;
            } else {
                for (int i = 0; i < cb->field_count; i++) {
                    if (strcmp(cb->fields[i].name, ren_start) == 0) {
                        f->length = cb->fields[i].length;
                        break;
                    }
                }
            }
        }
        cb->field_count++;
        return 0;
    }

    char redefines_target[32] = "";
    if (strncmp(p, "REDEFINES", 9) == 0) {
        p += 9;
        while (*p == ' ' || *p == '\t') p++;
        int r_idx = 0;
        while (*p && *p != ' ' && *p != '\t' && *p != '.' && r_idx < 31) {
            redefines_target[r_idx++] = *p++;
        }
        redefines_target[r_idx] = '\0';
    }
    while (*p == ' ' || *p == '\t') p++;

    tsfi_cw_cobol_type type = COBOL_TYPE_ALPHA;
    int length = 0;
    int decimal_places = 0;
    
    const char *pic = strstr(p, "PIC ");
    if (!pic) pic = strstr(p, "PICTURE ");
    
    if (pic) {
        if (strncmp(pic, "PIC ", 4) == 0) p = pic + 4;
        else p = pic + 8;
        
        while (*p == ' ' || *p == '\t') p++;
        
        if (*p == 'S') p++; // signed indicator
        
        if (*p == 'X') {
            type = COBOL_TYPE_ALPHA;
            p++;
            if (*p == '(') {
                p++;
                length = atoi(p);
            } else {
                length = 1;
            }
        } else if (*p == '9') {
            type = COBOL_TYPE_NUMERIC;
            p++;
            if (*p == '(') {
                p++;
                length = atoi(p);
                while (*p && *p != ')') p++;
                if (*p == ')') p++;
            } else {
                length = 1;
                while (*p == '9') { length++; p++; }
            }
            
            if (*p == 'V') {
                p++;
                if (*p == '9') {
                    p++;
                    if (*p == '(') {
                        p++;
                        decimal_places = atoi(p);
                    } else {
                        decimal_places = 1;
                        while (*p == '9') { decimal_places++; p++; }
                    }
                }
            }
        }
    }

    char value_clause[32] = "";
    const char *val_ptr = strstr(copybook_line, "VALUE '");
    if (val_ptr) {
        val_ptr += 7;
        int v_idx = 0;
        while (*val_ptr && *val_ptr != '\'' && v_idx < 31) {
            value_clause[v_idx++] = *val_ptr++;
        }
        value_clause[v_idx] = '\0';
    }

    int occurs = 1;
    const char *occ_ptr = strstr(copybook_line, "OCCURS ");
    if (occ_ptr) {
        occ_ptr += 7;
        while (*occ_ptr == ' ' || *occ_ptr == '\t') occ_ptr++;
        occurs = atoi(occ_ptr);
        if (occurs <= 0) occurs = 1;
    }

    while (*p && *p != '.') {
        if (strncmp(p, "COMP-5", 6) == 0) {
            type = COBOL_TYPE_COMP5;
            p += 6;
        } else if (strncmp(p, "COMP-3", 6) == 0) {
            type = COBOL_TYPE_COMP3;
            p += 6;
        } else if (strncmp(p, "COMP", 4) == 0) {
            type = COBOL_TYPE_COMP3;
            p += 4;
        } else {
            p++;
        }
    }

    tsfi_cw_cobol_usage usage = COBOL_USAGE_DISPLAY;
    if (strstr(copybook_line, "USAGE IS COMP-3") || strstr(copybook_line, "USAGE COMP-3")) {
        usage = COBOL_USAGE_COMP3;
        type = COBOL_TYPE_COMP3;
    } else if (strstr(copybook_line, "USAGE IS COMP-5") || strstr(copybook_line, "USAGE COMP-5")) {
        usage = COBOL_USAGE_COMP5;
        type = COBOL_TYPE_COMP5;
    } else if (strstr(copybook_line, "USAGE IS COMP") || strstr(copybook_line, "USAGE COMP")) {
        usage = COBOL_USAGE_COMP;
        type = COBOL_TYPE_COMP5;
    } else if (strstr(copybook_line, "USAGE IS DISPLAY") || strstr(copybook_line, "USAGE DISPLAY")) {
        usage = COBOL_USAGE_DISPLAY;
    }

    char depending_on[32] = "";
    const char *dep_ptr = strstr(copybook_line, "DEPENDING ON");
    if (dep_ptr) {
        dep_ptr += 12;
        while (*dep_ptr == ' ' || *dep_ptr == '\t') dep_ptr++;
        int d_idx = 0;
        while (*dep_ptr && *dep_ptr != ' ' && *dep_ptr != '\t' && *dep_ptr != '.' && d_idx < 31) {
            depending_on[d_idx++] = *dep_ptr++;
        }
        depending_on[d_idx] = '\0';
    }

    char indexed_by[32] = "";
    const char *idx_ptr = strstr(copybook_line, "INDEXED BY");
    if (idx_ptr) {
        idx_ptr += 10;
        while (*idx_ptr == ' ' || *idx_ptr == '\t') idx_ptr++;
        int i_idx = 0;
        while (*idx_ptr && *idx_ptr != ' ' && *idx_ptr != '\t' && *idx_ptr != '.' && i_idx < 31) {
            indexed_by[i_idx++] = *idx_ptr++;
        }
        indexed_by[i_idx] = '\0';
    }

    int blank_when_zero = 0;
    if (strstr(copybook_line, "BLANK WHEN ZERO") || strstr(copybook_line, "BLANK ZERO")) {
        blank_when_zero = 1;
    }

    int justified_right = 0;
    if (strstr(copybook_line, "JUSTIFIED RIGHT") || strstr(copybook_line, "JUST RIGHT")) {
        justified_right = 1;
    }

    int sync_align = 0;
    if (strstr(copybook_line, "SYNCHRONIZED") || strstr(copybook_line, "SYNC")) {
        sync_align = 4;
    }

    int sign_leading = 0;
    int sign_separate = 0;
    if (strstr(copybook_line, "SIGN IS LEADING") || strstr(copybook_line, "SIGN LEADING")) {
        sign_leading = 1;
        if (strstr(copybook_line, "SEPARATE")) sign_separate = 1;
    } else if (strstr(copybook_line, "SIGN IS TRAILING") || strstr(copybook_line, "SIGN TRAILING")) {
        if (strstr(copybook_line, "SEPARATE")) sign_separate = 1;
    }

    tsfi_cw_cobol_field *f = &cb->fields[cb->field_count];
    f->level = level;
    snprintf(f->name, sizeof(f->name), "%s", name);
    f->type = type;
    f->usage = usage;
    snprintf(f->value, sizeof(f->value), "%s", value_clause);
    snprintf(f->redefines, sizeof(f->redefines), "%s", redefines_target);
    f->occurs = occurs;
    snprintf(f->depending_on, sizeof(f->depending_on), "%s", depending_on);
    snprintf(f->indexed_by, sizeof(f->indexed_by), "%s", indexed_by);
    f->blank_when_zero = blank_when_zero;
    f->justified_right = justified_right;
    f->sync_align = sync_align;
    f->sign_leading = sign_leading;
    f->sign_separate = sign_separate;
    
    int byte_len = length + decimal_places;
    int base_length = 0;
    if (type == COBOL_TYPE_COMP3) {
        base_length = (byte_len / 2) + 1;
    } else if (type == COBOL_TYPE_COMP5) {
        if (byte_len <= 4) {
            base_length = 2;
        } else if (byte_len <= 9) {
            base_length = 4;
        } else {
            base_length = 8;
        }
    } else {
        base_length = byte_len;
    }
    f->length = base_length * occurs;
    f->decimal_places = decimal_places;
    if (justified_right && strlen(value_clause) > (size_t)f->length) {
        return -11;
    }
    
    int final_offset = cb->record_length;
    if (strlen(redefines_target) > 0) {
        int found_target = 0;
        for (int i = 0; i < cb->field_count; i++) {
            if (strcmp(cb->fields[i].name, redefines_target) == 0) {
                found_target = 1;
                if (f->length > cb->fields[i].length) {
                    return -8;
                }
                final_offset = cb->fields[i].offset;
                break;
            }
        }
        if (!found_target) return -9;
    }
    if (sync_align > 0 && final_offset % sync_align != 0) {
        final_offset += (sync_align - (final_offset % sync_align));
    }
    f->offset = final_offset;

    if (strlen(redefines_target) == 0) {
        cb->record_length += f->length;
    }
    cb->field_count++;
    return 0;
}

uint32_t tsfi_cw_ieee_to_ibm_float(float ieee_float) {
    if (ieee_float == 0.0f) return 0;
    uint32_t sign = 0;
    if (ieee_float < 0.0f) {
        sign = 1;
        ieee_float = -ieee_float;
    }
    
    int exponent = 64;
    double val = (double)ieee_float;
    if (val >= 1.0) {
        while (val >= 1.0 && exponent < 127) {
            val /= 16.0;
            exponent++;
        }
    } else {
        while (val < 0.0625 && exponent > 0) {
            val *= 16.0;
            exponent--;
        }
    }
    
    uint32_t fraction = (uint32_t)(val * 16777216.0 + 0.5);
    if (fraction >= 0x01000000) {
        fraction >>= 4;
        exponent++;
    }
    return (sign << 31) | ((uint32_t)exponent << 24) | (fraction & 0x00FFFFFF);
}

float tsfi_cw_ibm_to_ieee_float(uint32_t ibm_float) {
    if (ibm_float == 0) return 0.0f;
    uint32_t sign = (ibm_float >> 31) & 1;
    int exponent = (int)((ibm_float >> 24) & 0x7F) - 64;
    uint32_t fraction = ibm_float & 0x00FFFFFF;
    
    double val = (double)fraction / 16777216.0;
    double scale = 1.0;
    if (exponent > 0) {
        for (int i = 0; i < exponent; i++) scale *= 16.0;
    } else {
        for (int i = 0; i < -exponent; i++) scale /= 16.0;
    }
    float result = (float)(val * scale);
    return sign ? -result : result;
}

int tsfi_cw_pack_zoned_sign(const char *ascii_num, uint8_t *ebcdic_out, int max_len, int leading) {
    if (!ascii_num || !ebcdic_out || max_len <= 0) return -1;
    int len = strlen(ascii_num);
    int is_neg = (ascii_num[0] == '-');
    int start = (ascii_num[0] == '-' || ascii_num[0] == '+') ? 1 : 0;
    int digits = len - start;
    if (digits > max_len) return -2;

    for (int i = 0; i < digits; i++) {
        char digit = ascii_num[start + i];
        ebcdic_out[i] = tsfi_cw_ascii_to_ebcdic(digit);
    }
    
    int sign_idx = leading ? 0 : (digits - 1);
    if (is_neg) {
        ebcdic_out[sign_idx] = (ebcdic_out[sign_idx] & 0x0F) | 0xD0;
    } else {
        ebcdic_out[sign_idx] = (ebcdic_out[sign_idx] & 0x0F) | 0xC0;
    }
    return digits;
}

int tsfi_cw_unpack_zoned_sign(const uint8_t *ebcdic_in, int len, char *ascii_out, int max_len, int leading) {
    if (!ebcdic_in || len <= 0 || !ascii_out || max_len <= len + 1) return -1;
    int sign_idx = leading ? 0 : (len - 1);
    uint8_t sign_nibble = ebcdic_in[sign_idx] & 0xF0;
    int is_neg = (sign_nibble == 0xD0);
    
    int w_idx = 0;
    if (is_neg) ascii_out[w_idx++] = '-';
    
    for (int i = 0; i < len; i++) {
        uint8_t raw = ebcdic_in[i];
        if (i == sign_idx) {
            raw = (raw & 0x0F) | 0xF0;
        }
        ascii_out[w_idx++] = (char)tsfi_cw_ebcdic_to_ascii(raw);
    }
    ascii_out[w_idx] = '\0';
    return 0;
}

int tsfi_cw_pack_zoned(const char *ascii_num, uint8_t *zoned_out, int max_out_len, int *out_len) {
    if (!ascii_num || !zoned_out || !out_len) return -1;
    int len = strlen(ascii_num);
    if (len == 0) return -2;

    int is_negative = 0;
    int start_idx = 0;
    if (ascii_num[0] == '-') {
        is_negative = 1;
        start_idx = 1;
    } else if (ascii_num[0] == '+') {
        start_idx = 1;
    }

    int digits_len = len - start_idx;
    if (digits_len > max_out_len) return -3;
    *out_len = digits_len;

    for (int i = 0; i < digits_len; i++) {
        char digit = ascii_num[start_idx + i];
        if (digit < '0' || digit > '9') return -4;
        zoned_out[i] = 0xF0 | (digit - '0');
    }

    // Set sign in last digit's zone nibble
    int last_idx = digits_len - 1;
    if (is_negative) {
        zoned_out[last_idx] = (zoned_out[last_idx] & 0x0F) | 0xD0;
    } else {
        zoned_out[last_idx] = (zoned_out[last_idx] & 0x0F) | 0xC0;
    }
    return 0;
}

int tsfi_cw_unpack_zoned(const uint8_t *zoned_in, int zoned_len, char *ascii_out, int max_ascii_len) {
    if (!zoned_in || zoned_len <= 0 || !ascii_out || max_ascii_len <= 0) return -1;
    if (zoned_len + 2 > max_ascii_len) return -2;

    int is_negative = 0;
    uint8_t sign_zone = zoned_in[zoned_len - 1] & 0xF0;
    if (sign_zone == 0xD0) {
        is_negative = 1;
    }

    int out_idx = 0;
    if (is_negative) {
        ascii_out[out_idx++] = '-';
    }

    for (int i = 0; i < zoned_len; i++) {
        uint8_t raw = zoned_in[i];
        if (i == zoned_len - 1) {
            raw = (raw & 0x0F) | 0xF0;
        }
        char digit = (char)tsfi_cw_ebcdic_to_ascii(raw);
        if (digit < '0' || digit > '9') return -3;
        ascii_out[out_idx++] = digit;
    }
    ascii_out[out_idx] = '\0';
    return 0;
}

int tsfi_cw_align_comp3_fractional(const char *ascii_num, int decimal_places, char *aligned_out, int max_len) {
    if (!ascii_num || !aligned_out || max_len <= 0) return -1;
    
    const char *dot = strchr(ascii_num, '.');
    if (!dot) {
        strcpy(aligned_out, ascii_num);
        if (decimal_places > 0) {
            strcat(aligned_out, ".");
            for (int i = 0; i < decimal_places; i++) {
                strcat(aligned_out, "0");
            }
        }
        return 0;
    }
    
    int int_part_len = dot - ascii_num;
    int frac_part_len = strlen(dot + 1);
    
    strncpy(aligned_out, ascii_num, int_part_len);
    aligned_out[int_part_len] = '\0';
    
    if (decimal_places > 0) {
        strcat(aligned_out, ".");
        strncat(aligned_out, dot + 1, decimal_places);
        int added = decimal_places - frac_part_len;
        for (int i = 0; i < added; i++) {
            strcat(aligned_out, "0");
        }
    }
    return 0;
}

int tsfi_cw_parse_punch_card(const char *card_line, tsfi_cw_punch_card *card_out) {
    if (!card_line || !card_out) return -1;
    int len = strlen(card_line);
    if (len < 80) return -2;
    memcpy(card_out->sequence, card_line, 6);
    card_out->sequence[6] = '\0';
    card_out->indicator = card_line[6];
    memcpy(card_out->area_a, card_line + 7, 4);
    card_out->area_a[4] = '\0';
    memcpy(card_out->area_b, card_line + 11, 61);
    card_out->area_b[61] = '\0';
    memcpy(card_out->identification, card_line + 72, 8);
    card_out->identification[8] = '\0';
    return 0;
}

int tsfi_cw_card_is_continuation(const tsfi_cw_punch_card *card) {
    if (!card) return 0;
    return (card->indicator == '-');
}

int tsfi_cw_card_is_comment(const tsfi_cw_punch_card *card) {
    if (!card) return 0;
    return (card->indicator == '*' || card->indicator == '/');
}

int tsfi_cw_pack_sign_separate(const char *ascii_num, char *separate_out, int max_len, int leading) {
    if (!ascii_num || !separate_out || max_len <= 0) return -1;
    int len = strlen(ascii_num);
    if (len == 0) return -2;
    int is_negative = 0;
    int start_idx = 0;
    if (ascii_num[0] == '-') {
        is_negative = 1;
        start_idx = 1;
    } else if (ascii_num[0] == '+') {
        start_idx = 1;
    }
    int digits_len = len - start_idx;
    if (digits_len + 1 >= max_len) return -3;
    if (leading) {
        separate_out[0] = is_negative ? '-' : '+';
        strcpy(separate_out + 1, ascii_num + start_idx);
    } else {
        strcpy(separate_out, ascii_num + start_idx);
        separate_out[digits_len] = is_negative ? '-' : '+';
        separate_out[digits_len + 1] = '\0';
    }
    return 0;
}

int tsfi_cw_unpack_sign_separate(const char *separate_in, char *ascii_out, int max_len, int leading) {
    if (!separate_in || !ascii_out || max_len <= 0) return -1;
    int len = strlen(separate_in);
    if (len <= 1) return -2;
    if (leading) {
        char sign = separate_in[0];
        if (sign == '-') {
            ascii_out[0] = '-';
            strcpy(ascii_out + 1, separate_in + 1);
        } else if (sign == '+') {
            strcpy(ascii_out, separate_in + 1);
        } else {
            strcpy(ascii_out, separate_in);
        }
    } else {
        char sign = separate_in[len - 1];
        if (sign == '-' || sign == '+') {
            int write_idx = 0;
            if (sign == '-') {
                ascii_out[write_idx++] = '-';
            }
            strncpy(ascii_out + write_idx, separate_in, len - 1);
            ascii_out[write_idx + len - 1] = '\0';
        } else {
            strcpy(ascii_out, separate_in);
        }
    }
    return 0;
}

int tsfi_cw_cobol_get_dynamic_record_length(tsfi_cw_copybook *cb, const char *dep_field_name, int dep_field_value) {
    if (!cb || !dep_field_name) return -1;
    int length = 0;
    for (int i = 0; i < cb->field_count; i++) {
        tsfi_cw_cobol_field *f = &cb->fields[i];
        if (f->level == 88) continue;
        int field_len = f->length;
        if (strcmp(f->depending_on, dep_field_name) == 0) {
            int base_len = f->length / f->occurs;
            field_len = base_len * dep_field_value;
        }
        if (f->offset + field_len > length) {
            length = f->offset + field_len;
        }
    }
    return length;
}

int tsfi_cw_cobol_validate_occurs_range(int current_occurs, int max_occurs) {
    if (current_occurs < 0 || current_occurs > max_occurs) return -12;
    return 0;
}

int tsfi_cw_cobol_validate_justified_right_dynamic(tsfi_cw_cobol_field *f, int dynamic_occurs, const char *val) {
    if (f->justified_right && val) {
        int base_len = f->length / f->occurs;
        if (strlen(val) > (size_t)(base_len * dynamic_occurs)) {
            return -14;
        }
    }
    return 0;
}

int tsfi_cw_cobol_validate_picture_numeric_bounds(const char *pic_str, double val) {
    if (!pic_str) return -1;
    int digits = 0;
    const char *p = pic_str;
    while (*p) {
        if (*p == '9') {
            digits++;
        } else if (*p == '(') {
            p++;
            int count = atoi(p);
            digits += count - 1;
            while (*p && *p != ')') p++;
        }
        p++;
    }
    if (digits > 0) {
        double max_val = 1.0;
        for (int i = 0; i < digits; i++) max_val *= 10.0;
        double limit = max_val - 0.0001;
        double check_val = val < 0 ? -val : val;
        if (check_val >= limit) {
            return -17;
        }
    }
    return 0;
}

int tsfi_cw_cobol_validate_record_offset(int calculated_offset, int max_buffer_limit) {
    if (calculated_offset < 0 || calculated_offset > max_buffer_limit) return -19;
    return 0;
}

int tsfi_cw_cobol_validate_offset_overlap(int field_offset, int preceding_field_end) {
    if (field_offset < preceding_field_end) return -23;
    return 0;
}

int tsfi_cw_cobol_validate_custom_padding(char pad_char) {
    if (pad_char == '\0' || pad_char == '\n' || pad_char == '\r') {
        return -26;
    }
    return 0;
}

int tsfi_cw_cobol_validate_custom_padding_ex(char pad_char, int pad_len, int max_len) {
    if (pad_char == '\0' || pad_char == '\n' || pad_char == '\r') return -26;
    if (pad_len < 0 || pad_len > max_len) return -30;
    return 0;
}

int tsfi_cw_cobol_map_custom_padding_byte(char input_char, char *mapped_out) {
    if (!mapped_out) return -1;
    if (input_char == '\0' || input_char == '\n') {
        *mapped_out = ' ';
        return 0;
    }
    *mapped_out = input_char;
    return 0;
}

int tsfi_cw_cobol_validate_padding_alignment(int field_offset, int alignment_modulus) {
    if (alignment_modulus <= 0) return -1;
    if (field_offset % alignment_modulus != 0) return -34;
    return 0;
}

int tsfi_cw_cobol_validate_padding_limits(int pad_len, int max_allowed) {
    if (pad_len < 0 || pad_len > max_allowed) return -30;
    return 0;
}

int tsfi_cw_cobol_get_alignment_padding(int offset, int alignment_modulus, int *padding_out) {
    if (alignment_modulus <= 0 || !padding_out) return -1;
    int rem = offset % alignment_modulus;
    *padding_out = (rem == 0) ? 0 : (alignment_modulus - rem);
    return 0;
}
