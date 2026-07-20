#include "tsfi_encodings.h"
#include <string.h>
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
