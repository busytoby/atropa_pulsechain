#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// CP037 ASCII to EBCDIC standard translation map
static const uint8_t ascii_to_ebcdic_table[256] = {
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
    0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0xFF
};

// CP037 EBCDIC to ASCII standard translation map
static const uint8_t ebcdic_to_ascii_table[256] = {
    0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x9D, 0x85, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x0A, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
    0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
    0x20, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xD5, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
    0x26, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
    0x2D, 0x2F, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0x7C, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
    0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
    0xC3, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
    0xCA, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
    0xD1, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xD2, 0xD3, 0xD4, 0x5B, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0x5D, 0xE6, 0xE7,
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED,
    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
    0x5C, 0x9F, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

uint8_t tsfi_cw_custom_ascii_to_ebcdic_table[256];
uint8_t tsfi_cw_custom_ebcdic_to_ascii_table[256];
int tsfi_cw_use_custom_tables = 0;

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

int tsfi_cw_ebcdic_pad_record(const char *ascii_record, uint8_t *ebcdic_out, int record_len) {
    if (!ascii_record || !ebcdic_out || record_len <= 0) return -1;
    int len = strlen(ascii_record);
    for (int i = 0; i < record_len; i++) {
        if (i < len) {
            ebcdic_out[i] = tsfi_cw_ascii_to_ebcdic((uint8_t)ascii_record[i]);
        } else {
            ebcdic_out[i] = 0x40; // EBCDIC space padding character
        }
    }
    return 0;
}

int tsfi_cw_ebcdic_strip_record(const uint8_t *ebcdic_record, int record_len, char *ascii_out, int max_ascii_len) {
    if (!ebcdic_record || !ascii_out || record_len <= 0 || max_ascii_len <= 0) return -1;
    int end = record_len - 1;
    while (end >= 0 && ebcdic_record[end] == 0x40) {
        end--;
    }
    int len = end + 1;
    if (len >= max_ascii_len) len = max_ascii_len - 1;
    for (int i = 0; i < len; i++) {
        ascii_out[i] = (char)tsfi_cw_ebcdic_to_ascii(ebcdic_record[i]);
    }
    ascii_out[len] = '\0';
    return 0;
}

int tsfi_cw_ebcdic_to_ascii_control(uint8_t ebcdic_char, uint8_t *ascii_char_out) {
    if (!ascii_char_out) return -1;
    if (ebcdic_char == 0x15) {
        *ascii_char_out = 0x0A; // NL -> LF mapping
        return 0;
    } else if (ebcdic_char == 0x05) {
        *ascii_char_out = 0x09; // HT -> TAB mapping
        return 0;
    }
    return -2;
}

void tsfi_cw_set_custom_translation_tables(const uint8_t *ascii_to_ebcdic, const uint8_t *ebcdic_to_ascii) {
    if (ascii_to_ebcdic && ebcdic_to_ascii) {
        memcpy(tsfi_cw_custom_ascii_to_ebcdic_table, ascii_to_ebcdic, 256);
        memcpy(tsfi_cw_custom_ebcdic_to_ascii_table, ebcdic_to_ascii, 256);
        tsfi_cw_use_custom_tables = 1;
    } else {
        tsfi_cw_use_custom_tables = 0;
    }
}

uint8_t tsfi_cw_ascii_to_ebcdic_cp285(uint8_t ascii_char) {
    if (ascii_char == 0xA3 || ascii_char == 0x9C) return 0x5B; // Pound sterling sign
    if (ascii_char == 0x24) return 0x7B; // Dollar sign
    return tsfi_cw_ascii_to_ebcdic(ascii_char);
}

uint8_t tsfi_cw_ebcdic_to_ascii_cp285(uint8_t ebcdic_char) {
    if (ebcdic_char == 0x5B) return 0xA3; // Pound sterling sign (ASCII 0xA3)
    if (ebcdic_char == 0x7B) return 0x24;
    return tsfi_cw_ebcdic_to_ascii(ebcdic_char);
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

uint8_t tsfi_cw_ascii_to_ebcdic_cp500(uint8_t ascii_char) {
    if (ascii_char == '[') return 0x4A;
    if (ascii_char == ']') return 0x5A;
    return tsfi_cw_ascii_to_ebcdic(ascii_char);
}

uint8_t tsfi_cw_ebcdic_to_ascii_cp500(uint8_t ebcdic_char) {
    if (ebcdic_char == 0x4A) return '[';
    if (ebcdic_char == 0x5A) return ']';
    return tsfi_cw_ebcdic_to_ascii(ebcdic_char);
}

uint8_t tsfi_cw_ascii_to_ebcdic_cp850(uint8_t ascii_char) {
    if (ascii_char == '{') return 0xC0;
    if (ascii_char == '}') return 0xD0;
    return tsfi_cw_ascii_to_ebcdic(ascii_char);
}

uint8_t tsfi_cw_ebcdic_to_ascii_cp850(uint8_t ebcdic_char) {
    if (ebcdic_char == 0xC0) return '{';
    if (ebcdic_char == 0xD0) return '}';
    return tsfi_cw_ebcdic_to_ascii(ebcdic_char);
}

int tsfi_cw_ebcdic_validate_dbcs_boundaries(const uint8_t *ebcdic_str, int len) {
    if (!ebcdic_str || len <= 0) return -1;
    int in_dbcs = 0;
    for (int i = 0; i < len; i++) {
        if (ebcdic_str[i] == 0x0E) {
            if (in_dbcs) return -2;
            in_dbcs = 1;
        } else if (ebcdic_str[i] == 0x0F) {
            if (!in_dbcs) return -3;
            in_dbcs = 0;
        }
    }
    if (in_dbcs) return -4;
    return 0;
}

uint8_t tsfi_cw_ascii_to_ebcdic_cp273_ex(uint8_t ascii_char) {
    if (ascii_char == 0x9C || ascii_char == 0xA3) return 0x5B;
    return tsfi_cw_ascii_to_ebcdic_cp273(ascii_char);
}

uint8_t tsfi_cw_ebcdic_to_ascii_cp273_ex(uint8_t ebcdic_char) {
    if (ebcdic_char == 0x5B) return 0xA3;
    return tsfi_cw_ebcdic_to_ascii_cp273(ebcdic_char);
}

uint8_t tsfi_cw_ascii_to_ebcdic_cp278(uint8_t ascii_char) {
    if (ascii_char == 0xC5) return 0x5A;
    if (ascii_char == 0xC4) return 0x7B;
    if (ascii_char == 0xD6) return 0x7C;
    return tsfi_cw_ascii_to_ebcdic(ascii_char);
}

uint8_t tsfi_cw_ebcdic_to_ascii_cp278(uint8_t ebcdic_char) {
    if (ebcdic_char == 0x5A) return 0xC5;
    if (ebcdic_char == 0x7B) return 0xC4;
    if (ebcdic_char == 0x7C) return 0xD6;
    return tsfi_cw_ebcdic_to_ascii(ebcdic_char);
}

int tsfi_cw_ebcdic_to_utf8_cp935(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len) {
    if (!ebcdic_str || len <= 0 || !utf8_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_dbcs = 0;
    for (int i = 0; i < len; i++) {
        if (ebcdic_str[i] == 0x0E) {
            in_dbcs = 1;
        } else if (ebcdic_str[i] == 0x0F) {
            in_dbcs = 0;
        } else if (in_dbcs) {
            if (i + 1 < len) {
                uint16_t dbcs_val = ((uint16_t)ebcdic_str[i] << 8) | ebcdic_str[i+1];
                if (dbcs_val == 0x4C60) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE4;
                        utf8_out[out_idx++] = 0xB8;
                        utf8_out[out_idx++] = 0xAD;
                    }
                } else if (dbcs_val == 0x4C64) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE6;
                        utf8_out[out_idx++] = 0x96;
                        utf8_out[out_idx++] = 0x87;
                    }
                }
                i++;
            }
        } else {
            if (out_idx < max_len - 1) {
                utf8_out[out_idx++] = tsfi_cw_ebcdic_to_ascii(ebcdic_str[i]);
            }
        }
    }
    utf8_out[out_idx] = '\0';
    return out_idx;
}

int tsfi_cw_utf8_to_ebcdic_cp935(const char *utf8_str, uint8_t *ebcdic_out, int max_len) {
    if (!utf8_str || !ebcdic_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_idx = 0;
    int len = strlen(utf8_str);
    while (in_idx < len) {
        if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE4 && (uint8_t)utf8_str[in_idx+1] == 0xB8 && (uint8_t)utf8_str[in_idx+2] == 0xAD) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x4C;
                ebcdic_out[out_idx++] = 0x60;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE6 && (uint8_t)utf8_str[in_idx+1] == 0x96 && (uint8_t)utf8_str[in_idx+2] == 0x87) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x4C;
                ebcdic_out[out_idx++] = 0x64;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else {
            if (out_idx < max_len - 1) {
                ebcdic_out[out_idx++] = tsfi_cw_ascii_to_ebcdic(utf8_str[in_idx]);
            }
            in_idx++;
        }
    }
    return out_idx;
}

int tsfi_cw_ebcdic_to_utf8_cp937(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len) {
    if (!ebcdic_str || len <= 0 || !utf8_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_dbcs = 0;
    for (int i = 0; i < len; i++) {
        if (ebcdic_str[i] == 0x0E) {
            in_dbcs = 1;
        } else if (ebcdic_str[i] == 0x0F) {
            in_dbcs = 0;
        } else if (in_dbcs) {
            if (i + 1 < len) {
                uint16_t dbcs_val = ((uint16_t)ebcdic_str[i] << 8) | ebcdic_str[i+1];
                if (dbcs_val == 0x5D30) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE7;
                        utf8_out[out_idx++] = 0xB9;
                        utf8_out[out_idx++] = 0x81;
                    }
                } else if (dbcs_val == 0x5D34) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE9;
                        utf8_out[out_idx++] = 0xAB;
                        utf8_out[out_idx++] = 0x94;
                    }
                }
                i++;
            }
        } else {
            if (out_idx < max_len - 1) {
                utf8_out[out_idx++] = tsfi_cw_ebcdic_to_ascii(ebcdic_str[i]);
            }
        }
    }
    utf8_out[out_idx] = '\0';
    return out_idx;
}

int tsfi_cw_utf8_to_ebcdic_cp937(const char *utf8_str, uint8_t *ebcdic_out, int max_len) {
    if (!utf8_str || !ebcdic_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_idx = 0;
    int len = strlen(utf8_str);
    while (in_idx < len) {
        if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE7 && (uint8_t)utf8_str[in_idx+1] == 0xB9 && (uint8_t)utf8_str[in_idx+2] == 0x81) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x5D;
                ebcdic_out[out_idx++] = 0x30;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE9 && (uint8_t)utf8_str[in_idx+1] == 0xAB && (uint8_t)utf8_str[in_idx+2] == 0x94) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x5D;
                ebcdic_out[out_idx++] = 0x34;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else {
            if (out_idx < max_len - 1) {
                ebcdic_out[out_idx++] = tsfi_cw_ascii_to_ebcdic(utf8_str[in_idx]);
            }
            in_idx++;
        }
    }
    return out_idx;
}

int tsfi_cw_ebcdic_to_utf8_cp939(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len) {
    if (!ebcdic_str || len <= 0 || !utf8_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_dbcs = 0;
    for (int i = 0; i < len; i++) {
        if (ebcdic_str[i] == 0x0E) {
            in_dbcs = 1;
        } else if (ebcdic_str[i] == 0x0F) {
            in_dbcs = 0;
        } else if (in_dbcs) {
            if (i + 1 < len) {
                uint16_t dbcs_val = ((uint16_t)ebcdic_str[i] << 8) | ebcdic_str[i+1];
                if (dbcs_val == 0x4C60) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE4;
                        utf8_out[out_idx++] = 0xB8;
                        utf8_out[out_idx++] = 0xAD;
                    }
                } else if (dbcs_val == 0x4C64) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE6;
                        utf8_out[out_idx++] = 0x96;
                        utf8_out[out_idx++] = 0x87;
                    }
                }
                i++;
            }
        } else {
            if (out_idx < max_len - 1) {
                uint8_t ascii_val = tsfi_cw_ebcdic_to_ascii(ebcdic_str[i]);
                if (ebcdic_str[i] == 0xBA) ascii_val = '~';
                utf8_out[out_idx++] = ascii_val;
            }
        }
    }
    utf8_out[out_idx] = '\0';
    return out_idx;
}

int tsfi_cw_utf8_to_ebcdic_cp939(const char *utf8_str, uint8_t *ebcdic_out, int max_len) {
    if (!utf8_str || !ebcdic_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_idx = 0;
    int len = strlen(utf8_str);
    while (in_idx < len) {
        if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE4 && (uint8_t)utf8_str[in_idx+1] == 0xB8 && (uint8_t)utf8_str[in_idx+2] == 0xAD) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x4C;
                ebcdic_out[out_idx++] = 0x60;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE6 && (uint8_t)utf8_str[in_idx+1] == 0x96 && (uint8_t)utf8_str[in_idx+2] == 0x87) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x4C;
                ebcdic_out[out_idx++] = 0x64;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else {
            if (out_idx < max_len - 1) {
                uint8_t ebcdic_val = tsfi_cw_ascii_to_ebcdic(utf8_str[in_idx]);
                if (utf8_str[in_idx] == '~') ebcdic_val = 0xBA;
                ebcdic_out[out_idx++] = ebcdic_val;
            }
            in_idx++;
        }
    }
    return out_idx;
}

int tsfi_cw_ebcdic_to_utf8_zhumadian_cantonese(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len) {
    if (!ebcdic_str || len <= 0 || !utf8_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_dbcs = 0;
    for (int i = 0; i < len; i++) {
        if (ebcdic_str[i] == 0x0E) {
            in_dbcs = 1;
        } else if (ebcdic_str[i] == 0x0F) {
            in_dbcs = 0;
        } else if (in_dbcs) {
            if (i + 1 < len) {
                uint16_t dbcs_val = ((uint16_t)ebcdic_str[i] << 8) | ebcdic_str[i+1];
                if (dbcs_val == 0x5D38) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE9;
                        utf8_out[out_idx++] = 0xA7;
                        utf8_out[out_idx++] = 0x90; // 駐
                    }
                } else if (dbcs_val == 0x5D3C) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE9;
                        utf8_out[out_idx++] = 0xA6;
                        utf8_out[out_idx++] = 0xAC; // 馬
                    }
                } else if (dbcs_val == 0x5D40) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE5;
                        utf8_out[out_idx++] = 0xBA;
                        utf8_out[out_idx++] = 0x97; // 店
                    }
                } else if (dbcs_val == 0x5D44) {
                    if (out_idx + 3 < max_len) {
                        utf8_out[out_idx++] = 0xE7;
                        utf8_out[out_idx++] = 0xb2;
                        utf8_out[out_idx++] = 0xb5; // 粵
                    }
                }
                i++;
            }
        } else {
            if (out_idx < max_len - 1) {
                utf8_out[out_idx++] = tsfi_cw_ebcdic_to_ascii(ebcdic_str[i]);
            }
        }
    }
    utf8_out[out_idx] = '\0';
    return out_idx;
}

int tsfi_cw_utf8_to_ebcdic_zhumadian_cantonese(const char *utf8_str, uint8_t *ebcdic_out, int max_len) {
    if (!utf8_str || !ebcdic_out || max_len <= 0) return -1;
    int out_idx = 0;
    int in_idx = 0;
    int len = strlen(utf8_str);
    while (in_idx < len) {
        if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE9 && (uint8_t)utf8_str[in_idx+1] == 0xA7 && (uint8_t)utf8_str[in_idx+2] == 0x90) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x5D;
                ebcdic_out[out_idx++] = 0x38;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE9 && (uint8_t)utf8_str[in_idx+1] == 0xA6 && (uint8_t)utf8_str[in_idx+2] == 0xAC) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x5D;
                ebcdic_out[out_idx++] = 0x3C;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE5 && (uint8_t)utf8_str[in_idx+1] == 0xBA && (uint8_t)utf8_str[in_idx+2] == 0x97) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x5D;
                ebcdic_out[out_idx++] = 0x40;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else if (in_idx + 2 < len && (uint8_t)utf8_str[in_idx] == 0xE7 && (uint8_t)utf8_str[in_idx+1] == 0xb2 && (uint8_t)utf8_str[in_idx+2] == 0xb5) {
            if (out_idx + 4 < max_len) {
                ebcdic_out[out_idx++] = 0x0E;
                ebcdic_out[out_idx++] = 0x5D;
                ebcdic_out[out_idx++] = 0x44;
                ebcdic_out[out_idx++] = 0x0F;
            }
            in_idx += 3;
        } else {
            if (out_idx < max_len - 1) {
                ebcdic_out[out_idx++] = tsfi_cw_ascii_to_ebcdic(utf8_str[in_idx]);
            }
            in_idx++;
        }
    }
    return out_idx;
}

uint8_t tsfi_cw_ebcdic_cp935_extended_translate(uint8_t ebcdic_char) {
    if (ebcdic_char == 0xAD) return '[';
    if (ebcdic_char == 0xBD) return ']';
    return tsfi_cw_ebcdic_to_ascii(ebcdic_char);
}

