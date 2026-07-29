            g_slinky_col = 0;
        } else {
            if (next_row == g_slinky_monster_row && next_col == g_slinky_monster_col && g_slinky_monster_stuck > 0) {
                g_slinky_score += 200;
                g_slinky_monster_row = 4;
                g_slinky_monster_col = 4;
                g_slinky_monster_stuck = 0;
                g_slinky_hole[next_row][next_col] = 0;
            } else {
                g_slinky_row = next_row;
                g_slinky_col = next_col;
                if (g_slinky_blocks[g_slinky_row][g_slinky_col] == 0) {
                    g_slinky_blocks[g_slinky_row][g_slinky_col] = 1;
                    g_slinky_score += 10;
                }
            }
        }
        
        bool win = true;
        for (int r = 0; r < 5; r++) {
            for (int c = 0; c <= r; c++) {
                if (g_slinky_blocks[r][c] == 0) win = false;
            }
        }
        if (win) {
            g_slinky_score += 500;
            memset(g_slinky_blocks, 0, sizeof(g_slinky_blocks));
            g_slinky_row = 0;
            g_slinky_col = 0;
        }
        
        redraw_slinkypanic_screen();
    }
}

static int tokenize_line(const char *line_text, uint8_t *dest) {
    int len = 0;
    bool in_quotes = false;
    for (int i = 0; line_text[i] != '\0'; i++) {
        char c = line_text[i];
        if (c == '"') {
            in_quotes = !in_quotes;
            dest[len++] = c;
            continue;
        }
        if (in_quotes) {
            dest[len++] = c;
            continue;
        }
        if (strncasecmp(&line_text[i], "POKE", 4) == 0) { dest[len++] = 0x97; i += 3; continue; }
        if (strncasecmp(&line_text[i], "PRINT", 5) == 0) { dest[len++] = 0x99; i += 4; continue; }
        if (strncasecmp(&line_text[i], "FOR", 3) == 0) { dest[len++] = 0x81; i += 2; continue; }
        if (strncasecmp(&line_text[i], "TO", 2) == 0) { dest[len++] = 0xA4; i += 1; continue; }
        if (strncasecmp(&line_text[i], "NEXT", 4) == 0) { dest[len++] = 0x82; i += 3; continue; }
        if (strncasecmp(&line_text[i], "GOTO", 4) == 0) { dest[len++] = 0x89; i += 3; continue; }
        if (strncasecmp(&line_text[i], "IF", 2) == 0) { dest[len++] = 0x8B; i += 1; continue; }
        if (strncasecmp(&line_text[i], "THEN", 4) == 0) { dest[len++] = 0xA7; i += 3; continue; }
        if (strncasecmp(&line_text[i], "REM", 3) == 0) { dest[len++] = 0x8F; i += 2; continue; }
        if (strncasecmp(&line_text[i], "SYS", 3) == 0) { dest[len++] = 0x9E; i += 2; continue; }
        if (strncasecmp(&line_text[i], "PEEK", 4) == 0) { dest[len++] = 0xC2; i += 3; continue; }
        if (strncasecmp(&line_text[i], "AND", 3) == 0) { dest[len++] = 0xAF; i += 2; continue; }
        if (strncasecmp(&line_text[i], "OR", 2) == 0) { dest[len++] = 0xB0; i += 1; continue; }
        if (strncasecmp(&line_text[i], "CLR", 3) == 0) { dest[len++] = 0x9C; i += 2; continue; }
        dest[len++] = c;
    }
    dest[len++] = 0x00;
    return len;
}

static void inject_basic_program(const char *raw_basic) {
    if (strstr(raw_basic, "Kwitowski's Sound Concept") != NULL) {
        // Inject 6502 Sounder machine binary at $0D00 (3328 decimal)
        uint8_t sounder_bin[] = {
            0xA9, 0x00, 0x85, 0xFB, 0xA9, 0x10, 0x85, 0xFC, 
            0xA0, 0x02, 0xB1, 0xFB, 0xF0, 0x32, 0x85, 0xFD, 
            0x88, 0xB1, 0xFB, 0xAA, 0x88, 0xB1, 0xFB, 0x38, 
            0xE9, 0x0A, 0xA8, 0x8A, 0x99, 0x0A, 0x90, 0xA9, 
            0x0F, 0x8D, 0x0E, 0x90, 0x20, 0x46, 0x0D, 0xC6, 
            0xFD, 0xD0, 0xF9, 0xA9, 0x00, 0x99, 0x0A, 0x90, 
            0xA5, 0xFB, 0x18, 0x69, 0x03, 0x85, 0xFB, 0x90, 
            0xCF, 0xE6, 0xFC, 0x4C, 0x08, 0x0D, 0xA9, 0x00, 
            0x8D, 0x0E, 0x90, 0x60, 0xA2, 0xFF, 0xCA, 0xD0, 
            0xFD, 0x60
        };
        for (size_t i = 0; i < sizeof(sounder_bin); i++) {
            vm_poke64(&vm, 3328 + i, sounder_bin[i]);
        }
        // Inject 3-byte sound queue data table at $1000 (4096 decimal)
        uint8_t sound_data[] = {
            (uint8_t)g_creator_concept_v1, (uint8_t)g_creator_concept_f1, (uint8_t)g_creator_concept_d1,
            (uint8_t)g_creator_concept_v2, (uint8_t)g_creator_concept_f2, (uint8_t)g_creator_concept_d2,
            (uint8_t)g_creator_concept_v3, (uint8_t)g_creator_concept_f3, (uint8_t)g_creator_concept_d3,
            0, 0, 0
        };
        for (size_t i = 0; i < sizeof(sound_data); i++) {
            vm_poke64(&vm, 4096 + i, sound_data[i]);
        }
    }
    
    uint64_t addr = 2049;
    char *copy = strdup(raw_basic);
    char *line = strtok(copy, "\n\r");
    while (line != NULL) {
        char *line_ptr = line;
        while (*line_ptr == ' ') line_ptr++;
        if (isdigit((unsigned char)*line_ptr)) {
            char *end_num;
            unsigned int line_num = strtoul(line_ptr, &end_num, 10);
            uint8_t tokenized[256];
            int token_len = tokenize_line(end_num, tokenized);
            uint64_t next_line_link = addr + 4 + token_len;
            vm_poke64(&vm, addr, next_line_link & 0xFF);
            vm_poke64(&vm, addr + 1, (next_line_link >> 8) & 0xFF);
            vm_poke64(&vm, addr + 2, line_num & 0xFF);
            vm_poke64(&vm, addr + 3, (line_num >> 8) & 0xFF);
            for (int b = 0; b < token_len; b++) {
                vm_poke64(&vm, addr + 4 + b, tokenized[b]);
            }
            addr = next_line_link;
        }
        line = strtok(NULL, "\n\r");
    }
    vm_poke64(&vm, addr, 0x00);
    vm_poke64(&vm, addr + 1, 0x00);
    uint64_t end_of_prog = addr + 2;
    vm_poke64(&vm, 43, 0x01);
    vm_poke64(&vm, 44, 0x08);
    vm_poke64(&vm, 45, end_of_prog & 0xFF);
    vm_poke64(&vm, 46, (end_of_prog >> 8) & 0xFF);
    vm_poke64(&vm, 47, end_of_prog & 0xFF);
    vm_poke64(&vm, 48, (end_of_prog >> 8) & 0xFF);
    vm_poke64(&vm, 49, end_of_prog & 0xFF);
    vm_poke64(&vm, 50, (end_of_prog >> 8) & 0xFF);
    free(copy);
}

static void write_basic_lines(const char *raw_output, bool compact) {
    if (!compact) {
        lau_vram_write_string(g_vram, raw_output, strlen(raw_output));
        return;
    }
    const char *header = "--- COMPACTED BASIC OUTPUT ---\r\n";
    lau_vram_write_string(g_vram, header, strlen(header));
    bool in_quotes = false;
    bool in_rem = false;
    for (size_t i = 0; raw_output[i] != '\0'; i++) {
        char c = raw_output[i];
        if (!in_quotes && !in_rem && i + 3 < strlen(raw_output) && strncasecmp(&raw_output[i], "REM", 3) == 0) {
            in_rem = true;
        }
        if (c == '\n' || c == '\r') {
            in_quotes = false;
            in_rem = false;
            lau_vram_write_char(g_vram, c);
            continue;
        }
        if (c == '"') {
            in_quotes = !in_quotes;
        }
        if (c == ' ' && !in_quotes && !in_rem) {
            continue;
        }
        lau_vram_write_char(g_vram, c);
    }
}

typedef struct {
    int old_num;
    int new_num;
    char text[256];
} BasicLine;

static void execute_renumber(const char *filename) {
    // Execute Diyat tax rule via VM (Method 15 selector: 0xd17a57a8)
    char tax_cmd[128];
    snprintf(tax_cmd, sizeof(tax_cmd), "YULEXEC \"cpu6502\", \"d17a57a8\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, tax_cmd);
    
    char *endptr = NULL;
    uint64_t ret = 0;
    size_t out_len = strlen(vm.output_buffer);
    if (out_len >= 16) {
        ret = strtoull(vm.output_buffer + out_len - 16, &endptr, 16);
    } else {
        ret = strtoull(vm.output_buffer, &endptr, 16);
    }
    
    if (ret == 0) {
        lau_vram_write_string(g_vram, "Error: Diyat tax execution failed. Renumbering requires 10 OTRT units tax paid via standalone Diyat contract.\r\n", 112);
        return;
    }
    
    lau_vram_write_string(g_vram, "[TAX SUCCESS] Diyat tax of 10 OTRT units verified.\r\n", 53);

    if (!filename || !*filename) {
        filename = "/home/mariarahel/src/tsfi2/atropa_pulsechain/basic_program.txt";
    }
    FILE *f = fopen(filename, "r");
    if (!f) {
        f = fopen(filename, "w");
        if (f) {
            fprintf(f, "10 PRINT \"START\"\n");
            fprintf(f, "30 PRINT \"LOOP\"\n");
            fprintf(f, "20 GOSUB 30\n");
            fprintf(f, "40 GOTO 10\n");
            fclose(f);
            f = fopen(filename, "r");
        }
    }
    if (!f) {
        lau_vram_write_string(g_vram, "Error: Could not open file.\r\n", 29);
        return;
    }
    
    BasicLine lines[100];
    int count = 0;
    char line_buf[512];
    while (fgets(line_buf, sizeof(line_buf), f) && count < 100) {
        line_buf[strcspn(line_buf, "\r\n")] = '\0';
        char *p = line_buf;
        while (*p == ' ' || *p == '\t') p++;
        if (isdigit((unsigned char)*p)) {
            lines[count].old_num = atoi(p);
            while (isdigit((unsigned char)*p)) p++;
            while (*p == ' ' || *p == '\t') p++;
            snprintf(lines[count].text, sizeof(lines[count].text), "%.255s", p);
            count++;
        }
    }
    fclose(f);
    
    if (count == 0) {
        lau_vram_write_string(g_vram, "Error: No line-numbered BASIC code found.\r\n", 43);
        return;
    }
    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (lines[j].old_num > lines[j+1].old_num) {
                BasicLine temp = lines[j];
                lines[j] = lines[j+1];
                lines[j+1] = temp;
            }
        }
    }
    
    for (int i = 0; i < count; i++) {
        lines[i].new_num = (i + 1) * 10;
    }

    // Linting/Error Pass: Identify invalid/dangling reference targets (GOTO/GOSUB/THEN)
    char lint_msg[256];
    for (int i = 0; i < count; i++) {
        // Syntax Checks (quotes and parentheses)
        int quote_count = 0;
        int paren_depth = 0;
        char *s_ptr = lines[i].text;
        while (*s_ptr) {
            if (*s_ptr == '"') quote_count++;
            if (*s_ptr == '(') paren_depth++;
            if (*s_ptr == ')') paren_depth--;
            s_ptr++;
        }
        if (quote_count % 2 != 0) {
            snprintf(lint_msg, sizeof(lint_msg), "⚠️ [LINT WARNING] Line %d has unmatched double quotes (\").\r\n", lines[i].old_num);
            lau_vram_write_string(g_vram, lint_msg, strlen(lint_msg));
        }
        if (paren_depth != 0) {
            snprintf(lint_msg, sizeof(lint_msg), "⚠️ [LINT WARNING] Line %d has unbalanced parentheses (depth=%d).\r\n", lines[i].old_num, paren_depth);
            lau_vram_write_string(g_vram, lint_msg, strlen(lint_msg));
        }

        char *ptr = lines[i].text;
        while (*ptr) {
            bool is_ref = false;
            int offset = 0;
            if (strncasecmp(ptr, "GOTO", 4) == 0) { is_ref = true; offset = 4; }
            else if (strncasecmp(ptr, "GOSUB", 5) == 0) { is_ref = true; offset = 5; }
            else if (strncasecmp(ptr, "THEN", 4) == 0) { is_ref = true; offset = 4; }
            
            if (is_ref) {
                ptr += offset;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                if (isdigit((unsigned char)*ptr)) {
                    int ref_num = atoi(ptr);
                    bool target_found = false;
                    for (int k = 0; k < count; k++) {
                        if (lines[k].old_num == ref_num) {
                            target_found = true;
                            break;
                        }
                    }
                    if (!target_found) {
                        snprintf(lint_msg, sizeof(lint_msg), "⚠️ [LINT WARNING] Line %d references invalid destination line: %d\r\n", lines[i].old_num, ref_num);
                        lau_vram_write_string(g_vram, lint_msg, strlen(lint_msg));
                    }
                }
            } else {
                ptr++;
            }
        }
    }
    
    for (int i = 0; i < count; i++) {
        char new_text[256] = "";
        char *src = lines[i].text;
        char *dest = new_text;
        size_t dest_avail = sizeof(new_text) - 1;
        
        while (*src) {
            bool match = false;
            int offset = 0;
            if (strncasecmp(src, "GOTO", 4) == 0) { match = true; offset = 4; }
            else if (strncasecmp(src, "GOSUB", 5) == 0) { match = true; offset = 5; }
            else if (strncasecmp(src, "THEN", 4) == 0) { match = true; offset = 4; }
            
            if (match) {
                int copy_len = snprintf(dest, dest_avail, "%.*s", offset, src);
                if (copy_len > 0) {
                    dest += copy_len;
                    dest_avail -= (size_t)copy_len;
                }
                src += offset;
                
                while (*src == ' ' || *src == '\t') {
                    if (dest_avail > 0) {
                        *dest++ = *src;
                        dest_avail--;
                    }
                    src++;
                }
                
                if (isdigit((unsigned char)*src)) {
                    int ref_num = atoi(src);
                    while (isdigit((unsigned char)*src)) src++;
                    
                    int new_ref = -1;
                    for (int k = 0; k < count; k++) {
                        if (lines[k].old_num == ref_num) {
                            new_ref = lines[k].new_num;
                            break;
                        }
                    }
                    
                    if (new_ref != -1) {
                        int written = snprintf(dest, dest_avail, "%d", new_ref);
                        if (written > 0) {
                            dest += written;
                            dest_avail -= (size_t)written;
                        }
                    } else {
                        int written = snprintf(dest, dest_avail, "%d", ref_num);
                        if (written > 0) {
                            dest += written;
                            dest_avail -= (size_t)written;
                        }
                    }
                }
            } else {
                if (dest_avail > 0) {
                    *dest++ = *src++;
                    dest_avail--;
                } else {
                    break;
                }
            }
        }
        *dest = '\0';
        snprintf(lines[i].text, sizeof(lines[i].text), "%.255s", new_text);
    }
    
    f = fopen(filename, "w");
    if (!f) {
        lau_vram_write_string(g_vram, "Error: Could not write file.\r\n", 30);
        return;
    }
    
    // Check if filename or args specify compression
    bool compress = false;
    if (strstr(filename, "COMPRESS") || strstr(filename, "compress")) {
        compress = true;
    }
    
    char out_buf[1024];
    lau_vram_write_string(g_vram, "--- RENUMBERING RESULTS ---\r\n", 29);
    for (int i = 0; i < count; i++) {
        char processed_text[256] = "";
        if (compress) {
            // Minification/Compression: Strip REM comments and redundant whitespaces
            char *src = lines[i].text;
            char *dest = processed_text;
            size_t avail = sizeof(processed_text) - 1;
            
            // Skip REM lines entirely if they match
            if (strncasecmp(src, "REM", 3) == 0) {
                // Keep minimal line or skip body text
                strcpy(processed_text, "REM");
            } else {
                bool in_quotes = false;
                while (*src && avail > 0) {
                    if (*src == '"') {
                        in_quotes = !in_quotes;
                    }
                    // Strip non-quoted spaces
                    if (!in_quotes && (*src == ' ' || *src == '\t')) {
                        src++;
                        continue;
                    }
                    *dest++ = *src++;
                    avail--;
                }
                *dest = '\0';
            }
        } else {
            strncpy(processed_text, lines[i].text, sizeof(processed_text) - 1);
        }
        
        // Multi-statement line concatenation logic for compressor:
        // If the next line has text, compress is true, and the total length is under 80 chars, merge them using a colon.
        if (compress && i < count - 1) {
            char next_processed[256] = "";
            char *nsrc = lines[i+1].text;
            char *ndest = next_processed;
            size_t navail = sizeof(next_processed) - 1;
            if (strncasecmp(nsrc, "REM", 3) != 0) {
                bool in_q = false;
                while (*nsrc && navail > 0) {
                    if (*nsrc == '"') in_q = !in_q;
                    if (!in_q && (*nsrc == ' ' || *nsrc == '\t')) {
                        nsrc++;
                        continue;
                    }
                    *ndest++ = *nsrc++;
                    navail--;
                }
                *ndest = '\0';
                
                // If combined length is under 80, concatenate and skip the next line from writing independently
                if (strlen(processed_text) + strlen(next_processed) + 2 < 80) {
                    strcat(processed_text, ":");
                    strcat(processed_text, next_processed);
                    // Update the next line to have empty text so it's skipped or bypassed
                    lines[i+1].text[0] = '\0';
                }
            }
        }
        
        // Skip writing lines that have been concatenated/emptied
        if (processed_text[0] == '\0') {
            continue;
        }
        
        fprintf(f, "%d %s\n", lines[i].new_num, processed_text);
        snprintf(out_buf, sizeof(out_buf), "%d %s\r\n", lines[i].new_num, processed_text);
        lau_vram_write_string(g_vram, out_buf, strlen(out_buf));
    }
    fclose(f);
    
    // Diagnostic Performance/Optimization metrics calculation
    if (compress) {
        int original_char_count = 0;
        int compressed_char_count = 0;
        int original_line_count = count;
        int compressed_line_count = 0;
        for (int i = 0; i < count; i++) {
            original_char_count += strlen(lines[i].text);
            if (lines[i].text[0] != '\0') {
                compressed_line_count++;
                // Add estimated space for line numbers
                compressed_char_count += strlen(lines[i].text) + 6;
            }
        }
        char metrics[256];
        snprintf(metrics, sizeof(metrics),
                 "\r\n--- COMPRESSION DIAGNOSTIC PROFILE ---\r\n"
                 " Original Lines: %d   | Compressed Lines: %d\r\n"
                 " Original Size: %d B  | Compressed Size: %d B\r\n"
                 " Size Savings: %.1f%%  | Saved Footprint: %d Bytes\r\n",
                 original_line_count, compressed_line_count,
                 original_char_count, compressed_char_count,
                 original_char_count > 0 ? (1.0f - (float)compressed_char_count / original_char_count) * 100.0f : 0.0f,
                 original_char_count - compressed_char_count);
        lau_vram_write_string(g_vram, metrics, strlen(metrics));
    }
    
    lau_vram_write_string(g_vram, "----------------------------\r\nBASIC renumbering completed.\r\n", 58);
}

static void execute_minder(const char *args) {
    if (!args || !*args) {
        lau_vram_write_string(g_vram, "Usage: MINDER ADD <YYYYMMDD> <memo>\r\n       MINDER LIST <YYYYMMDD>\r\n       MINDER DEL <YYYYMMDD> <index>\r\n", 112);
        return;
    }
    char buf[512];
    snprintf(buf, sizeof(buf), "%s", args);
    char *subcmd = strtok(buf, " \t");
    if (!subcmd) return;

    if (strcasecmp(subcmd, "ADD") == 0) {
        char *date_str = strtok(NULL, " \t");
        char *memo_str = strtok(NULL, "");
        if (!date_str || !memo_str) {
            lau_vram_write_string(g_vram, "Usage: MINDER ADD <YYYYMMDD> <memo>\r\n", 37);
            return;
        }
        while (*memo_str == ' ' || *memo_str == '\t') memo_str++;
        uint64_t date = strtoull(date_str, NULL, 10);

        // Pack memo into two 32-byte fields (part1, part2)
        uint8_t part1[32] = {0};
        uint8_t part2[32] = {0};
        size_t memo_len = strlen(memo_str);
        if (memo_len > 64) memo_len = 64;
        for (size_t i = 0; i < memo_len; i++) {
            if (i < 32) part1[i] = (uint8_t)memo_str[i];
            else part2[i - 32] = (uint8_t)memo_str[i];
        }

        // Call addReminder(date, part1, part2)
        uint8_t cd[100];
        cd[0] = 0x5a; cd[1] = 0x55; cd[2] = 0x7b; cd[3] = 0x7c;
        for (int i = 0; i < 32; i++) {
            cd[4 + i] = (uint8_t)((date >> ((31 - i) * 8)) & 0xFF);
            cd[36 + i] = part1[i];
            cd[68 + i] = part2[i];
        }

        uint8_t ret_val[32];
        size_t ret_len = 32;
        extern bool lau_yul_thunk_execute(const char*, const uint8_t*, size_t, uint8_t*, size_t*);
        bool success = lau_yul_thunk_execute("minder", cd, 100, ret_val, &ret_len);
        if (success && ret_val[31] == 1) {
            lau_vram_write_string(g_vram, "Reminder added successfully to on-chain database.\r\n", 51);
        } else {
            lau_vram_write_string(g_vram, "Error adding reminder.\r\n", 24);
        }
    }
    else if (strcasecmp(subcmd, "LIST") == 0) {
        char *date_str = strtok(NULL, " \t");
        if (!date_str) {
            lau_vram_write_string(g_vram, "Usage: MINDER LIST <YYYYMMDD>\r\n", 31);
            return;
        }
        uint64_t date = strtoull(date_str, NULL, 10);

        // Call getReminderCount(date)
        uint8_t cd[36];
        cd[0] = 0xb5; cd[1] = 0xf2; cd[2] = 0x69; cd[3] = 0xa8;
        for (int i = 0; i < 32; i++) {
            cd[4 + i] = (uint8_t)((date >> ((31 - i) * 8)) & 0xFF);
        }

        uint8_t count_ret[32];
        size_t count_ret_len = 32;
        extern bool lau_yul_thunk_execute(const char*, const uint8_t*, size_t, uint8_t*, size_t*);
        bool count_success = lau_yul_thunk_execute("minder", cd, 36, count_ret, &count_ret_len);
        uint64_t count = 0;
        if (count_success) {
            for (int i = 0; i < 32; i++) {
                count = (count << 8) | count_ret[i];
            }
        }

        char out[256];
        snprintf(out, sizeof(out), "Minder database returned %lu reminders for %s:\r\n", (unsigned long)count, date_str);
        lau_vram_write_string(g_vram, out, strlen(out));

        for (uint64_t idx = 0; idx < count; idx++) {
            // Call getReminder(date, index)
            uint8_t cd_get[68];
            cd_get[0] = 0xd6; cd_get[1] = 0x53; cd_get[2] = 0x3f; cd_get[3] = 0x81;
            for (int i = 0; i < 32; i++) {
                cd_get[4 + i] = (uint8_t)((date >> ((31 - i) * 8)) & 0xFF);
                cd_get[36 + i] = (uint8_t)((idx >> ((31 - i) * 8)) & 0xFF);
            }

            uint8_t item_ret[64];
            size_t item_ret_len = 64;
            bool get_success = lau_yul_thunk_execute("minder", cd_get, 68, item_ret, &item_ret_len);
            if (get_success) {
                char memo[65] = {0};
                for (int i = 0; i < 32; i++) {
                    memo[i] = (char)item_ret[i];
                    memo[32 + i] = (char)item_ret[32 + i];
                }
                snprintf(out, sizeof(out), " [%lu] %s\r\n", (unsigned long)idx, memo);
                lau_vram_write_string(g_vram, out, strlen(out));
            }
        }
    }
    else if (strcasecmp(subcmd, "DEL") == 0) {
        char *date_str = strtok(NULL, " \t");
        char *idx_str = strtok(NULL, " \t");
        if (!date_str || !idx_str) {
            lau_vram_write_string(g_vram, "Usage: MINDER DEL <YYYYMMDD> <index>\r\n", 38);
            return;
        }
        uint64_t date = strtoull(date_str, NULL, 10);
        uint64_t index = strtoull(idx_str, NULL, 10);

        // Call deleteReminder(date, index)
        uint8_t cd[68];
        cd[0] = 0x93; cd[1] = 0xbb; cd[2] = 0x22; cd[3] = 0x21;
        for (int i = 0; i < 32; i++) {
            cd[4 + i] = (uint8_t)((date >> ((31 - i) * 8)) & 0xFF);
            cd[36 + i] = (uint8_t)((index >> ((31 - i) * 8)) & 0xFF);
        }

        uint8_t ret_val[32];
        size_t ret_len = 32;
        extern bool lau_yul_thunk_execute(const char*, const uint8_t*, size_t, uint8_t*, size_t*);
        bool success = lau_yul_thunk_execute("minder", cd, 68, ret_val, &ret_len);
        if (success && ret_val[31] == 1) {
            lau_vram_write_string(g_vram, "Reminder deleted successfully from on-chain database.\r\n", 54);
        } else {
            lau_vram_write_string(g_vram, "Error deleting reminder (index out of bounds).\r\n", 48);
        }
    }
    else {
        lau_vram_write_string(g_vram, "Unknown Minder command.\r\n", 25);
    }
}

static void execute_base(const char *args) {
    if (!args || !*args) {
        lau_vram_write_string(g_vram, "Usage: BASE <number> <from_base> <to_base>\r\n", 44);
        return;
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "%.255s", args);
    char *num_str = strtok(buf, " \t");
    char *from_str = strtok(NULL, " \t");
    char *to_str = strtok(NULL, " \t");
    if (!num_str || !from_str || !to_str) {
        lau_vram_write_string(g_vram, "Usage: BASE <number> <from_base> <to_base>\r\n", 44);
        return;
    }
    int from_base = atoi(from_str);
    int to_base = atoi(to_str);
    if (from_base < 2 || from_base > 36 || to_base < 2 || to_base > 36) {
        lau_vram_write_string(g_vram, "Error: Bases must be between 2 and 36.\r\n", 40);
        return;
    }
    char *endptr;
    unsigned long long val = strtoull(num_str, &endptr, from_base);
    if (*endptr != '\0') {
        lau_vram_write_string(g_vram, "Error: Invalid number representation for the given base.\r\n", 57);
        return;
    }
    
    char out_digits[128];
    int pos = 0;
    if (val == 0) {
        out_digits[pos++] = '0';
    } else {
        while (val > 0) {
            int rem = val % to_base;
            if (rem < 10) {
                out_digits[pos++] = '0' + rem;
            } else {
                out_digits[pos++] = 'A' + (rem - 10);
            }
            val /= to_base;
        }
    }
    char out_str[256];
    int out_pos = 0;
    out_pos += snprintf(out_str + out_pos, sizeof(out_str) - out_pos, "Result (base %d): ", to_base);
    for (int i = pos - 1; i >= 0; i--) {
        if (out_pos < (int)sizeof(out_str) - 2) {
            out_str[out_pos++] = out_digits[i];
        }
    }
    out_str[out_pos++] = '\r';
    out_str[out_pos++] = '\n';
    out_str[out_pos] = '\0';
    lau_vram_write_string(g_vram, out_str, strlen(out_str));
}

static void execute_cols(const char *args) {
    if (!args || !*args) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Current display columns: %d (Mode: %s)\r\nUsage: COLS <40|80|132|AUTO>\r\n", 
                 g_superterm_cols, g_superterm_mode ? "Fixed" : "Auto");
        lau_vram_write_string(g_vram, msg, strlen(msg));
        return;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%.63s", args);
    char *arg = strtok(buf, " \t");
    if (!arg) return;

    if (strcasecmp(arg, "AUTO") == 0) {
        g_superterm_mode = false;
        lau_vram_write_string(g_vram, "Display mode updated to automatic window sizing.\r\n", 50);
    } else {
        int cols = atoi(arg);
        if (cols < 10 || cols > 256) {
            lau_vram_write_string(g_vram, "Error: Columns must be between 10 and 256.\r\n", 44);
            return;
        }
        g_superterm_cols = cols;
        g_superterm_mode = true;
        g_superterm_scroll_x = 0;
        char msg[128];
        snprintf(msg, sizeof(msg), "Display mode updated to %d columns.\r\n", cols);
        lau_vram_write_string(g_vram, msg, strlen(msg));
    }
}

static void execute_command(const char *cmd) {
    char cmd_log[512];
    snprintf(cmd_log, sizeof(cmd_log), "Executed command: %s", cmd);
    log_telemetry(cmd_log);
    if (strcasecmp(cmd, "exit") == 0) {
        running = false;
        return;
    }
    
    char cmd_copy[512];
    snprintf(cmd_copy, sizeof(cmd_copy), "%s", cmd);
    char *first_word = strtok(cmd_copy, " \t");
    
    if (first_word && strcasecmp(first_word, "PEEK") == 0) {
        char *addr_str = strtok(NULL, " \t");
        char *count_str = strtok(NULL, " \t");
        if (addr_str) {
            uint64_t addr = strtoull(addr_str, NULL, 0);
            uint64_t count = count_str ? strtoull(count_str, NULL, 0) : 1;
            if (count > 256) count = 256;
            printf("[PEEK] Memory dump from address %lu (count %lu):\n", addr, count);
            for (uint64_t i = 0; i < count; i++) {
                uint64_t val = vm_peek(&vm, addr + i);
                printf("  $%04lX (%lu) : $%02lX (%lu)\n", addr + i, addr + i, val, val);
            }
            fflush(stdout);
        } else {
            printf("[PEEK] Usage: PEEK <address> [<count>]\n");
            fflush(stdout);
        }
        return;
    }
    
    if (first_word) {
        if (strcasecmp(first_word, "DASHBOARD") == 0) {
            first_word = "GO";
            cmd = "GO DASHBOARD";
        } else if (strcasecmp(first_word, "AITEST") == 0) {
            first_word = "GO";
            cmd = "GO AITEST";
        } else if (g_aitest_active) {
            if (strcasecmp(first_word, "RUN") == 0) {
                char *num = strtok(NULL, " \t");
                if (num) {
                    if (strcmp(num, "1") == 0) { first_word = "CHOPLIFTER"; cmd = "CHOPLIFTER"; }
                    else if (strcmp(num, "2") == 0) { first_word = "FORTAPOCALYPSE"; cmd = "FORTAPOCALYPSE"; }
                    else if (strcmp(num, "3") == 0) { first_word = "HOMEWORD"; cmd = "HOMEWORD"; }
                    else if (strcmp(num, "4") == 0) { first_word = "HOMETAX"; cmd = "HOMETAX"; }
                    else if (strcmp(num, "5") == 0) { first_word = "GTIACOL"; cmd = "GTIACOL"; }
                    else if (strcmp(num, "6") == 0) { first_word = "SEGAVDP"; cmd = "SEGAVDP"; }
                    else if (strcmp(num, "7") == 0) { first_word = "SATURNVDP"; cmd = "SATURNVDP"; }
                    else if (strcmp(num, "8") == 0) { first_word = "WORDPAC"; cmd = "WORDPAC"; }
                    else if (strcmp(num, "9") == 0) { first_word = "DATAPAC"; cmd = "DATAPAC"; }
                    else if (strcmp(num, "10") == 0) { first_word = "PROTECTO"; cmd = "PROTECTO"; }
                    else if (strcmp(num, "11") == 0) { first_word = "MICROMINDER"; cmd = "MICROMINDER"; }
                    else if (strcmp(num, "12") == 0) { first_word = "SALVAGEDIVER"; cmd = "SALVAGEDIVER"; }
                    else if (strcmp(num, "13") == 0) { first_word = "DOS"; cmd = "DOS"; }
                    else if (strcmp(num, "14") == 0) { first_word = "SOUNDEXPLORER"; cmd = "SOUNDEXPLORER"; }
                    else if (strcmp(num, "15") == 0) { first_word = "CASTLEDARKNESS"; cmd = "CASTLEDARKNESS"; }
                    else if (strcmp(num, "16") == 0) { first_word = "BASECONVERSIONS"; cmd = "BASECONVERSIONS"; }
                    else if (strcmp(num, "17") == 0) { first_word = "LAWNJOB"; cmd = "LAWNJOB"; }
                    else if (strcmp(num, "18") == 0) { first_word = "EMERALDELEPHANT"; cmd = "EMERALDELEPHANT"; }
                    else if (strcmp(num, "19") == 0) { first_word = "VIC40OS"; cmd = "VIC40OS"; }
                    else if (strcmp(num, "20") == 0) { first_word = "BAMREADPRINT"; cmd = "BAMREADPRINT"; }
                    else if (strcmp(num, "21") == 0) { first_word = "TUNNEL"; cmd = "TUNNEL"; }
                    else if (strcmp(num, "22") == 0) { first_word = "PTE"; cmd = "PTE"; }
                    else if (strcmp(num, "23") == 0) { first_word = "BLOCKEDIT"; cmd = "BLOCKEDIT"; }
                    else if (strcmp(num, "24") == 0) { first_word = "CHARSET"; cmd = "CHARSET"; }
                    else if (strcasecmp(num, "ALL") == 0) { first_word = "TESTALL"; cmd = "TESTALL"; }
                }
            } else {
                if (strcmp(first_word, "1") == 0) { first_word = "CHOPLIFTER"; cmd = "CHOPLIFTER"; }
                else if (strcmp(first_word, "2") == 0) { first_word = "FORTAPOCALYPSE"; cmd = "FORTAPOCALYPSE"; }
                else if (strcmp(first_word, "3") == 0) { first_word = "HOMEWORD"; cmd = "HOMEWORD"; }
                else if (strcmp(first_word, "4") == 0) { first_word = "HOMETAX"; cmd = "HOMETAX"; }
                else if (strcmp(first_word, "5") == 0) { first_word = "GTIACOL"; cmd = "GTIACOL"; }
                else if (strcmp(first_word, "6") == 0) { first_word = "SEGAVDP"; cmd = "SEGAVDP"; }
                else if (strcmp(first_word, "7") == 0) { first_word = "SATURNVDP"; cmd = "SATURNVDP"; }
                else if (strcmp(first_word, "8") == 0) { first_word = "WORDPAC"; cmd = "WORDPAC"; }
                else if (strcmp(first_word, "9") == 0) { first_word = "DATAPAC"; cmd = "DATAPAC"; }
                else if (strcmp(first_word, "10") == 0) { first_word = "PROTECTO"; cmd = "PROTECTO"; }
                else if (strcmp(first_word, "11") == 0) { first_word = "MICROMINDER"; cmd = "MICROMINDER"; }
                else if (strcmp(first_word, "12") == 0) { first_word = "SALVAGEDIVER"; cmd = "SALVAGEDIVER"; }
                else if (strcmp(first_word, "13") == 0) { first_word = "DOS"; cmd = "DOS"; }
                else if (strcmp(first_word, "14") == 0) { first_word = "SOUNDEXPLORER"; cmd = "SOUNDEXPLORER"; }
                else if (strcmp(first_word, "15") == 0) { first_word = "CASTLEDARKNESS"; cmd = "CASTLEDARKNESS"; }
                else if (strcmp(first_word, "16") == 0) { first_word = "BASECONVERSIONS"; cmd = "BASECONVERSIONS"; }
                else if (strcmp(first_word, "17") == 0) { first_word = "LAWNJOB"; cmd = "LAWNJOB"; }
                else if (strcmp(first_word, "18") == 0) { first_word = "EMERALDELEPHANT"; cmd = "EMERALDELEPHANT"; }
                else if (strcmp(first_word, "19") == 0) { first_word = "VIC40OS"; cmd = "VIC40OS"; }
                else if (strcmp(first_word, "20") == 0) { first_word = "BAMREADPRINT"; cmd = "BAMREADPRINT"; }
                else if (strcmp(first_word, "21") == 0) { first_word = "TUNNEL"; cmd = "TUNNEL"; }
                else if (strcmp(first_word, "22") == 0) { first_word = "PTE"; cmd = "PTE"; }
                else if (strcmp(first_word, "23") == 0) { first_word = "BLOCKEDIT"; cmd = "BLOCKEDIT"; }
                else if (strcmp(first_word, "24") == 0) { first_word = "CHARSET"; cmd = "CHARSET"; }
                else if (strcmp(first_word, "25") == 0) { first_word = "TESTALL"; cmd = "TESTALL"; }
                else if (strcmp(first_word, "26") == 0) { first_word = "GO"; cmd = "GO MENU"; }
            }
        } else if (g_dashboard_active) {
            if (strcmp(first_word, "1") == 0) { first_word = "CHOPLIFTER"; cmd = "CHOPLIFTER"; }
            else if (strcmp(first_word, "2") == 0) { first_word = "FORTAPOCALYPSE"; cmd = "FORTAPOCALYPSE"; }
            else if (strcmp(first_word, "3") == 0) { first_word = "HOMEWORD"; cmd = "HOMEWORD"; }
            else if (strcmp(first_word, "4") == 0) { first_word = "HOMETAX"; cmd = "HOMETAX"; }
            else if (strcmp(first_word, "5") == 0) { first_word = "GTIACOL"; cmd = "GTIACOL"; }
            else if (strcmp(first_word, "6") == 0) { first_word = "SEGAVDP"; cmd = "SEGAVDP"; }
            else if (strcmp(first_word, "7") == 0) { first_word = "SATURNVDP"; cmd = "SATURNVDP"; }
            else if (strcmp(first_word, "8") == 0) { first_word = "WORDPAC"; cmd = "WORDPAC"; }
            else if (strcmp(first_word, "9") == 0) { first_word = "DATAPAC"; cmd = "DATAPAC"; }
            else if (strcmp(first_word, "10") == 0) { first_word = "PROTECTO"; cmd = "PROTECTO"; }
            else if (strcmp(first_word, "11") == 0) { first_word = "MICROMINDER"; cmd = "MICROMINDER"; }
            else if (strcmp(first_word, "12") == 0) { first_word = "SALVAGEDIVER"; cmd = "SALVAGEDIVER"; }
            else if (strcmp(first_word, "13") == 0) { first_word = "DOS"; cmd = "DOS"; }
            else if (strcmp(first_word, "14") == 0) { first_word = "SOUNDEXPLORER"; cmd = "SOUNDEXPLORER"; }
            else if (strcmp(first_word, "15") == 0) { first_word = "CASTLEDARKNESS"; cmd = "CASTLEDARKNESS"; }
            else if (strcmp(first_word, "16") == 0) { first_word = "BASECONVERSIONS"; cmd = "BASECONVERSIONS"; }
            else if (strcmp(first_word, "17") == 0) { first_word = "LAWNJOB"; cmd = "LAWNJOB"; }
            else if (strcmp(first_word, "18") == 0) { first_word = "EMERALDELEPHANT"; cmd = "EMERALDELEPHANT"; }
            else if (strcmp(first_word, "19") == 0) { first_word = "VIC40OS"; cmd = "VIC40OS"; }
            else if (strcmp(first_word, "20") == 0) { first_word = "BAMREADPRINT"; cmd = "BAMREADPRINT"; }
            else if (strcmp(first_word, "21") == 0) { first_word = "TUNNEL"; cmd = "TUNNEL"; }
            else if (strcmp(first_word, "22") == 0) { first_word = "PTE"; cmd = "PTE"; }
            else if (strcmp(first_word, "23") == 0) { first_word = "BLOCKEDIT"; cmd = "BLOCKEDIT"; }
            else if (strcmp(first_word, "24") == 0) { first_word = "CHARSET"; cmd = "CHARSET"; }
            else if (strcmp(first_word, "25") == 0) { first_word = "TESTALL"; cmd = "TESTALL"; }
            else if (strcmp(first_word, "26") == 0) { first_word = "GO"; cmd = "GO MENU"; }
        }
    }
    
    if (first_word && strcasecmp(first_word, "SODARO") != 0 && strcasecmp(first_word, "MERCENARY") != 0 && strcasecmp(first_word, "PONG") != 0 &&
        strcasecmp(first_word, "WORDCRAFT") != 0 && strcasecmp(first_word, "EASYSCRIPT") != 0 && strcasecmp(first_word, "DNATYPEWRITER") != 0 &&
        strcasecmp(first_word, "YULBUILD") != 0 &&
        strcasecmp(first_word, "INSTA") != 0 && strcasecmp(first_word, "CALC") != 0 && strcasecmp(first_word, "INSTACALC") != 0 &&
        strcasecmp(first_word, "PANIC") != 0 && strcasecmp(first_word, "APPLEPANIC") != 0 &&
        strcasecmp(first_word, "SLINKY") != 0 && strcasecmp(first_word, "SLINKYBEAR") != 0 &&
        strcasecmp(first_word, "SLINKYPANIC") != 0 && strcasecmp(first_word, "COMTERM") != 0 &&
        strcasecmp(first_word, "AMTYPE") != 0 && strcasecmp(first_word, "INVISICLUE") != 0 &&
        strcasecmp(first_word, "SUBLOGIC") != 0 && strcasecmp(first_word, "LANDER") != 0 &&
        strcasecmp(first_word, "ALPINER") != 0 &&
        strcasecmp(first_word, "BUGREPELLENT") != 0 && strcasecmp(first_word, "REPELLENT") != 0) {
        g_mercenary_active = false;
        g_pong_active = false;
    }
    
    if (first_word && (strcasecmp(first_word, "BUGREPELLENT") == 0 || strcasecmp(first_word, "REPELLENT") == 0)) {
        const char *input_str = cmd + strlen(first_word);
        while (*input_str == ' ' || *input_str == '\t') input_str++;
        
        if (strlen(input_str) == 0) {
            lau_vram_write_string(g_vram, "\r\n  Usage: BUGREPELLENT <line of BASIC code>\r\n  Example: BUGREPELLENT 10 PRINT \"HELLO\"\r\n", 89);
            return;
        }
        
        size_t str_len = strlen(input_str);
        size_t padded_len = ((str_len + 31) / 32) * 32;
        size_t calldata_size = 4 + 32 + 32 + padded_len;
        uint8_t *calldata = calloc(1, calldata_size);
        
        // Selector: 0x228cf1aa
        calldata[0] = 0x22; calldata[1] = 0x8c; calldata[2] = 0xf1; calldata[3] = 0xaa;
        // Offset: 32 (0x20)
        calldata[35] = 0x20;
        // Length of string
        calldata[67] = str_len & 0xFF;
        calldata[66] = (str_len >> 8) & 0xFF;
        calldata[65] = (str_len >> 16) & 0xFF;
        calldata[64] = (str_len >> 24) & 0xFF;
        
        memcpy(calldata + 68, input_str, str_len);
        
        uint8_t retval[32] = {0};
        size_t retval_len = 32;
        
        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len);
        if (lau_yul_thunk_execute("diskSystem", calldata, calldata_size, retval, &retval_len)) {
            char checksum[5];
            checksum[0] = retval[28];
            checksum[1] = retval[29];
            checksum[2] = retval[30];
            checksum[3] = retval[31];
            checksum[4] = '\0';
            
            char output_buf[256];
            snprintf(output_buf, sizeof(output_buf), "\r\n  Line: \"%s\"\r\n  [Ahoy! Bug Repellent Checksum: %s]\r\n", input_str, checksum);
            lau_vram_write_string(g_vram, output_buf, strlen(output_buf));
        } else {
            lau_vram_write_string(g_vram, "\r\n  [ERROR: Checksum generation failed!]\r\n", 43);
        }
        free(calldata);
        return;
    }
    
    if (first_word && strcasecmp(first_word, "HURWOOD") == 0) {
        char *arg = strtok(NULL, " \t");
        char *arg2 = strtok(NULL, " \t");
        bool compact = false;
        bool stage = false;
        if (arg2 && strcasecmp(arg2, "COMPACT") == 0) {
            compact = true;
        } else if (arg2 && strcasecmp(arg2, "STAGE") == 0) {
            stage = true;
        }
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        const char *output = NULL;
        
        if (arg && strcasecmp(arg, "SOUND") == 0) {
            uint32_t freq_val = (uint32_t)(g_creator_sound_freq * 16.40277);
            uint8_t freq_lo = freq_val & 0xFF;
            uint8_t freq_hi = (freq_val >> 8) & 0xFF;
            uint8_t wave_val = 17; // default triangle
            if (strcasecmp(g_creator_sound_wave, "Sawtooth") == 0) {
                wave_val = 33;
            } else if (strcasecmp(g_creator_sound_wave, "Pulse") == 0) {
                wave_val = 65;
            } else if (strcasecmp(g_creator_sound_wave, "Noise") == 0) {
                wave_val = 129;
            }
            
            static char dynamic_sound_buf[1024];
            snprintf(dynamic_sound_buf, sizeof(dynamic_sound_buf),
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 SID SOUND DESIGN   \r\n"
                "==================================================\r\n"
                " Generating C64 SID %s Wave (Freq %d Hz)...\r\n\r\n"
                " 10 FOR I = 54272 TO 54296: POKE I, 0: NEXT I\r\n"
                " 20 POKE 54277, 15: POKE 54278, 240: REM ADSR\r\n"
                " 30 POKE 54273, %d: POKE 54272, %d: REM FREQ\r\n"
                " 40 POKE 54290, 15: REM VOLUME\r\n"
                " 50 POKE 54276, %d: REM START WAVE\r\n"
                " 60 FOR T = 1 TO 2000: NEXT T\r\n"
                " 70 POKE 54276, 16: REM STOP AUDIO\r\n"
                " READY.\r\n\r\n"
                " [POKEY/SID register sweeps code generated.]\r\n"
                "==================================================\r\n",
                g_creator_sound_wave, g_creator_sound_freq, freq_hi, freq_lo, wave_val);
            output = dynamic_sound_buf;
        } else if (arg && strcasecmp(arg, "SPRITE") == 0) {
            static char dynamic_sprite_buf[1024];
            snprintf(dynamic_sprite_buf, sizeof(dynamic_sprite_buf),
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 VIC-II SPRITES     \r\n"
                "==================================================\r\n"
                " Generating C64 Sprite 0 Generation Code...\r\n\r\n"
                " 10 POKE 2040, 13: REM SPRITE 0 PTR TO $0340\r\n"
                " 20 FOR I = 832 TO 894: POKE I, 255: NEXT I\r\n"
                " 30 POKE 53248, %d: POKE 53249, %d: REM COORDS\r\n"
                " 40 POKE 53269, 1: REM ENABLE SPRITE\r\n"
                " 50 POKE 53287, %d: REM COLOR\r\n"
                " READY.\r\n\r\n"
                " [VIC-II visual sprite generation code completed.]\r\n"
                "==================================================\r\n",
                g_creator_sprite_x, g_creator_sprite_y, g_creator_sprite_color);
            output = dynamic_sprite_buf;
        } else if (arg && strcasecmp(arg, "CHARSET") == 0) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 CUSTOM CHARACTER   \r\n"
                "==================================================\r\n"
                " Redefining C64 Character ROM to RAM ($3000)...\r\n\r\n"
                " 10 POKE 52, 48: POKE 56, 48: CLR: REM RESERVE RAM\r\n"
                " 20 POKE 56334, PEEK(56334) AND 254: REM NO INT\r\n"
                " 30 POKE 1, PEEK(1) AND 251: REM SW CHAR ROM IN\r\n"
                " 40 FOR I = 0 TO 2047: POKE 12288+I, PEEK(53248+I): NEXT I\r\n"
                " 50 POKE 1, PEEK(1) OR 4: REM SW CHAR ROM OUT\r\n"
                " 60 POKE 56334, PEEK(56334) OR 1: REM INT ON\r\n"
                " 70 POKE 53272, (PEEK(53272) AND 240) OR 12\r\n"
                " READY.\r\n\r\n"
                " [Character set relocation BASIC layout generated.]\r\n"
                "==================================================\r\n";
        } else if (arg && strcasecmp(arg, "RASTER") == 0) {
            static char dynamic_raster_buf[1024];
            snprintf(dynamic_raster_buf, sizeof(dynamic_raster_buf),
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 RASTER INTERRUPT   \r\n"
                "==================================================\r\n"
                " Generating C64 Raster Sync Split-Screen...\r\n\r\n"
                " 10 POKE 56333, 127: REM DISABLE CIA TIMER INTERRUPTS\r\n"
                " 20 POKE 53265, PEEK(53265) AND 127: REM CLEAR HIGH BIT\r\n"
                " 30 POKE 53266, %d: REM INTERRUPT SCANLINE target = %d\r\n"
                " 35 POKE 53280, %d: REM SPLIT COLOR ON INTERRUPT\r\n"
                " 40 POKE 788, 0: POKE 789, 13: REM REDIRECT VECTOR TO $0D00\r\n"
                " 50 POKE 53274, 1: REM ENABLE VIC-II RASTER INTERRUPT\r\n"
                " 60 SYS 3328: REM ACTIVATE ASSEMBLY HOOK\r\n"
                " READY.\r\n\r\n"
                " [Raster split-screen dynamic configuration generated.]\r\n"
                "==================================================\r\n",
                g_creator_raster_line, g_creator_raster_line, g_creator_raster_color);
            output = dynamic_raster_buf;
        } else if (arg && strcasecmp(arg, "JOYSTICK") == 0) {
            int joy_addr = (g_creator_joystick_port == 1) ? 56321 : 56320;
            static char dynamic_joystick_buf[1024];
            snprintf(dynamic_joystick_buf, sizeof(dynamic_joystick_buf),
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 JOYSTICK SCANNER   \r\n"
                "==================================================\r\n"
                " Generating C64 Joystick Port %d Scanner...\r\n\r\n"
                " 10 J = %d: REM PORT %d ADDRESS\r\n"
                " 20 V = PEEK(J)\r\n"
                " 30 UP = (V AND 1) = 0: DN = (V AND 2) = 0\r\n"
                " 40 LT = (V AND 4) = 0: RT = (V AND 8) = 0\r\n"
                " 50 FR = (V AND 16) = 0: REM FIRE BUTTON\r\n"
                " 60 PRINT \"UP:\"UP\" DN:\"DN\" LT:\"LT\" RT:\"RT\" FIRE:\"FR\r\n"
                " 70 GOTO 20\r\n"
                " READY.\r\n\r\n"
                " [Joystick interactive scanner loop generated.]\r\n"
                "==================================================\r\n",
                g_creator_joystick_port, joy_addr, g_creator_joystick_port);
            output = dynamic_joystick_buf;
        } else if (arg && strcasecmp(arg, "CONCEPT") == 0) {
            static char dynamic_concept_buf[1024];
            snprintf(dynamic_concept_buf, sizeof(dynamic_concept_buf),
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64/VIC-20 SOUND CONCEPT\r\n"
                "==================================================\r\n"
                " Generating A.J. Kwitowski's Sound Concept...\r\n\r\n"
                " 10 DATA %d, %d, %d: REM V1, FREQ=%d, DUR=%d\r\n"
                " 20 DATA %d, %d, %d: REM V2, FREQ=%d, DUR=%d\r\n"
                " 30 DATA %d, %d, %d: REM V3, FREQ=%d, DUR=%d\r\n"
                " 35 DATA -1, 0, 0: REM SENTINEL END\r\n"
                " 40 READ V, F, D\r\n"
                " 50 IF V < 0 THEN END\r\n"
                " 60 POKE 36874 + (V - 10), F: REM SET FREQ\r\n"
                " 70 POKE 36878, 15: REM SET MASTER VOLUME\r\n"
                " 80 FOR T = 1 TO D * 10: NEXT T\r\n"
                " 90 POKE 36874 + (V - 10), 0: REM STOP VOICE\r\n"
                " 100 GOTO 40\r\n"
                " READY.\r\n\r\n"
                " [Kwitowski Sound Concept 3-byte queue generated.]\r\n"
                "==================================================\r\n",
                g_creator_concept_v1, g_creator_concept_f1, g_creator_concept_d1, g_creator_concept_f1, g_creator_concept_d1,
                g_creator_concept_v2, g_creator_concept_f2, g_creator_concept_d2, g_creator_concept_f2, g_creator_concept_d2,
                g_creator_concept_v3, g_creator_concept_f3, g_creator_concept_d3, g_creator_concept_f3, g_creator_concept_d3);
            output = dynamic_concept_buf;
        } else if (arg && (strcasecmp(arg, "SMITH") == 0 || strcasecmp(arg, "MARSHALL") == 0)) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: MARSHALL F. SMITH TRIBUTE\r\n"
                "==================================================\r\n"
                " Generating Production Line Simulator...\r\n\r\n"
                " 10 PRINT \"\\x93\": REM CLEAR SCREEN\r\n"
                " 20 POKE 53280, 0: POKE 53281, 0: REM BLACK SCREEN\r\n"
                " 30 S = 0\r\n"
                " 40 PRINT \"\\x13\": REM HOME CURSOR\r\n"
                " 50 PRINT \"  MARSHALL F. SMITH'S PRODUCTION LINE\"\r\n"
                " 60 PRINT \"  -----------------------------------\"\r\n"
                " 70 PRINT \"  TIME ELAPSED: \"; S; \" SECONDS\"\r\n"
                " 80 PRINT \"  MACINTOSHES : \"; INT(S/26)\r\n"
                " 90 PRINT \"  IBM PCS      : \"; INT(S/16)\r\n"
                " 100 PRINT \"  COMMODORE 64S: \"; INT(S/5)\r\n"
                " 110 S = S + 1\r\n"
                " 120 FOR T = 1 TO 800: NEXT T\r\n"
                " 130 GOTO 40\r\n"
                " READY.\r\n\r\n"
                " [Marshall F. Smith comparison program generated.]\r\n"
                "==================================================\r\n";
        } else if (arg && strcasecmp(arg, "COMTERM") == 0) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: BYTEC-COMTERM TRIBUTE  \r\n"
                "==================================================\r\n"
                " Generating Comterm Bilingual Terminal Emulator...\r\n\r\n"
                " 10 PRINT \"\\x93\": REM CLEAR SCREEN\r\n"
                " 20 PRINT \"*** BYTEC-COMTERM HYPERION RETRO-BILINGUAL ***\"\r\n"
                " 30 PRINT \"  ENG: THE ARABIC LANGUAGE\"\r\n"
                " 40 PRINT \"  ARB: AL-ARABIYYAH (العربية)\"\r\n"
                " 50 PRINT \"----------------------------------------------\"\r\n"
                " 60 PRINT \" MAPPING UNICODE TO RETRO COMTERM CODEPAGE...\"\r\n"
                " 70 DATA 1575, 1604, 1593, 1585, 1576, 1610, 1577\r\n"
                " 80 FOR I = 1 TO 7: READ C: PRINT C; \" \";: NEXT I\r\n"
                " 90 PRINT \"\\n SHAPING ALGORITHM COMPLETED.\"\r\n"
                " READY.\r\n\r\n"
                " [Bytec-Comterm bilingual terminal simulator ready.]\r\n"
                "==================================================\r\n";
        } else if (arg && strcasecmp(arg, "3DPROJ") == 0) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 3D CUBE PROJECTION \r\n"
                "==================================================\r\n"
                " Generating Perspective 3D Cube Projection...\r\n\r\n"
                " 10 PRINT \"\\x93\": REM CLEAR SCREEN\r\n"
                " 20 POKE 53280, 0: POKE 53281, 0: REM BLACK BACKGROUND\r\n"
                " 30 D = 150: REM PERSPECTIVE CAMERA DISTANCE\r\n"
                " 40 FOR I = 0 TO 7\r\n"
                " 50 READ X, Y, Z: REM READ CUBE VERTEX\r\n"
                " 60 XP = (X * D) / (Z + D + 100) + 160\r\n"
                " 70 YP = (Y * D) / (Z + D + 100) + 100\r\n"
                " 80 PRINT \"VERTEX\"; I; \":\"; INT(XP); \",\"; INT(YP)\r\n"
                " 90 NEXT I\r\n"
                " 100 DATA -20,-20,-20,  20,-20,-20,  20, 20,-20, -20, 20,-20\r\n"
                " 110 DATA -20,-20, 20,  20,-20, 20,  20, 20, 20, -20, 20, 20\r\n"
                " READY.\r\n\r\n"
                " [3D perspective projection graphics listing complete.]\r\n"
                "==================================================\r\n";
        } else if (arg && strcasecmp(arg, "PLATFORM") == 0) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 PLATFORMER LOOP    \r\n"
                "==================================================\r\n"
                " Generating C64 BASIC Platformer Game Loop...\r\n\r\n"
                " 10 PRINT \"\\x93\": REM CLEAR SCREEN\r\n"
                " 20 POKE 53280, 0: POKE 53281, 0: REM BLACK COL\r\n"
                " 30 X = 20: Y = 20: DX = 0: DY = 0: G = 1: REM PLAYER\r\n"
                " 40 PRINT \"\\x13\";: REM HOME CURSOR\r\n"
                " 50 REM DRAW PLATFORMS AND JUMPING CHARACTER\r\n"
                " 60 POKE 1024 + Y * 40 + X, 81: REM DRAW 'Q'\r\n"
                " 70 K = PEEK(56320): REM SCAN JOYSTICK PORT 2\r\n"
                " 80 IF (K AND 4) = 0 THEN X = X - 1: REM LEFT\r\n"
                " 90 IF (K AND 8) = 0 THEN X = X + 1: REM RIGHT\r\n"
                " 100 IF (K AND 16) = 0 AND G = 1 THEN DY = -2: G = 0: REM JUMP\r\n"
                " 110 DY = DY + 0.2: REM GRAVITY EFFECT\r\n"
                " 120 Y = Y + DY\r\n"
                " 130 IF Y >= 24 THEN Y = 24: DY = 0: G = 1: REM COLLISION\r\n"
                " 140 FOR T = 1 TO 50: NEXT T: GOTO 40\r\n"
                " READY.\r\n\r\n"
                " [Platformer basic game loop generation complete.]\r\n"
                "==================================================\r\n";
        } else if (arg && strcasecmp(arg, "TRIANGLE") == 0) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 LETTER TRIANGLE    \r\n"
                "==================================================\r\n"
                " Generating C64 BASIC Letter Triangle Program...\r\n\r\n"
                " 10 FOR L = 1 TO 26\r\n"
                " 20 FOR N = 1 TO L\r\n"
                " 30 PRINT CHR$(L + 64);\r\n"
                " 40 NEXT N\r\n"
                " 50 PRINT\r\n"
                " 60 NEXT L\r\n"
                " READY.\r\n\r\n"
                " [Letter triangle program listing complete.]\r\n"
                "==================================================\r\n";
        } else if (arg && strcasecmp(arg, "PALIN") == 0) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 PALINDROME SOLVER  \r\n"
                "==================================================\r\n"
                " Generating C64 BASIC Palindrome Solver...\r\n\r\n"
                " 1 INPUT N$: L = LEN(N$)\r\n"
                " 2 FOR X = 1 TO L\r\n"
                " 3 IF MID$(N$, X, 1) = MID$(N$, L + 1 - X, 1) THEN NEXT: PRINT \"YES\"\r\n"
                " READY.\r\n\r\n"
                " [Palindrome solver program generated.]\r\n"
                "==================================================\r\n";
        } else if (arg && strcasecmp(arg, "REVERSE") == 0) {
            output = 
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 REVERSE TYPIST     \r\n"
                "==================================================\r\n"
                " Generating C64 BASIC Right-To-Left Reverse Typist...\r\n\r\n"
                " 10 X = 39: B$ = CHR$(32): C$ = CHR$(13)\r\n"
                " 20 GET A$: IF A$ = \"\" THEN 20\r\n"
                " 30 X = X - 1: PRINT SPC(X) A$ \"\\x91\";: REM CURSOR UP\r\n"
                " 40 IF A$ = B$ AND X <= 10 THEN X = 39: PRINT\r\n"
                " 50 IF X = 0 THEN X = 39: PRINT\r\n"
                " 60 IF A$ = C$ THEN X = 39: PRINT\r\n"
                " 70 GOTO 20\r\n"
                " READY.\r\n\r\n"
                " [Right-to-left reverse typist listing ready.]\r\n"
                "==================================================\r\n";
        } else {
            static char dynamic_maze_buf[1024];
            snprintf(dynamic_maze_buf, sizeof(dynamic_maze_buf),
                "==================================================\r\n"
                "   HURWOOD CODE GENERATOR: C64 MAZE GRAPHICS      \r\n"
                "==================================================\r\n"
                " Generating C64 BASIC Vector Maze Program...\r\n\r\n"
                " 10 PRINT \"\\x93\": REM CLEAR SCREEN\r\n"
                " 20 POKE 53280, %d: POKE 53281, %d: REM COLORS\r\n"
                " 30 FOR I = 1 TO 1000\r\n"
                " 40 R = INT(RND(1)*2)\r\n"
                " 50 IF R = 0 THEN PRINT \"%s\";: GOTO 70\r\n"
                " 60 PRINT \"%s\";\r\n"
                " 70 NEXT I\r\n"
                " 80 PRINT \"\\nGENERATION COMPLETE.\"\r\n"
                " READY.\r\n\r\n"
                " [Usage: HURWOOD [MAZE | SOUND | SPRITE | CHARSET | RASTER | JOYSTICK | CONCEPT | SMITH | PLATFORM | TRIANGLE | PALIN | REVERSE] [COMPACT | STAGE]]\r\n"
                "==================================================\r\n",
                g_creator_maze_bg, g_creator_maze_border, g_creator_maze_char1, g_creator_maze_char2);
            output = dynamic_maze_buf;
        }
        
        write_basic_lines(output, compact);
        if (stage && output) {
            inject_basic_program(output);
            lau_vram_write_string(g_vram, "\r\n  [SUCCESS: Program tokenized & staged directly in virtual RAM!]\r\n", 67);
        }
        log_telemetry("Rendered Hurwood Code Generator Screen");
        return;
    }

    if (first_word && strcasecmp(first_word, "YULBUILD") == 0) {
        g_editor_mode = MODE_YULBUILD;
        g_mercenary_active = false;
        g_pong_active = false;
        init_yulbuild_state();
        redraw_yulbuild_screen();
        return;
    }

    if (first_word && strcasecmp(first_word, "CREATOR") == 0) {
        g_editor_mode = MODE_CREATOR;
        g_mercenary_active = false;
        g_pong_active = false;
        g_creator_step = 0;
        g_creator_selection = 0;
        g_creator_param_index = 0;
        g_creator_editing_value = false;
        redraw_creator_screen();
        return;
    }

    if (first_word && strcasecmp(first_word, "WORDCRAFT") == 0) {
        g_editor_mode = MODE_WORDCRAFT;
        g_mercenary_active = false;
        g_pong_active = false;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        const char *header = 
            "--------------------------------------------------\r\n"
            "       WORDCRAFT 80 ULTRA: ON-CHAIN OFFICE        \r\n"
            "--------------------------------------------------\r\n"
            "  LINE: 1   COL: 1   SPACE: 32K FREE   REC: 1/1   \r\n"
            "==================================================\r\n"
            " [Press ESC to return to Terminal Menu]          \r\n\r\n";
        lau_vram_write_string(g_vram, header, strlen(header));
        log_telemetry("Rendered Wordcraft Screen");
        return;
    }
    if (first_word && strcasecmp(first_word, "EASYSCRIPT") == 0) {
        g_editor_mode = MODE_EASYSCRIPT;
        g_mercenary_active = false;
        g_pong_active = false;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        const char *header = 
            "==================================================\r\n"
            "        EASYSCRIPT 64 WORD PROCESSOR v1.0         \r\n"
            "==================================================\r\n"
            "1....+....2....+....3....+....4....+....5....+....\r\n"
            "  READY. 40960 BYTES FREE.                        \r\n"
            "--------------------------------------------------\r\n"
            " [Press ESC to return to Terminal Menu]          \r\n\r\n";
        lau_vram_write_string(g_vram, header, strlen(header));
        log_telemetry("Rendered EasyScript Screen");
        return;
    }
    if (first_word && strcasecmp(first_word, "DNATYPEWRITER") == 0) {
        g_editor_mode = MODE_DNATYPEWRITER;
        g_mercenary_active = false;
        g_pong_active = false;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        const char *header = 
            "##################################################\r\n"
            "       TSFI DNA TYPEWRITER - VECTOR SYLLABLES     \r\n"
            "##################################################\r\n"
            "  CODON: ATG   GENE: TSFI-V2   STABILITY: 99.8%   \r\n"
            "==================================================\r\n"
            " [Press ESC to return to Terminal Menu]          \r\n\r\n";
        lau_vram_write_string(g_vram, header, strlen(header));
        log_telemetry("Rendered DNATypewriter Screen");
        return;
    }
     
    if (first_word && strcasecmp(first_word, "INSTA") == 0) {
         g_editor_mode = MODE_INSTAWRITER;
         g_mercenary_active = false;
         g_pong_active = false;
         const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
         lau_vram_write_string(g_vram, clear_seq, 3);
         const char *header = 
             "==================================================\r\n"
             "        INSTA-WRITER C64 WORD PROCESSOR (CIMARRON) \r\n"
             "==================================================\r\n"
             "  - MEMORY: 38K FREE     - STATUS: DOCUMENT MODE   \r\n"
             "  - MARGINS: L=10 R=70   - SPACING: SINGLE         \r\n"
             "==================================================\r\n"
             " [Press ESC to return to Terminal Menu]          \r\n\r\n";
         lau_vram_write_string(g_vram, header, strlen(header));
         log_telemetry("Rendered Insta-Writer Screen");
         return;
    }
    
    if (first_word && (strcasecmp(first_word, "CALC") == 0 || strcasecmp(first_word, "INSTACALC") == 0)) {
         g_editor_mode = MODE_INSTACALC;
         g_mercenary_active = false;
         g_pong_active = false;
