#include "tsfi_encodings.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"

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
uint8_t g_ltrs_map[32] = {
    0, 'E', '\n', 'A', ' ', 'S', 'I', 'U', '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
    'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q', 'O', 'B', 'G', 0, 'M', 'X', 'V', 'Z'
};
uint8_t g_figs_map[32] = {
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
            if (g_ltrs_map[j] == c) { code = j; target_figs = false; break; }
        }
        if (code == -1) {
            for (int j = 0; j < 32; j++) {
                if (g_figs_map[j] == c) { code = j; target_figs = true; break; }
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
        char c = is_figs ? g_figs_map[code] : g_ltrs_map[code];
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

extern bool g_gguf_acab_found;
extern uint8_t g_gguf_acab_root[32];

// 10. Oregon Trail (OT) Baudot (Baud) LLM-Tokenized .dat.bin (DAT) exporter
int tsfi_ot_baud_llm_dat(const char *dat_bin_path) {
    if (!dat_bin_path) return -1;
    
    // Step 1: Run Oregon Trail game simulation step
    TsfiOregonTrail game;
    tsfi_oregon_trail_init(&game);
    
    // Seed initial supply parameters from ACAB memory root if present
    if (g_gguf_acab_found) {
        int extra_oxen = g_gguf_acab_root[0] % 4;
        int extra_food = (g_gguf_acab_root[1] % 5) * 100;
        int extra_bullets = (g_gguf_acab_root[2] % 5) * 50;
        tsfi_oregon_trail_buy_supplies(&game, 4 + extra_oxen, 300 + extra_food, 200 + extra_bullets);
    } else {
        tsfi_oregon_trail_buy_supplies(&game, 4, 300, 200);
    }
    
    tsfi_oregon_trail_step(&game, 0, 0);
    
    // Step 2: Format status report
    char status[128];
    snprintf(status, sizeof(status), "MILES %d FOOD %d OXEN %d", 
             game.miles_traveled, game.food_lbs, game.oxen);
             
    // Step 3: Encode status using Baudot Code (ITA2)
    uint8_t baud_buf[128];
    int baud_len = tsfi_encode_baudot(status, baud_buf, 128);
    if (baud_len <= 0) return -2;
    
    // Step 4: Tokenize Baudot stream (simulated LLM tokenization)
    uint32_t tokens[128];
    for (int i = 0; i < baud_len; i++) {
        tokens[i] = (uint32_t)baud_buf[i];
    }
    
    // Step 5: Save to binary block-ledger asset (.dat.bin)
    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) return -3;
    
    uint32_t count = (uint32_t)baud_len;
    fwrite(&count, sizeof(uint32_t), 1, f);
    fwrite(tokens, sizeof(uint32_t), count, f);
    fclose(f);
    
    return 0;
}

// 11. EER (Enhanced Entity-Relationship) Bridge for Oregon Trail (OT) over ACAB
int tsfi_eer_bridge_ot_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    // Read the binary block-ledger asset (.dat.bin)
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 128) {
        fclose(f);
        return -3;
    }
    
    uint32_t tokens[128];
    if (fread(tokens, sizeof(uint32_t), count, f) != count) {
        fclose(f);
        return -4;
    }
    fclose(f);
    
    // Decode Baudot tokens back to string
    uint8_t baud_buf[128];
    for (uint32_t i = 0; i < count; i++) {
        baud_buf[i] = (uint8_t)tokens[i];
    }
    char status_dec[128];
    int dec_len = tsfi_decode_baudot(baud_buf, count, status_dec, 128);
    if (dec_len <= 0) return -5;
    
    // Parse values (Miles) from game status
    int miles = 0;
    sscanf(status_dec, "MILES %d", &miles);
    
    // ER modeling: Create a basic incident entity
    uint32_t incident_id = 2000 + (uint32_t)miles;
    int defcon = (miles < 100) ? 5 : 2;
    int type = (miles < 100) ? 2 : 1; // 2 = TaxAuditConflict (basic), 1 = NuclearAlert (enhanced subclass)
    
    tsfi_eer_db_init(db);
    
    // Insert incident (ER entity definition)
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    // Insert agencies (EER specializations)
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1); // NORAD
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);   // IRS
    
    // Establish relationship mapping (ER relationship definition)
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id); // NORAD responds to NuclearAlert
    } else {
        tsfi_eer_link_response(db, 102, incident_id); // IRS responds to TaxAuditConflict
    }
    
    // Insert tapped ACAB channel relation
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped ACAB memory address
        chan->encryption_type = 3;  // STANAG 5066
        chan->frequency_band = 144000;
    }
    
    return 0;
}

// 12. Hamming(7,4) FEC Implementation
void tsfi_encode_hamming74(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    for (int i = 0; i < len; i++) {
        uint8_t b = in[i];
        for (int nibble = 0; nibble < 2; nibble++) {
            uint8_t data = (nibble == 0) ? (b & 0x0F) : ((b >> 4) & 0x0F);
            uint8_t d1 = (data >> 3) & 1;
            uint8_t d2 = (data >> 2) & 1;
            uint8_t d3 = (data >> 1) & 1;
            uint8_t d4 = data & 1;
            uint8_t p1 = d1 ^ d2 ^ d4;
            uint8_t p2 = d1 ^ d3 ^ d4;
            uint8_t p3 = d2 ^ d3 ^ d4;
            out[i * 2 + nibble] = (p1 << 6) | (p2 << 5) | (d1 << 4) | (p3 << 3) | (d2 << 2) | (d3 << 1) | d4;
        }
    }
}

void tsfi_decode_hamming74(const uint8_t *in, int coded_len, uint8_t *out) {
    if (!in || !out || coded_len <= 0) return;
    int bytes = coded_len / 2;
    for (int i = 0; i < bytes; i++) {
        uint8_t nibbles[2];
        for (int nibble = 0; nibble < 2; nibble++) {
            uint8_t code = in[i * 2 + nibble];
            uint8_t p1 = (code >> 6) & 1;
            uint8_t p2 = (code >> 5) & 1;
            uint8_t d1 = (code >> 4) & 1;
            uint8_t p3 = (code >> 3) & 1;
            uint8_t d2 = (code >> 2) & 1;
            uint8_t d3 = (code >> 1) & 1;
            uint8_t d4 = code & 1;
            
            uint8_t s1 = p1 ^ d1 ^ d2 ^ d4;
            uint8_t s2 = p2 ^ d1 ^ d3 ^ d4;
            uint8_t s3 = p3 ^ d2 ^ d3 ^ d4;
            uint8_t syndrome = (s1 << 2) | (s2 << 1) | s3;
            
            if (syndrome == 7) d4 ^= 1;
            else if (syndrome == 6) d1 ^= 1;
            else if (syndrome == 5) d2 ^= 1;
            else if (syndrome == 3) d3 ^= 1;
            
            nibbles[nibble] = (d1 << 3) | (d2 << 2) | (d3 << 1) | d4;
        }
        out[i] = nibbles[0] | (nibbles[1] << 4);
    }
}

// 13. Dynamic read-write Tone Wheel punch simulator
int tsfi_punch_tone_wheel(uint16_t *wheel, int max_cols, int col, char c) {
    if (!wheel || col < 0 || col >= max_cols) return -1;
    uint16_t val = 0;
    c = toupper((unsigned char)c);
    if (c >= 'A' && c <= 'I') val = 0x800 | (1 << (c - 'A'));
    else if (c >= 'J' && c <= 'R') val = 0x400 | (1 << (c - 'J'));
    else if (c >= 'S' && c <= 'Z') val = 0x200 | (1 << (c - 'S' + 1));
    else if (c >= '1' && c <= '9') val = 1 << (c - '1');
    else if (c == '0') val = 0x200;
    else if (c == ' ') val = 0;
    wheel[col] = val;
    return 0;
}

// 14. EER Declarative Datalog Specialization Engine
int tsfi_eer_datalog_specialization(TSFiEerDatabase *db, const char *incident_type, const char *severity) {
    if (!db || !incident_type || !severity) return -1;
    
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *rel_dat = tsfi_dat_compile_relation(trie_root, incident_type, severity, "NORAD_RESPONDER");
    if (!rel_dat) {
        tsfi_trie_destroy(trie_root);
        return -1;
    }
    
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "%s/%s/NORAD_RESPONDER", incident_type, severity);
    const char *res = tsfi_dat_search(rel_dat, search_key);
    int assigned = 0;
    if (res && strcmp(res, "RELATION_TRUE") == 0) {
        assigned = 1;
        tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    }
    
    tsfi_dat_destroy(rel_dat);
    tsfi_trie_destroy(trie_root);
    return assigned;
}

// 15. Vulkan-based Phase Convergence Visualization Projection
void tsfi_pll_vulkan_project(float error_voltage, float phase_diff, float *ndc_x, float *ndc_y) {
    if (!ndc_x || !ndc_y) return;
    if (error_voltage < -5.0f) error_voltage = -5.0f;
    if (error_voltage > 5.0f) error_voltage = 5.0f;
    if (phase_diff < -3.14159f) phase_diff = -3.14159f;
    if (phase_diff > 3.14159f) phase_diff = 3.14159f;
    
    *ndc_x = error_voltage / 5.0f;
    *ndc_y = phase_diff / 3.14159f;
}

// 16. OT Accumulator (OT) Baudot (Baud) LLM-Tokenized .dat.bin (DAT) exporter on the ACAB
int tsfi_ot_accum_baud_llm_dat(const char *dat_bin_path) {
    if (!dat_bin_path) return -1;
    
    TSFiOTAccumulator acc;
    tsfi_ot_accumulator_init(&acc);
    
    // Seed initial potential or coordinate weights from ACAB memory root if present
    float weight1 = 12.5f;
    float weight2 = 35.2f;
    if (g_gguf_acab_found) {
        weight1 += (float)g_gguf_acab_root[0] / 10.0f;
        weight2 += (float)g_gguf_acab_root[1] / 10.0f;
    }
    
    tsfi_ot_accumulator_add(&acc, "coord_001", weight1);
    tsfi_ot_accumulator_add(&acc, "coord_002", weight2);
    
    char status[128];
    snprintf(status, sizeof(status), "COUNT %d POTENTIAL %.2f", 
             acc.count, tsfi_ot_accumulator_get_potential(&acc));
             
    uint8_t baud_buf[128];
    int baud_len = tsfi_encode_baudot(status, baud_buf, 128);
    if (baud_len <= 0) return -2;
    
    uint32_t tokens[128];
    for (int i = 0; i < baud_len; i++) {
        tokens[i] = (uint32_t)baud_buf[i];
    }
    
    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) return -3;
    
    uint32_t count = (uint32_t)baud_len;
    fwrite(&count, sizeof(uint32_t), 1, f);
    fwrite(tokens, sizeof(uint32_t), count, f);
    fclose(f);
    
    return 0;
}

// 17. EER Bridge for OT Accumulator (OT) over ACAB
int tsfi_eer_bridge_ot_accum_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 128) {
        fclose(f);
        return -3;
    }
    
    uint32_t tokens[128];
    if (fread(tokens, sizeof(uint32_t), count, f) != count) {
        fclose(f);
        return -4;
    }
    fclose(f);
    
    uint8_t baud_buf[128];
    for (uint32_t i = 0; i < count; i++) {
        baud_buf[i] = (uint8_t)tokens[i];
    }
    char status_dec[128];
    int dec_len = tsfi_decode_baudot(baud_buf, count, status_dec, 128);
    if (dec_len <= 0) return -5;
    
    float potential = 0.0f;
    sscanf(status_dec, "COUNT %*d POTENTIAL %f", &potential);
    
    uint32_t incident_id = 3000 + (uint32_t)potential;
    int defcon = (potential < 50.0f) ? 5 : 1;
    int type = (potential < 50.0f) ? 2 : 1; // 2 = TaxAuditConflict (basic), 1 = NuclearAlert (enhanced subclass)
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);
    
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id);
    } else {
        tsfi_eer_link_response(db, 102, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped ACAB memory address
        chan->encryption_type = 3;
        chan->frequency_band = 144000;
    }
    
    return 0;
}

// 18. Extended Hamming(8,4) SECDED Implementation
void tsfi_encode_hamming84(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    for (int i = 0; i < len; i++) {
        uint8_t b = in[i];
        for (int nibble = 0; nibble < 2; nibble++) {
            uint8_t data = (nibble == 0) ? (b & 0x0F) : ((b >> 4) & 0x0F);
            uint8_t d1 = (data >> 3) & 1;
            uint8_t d2 = (data >> 2) & 1;
            uint8_t d3 = (data >> 1) & 1;
            uint8_t d4 = data & 1;
            uint8_t p1 = d1 ^ d2 ^ d4;
            uint8_t p2 = d1 ^ d3 ^ d4;
            uint8_t p3 = d2 ^ d3 ^ d4;
            uint8_t c = (p1 << 6) | (p2 << 5) | (d1 << 4) | (p3 << 3) | (d2 << 2) | (d3 << 1) | d4;
            uint8_t parity = 0;
            for (int bit = 0; bit < 7; bit++) {
                parity ^= (c >> bit) & 1;
            }
            out[i * 2 + nibble] = (parity << 7) | c;
        }
    }
}

int tsfi_decode_hamming84(const uint8_t *in, int coded_len, uint8_t *out) {
    if (!in || !out || coded_len <= 0) return -1;
    int bytes = coded_len / 2;
    int double_errors = 0;
    for (int i = 0; i < bytes; i++) {
        uint8_t nibbles[2];
        for (int nibble = 0; nibble < 2; nibble++) {
            uint8_t code = in[i * 2 + nibble];
            uint8_t received_parity = (code >> 7) & 1;
            uint8_t c = code & 0x7F;
            uint8_t p1 = (c >> 6) & 1;
            uint8_t p2 = (c >> 5) & 1;
            uint8_t d1 = (c >> 4) & 1;
            uint8_t p3 = (c >> 3) & 1;
            uint8_t d2 = (c >> 2) & 1;
            uint8_t d3 = (c >> 1) & 1;
            uint8_t d4 = c & 1;
            
            uint8_t s1 = p1 ^ d1 ^ d2 ^ d4;
            uint8_t s2 = p2 ^ d1 ^ d3 ^ d4;
            uint8_t s3 = p3 ^ d2 ^ d3 ^ d4;
            uint8_t syndrome = (s1 << 2) | (s2 << 1) | s3;
            
            uint8_t actual_parity = 0;
            for (int bit = 0; bit < 7; bit++) {
                actual_parity ^= (c >> bit) & 1;
            }
            
            bool parity_error = (received_parity != actual_parity);
            
            if (syndrome != 0) {
                if (parity_error) {
                    if (syndrome == 7) d4 ^= 1;
                    else if (syndrome == 6) d1 ^= 1;
                    else if (syndrome == 5) d2 ^= 1;
                    else if (syndrome == 3) d3 ^= 1;
                } else {
                    double_errors++;
                }
            }
            nibbles[nibble] = (d1 << 3) | (d2 << 2) | (d3 << 1) | d4;
        }
        out[i] = nibbles[0] | (nibbles[1] << 4);
    }
    return double_errors > 0 ? 1 : 0;
}

// 19. Lock-free SPSC Accumulator queue simulation
int tsfi_ot_accumulator_spsc_push(TSFiOTAccumulator *acc, const char *coord, float weight) {
    if (!acc || !coord) return -1;
    int current = acc->count;
    if (current >= MAX_ACCUMULATED_PATHS) return -2;
    
    strncpy(acc->entries[current].coordinate, coord, 127);
    acc->entries[current].coordinate[127] = '\0';
    acc->entries[current].path_weight = weight;
    
    acc->count = current + 1;
    acc->cumulative_potential += weight;
    return 0;
}

// 20. Cascading Datalog rules EER resolver
int tsfi_eer_datalog_cascade(TSFiEerDatabase *db, const char *entity, const char *rule1, const char *rule2) {
    if (!db || !entity || !rule1 || !rule2) return -1;
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    tsfi_dat *dat1 = tsfi_dat_compile_relation(trie_root, entity, rule1, "TRUE");
    tsfi_dat *dat2 = tsfi_dat_compile_relation(trie_root, entity, rule2, "TRUE");
    
    int resolved = 0;
    if (dat1 && dat2) {
        char key1[128], key2[128];
        snprintf(key1, sizeof(key1), "%s/%s/TRUE", entity, rule1);
        snprintf(key2, sizeof(key2), "%s/%s/TRUE", entity, rule2);
        
        const char *res1 = tsfi_dat_search(dat1, key1);
        const char *res2 = tsfi_dat_search(dat2, key2);
        
        if (res1 && strcmp(res1, "RELATION_TRUE") == 0 && res2 && strcmp(res2, "RELATION_TRUE") == 0) {
            resolved = 1;
            tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
        }
    }
    if (dat1) tsfi_dat_destroy(dat1);
    if (dat2) tsfi_dat_destroy(dat2);
    tsfi_trie_destroy(trie_root);
    return resolved;
}

// 21. Proportional-Integral (PI) PLL Loop Tuning
void tsfi_pll_pi_tune(float error_voltage, float last_integral, float kp, float ki, float dt, float *output_voltage, float *next_integral) {
    if (!output_voltage || !next_integral) return;
    float integral = last_integral + error_voltage * dt;
    *output_voltage = kp * error_voltage + ki * integral;
    *next_integral = integral;
}

// 22. Dynamic Baudot Map update API
extern uint8_t g_ltrs_map[32];
extern uint8_t g_figs_map[32];

int tsfi_baudot_update_maps(const uint8_t *new_ltrs, const uint8_t *new_figs) {
    if (!new_ltrs || !new_figs) return -1;
    memcpy(g_ltrs_map, new_ltrs, 32);
    memcpy(g_figs_map, new_figs, 32);
    return 0;
}

// 23. BCH(15,7) Encoder and Decoder
void tsfi_encode_bch15_7(const uint8_t *in, int len, uint16_t *out) {
    if (!in || !out || len <= 0) return;
    for (int i = 0; i < len; i++) {
        uint16_t msg = in[i] & 0x7F;
        uint32_t codeword = msg << 8;
        for (int bit = 14; bit >= 8; bit--) {
            if (codeword & (1 << bit)) {
                codeword ^= (0x1D5 << (bit - 8));
            }
        }
        out[i] = (msg << 8) | (codeword & 0xFF);
    }
}

int tsfi_decode_bch15_7(const uint16_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return -1;
    int uncorrectable = 0;
    for (int i = 0; i < len; i++) {
        uint16_t cw = in[i] & 0x7FFF;
        uint32_t rem = cw;
        for (int bit = 14; bit >= 8; bit--) {
            if (rem & (1 << bit)) {
                rem ^= (0x1D5 << (bit - 8));
            }
        }
        if (rem != 0) {
            int corrected = 0;
            for (int b1 = 0; b1 < 15 && !corrected; b1++) {
                uint16_t temp1 = cw ^ (1 << b1);
                uint32_t rem1 = temp1;
                for (int bit = 14; bit >= 8; bit--) {
                    if (rem1 & (1 << bit)) rem1 ^= (0x1D5 << (bit - 8));
                }
                if (rem1 == 0) {
                    cw = temp1;
                    corrected = 1;
                }
            }
            for (int b1 = 0; b1 < 15 && !corrected; b1++) {
                for (int b2 = b1 + 1; b2 < 15 && !corrected; b2++) {
                    uint16_t temp2 = cw ^ (1 << b1) ^ (1 << b2);
                    uint32_t rem2 = temp2;
                    for (int bit = 14; bit >= 8; bit--) {
                        if (rem2 & (1 << bit)) rem2 ^= (0x1D5 << (bit - 8));
                    }
                    if (rem2 == 0) {
                        cw = temp2;
                        corrected = 1;
                    }
                }
            }
            if (!corrected) {
                uncorrectable++;
            }
        }
        out[i] = (uint8_t)(cw >> 8);
    }
    return uncorrectable;
}

// 24. PID PLL with AGC Tuning
void tsfi_pll_pid_agc_tune(float error_voltage, float last_integral, float last_error, float kp, float ki, float kd, float dt, float input_signal_amp, float *output_voltage, float *next_integral, float *next_error, float *agc_gain) {
    if (!output_voltage || !next_integral || !next_error || !agc_gain) return;
    float integral = last_integral + error_voltage * dt;
    float derivative = (error_voltage - last_error) / dt;
    float target_amp = 1.0f;
    float gain = 1.0f;
    if (input_signal_amp > 0.01f) {
        gain = target_amp / input_signal_amp;
    }
    *output_voltage = (kp * error_voltage + ki * integral + kd * derivative) * gain;
    *next_integral = integral;
    *next_error = error_voltage;
    *agc_gain = gain;
}

// 25. EER Referential Integrity Cascade Delete
int tsfi_eer_delete_incident(TSFiEerDatabase *db, uint32_t incident_id) {
    if (!db) return -1;
    int found_idx = -1;
    for (int i = 0; i < db->incident_count; i++) {
        if (db->incidents[i].incident_id == incident_id) {
            found_idx = i;
            break;
        }
    }
    if (found_idx == -1) return -2;
    for (int i = found_idx; i < db->incident_count - 1; i++) {
        db->incidents[i] = db->incidents[i + 1];
    }
    db->incident_count--;
    int r = 0;
    while (r < db->responds_count) {
        if (db->responds[r].incident_id == incident_id) {
            for (int j = r; j < db->responds_count - 1; j++) {
                db->responds[j] = db->responds[j + 1];
            }
            db->responds_count--;
        } else {
            r++;
        }
    }
    return 0;
}

// 26. Cryptographic Oblivious Transfer (OT) basic model
int tsfi_ot_crypto_baud_llm_dat(const char *dat_bin_path, int choice_b, const char *msg0, const char *msg1) {
    if (!dat_bin_path || !msg0 || !msg1) return -1;
    if (choice_b < 0 || choice_b > 1) choice_b = 0;
    
    // Simulate 1-out-of-2 Oblivious Transfer key derivation
    uint64_t C = 953467954114363ULL; // Motzkin Prime constant
    if (g_gguf_acab_found) {
        C ^= (uint64_t)g_gguf_acab_root[0] << 32;
    }
    
    uint64_t pk = (choice_b == 0) ? 0x12345ULL : (C - 0x12345ULL);
    
    // Simulate encryption keys
    uint64_t k0 = pk ^ 0xFEEDFACEULL;
    uint64_t k1 = (C - pk) ^ 0xFEEDFACEULL;
    
    // Receiver decrypts only the chosen message
    char decrypted[128];
    if (choice_b == 0) {
        snprintf(decrypted, sizeof(decrypted), "OT %d KEY %llx MSG %s", choice_b, (unsigned long long)k0, msg0);
    } else {
        snprintf(decrypted, sizeof(decrypted), "OT %d KEY %llx MSG %s", choice_b, (unsigned long long)k1, msg1);
    }
    
    uint8_t baud_buf[256];
    int baud_len = tsfi_encode_baudot(decrypted, baud_buf, 256);
    if (baud_len <= 0) return -2;
    
    uint32_t tokens[256];
    for (int i = 0; i < baud_len; i++) {
        tokens[i] = (uint32_t)baud_buf[i];
    }
    
    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) return -3;
    
    uint32_t count = (uint32_t)baud_len;
    fwrite(&count, sizeof(uint32_t), 1, f);
    fwrite(tokens, sizeof(uint32_t), count, f);
    fclose(f);
    
    return 0;
}

// 27. Cryptographic Oblivious Transfer (OT) EER bridge
int tsfi_eer_bridge_ot_crypto_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 256) {
        fclose(f);
        return -3;
    }
    
    uint32_t tokens[256];
    if (fread(tokens, sizeof(uint32_t), count, f) != count) {
        fclose(f);
        return -4;
    }
    fclose(f);
    
    uint8_t baud_buf[256];
    for (uint32_t i = 0; i < count; i++) {
        baud_buf[i] = (uint8_t)tokens[i];
    }
    char decrypted[256];
    int dec_len = tsfi_decode_baudot(baud_buf, count, decrypted, 256);
    if (dec_len <= 0) return -5;
    
    int choice = 0;
    char msg[128] = {0};
    sscanf(decrypted, "OT %d KEY %*x MSG %127s", &choice, msg);
    
    uint32_t incident_id = 4000 + choice;
    // Map choice 0 to minor audit (type 2), choice 1 to nuclear security audit (type 1)
    int type = (choice == 0) ? 2 : 1;
    int defcon = (choice == 0) ? 5 : 1;
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);
    
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id);
    } else {
        tsfi_eer_link_response(db, 102, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped ACAB channel
        chan->encryption_type = 3;
        chan->frequency_band = 144000;
    }
    
    return 0;
}

// 28. STANAG 5066 Framed Cryptographic Oblivious Transfer (OT)
int tsfi_ot_crypto_stanag_baud_llm_dat(const char *dat_bin_path, int choice_b, const char *msg0, const char *msg1) {
    if (!dat_bin_path || !msg0 || !msg1) return -1;
    if (choice_b < 0 || choice_b > 1) choice_b = 0;
    
    uint64_t C = 953467954114363ULL;
    if (g_gguf_acab_found) {
        C ^= (uint64_t)g_gguf_acab_root[0] << 32;
    }
    uint64_t pk = (choice_b == 0) ? 0x12345ULL : (C - 0x12345ULL);
    uint64_t k0 = pk ^ 0xFEEDFACEULL;
    uint64_t k1 = (C - pk) ^ 0xFEEDFACEULL;
    
    char decrypted[128];
    if (choice_b == 0) {
        snprintf(decrypted, sizeof(decrypted), "OT %d KEY %llx MSG %s", choice_b, (unsigned long long)k0, msg0);
    } else {
        snprintf(decrypted, sizeof(decrypted), "OT %d KEY %llx MSG %s", choice_b, (unsigned long long)k1, msg1);
    }
    
    uint8_t baud_buf[256];
    int baud_len = tsfi_encode_baudot(decrypted, baud_buf, 256);
    if (baud_len <= 0) return -2;
    
    // Assemble STANAG 5066 frame:
    // Preamble (2 bytes) + Src SAP (1 byte) + Dest SAP (1 byte) + Size (2 bytes) + Payload + CRC (2 bytes)
    int frame_len = 2 + 1 + 1 + 2 + baud_len + 2;
    uint8_t frame[512];
    frame[0] = 0xE1;
    frame[1] = 0x4A;
    frame[2] = 0x0F; // Src SAP
    frame[3] = 0x0E; // Dest SAP
    frame[4] = (uint8_t)((baud_len >> 8) & 0xFF);
    frame[5] = (uint8_t)(baud_len & 0xFF);
    memcpy(frame + 6, baud_buf, baud_len);
    
    // Additive checksum as CRC representation
    uint16_t checksum = 0;
    for (int i = 0; i < 6 + baud_len; i++) {
        checksum += frame[i];
    }
    frame[6 + baud_len] = (uint8_t)((checksum >> 8) & 0xFF);
    frame[6 + baud_len + 1] = (uint8_t)(checksum & 0xFF);
    
    uint32_t tokens[512];
    for (int i = 0; i < frame_len; i++) {
        tokens[i] = (uint32_t)frame[i];
    }
    
    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) return -3;
    
    uint32_t count = (uint32_t)frame_len;
    fwrite(&count, sizeof(uint32_t), 1, f);
    fwrite(tokens, sizeof(uint32_t), count, f);
    fclose(f);
    
    return 0;
}

// 29. STANAG 5066 Framed Cryptographic Oblivious Transfer (OT) EER bridge
int tsfi_eer_bridge_ot_crypto_stanag_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count < 8 || count > 512) {
        fclose(f);
        return -3;
    }
    
    uint32_t tokens[512];
    if (fread(tokens, sizeof(uint32_t), count, f) != count) {
        fclose(f);
        return -4;
    }
    fclose(f);
    
    uint8_t frame[512];
    for (uint32_t i = 0; i < count; i++) {
        frame[i] = (uint8_t)tokens[i];
    }
    
    // Validate STANAG 5066 sync bytes
    if (frame[0] != 0xE1 || frame[1] != 0x4A) {
        return -5;
    }
    
    uint16_t baud_len = ((uint16_t)frame[4] << 8) | frame[5];
    if ((uint32_t)(baud_len + 8) != count) {
        return -6;
    }
    
    char decrypted[256];
    int dec_len = tsfi_decode_baudot(frame + 6, baud_len, decrypted, 256);
    if (dec_len <= 0) return -7;
    
    int choice = 0;
    char msg[128] = {0};
    sscanf(decrypted, "OT %d KEY %*x MSG %127s", &choice, msg);
    
    uint32_t incident_id = 5000 + choice;
    int type = (choice == 0) ? 2 : 1;
    int defcon = (choice == 0) ? 5 : 1;
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);
    
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id);
    } else {
        tsfi_eer_link_response(db, 102, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped ACAB channel
        chan->encryption_type = 3;  // STANAG 5066
        chan->frequency_band = 144000;
    }
    
    return 0;
}

// 30. Dynamic STANAG 5066 SAP Routing
TSFiStanagRoute g_stanag_routes[MAX_STANAG_ROUTES];
int g_stanag_route_count = 0;

int tsfi_stanag_register_route(uint8_t sap, tsfi_stanag_sap_handler handler) {
    if (!handler || g_stanag_route_count >= MAX_STANAG_ROUTES) return -1;
    g_stanag_routes[g_stanag_route_count].sap = sap;
    g_stanag_routes[g_stanag_route_count].handler = handler;
    g_stanag_route_count++;
    return 0;
}

int tsfi_stanag_route_frame(TSFiEerDatabase *db, uint8_t sap, const uint8_t *payload, int len) {
    for (int i = 0; i < g_stanag_route_count; i++) {
        if (g_stanag_routes[i].sap == sap) {
            g_stanag_routes[i].handler(db, payload, len);
            return 0;
        }
    }
    return -1; // SAP route not found
}

// 31. Byte-wise Longitudinal Redundancy Check (LRC) encoder/decoder (replacing Reed-Solomon)
void tsfi_encode_lrc15_11(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    for (int i = 0; i < len; i += 11) {
        int chunk = (len - i < 11) ? (len - i) : 11;
        uint8_t msg[11] = {0};
        memcpy(msg, in + i, chunk);
        
        uint8_t parity[4] = {0};
        for (int j = 0; j < 11; j++) {
            uint8_t w = msg[j];
            uint8_t idx = (uint8_t)(j + 1);
            parity[0] += w;
            parity[1] += w * idx;
            parity[2] += w * idx * idx;
            parity[3] += w * idx * idx * idx;
        }
        memcpy(out + (i / 11) * 15, msg, 11);
        memcpy(out + (i / 11) * 15 + 11, parity, 4);
    }
}

int tsfi_decode_lrc15_11(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return -1;
    int uncorrectable = 0;
    for (int i = 0; i < len; i += 15) {
        uint8_t block[15];
        memcpy(block, in + i, 15);
        
        uint8_t s[4] = {0};
        for (int j = 0; j < 11; j++) {
            uint8_t w = block[j];
            uint8_t idx = (uint8_t)(j + 1);
            s[0] += w;
            s[1] += w * idx;
            s[2] += w * idx * idx;
            s[3] += w * idx * idx * idx;
        }
        s[0] -= block[11];
        s[1] -= block[12];
        s[2] -= block[13];
        s[3] -= block[14];
        
        if (s[0] || s[1] || s[2] || s[3]) {
            int corrected = 0;
            for (int pos = 0; pos < 15 && !corrected; pos++) {
                for (int val = 1; val < 256; val++) {
                    block[pos] ^= (uint8_t)val;
                    
                    uint8_t s_new[4] = {0};
                    for (int j = 0; j < 11; j++) {
                        uint8_t w = block[j];
                        uint8_t idx = (uint8_t)(j + 1);
                        s_new[0] += w;
                        s_new[1] += w * idx;
                        s_new[2] += w * idx * idx;
                        s_new[3] += w * idx * idx * idx;
                    }
                    s_new[0] -= block[11];
                    s_new[1] -= block[12];
                    s_new[2] -= block[13];
                    s_new[3] -= block[14];
                    
                    if (s_new[0] == 0 && s_new[1] == 0 && s_new[2] == 0 && s_new[3] == 0) {
                        corrected = 1;
                        break;
                    }
                    block[pos] ^= (uint8_t)val;
                }
            }
            if (!corrected) {
                uncorrectable++;
            }
        }
        memcpy(out + (i / 15) * 11, block, 11);
    }
    return uncorrectable;
}

// 32. Exponential Moving Average (EMA) noise estimator (replacing Kalman)
void tsfi_pll_ema_estimate(float measurement, float *state, float alpha) {
    if (!state) return;
    *state = alpha * measurement + (1.0f - alpha) * (*state);
}

// 33. EER database global invariants audit
int tsfi_eer_audit_invariants(const TSFiEerDatabase *db) {
    if (!db) return -1;
    for (int i = 0; i < db->incident_count; i++) {
        if (db->incidents[i].defcon_level == 1) {
            int has_norad = 0;
            for (int j = 0; j < db->responds_count; j++) {
                if (db->responds[j].incident_id == db->incidents[i].incident_id && db->responds[j].agency_id == 101) {
                    has_norad = 1;
                    break;
                }
            }
            if (!has_norad) return -2;
        }
    }
    return 0;
}

// 34. Optical Telemetry (OT) basic model
int tsfi_ot_optical_baud_llm_dat(const char *dat_bin_path) {
    if (!dat_bin_path) return -1;
    
    int temp = 25;
    int intensity = 80;
    int drift = 0;
    
    if (g_gguf_acab_found) {
        temp += (int)g_gguf_acab_root[0] % 10;
        intensity += (int)g_gguf_acab_root[1] % 20;
        drift += (int)g_gguf_acab_root[2] % 10;
    }
    
    char status[128];
    snprintf(status, sizeof(status), "OPTO TEMP %d INTENSITY %d DRIFT %d", temp, intensity, drift);
    
    uint8_t baud_buf[128];
    int baud_len = tsfi_encode_baudot(status, baud_buf, 128);
    if (baud_len <= 0) return -2;
    
    uint32_t tokens[128];
    for (int i = 0; i < baud_len; i++) {
        tokens[i] = (uint32_t)baud_buf[i];
    }
    
    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) return -3;
    
    uint32_t count = (uint32_t)baud_len;
    fwrite(&count, sizeof(uint32_t), 1, f);
    fwrite(tokens, sizeof(uint32_t), count, f);
    fclose(f);
    
    return 0;
}

// 35. Optical Telemetry (OT) EER bridge
int tsfi_eer_bridge_ot_optical_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 128) {
        fclose(f);
        return -3;
    }
    
    uint32_t tokens[128];
    if (fread(tokens, sizeof(uint32_t), count, f) != count) {
        fclose(f);
        return -4;
    }
    fclose(f);
    
    uint8_t baud_buf[128];
    for (uint32_t i = 0; i < count; i++) {
        baud_buf[i] = (uint8_t)tokens[i];
    }
    char status_dec[128];
    int dec_len = tsfi_decode_baudot(baud_buf, count, status_dec, 128);
    if (dec_len <= 0) return -5;
    
    int temp = 0;
    int intensity = 0;
    int drift = 0;
    sscanf(status_dec, "OPTO TEMP %d INTENSITY %d DRIFT %d", &temp, &intensity, &drift);
    
    uint32_t incident_id = 6000 + (uint32_t)drift;
    int type = (drift >= 5) ? 1 : 2;
    int defcon = (drift >= 5) ? 1 : 5;
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);
    
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id);
    } else {
        tsfi_eer_link_response(db, 102, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped ACAB channel
        chan->encryption_type = 3;
        chan->frequency_band = 144000;
    }
    
    return 0;
}

// 36. Generic OT (OT can be anything) Baudot LLM DAT exporter
int tsfi_generic_ot_baud_llm_dat(const char *dat_bin_path, const char *payload_text) {
    if (!dat_bin_path || !payload_text) return -1;
    
    uint8_t baud_buf[256];
    int baud_len = tsfi_encode_baudot(payload_text, baud_buf, 256);
    if (baud_len <= 0) return -2;
    
    uint32_t tokens[256];
    for (int i = 0; i < baud_len; i++) {
        tokens[i] = (uint32_t)baud_buf[i];
    }
    
    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) return -3;
    
    uint32_t count = (uint32_t)baud_len;
    fwrite(&count, sizeof(uint32_t), 1, f);
    fwrite(tokens, sizeof(uint32_t), count, f);
    fclose(f);
    
    return 0;
}

// 37. Generic OT ER & EER bridge
int tsfi_eer_bridge_generic_ot_acab(TSFiEerDatabase *db, const char *dat_bin_path, int (*parse_callback)(const char *decrypted, uint32_t *incident_id, int *type, int *defcon)) {
    if (!db || !dat_bin_path || !parse_callback) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 256) {
        fclose(f);
        return -3;
    }
    
    uint32_t tokens[256];
    if (fread(tokens, sizeof(uint32_t), count, f) != count) {
        fclose(f);
        return -4;
    }
    fclose(f);
    
    uint8_t baud_buf[256];
    for (uint32_t i = 0; i < count; i++) {
        baud_buf[i] = (uint8_t)tokens[i];
    }
    char decrypted[256];
    int dec_len = tsfi_decode_baudot(baud_buf, count, decrypted, 256);
    if (dec_len <= 0) return -5;
    
    uint32_t incident_id = 0;
    int type = 0;
    int defcon = 0;
    
    if (parse_callback(decrypted, &incident_id, &type, &defcon) != 0) {
        return -6;
    }
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);
    
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id);
    } else {
        tsfi_eer_link_response(db, 102, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped ACAB channel
        chan->encryption_type = 3;
        chan->frequency_band = 144000;
    }
    
    return 0;
}

// 38. Convolutional / Block Interleaver for LRC
void tsfi_interleave_lrc(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    for (int i = 0; i < len; i += 15) {
        int chunk = (len - i < 15) ? (len - i) : 15;
        if (chunk < 15) {
            memcpy(out + i, in + i, chunk);
            continue;
        }
        uint8_t temp[15];
        memcpy(temp, in + i, 15);
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 5; c++) {
                out[i + c * 3 + r] = temp[r * 5 + c];
            }
        }
    }
}

void tsfi_deinterleave_lrc(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    for (int i = 0; i < len; i += 15) {
        int chunk = (len - i < 15) ? (len - i) : 15;
        if (chunk < 15) {
            memcpy(out + i, in + i, chunk);
            continue;
        }
        uint8_t temp[15];
        memcpy(temp, in + i, 15);
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 5; c++) {
                out[i + r * 5 + c] = temp[c * 3 + r];
            }
        }
    }
}

// 39. Double Exponential Smoothing (Holt-Linear)
void tsfi_pll_holt_estimate(float measurement, float *level, float *trend, float alpha, float beta) {
    if (!level || !trend) return;
    float last_level = *level;
    *level = alpha * measurement + (1.0f - alpha) * (last_level + *trend);
    *trend = beta * (*level - last_level) + (1.0f - beta) * (*trend);
}

// 40. STANAG Priority Routing (QoS)
TSFiStanagRoutePriority g_stanag_pri_routes[MAX_STANAG_ROUTES];
int g_stanag_pri_route_count = 0;

int tsfi_stanag_register_priority_route(uint8_t sap, tsfi_stanag_sap_handler handler, int priority) {
    if (!handler || g_stanag_pri_route_count >= MAX_STANAG_ROUTES) return -1;
    g_stanag_pri_routes[g_stanag_pri_route_count].sap = sap;
    g_stanag_pri_routes[g_stanag_pri_route_count].handler = handler;
    g_stanag_pri_routes[g_stanag_pri_route_count].priority = priority;
    
    // Simple sorting by priority (insertion sort)
    for (int i = g_stanag_pri_route_count; i > 0; i--) {
        if (g_stanag_pri_routes[i].priority < g_stanag_pri_routes[i - 1].priority) {
            TSFiStanagRoutePriority temp = g_stanag_pri_routes[i];
            g_stanag_pri_routes[i] = g_stanag_pri_routes[i - 1];
            g_stanag_pri_routes[i - 1] = temp;
        }
    }
    g_stanag_pri_route_count++;
    return 0;
}

int tsfi_stanag_route_priority_frame(TSFiEerDatabase *db, uint8_t sap, const uint8_t *payload, int len) {
    for (int i = 0; i < g_stanag_pri_route_count; i++) {
        if (g_stanag_pri_routes[i].sap == sap) {
            g_stanag_pri_routes[i].handler(db, payload, len);
            return 0;
        }
    }
    return -1;
}

// 41. EER relational path dependency audits
int tsfi_eer_audit_paths(const TSFiEerDatabase *db) {
    if (!db) return -1;
    for (int i = 0; i < db->responds_count; i++) {
        int has_channel = 0;
        for (int j = 0; j < db->channel_count; j++) {
            if (db->channels[j].channel_id == 0x0200) {
                has_channel = 1;
                break;
            }
        }
        if (!has_channel) return -2;
    }
    return 0;
}

// 42. Operator Terminal (OT) basic model
int tsfi_ot_terminal_baud_llm_dat(const char *dat_bin_path) {
    if (!dat_bin_path) return -1;
    
    int online = 1;
    int cols = 80;
    int baud = 110;
    
    if (g_gguf_acab_found) {
        online = (int)g_gguf_acab_root[0] % 2;
        cols = 40 + ((int)g_gguf_acab_root[1] % 41);
        baud = 50 + ((int)g_gguf_acab_root[2] % 100);
    }
    
    char status[128];
    snprintf(status, sizeof(status), "TTY ON %d COLS %d BAUD %d", online, cols, baud);
    
    uint8_t baud_buf[128];
    int baud_len = tsfi_encode_baudot(status, baud_buf, 128);
    if (baud_len <= 0) return -2;
    
    uint32_t tokens[128];
    for (int i = 0; i < baud_len; i++) {
        tokens[i] = (uint32_t)baud_buf[i];
    }
    
    FILE *f = fopen(dat_bin_path, "wb");
    if (!f) return -3;
    
    uint32_t count = (uint32_t)baud_len;
    fwrite(&count, sizeof(uint32_t), 1, f);
    fwrite(tokens, sizeof(uint32_t), count, f);
    fclose(f);
    
    return 0;
}

// 43. Operator Terminal (OT) EER bridge
int tsfi_eer_bridge_ot_terminal_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 128) {
        fclose(f);
        return -3;
    }
    
    uint32_t tokens[128];
    if (fread(tokens, sizeof(uint32_t), count, f) != count) {
        fclose(f);
        return -4;
    }
    fclose(f);
    
    uint8_t baud_buf[128];
    for (uint32_t i = 0; i < count; i++) {
        baud_buf[i] = (uint8_t)tokens[i];
    }
    char status_dec[128];
    int dec_len = tsfi_decode_baudot(baud_buf, count, status_dec, 128);
    if (dec_len <= 0) return -5;
    
    int online = 0;
    int cols = 0;
    int baud = 0;
    sscanf(status_dec, "TTY ON %d COLS %d BAUD %d", &online, &cols, &baud);
    
    uint32_t incident_id = 7000 + (uint32_t)baud;
    int type = (baud < 110) ? 3 : 4;
    int defcon = (baud < 110) ? 4 : 5;
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 103, "TELEX_MAINT", 3, 3);
    
    if (type == 3) {
        tsfi_eer_link_response(db, 103, incident_id);
    } else {
        tsfi_eer_link_response(db, 101, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped TTY channel
        chan->encryption_type = 1;
        chan->frequency_band = 75; // Baudot speed band
    }
    
    return 0;
}

// 44. Dynamic STANAG Transmit Window Scaling
int tsfi_stanag_scale_window(float noise_level) {
    if (noise_level > 2.0f) return 1;
    if (noise_level > 1.0f) return 4;
    if (noise_level > 0.5f) return 8;
    return 16;
}

// 45. Cascading LRC Checksum Layers
void tsfi_encode_cascading_lrc(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len <= 0) return;
    uint8_t global_lrc = 0;
    uint8_t sub1_lrc = 0;
    uint8_t sub2_lrc = 0;
    for (int i = 0; i < len; i++) {
        global_lrc += in[i];
        if (i < len / 2) {
            sub1_lrc += in[i] * (uint8_t)(i + 1);
        } else {
            sub2_lrc += in[i] * (uint8_t)(i + 1);
        }
    }
    memcpy(out, in, len);
    out[len] = global_lrc;
    out[len + 1] = sub1_lrc;
    out[len + 2] = sub2_lrc;
}

int tsfi_decode_cascading_lrc(const uint8_t *in, int len, uint8_t *out) {
    if (!in || !out || len < 3) return -1;
    int payload_len = len - 3;
    uint8_t global_lrc = 0;
    uint8_t sub1_lrc = 0;
    uint8_t sub2_lrc = 0;
    for (int i = 0; i < payload_len; i++) {
        global_lrc += in[i];
        if (i < payload_len / 2) {
            sub1_lrc += in[i] * (uint8_t)(i + 1);
        } else {
            sub2_lrc += in[i] * (uint8_t)(i + 1);
        }
    }
    memcpy(out, in, payload_len);
    if (global_lrc != in[payload_len] || sub1_lrc != in[payload_len + 1] || sub2_lrc != in[payload_len + 2]) {
        for (int i = 0; i < payload_len; i++) {
            for (int val = 1; val < 256; val++) {
                out[i] += (uint8_t)val;
                uint8_t g = 0, s1 = 0, s2 = 0;
                for (int j = 0; j < payload_len; j++) {
                    g += out[j];
                    if (j < payload_len / 2) s1 += out[j] * (uint8_t)(j + 1);
                    else s2 += out[j] * (uint8_t)(j + 1);
                }
                if (g == in[payload_len] && s1 == in[payload_len + 1] && s2 == in[payload_len + 2]) {
                    return 0; // Corrected!
                }
                out[i] -= (uint8_t)val;
            }
        }
        return -1;
    }
    return 0;
}

// 46. Adaptive Baudot Shift Timeout
int tsfi_baudot_decode_with_timeout(const uint8_t *in, int len, char *out, int max_out, int timeout_cycles) {
    if (!in || !out || len <= 0) return -1;
    int current_shift = 0;
    int silent_cycles = 0;
    int out_idx = 0;
    for (int i = 0; i < len; i++) {
        uint8_t code = in[i];
        if (code == 0) {
            silent_cycles++;
            if (silent_cycles >= timeout_cycles && current_shift != 0) {
                current_shift = 0;
            }
            continue;
        }
        silent_cycles = 0;
        if (code == 0x1F) {
            current_shift = 0;
            continue;
        }
        if (code == 0x1B) {
            current_shift = 1;
            continue;
        }
        char c = (current_shift == 0) ? (char)g_ltrs_map[code] : (char)g_figs_map[code];
        if (out_idx < max_out - 1) {
            out[out_idx++] = c;
        }
    }
    out[out_idx] = '\0';
    return out_idx;
}

// 47. Lock-Free EER Transaction Journaling
TSFiEerJournal g_eer_journal = { .head = 0, .tail = 0 };

int tsfi_eer_journal_push(uint32_t incident_id, int event_type, uint32_t timestamp) {
    int next = (g_eer_journal.head + 1) % EER_JOURNAL_SIZE;
    if (next == g_eer_journal.tail) return -1; // Full
    g_eer_journal.buffer[g_eer_journal.head].incident_id = incident_id;
    g_eer_journal.buffer[g_eer_journal.head].event_type = event_type;
    g_eer_journal.buffer[g_eer_journal.head].timestamp = timestamp;
    g_eer_journal.head = next;
    return 0;
}

int tsfi_eer_journal_pop(TSFiEerJournalEntry *entry) {
    if (!entry || g_eer_journal.tail == g_eer_journal.head) return -1; // Empty
    *entry = g_eer_journal.buffer[g_eer_journal.tail];
    g_eer_journal.tail = (g_eer_journal.tail + 1) % EER_JOURNAL_SIZE;
    return 0;
}

// 48. OT LLM Bandwidth Communication Link
int tsfi_ot_llm_bandwidth_comm_init(TSFiOtLlmBandwidthComm *comm, int sap, int priority) {
    if (!comm) return -1;
    comm->noise_level = 0.1f;
    comm->current_window_size = 16;
    comm->active_sap = sap;
    comm->priority = priority;
    return 0;
}

int tsfi_ot_llm_bandwidth_comm_send(TSFiOtLlmBandwidthComm *comm, const uint32_t *tokens, int count, uint8_t *out_frame, int *out_len) {
    if (!comm || !tokens || !out_frame || !out_len || count <= 0) return -1;
    
    // We group tokens (4 bytes each) into 11-byte chunks.
    int total_bytes = count * 4;
    int chunks = (total_bytes + 10) / 11;
    int payload_raw_len = chunks * 11;
    uint8_t *raw_buf = malloc(payload_raw_len);
    memset(raw_buf, 0, payload_raw_len);
    for (int i = 0; i < count; i++) {
        raw_buf[i * 4 + 0] = (uint8_t)((tokens[i] >> 24) & 0xFF);
        raw_buf[i * 4 + 1] = (uint8_t)((tokens[i] >> 16) & 0xFF);
        raw_buf[i * 4 + 2] = (uint8_t)((tokens[i] >> 8) & 0xFF);
        raw_buf[i * 4 + 3] = (uint8_t)(tokens[i] & 0xFF);
    }
    
    int coded_len = chunks * 15;
    uint8_t *coded_buf = malloc(coded_len);
    tsfi_encode_lrc15_11(raw_buf, payload_raw_len, coded_buf);
    
    uint8_t *interleaved_buf = malloc(coded_len);
    tsfi_interleave_lrc(coded_buf, coded_len, interleaved_buf);
    
    // Dynamic Window scaling based on noise
    comm->current_window_size = tsfi_stanag_scale_window(comm->noise_level);
    
    // Frame header: Sync (2 bytes: 0xE1, 0x4A), SAP (1 byte), Priority (1 byte)
    out_frame[0] = 0xE1;
    out_frame[1] = 0x4A;
    out_frame[2] = (uint8_t)comm->active_sap;
    out_frame[3] = (uint8_t)comm->priority;
    
    memcpy(out_frame + 4, interleaved_buf, coded_len);
    *out_len = 4 + coded_len;
    
    free(raw_buf);
    free(coded_buf);
    free(interleaved_buf);
    return 0;
}

int tsfi_ot_llm_bandwidth_comm_recv(TSFiOtLlmBandwidthComm *comm, const uint8_t *frame, int len, uint32_t *tokens_out, int *count_out) {
    if (!comm || !frame || !tokens_out || !count_out || len <= 4) return -1;
    
    if (frame[0] != 0xE1 || frame[1] != 0x4A) return -2; // Bad Sync
    int sap = frame[2];
    if (sap != comm->active_sap) return -3; // Bad Route
    
    int coded_len = len - 4;
    uint8_t *interleaved = malloc(coded_len);
    memcpy(interleaved, frame + 4, coded_len);
    
    uint8_t *coded = malloc(coded_len);
    tsfi_deinterleave_lrc(interleaved, coded_len, coded);
    
    int payload_len = (coded_len / 15) * 11;
    uint8_t *decoded = malloc(payload_len);
    int decode_rc = tsfi_decode_lrc15_11(coded, coded_len, decoded);
    if (decode_rc < 0) {
        free(interleaved);
        free(coded);
        free(decoded);
        return -4; // Uncorrectable transmission error
    }
    
    int token_count = payload_len / 4;
    for (int i = 0; i < token_count; i++) {
        tokens_out[i] = ((uint32_t)decoded[i * 4 + 0] << 24) |
                        ((uint32_t)decoded[i * 4 + 1] << 16) |
                        ((uint32_t)decoded[i * 4 + 2] << 8)  |
                        (uint32_t)decoded[i * 4 + 3];
    }
    *count_out = token_count;
    
    free(interleaved);
    free(coded);
    free(decoded);
    return 0;
}

// 49. EER & ER bridge for OT LLM Bandwidth Communication Link
int tsfi_eer_bridge_ot_llm_comm_acab(TSFiEerDatabase *db, const char *dat_bin_path) {
    if (!db || !dat_bin_path) return -1;
    
    FILE *f = fopen(dat_bin_path, "rb");
    if (!f) return -2;
    
    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, f) != 1 || count == 0 || count > 512) {
        fclose(f);
        return -3;
    }
    
    uint8_t *frame = malloc(count);
    if (fread(frame, 1, count, f) != count) {
        free(frame);
        fclose(f);
        return -4;
    }
    fclose(f);
    
    TSFiOtLlmBandwidthComm comm;
    tsfi_ot_llm_bandwidth_comm_init(&comm, 0x0E, 2);
    
    uint32_t tokens[512];
    int token_count = 0;
    int recv_rc = tsfi_ot_llm_bandwidth_comm_recv(&comm, frame, count, tokens, &token_count);
    free(frame);
    if (recv_rc != 0) return -5;
    
    uint32_t incident_id = tokens[0];
    int type = (incident_id == 9999) ? 1 : 4;
    int defcon = (incident_id == 9999) ? 1 : 5;
    
    tsfi_eer_db_init(db);
    tsfi_eer_insert_incident(db, incident_id, defcon, 1782000000U, type);
    
    tsfi_eer_insert_agency(db, 101, "NORAD_SECURE", 1, 1);
    tsfi_eer_insert_agency(db, 102, "IRS_AUDIT", 2, 2);
    
    if (type == 1) {
        tsfi_eer_link_response(db, 101, incident_id);
    } else {
        tsfi_eer_link_response(db, 102, incident_id);
    }
    
    if (db->channel_count < 16) {
        TSFiEerChannel *chan = &db->channels[db->channel_count++];
        chan->channel_id = 0x0200; // Tapped ACAB channel
        chan->encryption_type = 3;
        chan->frequency_band = 144000;
    }
    
    return 0;
}
