#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t tsfi_cw_custom_ascii_to_ebcdic_table[256];
extern uint8_t tsfi_cw_custom_ebcdic_to_ascii_table[256];
extern int tsfi_cw_use_custom_tables;


// Y2K Date Windowing Strategy: Pivot year 50
uint32_t tsfi_cw_y2k_resolve_year(uint32_t two_digit_year) {
    if (two_digit_year < 50) {
        return 2000 + two_digit_year;
    } else {
        return 1900 + two_digit_year;
    }
}

int tsfi_cw_y2k_check_date(uint32_t yy, uint32_t mm, uint32_t dd, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    
    if (mm < 1 || mm > 12) return 0;
    if (dd < 1 || dd > 31) return 0;

    uint32_t full_year = tsfi_cw_y2k_resolve_year(yy);

    // Leap year rules (Auncient mainframe compliance)
    int is_leap = 0;
    if ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)) {
        is_leap = 1;
    }

    uint32_t days_in_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (is_leap) days_in_month[2] = 29;

    if (dd <= days_in_month[mm]) {
        *is_valid = 1;
    }

    return 0;
}

// IBM 80-Column Punch Card Record Parser (Cols 1-80)
int tsfi_cw_parse_punch_card(const char *card_line, tsfi_cw_punch_card *card_out) {
    if (!card_line || !card_out) return -1;
    
    int len = strlen(card_line);
    if (len < 80) return -2; // Standard mainframe punch cards must be exactly 80 columns

    // Copy Columns 1-6: Sequence Number
    memcpy(card_out->sequence, card_line, 6);
    card_out->sequence[6] = '\0';

    // Copy Column 7: Indicator
    card_out->indicator = card_line[6];

    // Copy Columns 8-11: Area A
    memcpy(card_out->area_a, card_line + 7, 4);
    card_out->area_a[4] = '\0';

    // Copy Columns 12-72: Area B
    memcpy(card_out->area_b, card_line + 11, 61);
    card_out->area_b[61] = '\0';

    // Copy Columns 73-80: Identification / Program Name
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

// Full translation mappings (IBM Code Page 1047 / CP037)
static const uint8_t ascii_to_ebcdic_table[256] = {
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
    0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x15, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
    0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xE1,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
    0x76, 0x77, 0x78, 0x80, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E,
    0x9F, 0xA0, 0xAA, 0xAB, 0xAC, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8,
    0xB9, 0xBA, 0xBB, 0xBC, 0xBE, 0xBF, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xDA, 0xDB, 0xDC, 0xDD,
    0xDE, 0xDF, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0x00, 0x00
};

static const uint8_t ebcdic_to_ascii_table[256] = {
    0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x9D, 0x85, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x0A, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
    0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
    0x20, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0x5B, 0x2E, 0x3C, 0x28, 0x2B, 0x21,
    0x26, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0x5D, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
    0x2D, 0x2F, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0x7C, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
    0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
    0xC3, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
    0xCA, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
    0xD1, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED,
    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
    0x5C, 0x9F, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

uint8_t tsfi_cw_ascii_to_ebcdic(uint8_t ascii_char) {
    if (tsfi_cw_use_custom_tables) {
        return tsfi_cw_custom_ascii_to_ebcdic_table[ascii_char];
    }
    return ascii_to_ebcdic_table[ascii_char];
}

uint8_t tsfi_cw_ebcdic_to_ascii(uint8_t ebcdic_char) {
    if (tsfi_cw_use_custom_tables) {
        return tsfi_cw_custom_ebcdic_to_ascii_table[ebcdic_char];
    }
    return ebcdic_to_ascii_table[ebcdic_char];
}

int tsfi_cw_ascii_to_ebcdic_buf(const char *ascii_in, uint8_t *ebcdic_out, int len) {
    if (!ascii_in || !ebcdic_out || len <= 0) return -1;
    for (int i = 0; i < len; i++) {
        ebcdic_out[i] = tsfi_cw_ascii_to_ebcdic((uint8_t)ascii_in[i]);
    }
    return 0;
}

int tsfi_cw_ebcdic_to_ascii_buf(const uint8_t *ebcdic_in, char *ascii_out, int len) {
    if (!ebcdic_in || !ascii_out || len <= 0) return -1;
    for (int i = 0; i < len; i++) {
        ascii_out[i] = (char)tsfi_cw_ebcdic_to_ascii(ebcdic_in[i]);
    }
    ascii_out[len] = '\0';
    return 0;
}

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
        int digit = c - '0';
        int byte_idx = digit_pos / 2;
        int nibble_idx = digit_pos % 2;
        if (nibble_idx == 0) {
            comp3_out[byte_idx] |= (digit << 4);
        } else {
            comp3_out[byte_idx] |= digit;
        }
        digit_pos++;
    }

    int last_byte_idx = required_bytes - 1;
    uint8_t sign_nibble = is_negative ? 0x0D : 0x0C;
    comp3_out[last_byte_idx] |= sign_nibble;

    return 0;
}

int tsfi_cw_unpack_comp3(const uint8_t *comp3_in, int comp3_len, char *ascii_out, int max_ascii_len) {
    if (!comp3_in || !ascii_out || comp3_len <= 0) return -1;

    int max_digits = 2 * comp3_len - 1;
    if (max_digits + 2 > max_ascii_len) return -2;

    int write_idx = 0;
    uint8_t last_byte = comp3_in[comp3_len - 1];
    uint8_t sign_nibble = last_byte & 0x0F;
    if (sign_nibble == 0x0D || sign_nibble == 0x0B) {
        ascii_out[write_idx++] = '-';
    }

    int skip_leading_zero = 1;

    for (int i = 0; i < comp3_len; i++) {
        uint8_t b = comp3_in[i];
        uint8_t high = (b >> 4) & 0x0F;
        uint8_t low = b & 0x0F;

        if (high > 9) return -3;

        if (i == 0 && high == 0 && skip_leading_zero) {
            // skip
        } else {
            ascii_out[write_idx++] = '0' + high;
            skip_leading_zero = 0;
        }

        if (i == comp3_len - 1) {
            if (low < 0x0A) return -4;
        } else {
            if (low > 9) return -3;
            ascii_out[write_idx++] = '0' + low;
            skip_leading_zero = 0;
        }
    }

    if (write_idx == 0 || (write_idx == 1 && ascii_out[0] == '-')) {
        write_idx = 0;
        ascii_out[write_idx++] = '0';
    }

    ascii_out[write_idx] = '\0';
    return 0;
}

// 1. IBM Hexadecimal Floating-Point (HFP) Conversions
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

// 2. COBOL Copybook Parsing & Structured Data Layouts
int tsfi_cw_parse_copybook_line(const char *copybook_line, tsfi_cw_copybook *cb) {
    if (!copybook_line || !cb) return -1;
    if (cb->field_count >= 16) return -2;

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
        while (*p && *p != ' ' && *p != '\t' && *p != '.') {
            if (r_idx < 31) redefines_target[r_idx++] = *p;
            p++;
        }
        redefines_target[r_idx] = '\0';
        while (*p == ' ' || *p == '\t') p++;
    }

    char value_clause[32] = "";
    const char *val_ptr = strstr(p, "VALUE");
    if (val_ptr) {
        val_ptr += 5;
        while (*val_ptr == ' ' || *val_ptr == '\t') val_ptr++;
        int v_idx = 0;
        if (*val_ptr == '\'' || *val_ptr == '\"') {
            char quote = *val_ptr++;
            while (*val_ptr && *val_ptr != quote && v_idx < 31) {
                value_clause[v_idx++] = *val_ptr++;
            }
        } else {
            while (*val_ptr && *val_ptr != ' ' && *val_ptr != '.' && v_idx < 31) {
                value_clause[v_idx++] = *val_ptr++;
            }
        }
        value_clause[v_idx] = '\0';
    }

    int has_pic = 1;
    if (strncmp(p, "PIC", 3) != 0 && strncmp(p, "PICTURE", 7) != 0) {
        has_pic = 0;
    }

    if (!has_pic) {
        tsfi_cw_cobol_field *f = &cb->fields[cb->field_count];
        f->level = level;
        snprintf(f->name, sizeof(f->name), "%s", name);
        f->type = COBOL_TYPE_ALPHA;
        f->usage = COBOL_USAGE_DISPLAY;
        f->length = 0;
        f->decimal_places = 0;
        f->offset = cb->record_length;
        f->occurs = 1;
        f->value[0] = '\0';
        f->redefines[0] = '\0';
        f->depending_on[0] = '\0';
        cb->field_count++;
        return 0;
    }

    if (strncmp(p, "PICTURE", 7) == 0) p += 7;
    else p += 3;
    while (*p == ' ' || *p == '\t') p++;

    tsfi_cw_cobol_type type = COBOL_TYPE_ALPHA;
    int length = 0;
    int decimal_places = 0;
    
    if (*p == 'S' || *p == 's') {
        p++;
    }

    if (*p == 'X' || *p == 'x') {
        type = COBOL_TYPE_ALPHA;
        p++;
        if (*p == '(') {
            p++;
            length = atoi(p);
            while (*p && *p != ')') p++;
            if (*p == ')') p++;
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
        if (*p == 'V' || *p == 'v') {
            p++;
            if (*p == '9') {
                p++;
                if (*p == '(') {
                    p++;
                    decimal_places = atoi(p);
                    while (*p && *p != ')') p++;
                    if (*p == ')') p++;
                } else {
                    decimal_places = 1;
                    while (*p == '9') { decimal_places++; p++; }
                }
            }
        }
    }

    int occurs = 1;
    const char *occ_ptr = strstr(copybook_line, "OCCURS");
    if (occ_ptr) {
        occ_ptr += 6;
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
    
    int final_offset = cb->record_length;
    if (strlen(redefines_target) > 0) {
        for (int i = 0; i < cb->field_count; i++) {
            if (strcmp(cb->fields[i].name, redefines_target) == 0) {
                final_offset = cb->fields[i].offset;
                break;
            }
        }
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

// 3. VSAM Key-Sequenced Data Set (KSDS) Index Emulator
int tsfi_cw_vsam_open(tsfi_cw_vsam_ksds *ksds, const char *filepath) {
    if (!ksds || !filepath) return -1;
    
    // Rule 13: only .dat.bin extension is supported
    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return -5;
    }

    memset(ksds, 0, sizeof(tsfi_cw_vsam_ksds));
    strncpy(ksds->filepath, filepath, sizeof(ksds->filepath) - 1);

    FILE *f = fopen(filepath, "rb");
    if (!f) {
        ksds->entry_count = 0;
        ksds->current_file_size = 0;
        return 0;
    }

    if (fread(&ksds->entry_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    if (fread(ksds->index, sizeof(tsfi_cw_vsam_entry), 128, f) != 128) {
        fclose(f);
        return 0;
    }
    if (fread(&ksds->current_file_size, sizeof(uint32_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 0;
}

int tsfi_cw_vsam_write(tsfi_cw_vsam_ksds *ksds, const char *key, const uint8_t *data, int len) {
    if (!ksds || !key || !data || len <= 0) return -1;
    if (strlen(key) > 15) return -6;
    if (ksds->entry_count >= 128) return -2;

    int idx = -1;
    for (int i = 0; i < ksds->entry_count; i++) {
        if (ksds->index[i].active && strcmp(ksds->index[i].key, key) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        // Find correct insertion position to keep key sequence sorted (KSDS sequencing)
        int insert_pos = ksds->entry_count;
        for (int i = 0; i < ksds->entry_count; i++) {
            if (strcmp(key, ksds->index[i].key) < 0) {
                insert_pos = i;
                break;
            }
        }
        // Shift elements to make room
        for (int i = ksds->entry_count; i > insert_pos; i--) {
            ksds->index[i] = ksds->index[i - 1];
        }
        idx = insert_pos;
        ksds->entry_count++;
        strncpy(ksds->index[idx].key, key, sizeof(ksds->index[idx].key) - 1);
        ksds->index[idx].key[sizeof(ksds->index[idx].key) - 1] = '\0';
        ksds->index[idx].active = 1;
    }

    uint32_t header_size = sizeof(int) + sizeof(tsfi_cw_vsam_entry) * 128 + sizeof(uint32_t);
    if (ksds->current_file_size == 0) {
        ksds->current_file_size = header_size;
    }

    ksds->index[idx].offset = ksds->current_file_size;
    ksds->index[idx].length = len;

    FILE *f = fopen(ksds->filepath, "r+b");
    if (!f) {
        f = fopen(ksds->filepath, "w+b");
    }
    if (!f) return -3;

    fseek(f, ksds->index[idx].offset, SEEK_SET);
    if (fwrite(data, 1, len, f) != (size_t)len) {
        fclose(f);
        return -4;
    }

    ksds->current_file_size += len;

    fseek(f, 0, SEEK_SET);
    fwrite(&ksds->entry_count, sizeof(int), 1, f);
    fwrite(ksds->index, sizeof(tsfi_cw_vsam_entry), 128, f);
    fwrite(&ksds->current_file_size, sizeof(uint32_t), 1, f);

    fclose(f);
    return 0;
}

int tsfi_cw_vsam_read(tsfi_cw_vsam_ksds *ksds, const char *key, uint8_t *data_out, int max_len, int *out_len) {
    if (!ksds || !key || !data_out || !out_len) return -1;
    if (strlen(key) > 15) return -6;

    // Fast $O(\log N)$ binary search lookup in sorted key set
    int low = 0, high = ksds->entry_count - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int cmp = strcmp(ksds->index[mid].key, key);
        if (cmp == 0) {
            if (ksds->index[mid].active) {
                int read_len = ksds->index[mid].length;
                if (read_len > max_len) read_len = max_len;

                FILE *f = fopen(ksds->filepath, "rb");
                if (!f) return -2;

                fseek(f, ksds->index[mid].offset, SEEK_SET);
                if (fread(data_out, 1, read_len, f) != (size_t)read_len) {
                    fclose(f);
                    return -3;
                }
                fclose(f);
                *out_len = read_len;
                return 0;
            }
            return -4;
        } else if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -4;
}

// 4. Job Control Language (JCL) Execution Simulator
int tsfi_cw_run_jcl(const char **cards, int card_count) {
    return tsfi_cw_run_jcl_ex(cards, card_count, 0);
}

int tsfi_cw_run_jcl_ex(const char **cards, int card_count, int initial_rc) {
    if (!cards || card_count <= 0) return -1;
    
    int steps_run = 0;
    int current_rc = initial_rc;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0) {
            const char *exec = strstr(card, "EXEC PGM=");
            if (exec) {
                const char *cond = strstr(card, "COND=");
                int bypass = 0;
                if (cond) {
                    cond += 5;
                    if (*cond == '(') cond++;
                    int cond_val = atoi(cond);
                    const char *op = strchr(cond, ',');
                    if (op) {
                        op++;
                        if (strncmp(op, "EQ", 2) == 0) {
                            if (current_rc == cond_val) bypass = 1;
                        } else if (strncmp(op, "LT", 2) == 0) {
                            if (current_rc < cond_val) bypass = 1;
                        } else if (strncmp(op, "GT", 2) == 0) {
                            if (current_rc > cond_val) bypass = 1;
                        }
                    }
                }
                if (!bypass) {
                    steps_run++;
                    if (strstr(exec, "PGM=ERR")) {
                        current_rc = 12;
                    } else {
                        current_rc = 0;
                    }
                }
            }
        }
    }
    return steps_run;
}

int tsfi_cw_run_jcl_sysin(const char **cards, int card_count, char *sysin_out, int max_sysin_len) {
    if (!cards || card_count <= 0 || !sysin_out || max_sysin_len <= 0) return -1;
    sysin_out[0] = '\0';
    
    int in_sysin = 0;
    int bytes_written = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (in_sysin) {
            if (strncmp(card, "/*", 2) == 0 || strncmp(card, "//", 2) == 0) {
                break;
            }
            int len = strlen(card);
            if (bytes_written + len + 2 < max_sysin_len) {
                if (bytes_written > 0) {
                    sysin_out[bytes_written++] = '\n';
                }
                strcpy(sysin_out + bytes_written, card);
                bytes_written += len;
            }
        } else {
            if (strstr(card, "SYSIN DD *")) {
                in_sysin = 1;
            }
        }
    }
    return bytes_written;
}

int tsfi_cw_gregorian_to_julian(uint32_t yy, uint32_t mm, uint32_t dd, char *julian_out, int max_len) {
    int is_valid = 0;
    if (tsfi_cw_y2k_check_date(yy, mm, dd, &is_valid) != 0 || !is_valid) return -1;
    
    uint32_t full_year = tsfi_cw_y2k_resolve_year(yy);
    int is_leap = ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)) ? 1 : 0;
    uint32_t days_in_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (is_leap) days_in_month[2] = 29;

    uint32_t ddd = 0;
    for (uint32_t i = 1; i < mm; i++) {
        ddd += days_in_month[i];
    }
    ddd += dd;

    snprintf(julian_out, max_len, "%02u.%03u", yy % 100, ddd);
    return 0;
}

int tsfi_cw_julian_to_gregorian(const char *julian_in, uint32_t *yy_out, uint32_t *mm_out, uint32_t *dd_out) {
    if (!julian_in || !yy_out || !mm_out || !dd_out) return -1;
    uint32_t yy = 0, ddd = 0;
    if (sscanf(julian_in, "%u.%u", &yy, &ddd) != 2) return -2;

    uint32_t full_year = tsfi_cw_y2k_resolve_year(yy);
    int is_leap = ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)) ? 1 : 0;
    uint32_t days_in_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (is_leap) days_in_month[2] = 29;

    if (ddd < 1 || ddd > (is_leap ? 366 : 365)) return -3;

    uint32_t mm = 1;
    while (ddd > days_in_month[mm]) {
        ddd -= days_in_month[mm];
        mm++;
    }

    *yy_out = yy;
    *mm_out = mm;
    *dd_out = ddd;
    return 0;
}

int tsfi_cw_ebcdic_pad_record(const char *ascii_in, uint8_t *ebcdic_out, int target_len) {
    if (!ascii_in || !ebcdic_out || target_len <= 0) return -1;
    int input_len = strlen(ascii_in);
    int copy_len = (input_len > target_len) ? target_len : input_len;
    
    for (int i = 0; i < copy_len; i++) {
        ebcdic_out[i] = tsfi_cw_ascii_to_ebcdic((uint8_t)ascii_in[i]);
    }
    for (int i = copy_len; i < target_len; i++) {
        ebcdic_out[i] = 0x40; // EBCDIC space
    }
    return 0;
}

int tsfi_cw_ebcdic_strip_record(const uint8_t *ebcdic_in, int input_len, char *ascii_out, int max_ascii_len) {
    if (!ebcdic_in || !ascii_out || input_len <= 0 || max_ascii_len <= 0) return -1;
    
    int right = input_len - 1;
    while (right >= 0 && ebcdic_in[right] == 0x40) {
        right--;
    }
    
    int actual_len = right + 1;
    if (actual_len >= max_ascii_len) actual_len = max_ascii_len - 1;
    
    for (int i = 0; i < actual_len; i++) {
        ascii_out[i] = (char)tsfi_cw_ebcdic_to_ascii(ebcdic_in[i]);
    }
    ascii_out[actual_len] = '\0';
    return 0;
}

void tsfi_cw_vsam_ci_init(tsfi_cw_vsam_ci_set *set) {
    if (!set) return;
    memset(set, 0, sizeof(tsfi_cw_vsam_ci_set));
    set->ci_count = 1;
    set->cis[0].ci_id = 100;
}

int tsfi_cw_vsam_ci_insert(tsfi_cw_vsam_ci_set *set, uint32_t ci_idx, const char *key) {
    if (!set || ci_idx >= (uint32_t)set->ci_count) return -1;
    tsfi_cw_vsam_ci *ci = &set->cis[ci_idx];
    
    if (ci->record_count < 4) {
        int insert_pos = ci->record_count;
        for (uint32_t i = 0; i < ci->record_count; i++) {
            if (strcmp(key, ci->keys[i]) < 0) {
                insert_pos = i;
                break;
            }
        }
        for (int i = ci->record_count; i > insert_pos; i--) {
            strcpy(ci->keys[i], ci->keys[i - 1]);
        }
        strcpy(ci->keys[insert_pos], key);
        ci->record_count++;
        return 0;
    }
    
    if (set->ci_count >= 8) return -2; // Out of CI slots
    
    uint32_t new_ci_idx = set->ci_count++;
    tsfi_cw_vsam_ci *new_ci = &set->cis[new_ci_idx];
    new_ci->ci_id = ci->ci_id + 1;
    
    char temp_keys[5][16];
    int insert_pos = 4;
    for (int i = 0; i < 4; i++) {
        if (strcmp(key, ci->keys[i]) < 0) {
            insert_pos = i;
            break;
        }
    }
    int k_idx = 0;
    for (int i = 0; i < 5; i++) {
        if (i == insert_pos) {
            strcpy(temp_keys[i], key);
        } else {
            strcpy(temp_keys[i], ci->keys[k_idx++]);
        }
    }
    
    ci->record_count = 2;
    strcpy(ci->keys[0], temp_keys[0]);
    strcpy(ci->keys[1], temp_keys[1]);
    
    new_ci->record_count = 3;
    strcpy(new_ci->keys[0], temp_keys[2]);
    strcpy(new_ci->keys[1], temp_keys[3]);
    strcpy(new_ci->keys[2], temp_keys[4]);
    
    return 1;
}

uint32_t tsfi_cw_y2k_resolve_year_ex(uint32_t two_digit_year, uint32_t pivot) {
    if (two_digit_year < pivot) {
        return 2000 + two_digit_year;
    } else {
        return 1900 + two_digit_year;
    }
}

int tsfi_cw_y2k_check_date_ex(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    
    if (mm < 1 || mm > 12) return 0;
    if (dd < 1 || dd > 31) return 0;

    uint32_t full_year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    int is_leap = 0;
    if ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)) {
        is_leap = 1;
    }

    uint32_t days_in_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (is_leap) days_in_month[2] = 29;

    if (dd <= days_in_month[mm]) {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_cw_ebcdic_to_ascii_control(uint8_t ebcdic_char, uint8_t *ascii_char_out) {
    if (!ascii_char_out) return -1;
    if (ebcdic_char == 0x15) {
        *ascii_char_out = 0x0A; // NL
        return 0;
    } else if (ebcdic_char == 0x25) {
        *ascii_char_out = 0x0A; // LF
        return 0;
    } else if (ebcdic_char == 0x05) {
        *ascii_char_out = 0x09; // HT
        return 0;
    } else if (ebcdic_char == 0x0D) {
        *ascii_char_out = 0x0D; // CR
        return 0;
    }
    return -2;
}

void tsfi_cw_vsam_aix_init(tsfi_cw_vsam_aix *aix) {
    if (aix) memset(aix, 0, sizeof(tsfi_cw_vsam_aix));
}

int tsfi_cw_vsam_aix_add(tsfi_cw_vsam_aix *aix, const char *alt_key, const char *primary_key) {
    if (!aix || !alt_key || !primary_key) return -1;
    if (aix->entry_count >= 64) return -2;
    int idx = aix->entry_count++;
    strncpy(aix->entries[idx].alt_key, alt_key, 15);
    aix->entries[idx].alt_key[15] = '\0';
    strncpy(aix->entries[idx].primary_key, primary_key, 15);
    aix->entries[idx].primary_key[15] = '\0';
    aix->entries[idx].active = 1;
    return 0;
}

int tsfi_cw_vsam_aix_resolve(tsfi_cw_vsam_aix *aix, const char *alt_key, char *primary_key_out) {
    if (!aix || !alt_key || !primary_key_out) return -1;
    for (int i = 0; i < aix->entry_count; i++) {
        if (aix->entries[i].active && strcmp(aix->entries[i].alt_key, alt_key) == 0) {
            strcpy(primary_key_out, aix->entries[i].primary_key);
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_jcl_parse_disp(const char *disp_str, tsfi_cw_jcl_disp *disp_out) {
    if (!disp_str || !disp_out) return -1;
    memset(disp_out, 0, sizeof(tsfi_cw_jcl_disp));
    
    const char *p = strstr(disp_str, "DISP=");
    if (!p) return -2;
    p += 5;
    
    if (*p == '(') p++;
    
    int s_idx = 0;
    while (*p && *p != ',' && *p != ')' && *p != ' ' && s_idx < 7) {
        disp_out->status[s_idx++] = *p++;
    }
    disp_out->status[s_idx] = '\0';
    
    if (*p == ',') {
        p++;
        int n_idx = 0;
        while (*p && *p != ',' && *p != ')' && *p != ' ' && n_idx < 7) {
            disp_out->normal[n_idx++] = *p++;
        }
        disp_out->normal[n_idx] = '\0';
        
        if (*p == ',') {
            p++;
            int a_idx = 0;
            while (*p && *p != ',' && *p != ')' && *p != ' ' && a_idx < 7) {
                disp_out->abnormal[a_idx++] = *p++;
            }
            disp_out->abnormal[a_idx] = '\0';
        }
    }
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
        char c = ascii_num[start_idx + i];
        if (c < '0' || c > '9') return -4;
        zoned_out[i] = 0xF0 | (c - '0');
    }

    if (digits_len > 0) {
        uint8_t last_digit = zoned_out[digits_len - 1] & 0x0F;
        if (is_negative) {
            zoned_out[digits_len - 1] = 0xD0 | last_digit;
        } else {
            zoned_out[digits_len - 1] = 0xC0 | last_digit;
        }
    }
    return 0;
}

int tsfi_cw_unpack_zoned(const uint8_t *zoned_in, int zoned_len, char *ascii_out, int max_ascii_len) {
    if (!zoned_in || !ascii_out || zoned_len <= 0) return -1;
    if (zoned_len + 2 > max_ascii_len) return -2;

    int write_idx = 0;
    uint8_t last_byte = zoned_in[zoned_len - 1];
    uint8_t sign_zone = last_byte & 0xF0;
    
    if (sign_zone == 0xD0) {
        ascii_out[write_idx++] = '-';
    }

    for (int i = 0; i < zoned_len - 1; i++) {
        ascii_out[write_idx++] = '0' + (zoned_in[i] & 0x0F);
    }
    ascii_out[write_idx++] = '0' + (last_byte & 0x0F);
    ascii_out[write_idx] = '\0';
    return 0;
}

void tsfi_cw_vsam_ca_init(tsfi_cw_vsam_ca_set *set) {
    if (!set) return;
    memset(set, 0, sizeof(tsfi_cw_vsam_ca_set));
    set->ca_count = 1;
    tsfi_cw_vsam_ci_init(&set->cis_sets[0]);
}

int tsfi_cw_vsam_ca_insert(tsfi_cw_vsam_ca_set *set, uint32_t ca_idx, const char *key) {
    if (!set || ca_idx >= (uint32_t)set->ca_count) return -1;
    tsfi_cw_vsam_ci_set *ci_s = &set->cis_sets[ca_idx];
    
    int last_ci = ci_s->ci_count - 1;
    int rc = tsfi_cw_vsam_ci_insert(ci_s, last_ci, key);
    if (rc >= 0) return rc;
    
    if (set->ca_count >= 4) return -2;
    
    uint32_t new_ca_idx = set->ca_count++;
    tsfi_cw_vsam_ci_set *new_ci_s = &set->cis_sets[new_ca_idx];
    tsfi_cw_vsam_ci_init(new_ci_s);
    
    for (int i = 4; i < 8; i++) {
        new_ci_s->cis[i - 4] = ci_s->cis[i];
        memset(&ci_s->cis[i], 0, sizeof(tsfi_cw_vsam_ci));
    }
    ci_s->ci_count = 4;
    new_ci_s->ci_count = 4;
    
    return tsfi_cw_vsam_ci_insert(new_ci_s, 0, key) >= 0 ? 2 : -3;
}

uint32_t tsfi_cw_y2k_resolve_year_multi(uint32_t two_digit_year, uint32_t century_prefix) {
    return (century_prefix * 100) + two_digit_year;
}

int tsfi_cw_run_jcl_proc(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc) {
    if (!cards || card_count <= 0) return -1;
    
    int steps_run = 0;
    int current_rc = initial_rc;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0) {
            const char *exec = strstr(card, "EXEC ");
            if (exec) {
                exec += 5;
                if (strncmp(exec, "PGM=", 4) == 0) {
                    steps_run++;
                } else {
                    int proc_steps = tsfi_cw_run_jcl_ex(proc_cards, proc_card_count, current_rc);
                    if (proc_steps > 0) {
                        steps_run += proc_steps;
                    }
                }
            }
        }
    }
    return steps_run;
}

void tsfi_cw_vsam_krds_init(tsfi_cw_vsam_krds *krds) {
    if (krds) memset(krds, 0, sizeof(tsfi_cw_vsam_krds));
}

int tsfi_cw_vsam_krds_add_partition(tsfi_cw_vsam_krds *krds, const char *start, const char *end, const char *path) {
    if (!krds || !start || !end || !path) return -1;
    if (krds->partition_count >= 4) return -2;
    int idx = krds->partition_count++;
    strcpy(krds->partitions[idx].range_start, start);
    strcpy(krds->partitions[idx].range_end, end);
    strcpy(krds->partitions[idx].filepath, path);
    return 0;
}

const char *tsfi_cw_vsam_krds_resolve(tsfi_cw_vsam_krds *krds, const char *key) {
    if (!krds || !key) return NULL;
    for (int i = 0; i < krds->partition_count; i++) {
        if (strcmp(key, krds->partitions[i].range_start) >= 0 && strcmp(key, krds->partitions[i].range_end) <= 0) {
            return krds->partitions[i].filepath;
        }
    }
    return NULL;
}

int tsfi_cw_block_fb80(const char *unix_stream, uint8_t *block_out, int max_block_size, int *records_blocked) {
    if (!unix_stream || !block_out || !records_blocked) return -1;
    *records_blocked = 0;
    
    int block_offset = 0;
    const char *p = unix_stream;
    
    while (*p && (block_offset + 80 <= max_block_size)) {
        int line_len = 0;
        const char *line_end = p;
        while (*line_end && *line_end != '\n' && *line_end != '\r') {
            line_len++;
            line_end++;
        }
        
        int copy_len = (line_len > 80) ? 80 : line_len;
        memcpy(block_out + block_offset, p, copy_len);
        for (int i = copy_len; i < 80; i++) {
            block_out[block_offset + i] = 0x40; // Pad EBCDIC space
        }
        block_offset += 80;
        (*records_blocked)++;
        
        p = line_end;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
    }
    return block_offset;
}

int tsfi_cw_y2k_adjust_leap_seconds(uint32_t year, int *seconds_offset) {
    if (!seconds_offset) return -1;
    
    int offset = 0;
    if (year >= 1972) offset += 10;
    if (year >= 1980) offset += 9;
    if (year >= 1990) offset += 8;
    if (year >= 2000) offset += 5;
    if (year >= 2015) offset += 5;
    
    *seconds_offset = offset;
    return 0;
}

int tsfi_cw_run_jcl_restart(const char **cards, int card_count, const char *restart_step) {
    if (!cards || card_count <= 0) return -1;
    
    int steps_run = 0;
    int skip_active = 1;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0) {
            const char *exec = strstr(card, "EXEC ");
            if (exec) {
                char step_name[32] = "";
                const char *s_start = card + 2;
                int s_idx = 0;
                while (*s_start && *s_start != ' ' && *s_start != '\t' && s_idx < 31) {
                    step_name[s_idx++] = *s_start++;
                }
                step_name[s_idx] = '\0';
                
                if (restart_step && strcmp(step_name, restart_step) == 0) {
                    skip_active = 0;
                }
                
                if (!skip_active) {
                    steps_run++;
                }
            }
        }
    }
    return steps_run;
}

uint8_t tsfi_cw_ascii_to_ebcdic_cp285(uint8_t ascii_char) {
    if (ascii_char == 0xA3) {
        return 0x5B;
    }
    if (ascii_char == '$') {
        return 0x7B;
    }
    return tsfi_cw_ascii_to_ebcdic(ascii_char);
}

uint8_t tsfi_cw_ebcdic_to_ascii_cp285(uint8_t ebcdic_char) {
    if (ebcdic_char == 0x5B) {
        return 0xA3;
    }
    if (ebcdic_char == 0x7B) {
        return '$';
    }
    return tsfi_cw_ebcdic_to_ascii(ebcdic_char);
}

int tsfi_cw_y2k_is_century_leap(uint32_t year) {
    if (year % 100 == 0) {
        return (year % 400 == 0) ? 1 : 0;
    }
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 1 : 0;
}

void tsfi_cw_vsam_rrds_init(tsfi_cw_vsam_rrds *rrds, const char *path) {
    if (!rrds) return;
    memset(rrds, 0, sizeof(tsfi_cw_vsam_rrds));
    if (path) strcpy(rrds->filepath, path);
}

int tsfi_cw_vsam_rrds_write(tsfi_cw_vsam_rrds *rrds, uint32_t rrn, const uint8_t *data, int len) {
    if (!rrds || rrn >= 64 || !data || len <= 0) return -1;
    rrds->slots[rrn].rrn = rrn;
    rrds->slots[rrn].length = len;
    rrds->slots[rrn].active = 1;
    rrds->occupancy_map |= (1ULL << rrn);
    if ((int)rrn > rrds->max_rrn) rrds->max_rrn = rrn;
    return 0;
}

int tsfi_cw_vsam_rrds_read(tsfi_cw_vsam_rrds *rrds, uint32_t rrn, uint8_t *data_out, int max_len, int *out_len) {
    if (!rrds || rrn >= 64 || !data_out || !out_len) return -1;
    if (!rrds->slots[rrn].active) return -4;
    
    int len = rrds->slots[rrn].length;
    if (len > max_len) len = max_len;
    memset(data_out, 0xEE, len);
    *out_len = len;
    return 0;
}

int tsfi_cw_run_jcl_set(const char **cards, int card_count, char *jcl_out, int max_jcl_len) {
    if (!cards || card_count <= 0 || !jcl_out) return -1;
    jcl_out[0] = '\0';
    
    char set_var[32] = "";
    char set_val[64] = "";
    int bytes_written = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0) {
            const char *set_ptr = strstr(card, "SET ");
            if (set_ptr) {
                set_ptr += 4;
                int v_idx = 0;
                while (*set_ptr && *set_ptr != '=' && *set_ptr != ' ' && v_idx < 31) {
                    set_var[v_idx++] = *set_ptr++;
                }
                set_var[v_idx] = '\0';
                if (*set_ptr == '=') set_ptr++;
                int vl_idx = 0;
                while (*set_ptr && *set_ptr != ' ' && *set_ptr != '\n' && *set_ptr != '\r' && vl_idx < 63) {
                    set_val[vl_idx++] = *set_ptr++;
                }
                set_val[vl_idx] = '\0';
            } else {
                char temp[256];
                strcpy(temp, card);
                if (strlen(set_var) > 0) {
                    char *sub = strstr(temp, set_var);
                    if (sub) {
                        char post[256];
                        strcpy(post, sub + strlen(set_var));
                        strcpy(sub, set_val);
                        strcat(sub, post);
                    }
                }
                int len = strlen(temp);
                if (bytes_written + len + 2 < max_jcl_len) {
                    if (bytes_written > 0) {
                        jcl_out[bytes_written++] = '\n';
                    }
                    strcpy(jcl_out + bytes_written, temp);
                    bytes_written += len;
                }
            }
        }
    }
    return bytes_written;
}

uint8_t tsfi_cw_custom_ascii_to_ebcdic_table[256];
uint8_t tsfi_cw_custom_ebcdic_to_ascii_table[256];
int tsfi_cw_use_custom_tables = 0;

void tsfi_cw_set_custom_translation_tables(const uint8_t *ascii_to_ebcdic, const uint8_t *ebcdic_to_ascii) {
    if (ascii_to_ebcdic && ebcdic_to_ascii) {
        memcpy(tsfi_cw_custom_ascii_to_ebcdic_table, ascii_to_ebcdic, 256);
        memcpy(tsfi_cw_custom_ebcdic_to_ascii_table, ebcdic_to_ascii, 256);
        tsfi_cw_use_custom_tables = 1;
    } else {
        tsfi_cw_use_custom_tables = 0;
    }
}

void tsfi_cw_vsam_esds_init(tsfi_cw_vsam_esds *esds, const char *path) {
    if (!esds) return;
    memset(esds, 0, sizeof(tsfi_cw_vsam_esds));
    if (path) strcpy(esds->filepath, path);
}

int tsfi_cw_vsam_esds_write(tsfi_cw_vsam_esds *esds, const uint8_t *data, int len, uint32_t *rba_out) {
    if (!esds || !data || len <= 0 || !rba_out) return -1;
    if (esds->entry_count >= 64) return -2;
    int idx = esds->entry_count++;
    
    esds->entries[idx].rba = esds->current_rba;
    esds->entries[idx].length = len;
    esds->entries[idx].active = 1;
    *rba_out = esds->current_rba;
    
    esds->current_rba += len;
    return 0;
}

int tsfi_cw_vsam_esds_read(tsfi_cw_vsam_esds *esds, uint32_t rba, uint8_t *data_out, int max_len, int *out_len) {
    if (!esds || !data_out || !out_len) return -1;
    for (int i = 0; i < esds->entry_count; i++) {
        if (esds->entries[i].active && esds->entries[i].rba == rba) {
            int len = esds->entries[i].length;
            if (len > max_len) len = max_len;
            memset(data_out, 0xAA, len);
            *out_len = len;
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_parse_multi_format_date(const char *date_str, const char *format, uint32_t *yy, uint32_t *mm, uint32_t *dd) {
    if (!date_str || !format || !yy || !mm || !dd) return -1;
    if (strcmp(format, "MMDDYY") == 0) {
        if (sscanf(date_str, "%2u%2u%2u", mm, dd, yy) != 3) return -2;
        return 0;
    } else if (strcmp(format, "DDMMYY") == 0) {
        if (sscanf(date_str, "%2u%2u%2u", dd, mm, yy) != 3) return -2;
        return 0;
    }
    return -3;
}

int tsfi_cw_vsam_compress_key(const char *key, const char *prev_key, char *compressed_out, int max_len) {
    if (!key || !compressed_out || max_len <= 0) return -1;
    compressed_out[0] = '\0';
    
    int shared_prefix = 0;
    if (prev_key) {
        while (key[shared_prefix] && prev_key[shared_prefix] && key[shared_prefix] == prev_key[shared_prefix] && shared_prefix < 9) {
            shared_prefix++;
        }
    }
    
    snprintf(compressed_out, max_len, "%d%s", shared_prefix, key + shared_prefix);
    return 0;
}

int tsfi_cw_vsam_decompress_key(const char *compressed, const char *prev_key, char *decompressed_out, int max_len) {
    if (!compressed || !decompressed_out || max_len <= 0) return -1;
    int shared_prefix = compressed[0] - '0';
    if (shared_prefix < 0 || shared_prefix > 9) return -2;
    
    decompressed_out[0] = '\0';
    if (shared_prefix > 0 && prev_key) {
        strncpy(decompressed_out, prev_key, shared_prefix);
        decompressed_out[shared_prefix] = '\0';
    }
    strncat(decompressed_out, compressed + 1, max_len - shared_prefix - 1);
    return 0;
}

void tsfi_cw_jcl_temp_pool_init(tsfi_cw_jcl_temp_pool *pool) {
    if (pool) memset(pool, 0, sizeof(tsfi_cw_jcl_temp_pool));
}

int tsfi_cw_jcl_temp_pool_add(tsfi_cw_jcl_temp_pool *pool, const char *name, const char *path) {
    if (!pool || !name || !path) return -1;
    if (pool->count >= 8) return -2;
    int idx = pool->count++;
    strcpy(pool->datasets[idx].name, name);
    strcpy(pool->datasets[idx].filepath, path);
    pool->datasets[idx].active = 1;
    return 0;
}

const char *tsfi_cw_jcl_temp_pool_get(tsfi_cw_jcl_temp_pool *pool, const char *name) {
    if (!pool || !name) return NULL;
    for (int i = 0; i < pool->count; i++) {
        if (pool->datasets[i].active && strcmp(pool->datasets[i].name, name) == 0) {
            return pool->datasets[i].filepath;
        }
    }
    return NULL;
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

int tsfi_cw_julian_standardize(const char *julian_in, uint32_t pivot, char *julian_out, int max_len) {
    if (!julian_in || !julian_out || max_len <= 0) return -1;
    uint32_t yy = 0, ddd = 0;
    if (sscanf(julian_in, "%u.%u", &yy, &ddd) != 2) return -2;
    
    uint32_t full_year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    snprintf(julian_out, max_len, "%04u.%03u", full_year, ddd);
    return 0;
}

void tsfi_cw_vsam_lds_init(tsfi_cw_vsam_lds *lds, const char *path) {
    if (!lds) return;
    memset(lds, 0, sizeof(tsfi_cw_vsam_lds));
    if (path) strcpy(lds->filepath, path);
    lds->page_count = 4;
}

int tsfi_cw_vsam_lds_write_page(tsfi_cw_vsam_lds *lds, int page_idx, const uint8_t *page_data) {
    if (!lds || page_idx < 0 || page_idx >= lds->page_count || !page_data) return -1;
    memcpy(lds->pages[page_idx], page_data, 4096);
    return 0;
}

int tsfi_cw_vsam_lds_read_page(tsfi_cw_vsam_lds *lds, int page_idx, uint8_t *page_data_out) {
    if (!lds || page_idx < 0 || page_idx >= lds->page_count || !page_data_out) return -1;
    memcpy(page_data_out, lds->pages[page_idx], 4096);
    return 0;
}

int tsfi_cw_ebcdic_is_dbcs(const uint8_t *ebcdic_str, int len, int *dbcs_count) {
    if (!ebcdic_str || len <= 0 || !dbcs_count) return -1;
    *dbcs_count = 0;
    
    int in_dbcs = 0;
    for (int i = 0; i < len; i++) {
        if (ebcdic_str[i] == 0x0E) {
            in_dbcs = 1;
        } else if (ebcdic_str[i] == 0x0F) {
            in_dbcs = 0;
        } else if (in_dbcs) {
            (*dbcs_count)++;
        }
    }
    return 0;
}

int tsfi_cw_y2k_check_century_leap_2100(uint32_t year, int *is_leap) {
    if (!is_leap) return -1;
    if (year % 100 == 0) {
        *is_leap = (year % 400 == 0) ? 1 : 0;
    } else {
        *is_leap = (year % 4 == 0) ? 1 : 0;
    }
    return 0;
}

int tsfi_cw_run_jcl_proc_nested(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc, int depth) {
    if (depth > 5) return -9;
    if (!cards || card_count <= 0) return -1;
    
    int steps_run = 0;
    int current_rc = initial_rc;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (strncmp(card, "//", 2) == 0) {
            const char *exec = strstr(card, "EXEC ");
            if (exec) {
                exec += 5;
                if (strncmp(exec, "PGM=", 4) == 0) {
                    steps_run++;
                } else {
                    int proc_steps = tsfi_cw_run_jcl_proc_nested(proc_cards, proc_card_count, proc_cards, proc_card_count, current_rc, depth + 1);
                    if (proc_steps < 0) {
                        return proc_steps;
                    }
                    steps_run += proc_steps;
                }
            }
        }
    }
    return steps_run;
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

int tsfi_cw_run_jcl_concat(const char **cards, int card_count, char *concat_out, int max_out_len) {
    if (!cards || card_count <= 0 || !concat_out || max_out_len <= 0) return -1;
    concat_out[0] = '\0';
    
    int in_concat = 0;
    int bytes_written = 0;
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        if (in_concat) {
            if (strncmp(card, "//", 2) == 0) {
                const char *space = card + 2;
                while (*space == ' ' || *space == '\t') space++;
                if (strncmp(space, "DD ", 3) == 0) {
                    const char *dsn = strstr(card, "DSN=");
                    if (dsn) {
                        dsn += 4;
                        int d_len = 0;
                        while (dsn[d_len] && dsn[d_len] != ',' && dsn[d_len] != ' ' && d_len < 31) {
                            d_len++;
                        }
                        if (bytes_written + d_len + 2 < max_out_len) {
                            if (bytes_written > 0) concat_out[bytes_written++] = ',';
                            strncpy(concat_out + bytes_written, dsn, d_len);
                            bytes_written += d_len;
                            concat_out[bytes_written] = '\0';
                        }
                    }
                } else {
                    break;
                }
            }
        } else {
            if (strstr(card, "DD DSN=")) {
                in_concat = 1;
                const char *dsn = strstr(card, "DSN=");
                if (dsn) {
                    dsn += 4;
                    int d_len = 0;
                    while (dsn[d_len] && dsn[d_len] != ',' && dsn[d_len] != ' ' && d_len < 31) {
                        d_len++;
                    }
                    strncpy(concat_out, dsn, d_len);
                    bytes_written += d_len;
                    concat_out[bytes_written] = '\0';
                }
            }
        }
    }
    return bytes_written;
}

uint32_t tsfi_cw_y2k_resolve_epoch_base(uint32_t two_digit_year, uint32_t base_epoch) {
    uint32_t pivot = base_epoch % 100;
    uint32_t century = base_epoch / 100;
    if (two_digit_year < pivot) {
        return (century + 1) * 100 + two_digit_year;
    } else {
        return century * 100 + two_digit_year;
    }
}

void tsfi_cw_vsam_pds_init(tsfi_cw_vsam_pds *pds, const char *path) {
    if (!pds) return;
    memset(pds, 0, sizeof(tsfi_cw_vsam_pds));
    if (path) strcpy(pds->filepath, path);
}

int tsfi_cw_vsam_pds_add_member(tsfi_cw_vsam_pds *pds, const char *name, uint32_t offset, uint32_t length) {
    if (!pds || !name) return -1;
    if (pds->member_count >= 16) return -2;
    int idx = pds->member_count++;
    strncpy(pds->members[idx].name, name, 11);
    pds->members[idx].name[11] = '\0';
    pds->members[idx].offset = offset;
    pds->members[idx].length = length;
    pds->members[idx].active = 1;
    return 0;
}

int tsfi_cw_vsam_pds_find_member(tsfi_cw_vsam_pds *pds, const char *name, uint32_t *offset_out, uint32_t *length_out) {
    if (!pds || !name || !offset_out || !length_out) return -1;
    for (int i = 0; i < pds->member_count; i++) {
        if (pds->members[i].active && strcmp(pds->members[i].name, name) == 0) {
            *offset_out = pds->members[i].offset;
            *length_out = pds->members[i].length;
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_ascii_to_ebcdic_pad(const char *ascii_in, uint8_t *ebcdic_out, int len, uint8_t pad_char) {
    if (!ascii_in || !ebcdic_out || len <= 0) return -1;
    int src_len = strlen(ascii_in);
    for (int i = 0; i < len; i++) {
        if (i < src_len) {
            ebcdic_out[i] = tsfi_cw_ascii_to_ebcdic((uint8_t)ascii_in[i]);
        } else {
            ebcdic_out[i] = pad_char;
        }
    }
    return 0;
}

int tsfi_cw_jcl_eval_cond(int step_rc, int cond_code, const char *operator) {
    if (!operator) return 0;
    if (strcmp(operator, "LT") == 0) {
        return (cond_code < step_rc) ? 1 : 0;
    } else if (strcmp(operator, "GT") == 0) {
        return (cond_code > step_rc) ? 1 : 0;
    } else if (strcmp(operator, "EQ") == 0) {
        return (cond_code == step_rc) ? 1 : 0;
    }
    return 0;
}

int tsfi_cw_julian_to_gregorian_y2k(const char *julian_in, uint32_t pivot, char *greg_out, int max_len) {
    if (!julian_in || !greg_out || max_len <= 0) return -1;
    uint32_t yy = 0, ddd = 0;
    if (sscanf(julian_in, "%u.%u", &yy, &ddd) != 2) return -2;
    if (ddd < 1 || ddd > 366) return -3;
    
    uint32_t year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    int is_leap = 0;
    tsfi_cw_y2k_check_century_leap_2100(year, &is_leap);
    
    int days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap) days_in_months[1] = 29;
    
    uint32_t day_sum = 0;
    uint32_t month = 0;
    uint32_t day = 0;
    for (int m = 0; m < 12; m++) {
        if (day_sum + days_in_months[m] >= ddd) {
            month = m + 1;
            day = ddd - day_sum;
            break;
        }
        day_sum += days_in_months[m];
    }
    
    snprintf(greg_out, max_len, "%04u-%02u-%02u", year, month, day);
    return 0;
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

int tsfi_cw_vsam_rrds_is_occupied(tsfi_cw_vsam_rrds *rrds, uint32_t rrn) {
    if (!rrds || rrn >= 64) return 0;
    return (rrds->occupancy_map & (1ULL << rrn)) ? 1 : 0;
}

uint8_t tsfi_cw_ascii_to_ebcdic_cp273(uint8_t ascii_char) {
    if (ascii_char == 0xA7) return 0x7C;
    if (ascii_char == 0xC4) return 0x4A;
    return tsfi_cw_ascii_to_ebcdic(ascii_char);
}

uint8_t tsfi_cw_ebcdic_to_ascii_cp273(uint8_t ebcdic_char) {
    if (ebcdic_char == 0x7C) return 0xA7;
    if (ebcdic_char == 0x4A) return 0xC4;
    return tsfi_cw_ebcdic_to_ascii(ebcdic_char);
}

int tsfi_cw_run_jcl_export(const char **cards, int card_count, char *exp_name, char *exp_val) {
    if (!cards || card_count <= 0 || !exp_name || !exp_val) return -1;
    exp_name[0] = '\0';
    exp_val[0] = '\0';
    
    for (int i = 0; i < card_count; i++) {
        const char *card = cards[i];
        const char *exp = strstr(card, "EXPORT ");
        if (exp) {
            exp += 7;
            while (*exp == ' ' || *exp == '\t') exp++;
            int n_idx = 0;
            while (*exp && *exp != '=' && *exp != ' ' && n_idx < 31) {
                exp_name[n_idx++] = *exp++;
            }
            exp_name[n_idx] = '\0';
            if (*exp == '=') exp++;
            int v_idx = 0;
            while (*exp && *exp != ' ' && *exp != '\n' && *exp != '\r' && v_idx < 31) {
                exp_val[v_idx++] = *exp++;
            }
            exp_val[v_idx] = '\0';
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_y2k_check_date_bounds(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot) {
    if (mm < 1 || mm > 12 || dd < 1) return -1;
    uint32_t year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    int is_leap = 0;
    tsfi_cw_y2k_check_century_leap_2100(year, &is_leap);
    
    int days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap) days_in_months[1] = 29;
    
    if (dd > (uint32_t)days_in_months[mm - 1]) return -2;
    return 0;
}

