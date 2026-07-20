#include "tsfi_encodings.h"
#include <string.h>
#include <ctype.h>

// 1. ASCII
int tsfi_encode_ascii(const char *in, uint8_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int len = 0;
    while (in[len] && len < max_len) {
        out[len] = (uint8_t)in[len];
        len++;
    }
    return len;
}

int tsfi_decode_ascii(const uint8_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0 || len < 0) return -1;
    int limit = len < max_len - 1 ? len : max_len - 1;
    for (int i = 0; i < limit; i++) {
        out[i] = (char)in[i];
    }
    out[limit] = '\0';
    return limit;
}

// 2. EBCDIC (Simplified subset mapping for test)
static uint8_t to_ebcdic(char c) {
    c = toupper((unsigned char)c);
    if (c >= 'A' && c <= 'I') return 0xC1 + (c - 'A');
    if (c >= 'J' && c <= 'R') return 0xD1 + (c - 'J');
    if (c >= 'S' && c <= 'Z') return 0xE2 + (c - 'S');
    if (c >= '0' && c <= '9') return 0xF0 + (c - '0');
    if (c == ' ') return 0x40;
    return 0x6F; // question mark default
}

static char from_ebcdic(uint8_t e) {
    if (e >= 0xC1 && e <= 0xC9) return 'A' + (e - 0xC1);
    if (e >= 0xD1 && e <= 0xD9) return 'J' + (e - 0xD1);
    if (e >= 0xE2 && e <= 0xE9) return 'S' + (e - 0xE2);
    if (e >= 0xF0 && e <= 0xF9) return '0' + (e - 0xF0);
    if (e == 0x40) return ' ';
    return '?';
}

int tsfi_encode_ebcdic(const char *in, uint8_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int len = 0;
    while (in[len] && len < max_len) {
        out[len] = to_ebcdic(in[len]);
        len++;
    }
    return len;
}

int tsfi_decode_ebcdic(const uint8_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0 || len < 0) return -1;
    int limit = len < max_len - 1 ? len : max_len - 1;
    for (int i = 0; i < limit; i++) {
        out[i] = from_ebcdic(in[i]);
    }
    out[limit] = '\0';
    return limit;
}

// 3. Morse Code (simplified representation using space separators)
static const char *morse_table[36] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
    ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R
    "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", // S-Z
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----." // 0-9
};

int tsfi_encode_morse(const char *in, char *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int out_idx = 0;
    for (int i = 0; in[i] != '\0'; i++) {
        char c = toupper((unsigned char)in[i]);
        const char *code = NULL;
        if (c >= 'A' && c <= 'Z') code = morse_table[c - 'A'];
        else if (c >= '0' && c <= '9') code = morse_table[c - '0' + 26];
        else if (c == ' ') code = "/";
        
        if (code) {
            int code_len = strlen(code);
            if (out_idx + code_len + 1 >= max_len) break;
            if (out_idx > 0) out[out_idx++] = ' ';
            strcpy(out + out_idx, code);
            out_idx += code_len;
        }
    }
    out[out_idx] = '\0';
    return out_idx;
}

int tsfi_decode_morse(const char *in, char *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    char temp[256];
    int t_idx = 0;
    int out_idx = 0;
    
    for (int i = 0; ; i++) {
        if (in[i] == ' ' || in[i] == '\0') {
            if (t_idx > 0) {
                temp[t_idx] = '\0';
                char matched = '?';
                if (strcmp(temp, "/") == 0) {
                    matched = ' ';
                } else {
                    for (int j = 0; j < 36; j++) {
                        if (strcmp(temp, morse_table[j]) == 0) {
                            if (j < 26) matched = 'A' + j;
                            else matched = '0' + (j - 26);
                            break;
                        }
                    }
                }
                if (out_idx < max_len - 1) {
                    out[out_idx++] = matched;
                }
                t_idx = 0;
            }
            if (in[i] == '\0') break;
        } else {
            if (t_idx < 255) {
                temp[t_idx++] = in[i];
            }
        }
    }
    out[out_idx] = '\0';
    return out_idx;
}

// 4. Hollerith Code
int tsfi_encode_hollerith(const char *in, uint16_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int len = 0;
    while (in[len] && len < max_len) {
        char c = toupper((unsigned char)in[len]);
        uint16_t val = 0;
        if (c >= 'A' && c <= 'I') val = 0x800 | (1 << (c - 'A'));
        else if (c >= 'J' && c <= 'R') val = 0x400 | (1 << (c - 'J'));
        else if (c >= 'S' && c <= 'Z') val = 0x200 | (1 << (c - 'S' + 1));
        else if (c >= '1' && c <= '9') val = 1 << (c - '1');
        else if (c == '0') val = 0x200;
        else if (c == ' ') val = 0;
        out[len] = val;
        len++;
    }
    return len;
}

int tsfi_decode_hollerith(const uint16_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0 || len < 0) return -1;
    int limit = len < max_len - 1 ? len : max_len - 1;
    for (int i = 0; i < limit; i++) {
        uint16_t val = in[i];
        char c = '?';
        if (val == 0) c = ' ';
        else if (val == 0x200) c = '0';
        else if (val & 0x800) {
            uint16_t row = val & 0x1FF;
            for (int r = 0; r < 9; r++) {
                if (row == (1 << r)) { c = 'A' + r; break; }
            }
        } else if (val & 0x400) {
            uint16_t row = val & 0x1FF;
            for (int r = 0; r < 9; r++) {
                if (row == (1 << r)) { c = 'J' + r; break; }
            }
        } else if (val & 0x200) {
            uint16_t row = val & 0x1FF;
            for (int r = 1; r < 9; r++) {
                if (row == (1 << r)) { c = 'S' + r - 1; break; }
            }
        } else {
            for (int r = 0; r < 9; r++) {
                if (val == (1 << r)) { c = '1' + r; break; }
            }
        }
        out[i] = c;
    }
    out[limit] = '\0';
    return limit;
}

// 5. Baudot Code (ITA2)
static const uint8_t ltrs_map[32] = {
    0, 'E', '\n', 'A', ' ', 'S', 'I', 'U', '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
    'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q', 'O', 'B', 'G', 0, 'M', 'X', 'V', 'Z'
};
static const uint8_t figs_map[32] = {
    0, '3', '\n', '-', ' ', '\'', '8', '7', '\r', '$', '4', '\'', ',', '!', ':', '(',
    '5', '+', ')', '2', '#', '6', '0', '1', '9', '?', '&', 0, '.', '/', '=', 0
};

int tsfi_encode_baudot(const char *in, uint8_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int out_idx = 0;
    bool is_figs = false;
    for (int i = 0; in[i] != '\0' && out_idx < max_len; i++) {
        char c = toupper((unsigned char)in[i]);
        int code = -1;
        bool target_figs = false;
        
        for (int j = 0; j < 32; j++) {
            if (ltrs_map[j] == c) { code = j; target_figs = false; break; }
        }
        if (code == -1) {
            for (int j = 0; j < 32; j++) {
                if (figs_map[j] == c) { code = j; target_figs = true; break; }
            }
        }
        if (code != -1) {
            if (target_figs && !is_figs) {
                if (out_idx < max_len) out[out_idx++] = 0x1B; // FIGS shift
                is_figs = true;
            } else if (!target_figs && is_figs) {
                if (out_idx < max_len) out[out_idx++] = 0x1F; // LTRS shift
                is_figs = false;
            }
            if (out_idx < max_len) out[out_idx++] = (uint8_t)code;
        }
    }
    return out_idx;
}

int tsfi_decode_baudot(const uint8_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0 || len < 0) return -1;
    int out_idx = 0;
    bool is_figs = false;
    for (int i = 0; i < len && out_idx < max_len - 1; i++) {
        uint8_t code = in[i] & 0x1F;
        if (code == 0x1B) { is_figs = true; continue; }
        if (code == 0x1F) { is_figs = false; continue; }
        char c = is_figs ? figs_map[code] : ltrs_map[code];
        if (c != '\0') out[out_idx++] = c;
    }
    out[out_idx] = '\0';
    return out_idx;
}

// 6. BCDIC
int tsfi_encode_bcdic(const char *in, uint8_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int len = 0;
    while (in[len] && len < max_len) {
        char c = toupper((unsigned char)in[len]);
        uint8_t b = 0;
        if (c >= 'A' && c <= 'I') b = 0x11 + (c - 'A');
        else if (c >= 'J' && c <= 'R') b = 0x21 + (c - 'J');
        else if (c >= 'S' && c <= 'Z') b = 0x32 + (c - 'S');
        else if (c >= '1' && c <= '9') b = 0x01 + (c - '1');
        else if (c == '0') b = 0x0A;
        else if (c == ' ') b = 0x10;
        out[len] = b;
        len++;
    }
    return len;
}

int tsfi_decode_bcdic(const uint8_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0 || len < 0) return -1;
    int limit = len < max_len - 1 ? len : max_len - 1;
    for (int i = 0; i < limit; i++) {
        uint8_t b = in[i];
        char c = '?';
        if (b >= 0x11 && b <= 0x19) c = 'A' + (b - 0x11);
        else if (b >= 0x21 && b <= 0x29) c = 'J' + (b - 0x21);
        else if (b >= 0x32 && b <= 0x39) c = 'S' + (b - 0x32);
        else if (b >= 0x01 && b <= 0x09) c = '1' + (b - 0x01);
        else if (b == 0x0A) c = '0';
        else if (b == 0x10) c = ' ';
        out[i] = c;
    }
    out[limit] = '\0';
    return limit;
}

// 7. FIELDATA
int tsfi_encode_fieldata(const char *in, uint8_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int len = 0;
    while (in[len] && len < max_len) {
        char c = toupper((unsigned char)in[len]);
        uint8_t f = 0;
        if (c >= 'A' && c <= 'Z') f = 0x06 + (c - 'A');
        else if (c >= '0' && c <= '9') f = 0x30 + (c - '0');
        else if (c == ' ') f = 0x05;
        out[len] = f;
        len++;
    }
    return len;
}

int tsfi_decode_fieldata(const uint8_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0 || len < 0) return -1;
    int limit = len < max_len - 1 ? len : max_len - 1;
    for (int i = 0; i < limit; i++) {
        uint8_t f = in[i];
        char c = '?';
        if (f >= 0x06 && f <= 0x1F) c = 'A' + (f - 0x06);
        else if (f >= 0x30 && f <= 0x39) c = '0' + (f - 0x30);
        else if (f == 0x05) c = ' ';
        out[i] = c;
    }
    out[limit] = '\0';
    return limit;
}

// 8. Murray Code (ITA2 Variant layout for test mapping validation)
int tsfi_encode_murray(const char *in, uint8_t *out, int max_len) {
    return tsfi_encode_baudot(in, out, max_len);
}

int tsfi_decode_murray(const uint8_t *in, int len, char *out, int max_len) {
    return tsfi_decode_baudot(in, len, out, max_len);
}

// 9. Radix-50
static int get_radix50_val(char c) {
    c = toupper((unsigned char)c);
    if (c == ' ') return 0;
    if (c >= 'A' && c <= 'Z') return 1 + (c - 'A');
    if (c == '$') return 27;
    if (c == '.') return 28;
    if (c >= '0' && c <= '9') return 30 + (c - '0');
    return 0;
}

static char from_radix50_val(int v) {
    if (v == 0) return ' ';
    if (v >= 1 && v <= 26) return 'A' + (v - 1);
    if (v == 27) return '$';
    if (v == 28) return '.';
    if (v >= 30 && v <= 39) return '0' + (v - 30);
    return '?';
}

int tsfi_encode_radix50(const char *in, uint16_t *out, int max_len) {
    if (!in || !out || max_len <= 0) return -1;
    int len = strlen(in);
    int out_idx = 0;
    for (int i = 0; i < len && out_idx < max_len; i += 3) {
        int v1 = get_radix50_val(in[i]);
        int v2 = (i + 1 < len) ? get_radix50_val(in[i+1]) : 0;
        int v3 = (i + 2 < len) ? get_radix50_val(in[i+2]) : 0;
        out[out_idx++] = (uint16_t)(((v1 * 40) + v2) * 40 + v3);
    }
    return out_idx;
}

int tsfi_decode_radix50(const uint16_t *in, int len, char *out, int max_len) {
    if (!in || !out || max_len <= 0 || len < 0) return -1;
    int out_idx = 0;
    for (int i = 0; i < len && out_idx < max_len - 3; i++) {
        uint16_t val = in[i];
        int v3 = val % 40;
        val /= 40;
        int v2 = val % 40;
        val /= 40;
        int v1 = val % 40;
        out[out_idx++] = from_radix50_val(v1);
        out[out_idx++] = from_radix50_val(v2);
        out[out_idx++] = from_radix50_val(v3);
    }
    out[out_idx] = '\0';
    return out_idx;
}
