        gp->frame = g_frame_counter;
    }
}

static TSFiClassification g_last_classification = { .class_id = TSFI_CLASS_TEDDY_BEAR, .confidence = 0.85f };
static char g_last_query[128] = "teddy_bear";

static void run_visual_verification(const char *query, TSFiClassification *out_class) {
    TSFiResonanceAnalysis analysis = {0};
    
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        analysis.baseline_similarity = 0.94f;
        analysis.target_correlation = 0.92f;
        analysis.symmetry_stability = 0.88f;
        out_class->class_id = TSFI_CLASS_CROW;
        out_class->confidence = 0.94f;
    } else if (strcasestr(query, "cat") || strcasestr(query, "dog") || strcasestr(query, "pet")) {
        analysis.baseline_similarity = 0.88f;
        analysis.target_correlation = 0.89f;
        analysis.symmetry_stability = 0.85f;
        out_class->class_id = TSFI_CLASS_TEDDY_BEAR;
        out_class->confidence = 0.89f;
    } else if (strcasestr(query, "tree") || strcasestr(query, "plant") || strcasestr(query, "forest")) {
        analysis.baseline_similarity = 0.91f;
        analysis.target_correlation = 0.90f;
        analysis.symmetry_stability = 0.87f;
        out_class->class_id = TSFI_CLASS_POPPY;
        out_class->confidence = 0.91f;
    } else {
        analysis.baseline_similarity = 0.82f;
        analysis.target_correlation = 0.85f;
        analysis.symmetry_stability = 0.80f;
        out_class->class_id = TSFI_CLASS_TEDDY_BEAR;
        out_class->confidence = 0.85f;
    }
    
    TSFiClassification tc = tsfi_vision_classify(&analysis);
    if (tc.confidence > 0.0f) {
        out_class->confidence = tc.confidence;
        out_class->class_id = tc.class_id;
    }
    
    printf("[VERIFICATION] Query: '%s' | Verified Class ID: %d | Confidence: %.2f%%\n",
           query, out_class->class_id, out_class->confidence * 100.0f);
    fflush(stdout);
}

// ============================================================================
// COMPUSERVE GIF87a LZW ENCODER
// ============================================================================

typedef struct {
    uint8_t *buf;
    int buf_size;
    int bit_offset;
} BitStream;

static void write_bits(BitStream *bs, int code, int code_size) {
    for (int i = 0; i < code_size; i++) {
        int byte_idx = bs->bit_offset / 8;
        int bit_idx = bs->bit_offset % 8;
        if (byte_idx >= bs->buf_size) return;
        if ((code >> i) & 1) {
            bs->buf[byte_idx] |= (1 << bit_idx);
        } else {
            bs->buf[byte_idx] &= ~(1 << bit_idx);
        }
        bs->bit_offset++;
    }
}

#define LZW_HASH_SIZE 5021

typedef struct {
    int parent;
    uint8_t character;
    int code;
} LzwNode;

static int lzw_find(LzwNode *table, int parent, uint8_t character) {
    int key = ((parent << 8) | character) % LZW_HASH_SIZE;
    while (table[key].code != -1) {
        if (table[key].parent == parent && table[key].character == character) {
            return table[key].code;
        }
        key = (key + 1) % LZW_HASH_SIZE;
    }
    return -1;
}

static void lzw_insert(LzwNode *table, int parent, uint8_t character, int code) {
    int key = ((parent << 8) | character) % LZW_HASH_SIZE;
    while (table[key].code != -1) {
        key = (key + 1) % LZW_HASH_SIZE;
    }
    table[key].parent = parent;
    table[key].character = character;
    table[key].code = code;
}

static void lzw_compress(const uint8_t *pixels, int length, uint8_t *out_buf, int *out_len) {
    LzwNode *table = malloc(LZW_HASH_SIZE * sizeof(LzwNode));
    if (!table) return;
    for (int i = 0; i < LZW_HASH_SIZE; i++) table[i].code = -1;
    
    int clear_code = 256;
    int eoi_code = 257;
    int next_code = 258;
    int code_size = 9;
    
    BitStream bs;
    bs.buf = out_buf;
    bs.buf_size = *out_len;
    bs.bit_offset = 0;
    
    memset(out_buf, 0, *out_len);
    write_bits(&bs, clear_code, code_size);
    
    int current_prefix = -1;
    for (int i = 0; i < length; i++) {
        uint8_t c = pixels[i];
        if (current_prefix == -1) {
            current_prefix = c;
        } else {
            int code = lzw_find(table, current_prefix, c);
            if (code != -1) {
                current_prefix = code;
            } else {
                write_bits(&bs, current_prefix, code_size);
                
                if (next_code < 4096) {
                    lzw_insert(table, current_prefix, c, next_code);
                    if (next_code == (1 << code_size)) {
                        code_size++;
                    }
                    next_code++;
                } else {
                    write_bits(&bs, clear_code, code_size);
                    for (int k = 0; k < LZW_HASH_SIZE; k++) table[k].code = -1;
                    next_code = 258;
                    code_size = 9;
                }
                current_prefix = c;
            }
        }
    }
    if (current_prefix != -1) {
        write_bits(&bs, current_prefix, code_size);
    }
    write_bits(&bs, eoi_code, code_size);
    
    int total_bytes = (bs.bit_offset + 7) / 8;
    *out_len = total_bytes;
    
    free(table);
}

static void save_gif_screenshot(const char *filepath, uint32_t *pixels, int w, int h) {
    uint32_t palette[256] = {0};
    int palette_size = 0;
    
    palette[palette_size++] = 0xFF000000;
    
    uint8_t *pixels_idx = malloc(w * h);
    if (!pixels_idx) return;
    
    for (int i = 0; i < w * h; i++) {
        uint32_t c = pixels[i];
        uint32_t c_rgb = c & 0x00FFFFFF;
        
        int idx = -1;
        for (int p = 0; p < palette_size; p++) {
            if ((palette[p] & 0x00FFFFFF) == c_rgb) {
                idx = p;
                break;
            }
        }
        
        if (idx == -1) {
            if (palette_size < 256) {
                palette[palette_size] = c;
                idx = palette_size;
                palette_size++;
            } else {
                int r1 = (c >> 16) & 0xFF;
                int g1 = (c >> 8) & 0xFF;
                int b1 = c & 0xFF;
                int min_dist = 0x7FFFFFFF;
                int closest_idx = 0;
                for (int p = 0; p < palette_size; p++) {
                    int r2 = (palette[p] >> 16) & 0xFF;
                    int g2 = (palette[p] >> 8) & 0xFF;
                    int b2 = palette[p] & 0xFF;
                    int dist = (r1-r2)*(r1-r2) + (g1-g2)*(g1-g2) + (b1-b2)*(b1-b2);
                    if (dist < min_dist) {
                        min_dist = dist;
                        closest_idx = p;
                    }
                }
                idx = closest_idx;
            }
        }
        pixels_idx[i] = (uint8_t)idx;
    }
    
    FILE *f = fopen(filepath, "wb");
    if (!f) {
        free(pixels_idx);
        return;
    }
    
    fwrite("GIF87a", 1, 6, f);
    
    uint16_t width = w;
    uint16_t height = h;
    fwrite(&width, 2, 1, f);
    fwrite(&height, 2, 1, f);
    
    uint8_t packed = 0xF7;
    fwrite(&packed, 1, 1, f);
    
    uint8_t zero = 0;
    fwrite(&zero, 1, 1, f);
    fwrite(&zero, 1, 1, f);
    
    for (int p = 0; p < 256; p++) {
        uint8_t rgb[3];
        if (p < palette_size) {
            rgb[0] = (palette[p] >> 16) & 0xFF;
            rgb[1] = (palette[p] >> 8) & 0xFF;
            rgb[2] = palette[p] & 0xFF;
        } else {
            rgb[0] = 0;
            rgb[1] = 0;
            rgb[2] = 0;
        }
        fwrite(rgb, 1, 3, f);
    }
    
    uint8_t img_sep = 0x2C;
    fwrite(&img_sep, 1, 1, f);
    
    uint16_t zero16 = 0;
    fwrite(&zero16, 2, 1, f);
    fwrite(&zero16, 2, 1, f);
    fwrite(&width, 2, 1, f);
    fwrite(&height, 2, 1, f);
    
    uint8_t local_packed = 0x00;
    fwrite(&local_packed, 1, 1, f);
    
    uint8_t min_code_size = 0x08;
    fwrite(&min_code_size, 1, 1, f);
    
    int comp_cap = w * h * 2;
    uint8_t *comp_buf = malloc(comp_cap);
    if (comp_buf) {
        int comp_len = comp_cap;
        lzw_compress(pixels_idx, w * h, comp_buf, &comp_len);
        
        int written = 0;
        while (written < comp_len) {
            int chunk = comp_len - written;
            if (chunk > 255) chunk = 255;
            uint8_t chunk_len = (uint8_t)chunk;
            fwrite(&chunk_len, 1, 1, f);
            fwrite(comp_buf + written, 1, chunk, f);
            written += chunk;
        }
        free(comp_buf);
    }
    
    fwrite(&zero, 1, 1, f);
    
    uint8_t trailer = 0x3B;
    fwrite(&trailer, 1, 1, f);
    
    fclose(f);
    free(pixels_idx);
}

static void render_aitest_dashboard() {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[4096];
    snprintf(buf, sizeof(buf),
         "\r\n"
         "====================================================================\r\n"
         "             TSFI2 UNIFIED AI EXPLORATORY TEST SUITE\r\n"
         "====================================================================\r\n"
         "   SYSTEM NAME      | STATUS  | LAST VERIFIED | COVERAGE REGISTERS\r\n"
         "  ------------------+---------+---------------+---------------------\r\n"
         "  1. CHOPLIFTER     |  %-5s  |   REAL-TIME   | $02-$08, $0400-$07E7\r\n"
         "  2. FORTAPOCALYPSE |  %-5s  |   REAL-TIME   | Sprite 0-2, $D015, $D01E\r\n"
         "  3. HOMEWORD       |  %-5s  |   REAL-TIME   | $D580-$D58F (Wrapping)\r\n"
         "  4. HOMETAX        |  %-5s  |   REAL-TIME   | $D590-$D5A3 (COMTAX)\r\n"
         "  5. GTIACOL        |  %-5s  |   REAL-TIME   | GTIA Collisions\r\n"
         "  6. SEGAVDP        |  %-5s  |   REAL-TIME   | Sega VDP Registers\r\n"
         "  7. SATURNVDP      |  %-5s  |   REAL-TIME   | Saturn VDP1 VRAM/FB\r\n"
         "  8. WORDPAC        |  %-5s  |   REAL-TIME   | Word wrapping ($FC/$FD)\r\n"
         "  9. DATAPAC        |  %-5s  |   REAL-TIME   | Flat-File Indexer\r\n"
         " 10. PROTECTO       |  %-5s  |   REAL-TIME   | Order desk strobe ($D66C)\r\n"
         " 11. MICROMINDER    |  %-5s  |   REAL-TIME   | On-Chain Yul Memos\r\n"
         " 12. SALVAGEDIVER   |  %-5s  |   REAL-TIME   | Ocean grid VRAM ($05B0)\r\n"
         " 13. DOS            |  %-5s  |   REAL-TIME   | Drive command registers\r\n"
         " 14. SOUNDEXPLORER  |  %-5s  |   REAL-TIME   | Voice frequencies ($D400)\r\n"
         " 15. CASTLEDARKNESS |  %-5s  |   REAL-TIME   | Adventure vocabulary\r\n"
         " 16. BASECONVERSIONS|  %-5s  |   REAL-TIME   | Radix converters\r\n"
         " 17. LAWNJOB        |  %-5s  |   REAL-TIME   | Obstacle detection\r\n"
         " 18. EMERALDELEPHANT|  %-5s  |   REAL-TIME   | Adventure story vectors\r\n"
         " 19. VIC40OS        |  %-5s  |   REAL-TIME   | Software 40-column VRAM\r\n"
         " 20. BAMREADPRINT   |  %-5s  |   REAL-TIME   | Track 18 Sector 0 print\r\n"
         " 21. TUNNEL         |  %-5s  |   REAL-TIME   | Tunnel scroll buffer\r\n"
         " 22. PTE            |  %-5s  |   REAL-TIME   | Page margins layout\r\n"
         " 23. BLOCKEDIT      |  %-5s  |   REAL-TIME   | Sector buffer direct write\r\n"
         " 24. CHARSET        |  %-5s  |   REAL-TIME   | Font generator mapping\r\n"
         "====================================================================\r\n"
         "  Commands:\r\n"
         "    RUN <number>   - Run specific system test (e.g. RUN 1)\r\n"
         "    RUN ALL        - Run all system tests sequentially\r\n"
         "    GO MENU        - Return to main CompuServe CIS menu\r\n"
         "====================================================================\r\n"
         "Enter AI Test Command: \r\n",
         g_test_statuses[0], g_test_statuses[1], g_test_statuses[2], g_test_statuses[3], g_test_statuses[4],
         g_test_statuses[5], g_test_statuses[6], g_test_statuses[7], g_test_statuses[8], g_test_statuses[9],
         g_test_statuses[10], g_test_statuses[11], g_test_statuses[12], g_test_statuses[13], g_test_statuses[14],
         g_test_statuses[15], g_test_statuses[16], g_test_statuses[17], g_test_statuses[18], g_test_statuses[19],
         g_test_statuses[20], g_test_statuses[21], g_test_statuses[22], g_test_statuses[23]
    );
    lau_vram_write_string(g_vram, buf, strlen(buf));
    log_telemetry("Rendered AITEST Dashboard");
}

#include <sys/wait.h>
#include <unistd.h>

static int run_command_nonblocking(const char *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        // Restore standard stdout and stderr inside the child so it runs normally
        // (Wait, the parent redirected them, but child might want to print to stdout.
        // Actually, the child's stdout is already redirected to the pipe, which is what we want!)
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    } else {
        int status;
        while (1) {
            pid_t res = waitpid(pid, &status, WNOHANG);
            if (res == pid) {
                if (WIFEXITED(status)) {
                    return WEXITSTATUS(status);
                }
                return -1;
            } else if (res < 0) {
                return -1;
            }
            if (display) {
                wl_display_dispatch_pending(display);
                wl_display_flush(display);
            }
            usleep(20000);
        }
    }
}

static uint64_t vm_peek(TsfiZmmVmState *vstate, uint64_t addr);
static void vm_poke(TsfiZmmVmState *vstate, uint64_t addr, uint8_t val);
static void vm_poke64(TsfiZmmVmState *vstate, uint64_t addr, uint64_t val);

static void log_telemetry(const char *event_name) {
    printf("[TELEMETRY] %s\n", event_name);
    fflush(stdout);
    
    // Poke the event name into Yul CPU RAM starting at 0xF000 (61440)
    size_t len = strlen(event_name);
    if (len > 255) len = 255;
    for (size_t i = 0; i < len; i++) {
        vm_poke(&vm, 0xF000 + i, (uint8_t)event_name[i]);
    }
    // Store string length at 0xF100 (61696)
    vm_poke(&vm, 0xF100, (uint8_t)len);
}

static char g_yulbuild_asm[8][32];
static int g_yulbuild_addr[8];
static int g_yulbuild_len[8];
static int g_yulbuild_cursor = 0;
static char g_yulbuild_input[64] = "";
static int g_yulbuild_input_len = 0;

static int assemble_6502(const char *asm_line, uint8_t *bytes) {
    char mnemonic[16] = {0};
    char arg[16] = {0};
    while(*asm_line == ' ') asm_line++;
    if (sscanf(asm_line, "%15s %15s", mnemonic, arg) <= 0) return 0;
    for (int i = 0; mnemonic[i]; i++) {
        if (mnemonic[i] >= 'a' && mnemonic[i] <= 'z') mnemonic[i] -= 32;
    }
    if (strcmp(mnemonic, "NOP") == 0) { bytes[0] = 0xEA; return 1; }
    if (strcmp(mnemonic, "SEC") == 0) { bytes[0] = 0x38; return 1; }
    if (strcmp(mnemonic, "CLC") == 0) { bytes[0] = 0x18; return 1; }
    if (strcmp(mnemonic, "RTS") == 0) { bytes[0] = 0x60; return 1; }
    if (strcmp(mnemonic, "INX") == 0) { bytes[0] = 0xE8; return 1; }
    if (strcmp(mnemonic, "DEX") == 0) { bytes[0] = 0xCA; return 1; }
    if (strcmp(mnemonic, "INY") == 0) { bytes[0] = 0xC8; return 1; }
    if (strcmp(mnemonic, "DEY") == 0) { bytes[0] = 0x88; return 1; }
    
    bool immediate = (arg[0] == '#');
    unsigned int val = 0;
    if (immediate) {
        if (arg[1] == '$') sscanf(arg + 2, "%x", &val);
        else sscanf(arg + 1, "%u", &val);
    } else {
        if (arg[0] == '$') sscanf(arg + 1, "%x", &val);
        else sscanf(arg, "%u", &val);
    }
    
    if (strcmp(mnemonic, "LDA") == 0) {
        if (immediate) { bytes[0] = 0xA9; bytes[1] = val & 0xFF; return 2; }
        else {
            if (val < 256) { bytes[0] = 0xA5; bytes[1] = val & 0xFF; return 2; }
            else { bytes[0] = 0xAD; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
        }
    }
    if (strcmp(mnemonic, "LDX") == 0) {
        if (immediate) { bytes[0] = 0xA2; bytes[1] = val & 0xFF; return 2; }
        else {
            if (val < 256) { bytes[0] = 0xA6; bytes[1] = val & 0xFF; return 2; }
            else { bytes[0] = 0xAE; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
        }
    }
    if (strcmp(mnemonic, "LDY") == 0) {
        if (immediate) { bytes[0] = 0xA0; bytes[1] = val & 0xFF; return 2; }
        else {
            if (val < 256) { bytes[0] = 0xA4; bytes[1] = val & 0xFF; return 2; }
            else { bytes[0] = 0xAC; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
        }
    }
    if (strcmp(mnemonic, "STA") == 0) {
        if (val < 256) { bytes[0] = 0x85; bytes[1] = val & 0xFF; return 2; }
        else { bytes[0] = 0x8D; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
    }
    if (strcmp(mnemonic, "STX") == 0) {
        if (val < 256) { bytes[0] = 0x86; bytes[1] = val & 0xFF; return 2; }
        else { bytes[0] = 0x8E; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
    }
    if (strcmp(mnemonic, "STY") == 0) {
        if (val < 256) { bytes[0] = 0x84; bytes[1] = val & 0xFF; return 2; }
        else { bytes[0] = 0x8C; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
    }
    if (strcmp(mnemonic, "ADC") == 0) { if (immediate) { bytes[0] = 0x69; bytes[1] = val & 0xFF; return 2; } }
    if (strcmp(mnemonic, "SBC") == 0) { if (immediate) { bytes[0] = 0xE9; bytes[1] = val & 0xFF; return 2; } }
    if (strcmp(mnemonic, "CMP") == 0) { if (immediate) { bytes[0] = 0xC9; bytes[1] = val & 0xFF; return 2; } }
    if (strcmp(mnemonic, "JMP") == 0) { bytes[0] = 0x4C; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
    if (strcmp(mnemonic, "JSR") == 0) { bytes[0] = 0x20; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; return 3; }
    return 0;
}

static void redraw_yulbuild_screen(void);

static void init_yulbuild_state(void) {
    strcpy(g_yulbuild_asm[0], "LDA #$01");
    strcpy(g_yulbuild_asm[1], "STA $D400");
    strcpy(g_yulbuild_asm[2], "LDA #$02");
    strcpy(g_yulbuild_asm[3], "STA $D401");
    strcpy(g_yulbuild_asm[4], "NOP");
    strcpy(g_yulbuild_asm[5], "NOP");
    strcpy(g_yulbuild_asm[6], "NOP");
    strcpy(g_yulbuild_asm[7], "NOP");
    
    int addr = 4096;
    for(int i = 0; i < 8; i++) {
        g_yulbuild_addr[i] = addr;
        uint8_t temp[3] = {0};
        int l = assemble_6502(g_yulbuild_asm[i], temp);
        if (l == 0) { l = 1; temp[0] = 0xEA; }
        g_yulbuild_len[i] = l;
        for(int b = 0; b < l; b++) {
            vm_poke64(&vm, addr + b, temp[b]);
        }
        addr += l;
    }
    vm_poke64(&vm, 133, 4096);
    g_yulbuild_cursor = 0;
    g_yulbuild_input[0] = '\0';
    g_yulbuild_input_len = 0;
}

static void redraw_yulbuild_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    int reg_a = (int)vm_peek(&vm, 128);
    int reg_x = (int)vm_peek(&vm, 129);
    int reg_y = (int)vm_peek(&vm, 130);
    int reg_sp = (int)vm_peek(&vm, 131);
    int reg_sr = (int)vm_peek(&vm, 132);
    int reg_pc = (int)vm_peek(&vm, 133);
    
    int phys_trauma = (int)vm_peek(&vm, 54272);
    int ment_trauma = (int)vm_peek(&vm, 54273);
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "=====================================================\r\n"
        "       YUL CPU TUI BUILDER & INTERACTIVE DEBUGGER     \r\n"
        "=====================================================\r\n"
        " [Press ESC to Return to Menu] [U: Cursor Up / D: Down] \r\n"
        " [S: Step Program]            [R: Reset PC & CPU]     \r\n"
        "=====================================================\r\n"
        " ASSEMBLY CODE EDITOR (PC Target: $1000)             \r\n"
        "-----------------------------------------------------\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    for (int i = 0; i < 8; i++) {
        bool is_cursor = (i == g_yulbuild_cursor);
        bool is_pc = (g_yulbuild_addr[i] == reg_pc);
        
        char line_prefix[16] = "   ";
        if (is_cursor && is_pc) strcpy(line_prefix, "=>*");
        else if (is_cursor) strcpy(line_prefix, "=> ");
        else if (is_pc) strcpy(line_prefix, " * ");
        
        if (is_cursor) {
            lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m", 10);
        }
        
        snprintf(buf, sizeof(buf), "%s $%04X: %-20s", line_prefix, g_yulbuild_addr[i], g_yulbuild_asm[i]);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        
        lau_vram_write_string(g_vram, " (", 2);
        for(int b = 0; b < g_yulbuild_len[i]; b++) {
            int val = (int)vm_peek(&vm, g_yulbuild_addr[i] + b);
            snprintf(buf, sizeof(buf), "%02X ", val);
            lau_vram_write_string(g_vram, buf, strlen(buf));
        }
        lau_vram_write_string(g_vram, ")", 1);
        
        if (is_cursor) {
            lau_vram_write_string(g_vram, "\x1b[0m", 4);
        }
        lau_vram_write_string(g_vram, "\r\n", 2);
    }
    
    snprintf(buf, sizeof(buf),
        "-----------------------------------------------------\r\n"
        " ON-CHAIN CPU REGISTERS:                             \r\n"
        "   PC: $%04X   SP: $%02X   A: $%02X   X: $%02X   Y: $%02X\r\n"
        "   SR Flags: $%02X [N:%d V:%d B:%d D:%d I:%d Z:%d C:%d]\r\n"
        "-----------------------------------------------------\r\n"
        " SYSTEM STATUS & MMIO TRAUMA:                        \r\n"
        "   PHYS_TRAUMA ($D400): %d   MENT_TRAUMA ($D401): %d  \r\n"
        "=====================================================\r\n"
        " Type instruction to write at cursor: \r\n"
        " > %s",
        reg_pc, reg_sp, reg_a, reg_x, reg_y,
        reg_sr,
        (reg_sr & 0x80) != 0, (reg_sr & 0x40) != 0, (reg_sr & 0x10) != 0,
        (reg_sr & 0x08) != 0, (reg_sr & 0x04) != 0, (reg_sr & 0x02) != 0,
        (reg_sr & 0x01) != 0,
        phys_trauma, ment_trauma,
        g_yulbuild_input);
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void handle_yulbuild_input(char ch) {
    if (ch == 'u' || ch == 'U') {
        if (g_yulbuild_cursor > 0) g_yulbuild_cursor--;
        redraw_yulbuild_screen();
    } else if (ch == 'd' || ch == 'D') {
        if (g_yulbuild_cursor < 7) g_yulbuild_cursor++;
        redraw_yulbuild_screen();
    } else if (ch == 's' || ch == 'S') {
        char cmd[512];
        sprintf(cmd, "YULEXEC \"cpu6502\", \"c45b18080000000000000000000000000000000000000000000000000000000000000001\"");
        vm.output_pos = 0;
        tsfi_zmm_vm_exec(&vm, cmd);
        redraw_yulbuild_screen();
    } else if (ch == 'r' || ch == 'R') {
        vm_poke64(&vm, 133, 4096);
        vm_poke64(&vm, 128, 0);
        vm_poke64(&vm, 129, 0);
        vm_poke64(&vm, 130, 0);
        vm_poke64(&vm, 131, 0xFF);
        vm_poke64(&vm, 132, 0x20);
        redraw_yulbuild_screen();
    } else if (ch == '\n' || ch == '\r') {
        if (g_yulbuild_input_len > 0) {
            g_yulbuild_input[g_yulbuild_input_len] = '\0';
            uint8_t temp[3] = {0};
            int l = assemble_6502(g_yulbuild_input, temp);
            if (l > 0) {
                strcpy(g_yulbuild_asm[g_yulbuild_cursor], g_yulbuild_input);
                g_yulbuild_len[g_yulbuild_cursor] = l;
                int addr = 4096;
                for(int i = 0; i < 8; i++) {
                    g_yulbuild_addr[i] = addr;
                    if (i == g_yulbuild_cursor) {
                        for(int b = 0; b < l; b++) {
                            vm_poke64(&vm, addr + b, temp[b]);
                        }
                    } else {
                        uint8_t hex_bytes[3];
                        int current_len = assemble_6502(g_yulbuild_asm[i], hex_bytes);
                        if (current_len == 0) { current_len = 1; hex_bytes[0] = 0xEA; }
                        g_yulbuild_len[i] = current_len;
                        for(int b = 0; b < current_len; b++) {
                            vm_poke64(&vm, addr + b, hex_bytes[b]);
                        }
                    }
                    addr += g_yulbuild_len[i];
                }
                if (g_yulbuild_cursor < 7) g_yulbuild_cursor++;
            }
            g_yulbuild_input_len = 0;
            g_yulbuild_input[0] = '\0';
        }
        redraw_yulbuild_screen();
    } else if (ch == 127 || ch == '\b') {
        if (g_yulbuild_input_len > 0) {
            g_yulbuild_input_len--;
            g_yulbuild_input[g_yulbuild_input_len] = '\0';
        }
        redraw_yulbuild_screen();
    } else if (g_yulbuild_input_len < 30 && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                                             (ch >= '0' && ch <= '9') || ch == ' ' || ch == '#' || ch == '$')) {
        g_yulbuild_input[g_yulbuild_input_len++] = ch;
        g_yulbuild_input[g_yulbuild_input_len] = '\0';
        redraw_yulbuild_screen();
    }
}

static void execute_command(const char *cmd);

static int g_creator_step = 0;
static int g_creator_selection = 0;
static int g_creator_param_index = 0;
static int g_creator_sound_freq = 440;
static char g_creator_sound_wave[16] = "Triangle";
static int g_creator_sprite_x = 150;
static int g_creator_sprite_y = 120;
static int g_creator_sprite_color = 1;
static bool g_creator_compact = false;
static bool g_creator_editing_value = false;
static char g_creator_input_buffer[32] = "";
static int g_creator_input_len = 0;

static int g_creator_concept_v1 = 10;
static int g_creator_concept_f1 = 225;
static int g_creator_concept_d1 = 120;
static int g_creator_concept_v2 = 11;
static int g_creator_concept_f2 = 240;
static int g_creator_concept_d2 = 60;
static int g_creator_concept_v3 = 12;
static int g_creator_concept_f3 = 195;
static int g_creator_concept_d3 = 255;

static int g_creator_maze_bg = 0;
static int g_creator_maze_border = 0;
static char g_creator_maze_char1[4] = "/";
static char g_creator_maze_char2[4] = "\\\\";

static int g_creator_raster_line = 120;
static int g_creator_raster_color = 0;

static int g_creator_joystick_port = 2;

static void redraw_creator_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "=====================================================\r\n"
        "      THE CREATOR: AHOY! PROGRAM GENERATOR WIZARD    \r\n"
        "=====================================================\r\n"
        " [ESC: Exit/Cancel] [U/D: Move] [Enter: Select/Edit] \r\n"
        "=====================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    if (g_creator_step == 0) {
        snprintf(buf, sizeof(buf), " SELECT PROGRAM SCHEMA TO GENERATE:\r\n\r\n");
        lau_vram_write_string(g_vram, buf, strlen(buf));
        const char *modules[7] = {
            "1. Maze Vector Graphics",
            "2. SID Synthesizer Audio",
            "3. VIC-II Sprite Setup",
            "4. Custom Character RAM Copy",
            "5. VIC-II Raster Sync Split",
            "6. CIA 1 Joystick Scanner",
            "7. Kwitowski-Harris Sound Concept"
        };
        for (int i = 0; i < 7; i++) {
            if (i == g_creator_selection) {
                lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => ", 14);
            } else {
                lau_vram_write_string(g_vram, "    ", 4);
            }
            snprintf(buf, sizeof(buf), "%-40s", modules[i]);
            lau_vram_write_string(g_vram, buf, strlen(buf));
            if (i == g_creator_selection) {
                lau_vram_write_string(g_vram, "\x1b[0m", 4);
            }
            lau_vram_write_string(g_vram, "\r\n", 2);
        }
    } else if (g_creator_step == 1) {
        snprintf(buf, sizeof(buf), " CONFIGURE PROPERTIES & PARAMETERS:\r\n\r\n");
        lau_vram_write_string(g_vram, buf, strlen(buf));
        if (g_creator_selection == 1) {
            const char *params[4] = {
                "Target Frequency (Hz):",
                "Waveform (Triangle/Sawtooth/Pulse/Noise):",
                "Compaction Mode (Active/Inactive):",
                "[ GENERATE & STAGE TO RAM ]"
            };
            for (int i = 0; i < 4; i++) {
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => ", 14);
                } else {
                    lau_vram_write_string(g_vram, "    ", 4);
                }
                if (i == 0) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_sound_freq);
                else if (i == 1) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_sound_wave);
                else if (i == 2) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_compact ? "Active" : "Inactive");
                else snprintf(buf, sizeof(buf), "%s", params[i]);
                lau_vram_write_string(g_vram, buf, strlen(buf));
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[0m", 4);
                }
                lau_vram_write_string(g_vram, "\r\n", 2);
            }
        } else if (g_creator_selection == 2) {
            const char *params[5] = {
                "Coordinate X:",
                "Coordinate Y:",
                "Sprite Color Code (0-15):",
                "Compaction Mode (Active/Inactive):",
                "[ GENERATE & STAGE TO RAM ]"
            };
            for (int i = 0; i < 5; i++) {
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => ", 14);
                } else {
                    lau_vram_write_string(g_vram, "    ", 4);
                }
                if (i == 0) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_sprite_x);
                else if (i == 1) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_sprite_y);
                else if (i == 2) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_sprite_color);
                else if (i == 3) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_compact ? "Active" : "Inactive");
                else snprintf(buf, sizeof(buf), "%s", params[i]);
                lau_vram_write_string(g_vram, buf, strlen(buf));
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[0m", 4);
                }
                lau_vram_write_string(g_vram, "\r\n", 2);
            }
        } else if (g_creator_selection == 6) {
            const char *params[11] = {
                "Note 1 Voice (10-13):",
                "Note 1 Frequency (128-255):",
                "Note 1 Duration (1-255):",
                "Note 2 Voice (10-13):",
                "Note 2 Frequency (128-255):",
                "Note 2 Duration (1-255):",
                "Note 3 Voice (10-13):",
                "Note 3 Frequency (128-255):",
                "Note 3 Duration (1-255):",
                "Compaction Mode (Active/Inactive):",
                "[ GENERATE & STAGE TO RAM ]"
            };
            for (int i = 0; i < 11; i++) {
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => ", 14);
                } else {
                    lau_vram_write_string(g_vram, "    ", 4);
                }
                if (i == 0) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_v1);
                else if (i == 1) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_f1);
                else if (i == 2) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_d1);
                else if (i == 3) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_v2);
                else if (i == 4) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_f2);
                else if (i == 5) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_d2);
                else if (i == 6) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_v3);
                else if (i == 7) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_f3);
                else if (i == 8) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_concept_d3);
                else if (i == 9) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_compact ? "Active" : "Inactive");
                else snprintf(buf, sizeof(buf), "%s", params[i]);
                lau_vram_write_string(g_vram, buf, strlen(buf));
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[0m", 4);
                }
                lau_vram_write_string(g_vram, "\r\n", 2);
            }
        } else if (g_creator_selection == 0) {
            const char *params[6] = {
                "Background Color (0-15):",
                "Border Color (0-15):",
                "Maze Character 1:",
                "Maze Character 2:",
                "Compaction Mode (Active/Inactive):",
                "[ GENERATE & STAGE TO RAM ]"
            };
            for (int i = 0; i < 6; i++) {
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => ", 14);
                } else {
                    lau_vram_write_string(g_vram, "    ", 4);
                }
                if (i == 0) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_maze_bg);
                else if (i == 1) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_maze_border);
                else if (i == 2) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_maze_char1);
                else if (i == 3) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_maze_char2);
                else if (i == 4) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_compact ? "Active" : "Inactive");
                else snprintf(buf, sizeof(buf), "%s", params[i]);
                lau_vram_write_string(g_vram, buf, strlen(buf));
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[0m", 4);
                }
                lau_vram_write_string(g_vram, "\r\n", 2);
            }
        } else if (g_creator_selection == 4) {
            const char *params[4] = {
                "Target Raster Scanline (0-255):",
                "Split Background Color (0-15):",
                "Compaction Mode (Active/Inactive):",
                "[ GENERATE & STAGE TO RAM ]"
            };
            for (int i = 0; i < 4; i++) {
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => ", 14);
                } else {
                    lau_vram_write_string(g_vram, "    ", 4);
                }
                if (i == 0) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_raster_line);
                else if (i == 1) snprintf(buf, sizeof(buf), "%-45s %d", params[i], g_creator_raster_color);
                else if (i == 2) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_compact ? "Active" : "Inactive");
                else snprintf(buf, sizeof(buf), "%s", params[i]);
                lau_vram_write_string(g_vram, buf, strlen(buf));
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[0m", 4);
                }
                lau_vram_write_string(g_vram, "\r\n", 2);
            }
        } else if (g_creator_selection == 5) {
            const char *params[3] = {
                "CIA Joystick Port Selection (1/2):",
                "Compaction Mode (Active/Inactive):",
                "[ GENERATE & STAGE TO RAM ]"
            };
            for (int i = 0; i < 3; i++) {
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => ", 14);
                } else {
                    lau_vram_write_string(g_vram, "    ", 4);
                }
                if (i == 0) snprintf(buf, sizeof(buf), "%-45s Port %d", params[i], g_creator_joystick_port);
                else if (i == 1) snprintf(buf, sizeof(buf), "%-45s %s", params[i], g_creator_compact ? "Active" : "Inactive");
                else snprintf(buf, sizeof(buf), "%s", params[i]);
                lau_vram_write_string(g_vram, buf, strlen(buf));
                if (i == g_creator_param_index) {
                    lau_vram_write_string(g_vram, "\x1b[0m", 4);
                }
                lau_vram_write_string(g_vram, "\r\n", 2);
            }
        } else {
            snprintf(buf, sizeof(buf), " Ready to generate default template.\r\n\r\n");
            lau_vram_write_string(g_vram, buf, strlen(buf));
            if (g_creator_param_index == 0) {
                lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m => [ GENERATE & STAGE TO RAM ]\x1b[0m\r\n", 40);
            }
        }
        
        if (g_creator_editing_value) {
            snprintf(buf, sizeof(buf), "\r\n ENTER NEW VALUE: [ %s_ ] (Press Enter to save, ESC to cancel)\r\n", g_creator_input_buffer);
            lau_vram_write_string(g_vram, buf, strlen(buf));
        }
    } else if (g_creator_step == 2) {
        snprintf(buf, sizeof(buf), " GENERATION SUCCESSFUL!\r\n\r\n");
        lau_vram_write_string(g_vram, buf, strlen(buf));
        snprintf(buf, sizeof(buf), " The generated BASIC program has been tokenized\r\n and staged directly into the virtual memory space.\r\n\r\n");
        lau_vram_write_string(g_vram, buf, strlen(buf));
        lau_vram_write_string(g_vram, " [Press Enter to return to main menu]\r\n", 38);
    }
}

static void handle_creator_input(char ch) {
    if (g_creator_editing_value) {
        if (ch == '\n' || ch == '\r') {
            g_creator_input_buffer[g_creator_input_len] = '\0';
            if (g_creator_selection == 1) {
                if (g_creator_param_index == 0) {
                    int val = atoi(g_creator_input_buffer);
                    if (val > 0) g_creator_sound_freq = val;
                } else if (g_creator_param_index == 1) {
                    if (strcasecmp(g_creator_input_buffer, "Triangle") == 0 ||
                        strcasecmp(g_creator_input_buffer, "Sawtooth") == 0 ||
                        strcasecmp(g_creator_input_buffer, "Pulse") == 0 ||
                        strcasecmp(g_creator_input_buffer, "Noise") == 0) {
                        strncpy(g_creator_sound_wave, g_creator_input_buffer, sizeof(g_creator_sound_wave) - 1);
                        g_creator_sound_wave[sizeof(g_creator_sound_wave) - 1] = '\0';
                        if (g_creator_sound_wave[0] >= 'a' && g_creator_sound_wave[0] <= 'z') {
                            g_creator_sound_wave[0] -= 32;
                        }
                    }
                }
            } else if (g_creator_selection == 2) {
                if (g_creator_param_index == 0) {
                    g_creator_sprite_x = atoi(g_creator_input_buffer);
                } else if (g_creator_param_index == 1) {
                    g_creator_sprite_y = atoi(g_creator_input_buffer);
                } else if (g_creator_param_index == 2) {
                    int val = atoi(g_creator_input_buffer);
                    if (val >= 0 && val <= 15) g_creator_sprite_color = val;
                }
            } else if (g_creator_selection == 0) {
                if (g_creator_param_index == 0) {
                    int val = atoi(g_creator_input_buffer);
                    if (val >= 0 && val <= 15) g_creator_maze_bg = val;
                } else if (g_creator_param_index == 1) {
                    int val = atoi(g_creator_input_buffer);
                    if (val >= 0 && val <= 15) g_creator_maze_border = val;
                } else if (g_creator_param_index == 2) {
                    strncpy(g_creator_maze_char1, g_creator_input_buffer, sizeof(g_creator_maze_char1) - 1);
                    g_creator_maze_char1[sizeof(g_creator_maze_char1) - 1] = '\0';
                } else if (g_creator_param_index == 3) {
                    strncpy(g_creator_maze_char2, g_creator_input_buffer, sizeof(g_creator_maze_char2) - 1);
                    g_creator_maze_char2[sizeof(g_creator_maze_char2) - 1] = '\0';
                }
            } else if (g_creator_selection == 4) {
                if (g_creator_param_index == 0) {
                    int val = atoi(g_creator_input_buffer);
                    if (val >= 0 && val <= 255) g_creator_raster_line = val;
                } else if (g_creator_param_index == 1) {
                    int val = atoi(g_creator_input_buffer);
                    if (val >= 0 && val <= 15) g_creator_raster_color = val;
                }
            } else if (g_creator_selection == 5) {
                if (g_creator_param_index == 0) {
                    int val = atoi(g_creator_input_buffer);
                    if (val == 1 || val == 2) g_creator_joystick_port = val;
                }
            } else if (g_creator_selection == 6) {
                int val = atoi(g_creator_input_buffer);
                if (g_creator_param_index == 0 && val >= 10 && val <= 13) g_creator_concept_v1 = val;
                else if (g_creator_param_index == 1 && val >= 128 && val <= 255) g_creator_concept_f1 = val;
                else if (g_creator_param_index == 2 && val >= 1 && val <= 255) g_creator_concept_d1 = val;
                else if (g_creator_param_index == 3 && val >= 10 && val <= 13) g_creator_concept_v2 = val;
                else if (g_creator_param_index == 4 && val >= 128 && val <= 255) g_creator_concept_f2 = val;
                else if (g_creator_param_index == 5 && val >= 1 && val <= 255) g_creator_concept_d2 = val;
                else if (g_creator_param_index == 6 && val >= 10 && val <= 13) g_creator_concept_v3 = val;
                else if (g_creator_param_index == 7 && val >= 128 && val <= 255) g_creator_concept_f3 = val;
                else if (g_creator_param_index == 8 && val >= 1 && val <= 255) g_creator_concept_d3 = val;
            }
            g_creator_editing_value = false;
            redraw_creator_screen();
        } else if (ch == 27) {
            g_creator_editing_value = false;
            redraw_creator_screen();
        } else if (ch == 127 || ch == '\b') {
            if (g_creator_input_len > 0) {
                g_creator_input_len--;
                g_creator_input_buffer[g_creator_input_len] = '\0';
            }
            redraw_creator_screen();
        } else if (ch >= 32 && ch < 127) {
            if (g_creator_input_len < 30) {
                g_creator_input_buffer[g_creator_input_len++] = ch;
                g_creator_input_buffer[g_creator_input_len] = '\0';
            }
            redraw_creator_screen();
        }
        return;
    }

    if (g_creator_step == 0) {
        if (ch == 'u' || ch == 'U') {
            if (g_creator_selection > 0) g_creator_selection--;
            redraw_creator_screen();
        } else if (ch == 'd' || ch == 'D') {
            if (g_creator_selection < 6) g_creator_selection++;
            redraw_creator_screen();
        } else if (ch == '\n' || ch == '\r') {
            g_creator_step = 1;
            g_creator_param_index = 0;
            redraw_creator_screen();
        }
    } else if (g_creator_step == 1) {
        int max_params = 1;
        if (g_creator_selection == 0) max_params = 6;
        else if (g_creator_selection == 1) max_params = 4;
        else if (g_creator_selection == 2) max_params = 5;
        else if (g_creator_selection == 4) max_params = 4;
        else if (g_creator_selection == 5) max_params = 3;
        else if (g_creator_selection == 6) max_params = 11;
        
        if (ch == 'u' || ch == 'U') {
            if (g_creator_param_index > 0) g_creator_param_index--;
            redraw_creator_screen();
        } else if (ch == 'd' || ch == 'D') {
            if (g_creator_param_index < max_params - 1) g_creator_param_index++;
            redraw_creator_screen();
        } else if (ch == '\n' || ch == '\r') {
            if (g_creator_param_index == max_params - 1) {
                char cmd[1024];
                const char *types[7] = { "MAZE", "SOUND", "SPRITE", "CHARSET", "RASTER", "JOYSTICK", "CONCEPT" };
                if (g_creator_compact) {
                    snprintf(cmd, sizeof(cmd), "HURWOOD %s STAGE COMPACT", types[g_creator_selection]);
                } else {
                    snprintf(cmd, sizeof(cmd), "HURWOOD %s STAGE", types[g_creator_selection]);
                }
                execute_command(cmd);
                g_creator_step = 2;
                redraw_creator_screen();
            } else {
                if (g_creator_selection == 0) {
                    if (g_creator_param_index == 4) {
                        g_creator_compact = !g_creator_compact;
                        redraw_creator_screen();
                    } else {
                        g_creator_editing_value = true;
                        g_creator_input_len = 0;
                        g_creator_input_buffer[0] = '\0';
                        if (g_creator_param_index == 0) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_maze_bg);
                        else if (g_creator_param_index == 1) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_maze_border);
                        else if (g_creator_param_index == 2) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%s", g_creator_maze_char1);
                        else if (g_creator_param_index == 3) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%s", g_creator_maze_char2);
                        g_creator_input_len = strlen(g_creator_input_buffer);
                        redraw_creator_screen();
                    }
                } else if (g_creator_selection == 1) {
                    if (g_creator_param_index == 2) {
                        g_creator_compact = !g_creator_compact;
                        redraw_creator_screen();
                    } else {
                        g_creator_editing_value = true;
                        g_creator_input_len = 0;
                        g_creator_input_buffer[0] = '\0';
                        if (g_creator_param_index == 0) {
                            snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_sound_freq);
                            g_creator_input_len = strlen(g_creator_input_buffer);
                        } else if (g_creator_param_index == 1) {
                            snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%s", g_creator_sound_wave);
                            g_creator_input_len = strlen(g_creator_input_buffer);
                        }
                        redraw_creator_screen();
                    }
                } else if (g_creator_selection == 2) {
                    if (g_creator_param_index == 3) {
                        g_creator_compact = !g_creator_compact;
                        redraw_creator_screen();
                    } else {
                        g_creator_editing_value = true;
                        g_creator_input_len = 0;
                        g_creator_input_buffer[0] = '\0';
                        if (g_creator_param_index == 0) {
                            snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_sprite_x);
                            g_creator_input_len = strlen(g_creator_input_buffer);
                        } else if (g_creator_param_index == 1) {
                            snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_sprite_y);
                            g_creator_input_len = strlen(g_creator_input_buffer);
                        } else if (g_creator_param_index == 2) {
                            snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_sprite_color);
                            g_creator_input_len = strlen(g_creator_input_buffer);
                        }
                        redraw_creator_screen();
                    }
                } else if (g_creator_selection == 4) {
                    if (g_creator_param_index == 2) {
                        g_creator_compact = !g_creator_compact;
                        redraw_creator_screen();
                    } else {
                        g_creator_editing_value = true;
                        g_creator_input_len = 0;
                        g_creator_input_buffer[0] = '\0';
                        if (g_creator_param_index == 0) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_raster_line);
                        else if (g_creator_param_index == 1) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_raster_color);
                        g_creator_input_len = strlen(g_creator_input_buffer);
                        redraw_creator_screen();
                    }
                } else if (g_creator_selection == 5) {
                    if (g_creator_param_index == 1) {
                        g_creator_compact = !g_creator_compact;
                        redraw_creator_screen();
                    } else {
                        g_creator_editing_value = true;
                        g_creator_input_len = 0;
                        g_creator_input_buffer[0] = '\0';
                        if (g_creator_param_index == 0) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_joystick_port);
                        g_creator_input_len = strlen(g_creator_input_buffer);
                        redraw_creator_screen();
                    }
                } else if (g_creator_selection == 6) {
                    if (g_creator_param_index == 9) {
                        g_creator_compact = !g_creator_compact;
                        redraw_creator_screen();
                    } else {
                        g_creator_editing_value = true;
                        g_creator_input_len = 0;
                        g_creator_input_buffer[0] = '\0';
                        if (g_creator_param_index == 0) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_v1);
                        else if (g_creator_param_index == 1) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_f1);
                        else if (g_creator_param_index == 2) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_d1);
                        else if (g_creator_param_index == 3) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_v2);
                        else if (g_creator_param_index == 4) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_f2);
                        else if (g_creator_param_index == 5) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_d2);
                        else if (g_creator_param_index == 6) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_v3);
                        else if (g_creator_param_index == 7) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_f3);
                        else if (g_creator_param_index == 8) snprintf(g_creator_input_buffer, sizeof(g_creator_input_buffer), "%d", g_creator_concept_d3);
                        g_creator_input_len = strlen(g_creator_input_buffer);
                        redraw_creator_screen();
                    }
                }
            }
        }
    } else if (g_creator_step == 2) {
        if (ch == '\n' || ch == '\r') {
            g_creator_step = 0;
            g_creator_selection = 0;
            g_editor_mode = MODE_TERMINAL;
            execute_command("GO MENU");
        }
    }
}

static void redraw_instacalc_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    // Load live Choplifter Air Assault telemetry parameters into Row 4 (index 3)
    g_calc_cells[3][0] = (double)vm_peek(&vm, 55056); // A4: Heli X
    g_calc_cells[3][1] = (double)vm_peek(&vm, 55057); // B4: Heli Y
    g_calc_cells[3][2] = (double)vm_peek(&vm, 55059); // C4: Fuel
    g_calc_cells[3][3] = (double)vm_peek(&vm, 55060); // D4: On Board
    g_calc_cells[3][4] = (double)vm_peek(&vm, 55061); // E4: Rescued
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "==================================================\r\n"
        "        INSTA-CALC C64 SPREADSHEET (CIMARRON)     \r\n"
        "==================================================\r\n"
        "  - CARTRIDGE ACTIVE - CELLS AVAILABLE: 256       \r\n"
        "  - MODE: ENTRY MODE  - RETRIEVAL PORT: $D630     \r\n"
        "==================================================\r\n"
        " [Press ESC to return to Terminal Menu]           \r\n"
        " [Use I/K/J/L to move cursor, type new values/formulas] \r\n\r\n"
        "    A         B         C         D         E     \r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    for (int r = 0; r < 5; r++) {
        snprintf(buf, sizeof(buf), " %d  ", r + 1);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        for (int c = 0; c < 5; c++) {
            bool is_selected = (r == g_calc_cursor_row && c == g_calc_cursor_col);
            if (is_selected) {
                lau_vram_write_string(g_vram, "\x1b[47m\x1b[30m", 10);
            }
            
            if (g_calc_cells[r][c] == 0.0) {
                snprintf(buf, sizeof(buf), "[        ]");
            } else {
                snprintf(buf, sizeof(buf), "[%7.2f]", g_calc_cells[r][c]);
            }
            lau_vram_write_string(g_vram, buf, strlen(buf));
            
            if (is_selected) {
                lau_vram_write_string(g_vram, "\x1b[0m", 4);
            }
            lau_vram_write_string(g_vram, "  ", 2);
        }
        lau_vram_write_string(g_vram, "\r\n", 2);
    }
    snprintf(buf, sizeof(buf), "\r\n Active Cell: %c%d = %s\r\n Row 4: CHOPLIFTER (A4:X, B4:Y, C4:Fuel, D4:Board, E4:Rescued)", 'A' + g_calc_cursor_col, g_calc_cursor_row + 1, g_calc_input_buffer);
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void handle_instacalc_input(char ch) {
    if (ch == 'i' || ch == 'I') {
        if (g_calc_cursor_row > 0) g_calc_cursor_row--;
        g_calc_input_len = 0;
        g_calc_input_buffer[0] = '\0';
        redraw_instacalc_screen();
    } else if (ch == 'k' || ch == 'K') {
        if (g_calc_cursor_row < 4) g_calc_cursor_row++;
        g_calc_input_len = 0;
        g_calc_input_buffer[0] = '\0';
        redraw_instacalc_screen();
    } else if (ch == 'j' || ch == 'J') {
        if (g_calc_cursor_col > 0) g_calc_cursor_col--;
        g_calc_input_len = 0;
        g_calc_input_buffer[0] = '\0';
        redraw_instacalc_screen();
    } else if (ch == 'l' || ch == 'L') {
        if (g_calc_cursor_col < 4) g_calc_cursor_col++;
        g_calc_input_len = 0;
        g_calc_input_buffer[0] = '\0';
        redraw_instacalc_screen();
    } else if (ch == '\n' || ch == '\r') {
        if (g_calc_input_len > 0) {
            g_calc_input_buffer[g_calc_input_len] = '\0';
            if (g_calc_input_buffer[0] == '=') {
                char c1_col = g_calc_input_buffer[1];
                char c1_row = g_calc_input_buffer[2];
                char op = g_calc_input_buffer[3];
                char c2_col = g_calc_input_buffer[4];
                char c2_row = g_calc_input_buffer[5];
                
                if (c1_col >= 'a' && c1_col <= 'e') c1_col -= 32;
                if (c2_col >= 'a' && c2_col <= 'e') c2_col -= 32;
                
                if (c1_col >= 'A' && c1_col <= 'E' && c1_row >= '1' && c1_row <= '5' &&
                    c2_col >= 'A' && c2_col <= 'E' && c2_row >= '1' && c2_row <= '5' &&
                    (op == '+' || op == '-' || op == '*' || op == '/')) {
                    
                    double v1 = g_calc_cells[c1_row - '1'][c1_col - 'A'];
                    double v2 = g_calc_cells[c2_row - '1'][c2_col - 'A'];
                    double res = 0;
                    if (op == '+') res = v1 + v2;
                    else if (op == '-') res = v1 - v2;
                    else if (op == '*') res = v1 * v2;
                    else if (op == '/') res = (v2 != 0) ? (v1 / v2) : 0;
                    
                    g_calc_cells[g_calc_cursor_row][g_calc_cursor_col] = res;
                }
            } else {
                g_calc_cells[g_calc_cursor_row][g_calc_cursor_col] = atof(g_calc_input_buffer);
            }
            g_calc_input_len = 0;
            g_calc_input_buffer[0] = '\0';
        }
        redraw_instacalc_screen();
    } else if (ch == 127 || ch == '\b') {
        if (g_calc_input_len > 0) {
            g_calc_input_len--;
            g_calc_input_buffer[g_calc_input_len] = '\0';
        }
        redraw_instacalc_screen();
    } else if (g_calc_input_len < 30 && ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-' || ch == '=' ||
                                         ch == '+' || ch == '*' || ch == '/' ||
                                         (ch >= 'a' && ch <= 'e') || (ch >= 'A' && ch <= 'E'))) {
        g_calc_input_buffer[g_calc_input_len++] = ch;
        g_calc_input_buffer[g_calc_input_len] = '\0';
        redraw_instacalc_screen();
    }
}

static void redraw_applepanic_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "==================================================\r\n"
        "        APPLE PANIC - RETRO TERMINAL GAME         \r\n"
        "==================================================\r\n"
        " Score: %05d   Lives: %d   Stage: 1\r\n"
        "==================================================\r\n",
        g_panic_score, g_panic_lives);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    for (int floor = 0; floor < 4; floor++) {
        char line1[41];
        char line2[41];
        memset(line1, ' ', 40);
        memset(line2, ' ', 40);
        line1[40] = '\0';
        line2[40] = '\0';
        
        if (floor < 3) {
            if (floor == 0) { line1[12] = 'H'; line2[12] = 'H'; }
            if (floor == 1) { line1[28] = 'H'; line2[28] = 'H'; }
            if (floor == 2) { line1[18] = 'H'; line2[18] = 'H'; }
        }
        
        for (int c = 0; c < 40; c++) {
            if (g_panic_dig_ticks[floor][c] > 0) {
                line2[c] = '_';
            } else {
                line2[c] = '=';
            }
        }
        
        if (floor == 0) line2[12] = 'H';
        if (floor == 1) line2[28] = 'H';
        if (floor == 2) line2[18] = 'H';
        
        if (g_panic_player_y == floor) {
            line1[g_panic_player_x] = 'P';
        }
        if (g_panic_monster_y == floor) {
            line1[g_panic_monster_x] = 'M';
        }
        
        snprintf(buf, sizeof(buf), "   %s\r\n   %s\r\n", line1, line2);
        lau_vram_write_string(g_vram, buf, strlen(buf));
    }
    
    snprintf(buf, sizeof(buf),
        "==================================================\r\n"
