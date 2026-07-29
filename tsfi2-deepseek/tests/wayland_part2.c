        " [Press ESC to return to Terminal Menu]           \r\n"
        " [A/D to move, W/S to climb, SPACE to dig holes]  \r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void update_applepanic_game(void) {
    if (!g_applepanic_active) return;
    
    static int monster_tick = 0;
    monster_tick++;
    if (monster_tick % 20 == 0) {
        for (int f = 0; f < 4; f++) {
            for (int c = 0; c < 40; c++) {
                if (g_panic_dig_ticks[f][c] > 0) {
                    g_panic_dig_ticks[f][c]--;
                }
            }
        }
        
        if (g_panic_monster_stuck_ticks > 0) {
            g_panic_monster_stuck_ticks--;
            if (g_panic_monster_stuck_ticks == 0) {
                g_panic_dig_ticks[g_panic_monster_y][g_panic_monster_x] = 0;
            }
        } else {
            bool climbed = false;
            if (g_panic_monster_y == 1 && g_panic_monster_x == 12 && g_panic_player_y < 1) {
                g_panic_monster_y = 0; climbed = true;
            } else if (g_panic_monster_y == 0 && g_panic_monster_x == 12 && g_panic_player_y > 0) {
                g_panic_monster_y = 1; climbed = true;
            } else if (g_panic_monster_y == 2 && g_panic_monster_x == 28 && g_panic_player_y < 2) {
                g_panic_monster_y = 1; climbed = true;
            } else if (g_panic_monster_y == 1 && g_panic_monster_x == 28 && g_panic_player_y > 1) {
                g_panic_monster_y = 2; climbed = true;
            } else if (g_panic_monster_y == 3 && g_panic_monster_x == 18 && g_panic_player_y < 3) {
                g_panic_monster_y = 2; climbed = true;
            } else if (g_panic_monster_y == 2 && g_panic_monster_x == 18 && g_panic_player_y > 2) {
                g_panic_monster_y = 3; climbed = true;
            }

            if (!climbed) {
                if (g_panic_monster_x < g_panic_player_x) g_panic_monster_x++;
                else if (g_panic_monster_x > g_panic_player_x) g_panic_monster_x--;
            }
            
            if (g_panic_dig_ticks[g_panic_monster_y][g_panic_monster_x] > 0) {
                g_panic_monster_stuck_ticks = 30;
            }
            
            if (g_panic_monster_x == g_panic_player_x && g_panic_monster_y == g_panic_player_y) {
                g_panic_lives--;
                if (g_panic_lives <= 0) {
                    g_panic_lives = 3;
                    g_panic_score = 0;
                }
                g_panic_player_x = 2;
                g_panic_player_y = 3;
                g_panic_monster_x = 25;
                g_panic_monster_y = 3;
            }
        }
        redraw_applepanic_screen();
    }
}

static void handle_applepanic_input(char ch) {
    if (ch == 'a' || ch == 'A') {
        g_panic_player_facing = -1;
        if (g_panic_player_x > 0) g_panic_player_x--;
        redraw_applepanic_screen();
    } else if (ch == 'd' || ch == 'D') {
        g_panic_player_facing = 1;
        if (g_panic_player_x < 39) g_panic_player_x++;
        redraw_applepanic_screen();
    } else if (ch == 'w' || ch == 'W') {
        if (g_panic_player_y == 1 && g_panic_player_x == 12) g_panic_player_y = 0;
        else if (g_panic_player_y == 2 && g_panic_player_x == 28) g_panic_player_y = 1;
        else if (g_panic_player_y == 3 && g_panic_player_x == 18) g_panic_player_y = 2;
        redraw_applepanic_screen();
    } else if (ch == 's' || ch == 'S') {
        if (g_panic_player_y == 0 && g_panic_player_x == 12) g_panic_player_y = 1;
        else if (g_panic_player_y == 1 && g_panic_player_x == 28) g_panic_player_y = 2;
        else if (g_panic_player_y == 2 && g_panic_player_x == 18) g_panic_player_y = 3;
        redraw_applepanic_screen();
    } else if (ch == ' ') {
        int target_x = g_panic_player_x + g_panic_player_facing;
        if (target_x >= 0 && target_x < 40) {
            g_panic_dig_ticks[g_panic_player_y][target_x] = 50;
            if (g_panic_monster_y == g_panic_player_y && g_panic_monster_x == target_x && g_panic_monster_stuck_ticks > 0) {
                g_panic_score += 100;
                g_panic_monster_x = 30;
                g_panic_monster_y = 0;
                g_panic_monster_stuck_ticks = 0;
            }
        }
        redraw_applepanic_screen();
    }
}

static void redraw_airassault_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "==================================================\r\n"
        "   AIR ASSAULT - BY BOB LLORET (AHOY! ISSUE 5)    \r\n"
        "==================================================\r\n"
        " Score: %05d   Shields: %d/3   Stage: 1\r\n"
        "==================================================\r\n",
        g_air_score, g_air_shields);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    for (int y = 0; y < 8; y++) {
        char line[51];
        memset(line, ' ', 50);
        line[50] = '\0';
        
        if (y == 7) {
            line[g_air_player_x] = '^';
            if (g_air_player_x > 0) line[g_air_player_x - 1] = '=';
            if (g_air_player_x < 49) line[g_air_player_x + 1] = '=';
        }
        
        if (g_air_missile_y == y && g_air_missile_x >= 0 && g_air_missile_x < 50) {
            line[g_air_missile_x] = '|';
        }
        
        for (int i = 0; i < 5; i++) {
            if (g_air_invaders_y[i] == y && g_air_invaders_x[i] >= 0 && g_air_invaders_x[i] < 50) {
                line[g_air_invaders_x[i]] = 'v';
            }
        }
        
        snprintf(buf, sizeof(buf), "   %s\r\n", line);
        lau_vram_write_string(g_vram, buf, strlen(buf));
    }
    
    snprintf(buf, sizeof(buf),
        "==================================================\r\n"
        " [Press ESC to return to Terminal Menu]           \r\n"
        " [A/D to move, SPACE to fire missile]             \r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void update_airassault_game(void) {
    if (!g_airassault_active) return;
    
    static int air_tick = 0;
    static int last_shield_reward = 0;
    air_tick++;
    
    // Shield reward logic
    if (g_air_score - last_shield_reward >= 1000) {
        if (g_air_shields < 3) {
            g_air_shields++;
        }
        last_shield_reward = (g_air_score / 1000) * 1000;
    }
    
    if (air_tick % 5 == 0) {
        if (g_air_missile_y >= 0) {
            g_air_missile_y--;
            for (int i = 0; i < 5; i++) {
                if (g_air_invaders_y[i] == g_air_missile_y && abs(g_air_invaders_x[i] - g_air_missile_x) <= 1) {
                    g_air_score += 100;
                    g_air_invaders_y[i] = 0;
                    g_air_invaders_x[i] = rand() % 48 + 1;
                    g_air_missile_y = -1;
                    g_air_missile_x = -1;
                    break;
                }
            }
        }
        
        // Speed up invaders as score increases (speed_modulo decreases)
        int speed_modulo = 25 - (g_air_score / 1000) * 2;
        if (speed_modulo < 8) speed_modulo = 8;
        
        if (air_tick % speed_modulo == 0) {
            for (int i = 0; i < 5; i++) {
                g_air_invaders_y[i]++;
                if (g_air_invaders_y[i] >= 7) {
                    g_air_shields--;
                    if (g_air_shields <= 0) {
                        g_air_shields = 3;
                        g_air_score = 0;
                        last_shield_reward = 0;
                    }
                    g_air_invaders_y[i] = 0;
                    g_air_invaders_x[i] = rand() % 48 + 1;
                }
            }
        }
        redraw_airassault_screen();
    }
}

static void handle_airassault_input(char ch) {
    if (ch == 'a' || ch == 'A') {
        if (g_air_player_x > 1) g_air_player_x--;
        redraw_airassault_screen();
    } else if (ch == 'd' || ch == 'D') {
        if (g_air_player_x < 48) g_air_player_x++;
        redraw_airassault_screen();
    } else if (ch == ' ') {
        if (g_air_missile_y == -1) {
            g_air_missile_x = g_air_player_x;
            g_air_missile_y = 6;
        }
        redraw_airassault_screen();
    }
}

static void redraw_slinkybear_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "==================================================\r\n"
        "      SLINKY THE BEAR - AHOY! ISSUE 6 CUSTOM      \r\n"
        "==================================================\r\n"
        " Score: %05d   Lives: %d   Stage: 1\r\n"
        "==================================================\r\n\r\n",
        g_slinky_score, g_slinky_lives);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    for (int r = 0; r < 5; r++) {
        int leading_spaces = 24 - r * 3;
        for (int s = 0; s < leading_spaces; s++) {
            lau_vram_write_char(g_vram, ' ');
        }
        for (int c = 0; c <= r; c++) {
            lau_vram_write_string(g_vram, "/\\  ", 4);
        }
        lau_vram_write_string(g_vram, "\r\n", 2);
        
        for (int s = 0; s < leading_spaces - 1; s++) {
            lau_vram_write_char(g_vram, ' ');
        }
        for (int c = 0; c <= r; c++) {
            char val_str[32];
            if (g_slinky_row == r && g_slinky_col == c) {
                snprintf(val_str, sizeof(val_str), "/\x1b[33mB\x1b[0m\\");
            } else {
                if (g_slinky_blocks[r][c] == 0) {
                    snprintf(val_str, sizeof(val_str), "/0\\");
                } else {
                    snprintf(val_str, sizeof(val_str), "/\x1b[32m1\x1b[0m\\");
                }
            }
            lau_vram_write_string(g_vram, val_str, strlen(val_str));
            lau_vram_write_string(g_vram, "  ", 2);
        }
        lau_vram_write_string(g_vram, "\r\n", 2);
    }
    
    snprintf(buf, sizeof(buf),
        "\r\n==================================================\r\n"
        " [Press ESC to return to Terminal Menu]           \r\n"
        " [I/K/J/L to jump diagonally, color all blocks]  \r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void update_slinkybear_game(void) {
    // Turn-based game, no real-time logic needed.
}

static void handle_slinkybear_input(char ch) {
    int next_row = g_slinky_row;
    int next_col = g_slinky_col;
    bool moved = false;
    
    if (ch == 'i' || ch == 'I') {
        next_row--;
        next_col--;
        moved = true;
    } else if (ch == 'o' || ch == 'O') {
        next_row--;
        moved = true;
    } else if (ch == 'k' || ch == 'K') {
        next_row++;
        moved = true;
    } else if (ch == 'l' || ch == 'L') {
        next_row++;
        next_col++;
        moved = true;
    }
    
    if (moved) {
        printf("\x07");
        fflush(stdout);
        if (next_row < 0 || next_row > 4 || next_col < 0 || next_col > next_row) {
            g_slinky_lives--;
            if (g_slinky_lives <= 0) {
                g_slinky_lives = 3;
                g_slinky_score = 0;
                memset(g_slinky_blocks, 0, sizeof(g_slinky_blocks));
            }
            g_slinky_row = 0;
            g_slinky_col = 0;
        } else {
            g_slinky_row = next_row;
            g_slinky_col = next_col;
            if (g_slinky_blocks[g_slinky_row][g_slinky_col] == 0) {
                g_slinky_blocks[g_slinky_row][g_slinky_col] = 1;
                g_slinky_score += 10;
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
        
        redraw_slinkybear_screen();
    }
}

static void trigger_alpiner_speech(const char *phrase) {
    strncpy(g_alpiner_speech, phrase, sizeof(g_alpiner_speech) - 1);
    g_alpiner_speech[sizeof(g_alpiner_speech) - 1] = '\0';
    g_alpiner_speech_ticks = 20;
    g_alpiner_tms5220_status = 0x40; // TS=1
    g_alpiner_tms5220_cmd = 0x50; // Speak External
    g_alpiner_tms5220_fifo = 16;
    printf("\x07");
    fflush(stdout);
}

static void redraw_alpiner_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char buf[1024];
    const char *mountain_names[] = {
        "Mt. Hood",
        "Mt. Kilimanjaro",
        "Mt. Everest",
        "Mt. McKinley (Denali)",
        "Matterhorn",
        "Mt. Kenya"
    };
    const char *mt_name = mountain_names[(g_alpiner_mountain - 1) % 6];
    
    snprintf(buf, sizeof(buf),
        "==============================================================\r\n"
        "      ALPINER (TI-99/4A TMS5220 Speech Synth Demo)            \r\n"
        "==============================================================\r\n"
        " Mountain: %s   Score: %05d   Lives: %d\r\n"
        "==============================================================\r\n\r\n",
        mt_name, g_alpiner_score, g_alpiner_lives);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    for (int y = 1; y <= 12; y++) {
        int left_boundary = 22 - y;
        int right_boundary = 22 + y;
        
        for (int x = 0; x < left_boundary; x++) {
            lau_vram_write_char(g_vram, ' ');
        }
        lau_vram_write_char(g_vram, '/');
        
        for (int x = left_boundary + 1; x < right_boundary; x++) {
            if (g_alpiner_player_x == x && g_alpiner_player_y == y) {
                lau_vram_write_string(g_vram, "\x1b[1;32mP\x1b[0m", 9);
            } else if (g_alpiner_yeti_x == x && g_alpiner_yeti_y == y) {
                lau_vram_write_string(g_vram, "\x1b[1;35mY\x1b[0m", 9);
            } else if (g_alpiner_rock_x == x && g_alpiner_rock_y == y) {
                lau_vram_write_string(g_vram, "\x1b[1;31m*\x1b[0m", 9);
            } else if (y == 12) {
                lau_vram_write_char(g_vram, '_');
            } else if (y == 1 && x == 22) {
                lau_vram_write_string(g_vram, "\x1b[1;33mF\x1b[0m", 9);
            } else if ((x + y) % 9 == 0) {
                lau_vram_write_char(g_vram, 't');
            } else {
                lau_vram_write_char(g_vram, ' ');
            }
        }
        lau_vram_write_char(g_vram, '\\');
        lau_vram_write_string(g_vram, "\r\n", 2);
    }
    
    lau_vram_write_string(g_vram, "\r\n==============================================================\r\n", 64);
    
    char tms_status_str[128];
    snprintf(tms_status_str, sizeof(tms_status_str),
        " TS=1 (Talk Status) | BE=%d (Buf Empty) | BL=%d (Buf Low) | CMD=0x%02X\r\n",
        (g_alpiner_tms5220_status & 0x01) ? 1 : 0,
        (g_alpiner_tms5220_status & 0x02) ? 1 : 0,
        g_alpiner_tms5220_cmd);
        
    lau_vram_write_string(g_vram, " [TI TMS5220 LPC SPEECH CHIP EMULATION]\r\n", 41);
    lau_vram_write_string(g_vram, tms_status_str, strlen(tms_status_str));
    
    if (g_alpiner_speech_ticks > 0) {
        snprintf(buf, sizeof(buf), " TMS5220 Speech Output: \x1b[1;33m\"%s\"\x1b[0m\r\n", g_alpiner_speech);
    } else {
        snprintf(buf, sizeof(buf), " TMS5220 Speech Output: (idle)\r\n");
    }
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    lau_vram_write_string(g_vram, "==============================================================\r\n", 64);
    lau_vram_write_string(g_vram, " [Arrow Keys / W,A,S,D to climb | ESC to return to menu]\r\n", 57);
}

static void update_alpiner_game(void) {
    if (!g_alpiner_active) return;
    
    static int game_tick = 0;
    game_tick++;
    
    if (g_alpiner_speech_ticks > 0) {
        g_alpiner_speech_ticks--;
        if (g_alpiner_speech_ticks == 0) {
            g_alpiner_tms5220_status = 0x02; // BE=1
            g_alpiner_tms5220_cmd = 0x00;
            g_alpiner_tms5220_fifo = 0;
        }
    }
    
    if (game_tick % 3 == 0) {
        g_alpiner_rock_y++;
        if (g_alpiner_rock_y > 12) {
            g_alpiner_rock_y = 1;
            g_alpiner_rock_x = 22 + (rand() % 3 - 1);
        }
    }
    
    if (game_tick % 8 == 0) {
        int dir = rand() % 5;
        if (dir == 0 && g_alpiner_yeti_x > 22 - g_alpiner_yeti_y) g_alpiner_yeti_x--;
        else if (dir == 1 && g_alpiner_yeti_x < 22 + g_alpiner_yeti_y) g_alpiner_yeti_x++;
        else if (dir == 2 && g_alpiner_yeti_y > 1) g_alpiner_yeti_y--;
        else if (dir == 3 && g_alpiner_yeti_y < 12) g_alpiner_yeti_y++;
    }
    
    if (g_alpiner_player_x == g_alpiner_rock_x && g_alpiner_player_y == g_alpiner_rock_y) {
        g_alpiner_lives--;
        trigger_alpiner_speech("TIMBER!");
        g_alpiner_player_x = 22;
        g_alpiner_player_y = 12;
        if (g_alpiner_lives <= 0) {
            g_alpiner_lives = 3;
            g_alpiner_score = 0;
            g_alpiner_mountain = 1;
            trigger_alpiner_speech("OH NO!");
        }
    }
    
    if (g_alpiner_player_x == g_alpiner_yeti_x && g_alpiner_player_y == g_alpiner_yeti_y) {
        g_alpiner_lives--;
        trigger_alpiner_speech("OH NO!");
        g_alpiner_player_x = 22;
        g_alpiner_player_y = 12;
        if (g_alpiner_lives <= 0) {
            g_alpiner_lives = 3;
            g_alpiner_score = 0;
            g_alpiner_mountain = 1;
        }
    }
    
    if (game_tick % 2 == 0) {
        redraw_alpiner_screen();
    }
}

static void handle_alpiner_input(char ch) {
    if (ch == '\x1b') {
        g_alpiner_active = false;
        g_editor_mode = MODE_TERMINAL;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "CIS: ALPINER exited.\r\n", 22);
        return;
    }
    
    int next_x = g_alpiner_player_x;
    int next_y = g_alpiner_player_y;
    bool moved = false;
    
    if (ch == 'a' || ch == 'A' || ch == 'j' || ch == 'J') {
        next_x--;
        moved = true;
    } else if (ch == 'd' || ch == 'D' || ch == 'l' || ch == 'L') {
        next_x++;
        moved = true;
    } else if (ch == 'w' || ch == 'W' || ch == 'i' || ch == 'I') {
        next_y--;
        moved = true;
    } else if (ch == 's' || ch == 'S' || ch == 'k' || ch == 'K') {
        next_y++;
        moved = true;
    }
    
    if (moved) {
        if (next_y < 1) next_y = 1;
        if (next_y > 12) next_y = 12;
        int left_boundary = 22 - next_y;
        int right_boundary = 22 + next_y;
        if (next_x < left_boundary + 1) next_x = left_boundary + 1;
        if (next_x > right_boundary - 1) next_x = right_boundary - 1;
        
        g_alpiner_player_x = next_x;
        g_alpiner_player_y = next_y;
        
        if (rand() % 10 == 0) {
            trigger_alpiner_speech("GOIN' UP!");
        }
        
        if (g_alpiner_player_y == 1 && g_alpiner_player_x == 22) {
            g_alpiner_score += 500;
            g_alpiner_mountain++;
            if (g_alpiner_mountain > 6) g_alpiner_mountain = 1;
            g_alpiner_player_x = 22;
            g_alpiner_player_y = 12;
            trigger_alpiner_speech("ALPINER!");
        }
        
        redraw_alpiner_screen();
    }
}

static void save_checklist(void) {
    FILE *f = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/checklist_data.txt", "w");
    if (!f) {
        strncpy(g_checklist_status_msg, "Error: Could not save file.", sizeof(g_checklist_status_msg) - 1);
        return;
    }
    for (int i = 0; i < g_checklist_count; i++) {
        fprintf(f, "[%c] %s | %s\n", g_checklist_items[i].checked ? 'X' : ' ', g_checklist_items[i].category, g_checklist_items[i].text);
    }
    fclose(f);
    strncpy(g_checklist_status_msg, "Checklist saved successfully.", sizeof(g_checklist_status_msg) - 1);
}

static void load_checklist(void) {
    FILE *f = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/checklist_data.txt", "r");
    if (!f) {
        strncpy(g_checklist_status_msg, "No saved checklist found.", sizeof(g_checklist_status_msg) - 1);
        return;
    }
    g_checklist_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), f) && g_checklist_count < 30) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 6) continue;
        
        bool checked = (line[1] == 'X' || line[1] == 'x');
        char *bar = strchr(line + 4, '|');
        if (!bar) continue;
        *bar = '\0';
        
        char *cat = line + 4;
        while (*cat == ' ') cat++;
        char *cat_end = bar - 1;
        while (cat_end > cat && *cat_end == ' ') {
            *cat_end = '\0';
            cat_end--;
        }
        
        char *txt = bar + 1;
        while (*txt == ' ') txt++;
        
        g_checklist_items[g_checklist_count].checked = checked;
        snprintf(g_checklist_items[g_checklist_count].category, sizeof(g_checklist_items[g_checklist_count].category), "%.31s", cat);
        snprintf(g_checklist_items[g_checklist_count].text, sizeof(g_checklist_items[g_checklist_count].text), "%.63s", txt);
        
        g_checklist_count++;
    }
    fclose(f);
    g_checklist_cursor = 0;
    strncpy(g_checklist_status_msg, "Checklist loaded successfully.", sizeof(g_checklist_status_msg) - 1);
}

static void init_towers(void) {
    g_towers_counts[0] = 5;
    g_towers_counts[1] = 0;
    g_towers_counts[2] = 0;
    
    // Bottom disk is 5, top is 1
    g_towers_pegs[0][0] = 5;
    g_towers_pegs[0][1] = 4;
    g_towers_pegs[0][2] = 3;
    g_towers_pegs[0][3] = 2;
    g_towers_pegs[0][4] = 1;
    
    g_towers_moves = 0;
    g_towers_src = -1;
    snprintf(g_towers_status, sizeof(g_towers_status), "Select source peg: press [A/B/C].");
}

static void redraw_towers_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "============================================================\r\n"
             "                 TOWERS OF HANOI (Ahoy! Issue 18)           \r\n"
             "============================================================\r\n"
             "  Moves: %d   |   Selected Source: %s\r\n"
             "============================================================\r\n\r\n",
             g_towers_moves, (g_towers_src == 0) ? "Peg A" : (g_towers_src == 1) ? "Peg B" : (g_towers_src == 2) ? "Peg C" : "None");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render the pegs and disks from top to bottom (row 4 to 0)
    for (int h = 4; h >= 0; h--) {
        char line[128] = "";
        char peg_strs[3][32];
        for (int p = 0; p < 3; p++) {
            if (h < g_towers_counts[p]) {
                int disk_size = g_towers_pegs[p][h];
                int padding = 5 - disk_size;
                char disk_draw[32] = "";
                for (int i = 0; i < padding; i++) strcat(disk_draw, " ");
                for (int i = 0; i < disk_size * 2 - 1; i++) strcat(disk_draw, "*");
                for (int i = 0; i < padding; i++) strcat(disk_draw, " ");
                snprintf(peg_strs[p], sizeof(peg_strs[p]), "%-11s", disk_draw);
            } else {
                snprintf(peg_strs[p], sizeof(peg_strs[p]), "     |     ");
            }
        }
        snprintf(line, sizeof(line), "         %s       %s       %s\r\n", peg_strs[0], peg_strs[1], peg_strs[2]);
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "     ====================================================\r\n", 58);
    lau_vram_write_string(g_vram, "            [ Peg A ]           [ Peg B ]           [ Peg C ]\r\n\r\n", 71);
    lau_vram_write_string(g_vram, "============================================================\r\n", 62);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_towers_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: Press [A/B/C] to select Pegs, [R] Reset, [ESC] Exit\r\n", 64);
}

static void handle_towers_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }
    if (ch == 'r' || ch == 'R') {
        init_towers();
        redraw_towers_screen();
        return;
    }

    int peg_idx = -1;
    if (ch == 'a' || ch == 'A') peg_idx = 0;
    else if (ch == 'b' || ch == 'B') peg_idx = 1;
    else if (ch == 'c' || ch == 'C') peg_idx = 2;

    if (peg_idx != -1) {
        if (g_towers_src == -1) {
            if (g_towers_counts[peg_idx] > 0) {
                g_towers_src = peg_idx;
                snprintf(g_towers_status, sizeof(g_towers_status), "Source selected: Peg %c. Select target peg: [A/B/C].", 'A' + peg_idx);
            } else {
                snprintf(g_towers_status, sizeof(g_towers_status), "Peg %c is empty! Select another source peg.", 'A' + peg_idx);
            }
        } else {
            int src = g_towers_src;
            if (peg_idx == src) {
                g_towers_src = -1;
                snprintf(g_towers_status, sizeof(g_towers_status), "Selection cancelled. Select source peg: [A/B/C].");
            } else {
                int src_count = g_towers_counts[src];
                int tgt_count = g_towers_counts[peg_idx];
                int src_top = g_towers_pegs[src][src_count - 1];
                int tgt_top = (tgt_count == 0) ? 99 : g_towers_pegs[peg_idx][tgt_count - 1];

                if (src_top < tgt_top) {
                    g_towers_pegs[peg_idx][tgt_count] = src_top;
                    g_towers_counts[peg_idx]++;
                    g_towers_counts[src]--;
                    g_towers_moves++;
                    g_towers_src = -1;

                    if (g_towers_counts[2] == 5) {
                        snprintf(g_towers_status, sizeof(g_towers_status), "CONGRATULATIONS! Solved in %d moves!", g_towers_moves);
                    } else {
                        snprintf(g_towers_status, sizeof(g_towers_status), "Moved disk %d to Peg %c. Select source peg: [A/B/C].", src_top, 'A' + peg_idx);
                    }
                } else {
                    snprintf(g_towers_status, sizeof(g_towers_status), "Illegal move! Cannot place disk %d on %d. Select target peg.", src_top, tgt_top);
                }
            }
        }
    }

    redraw_towers_screen();
}

static void init_disintegrator(void) {
    g_disint_px = 4;
    g_disint_py = 4;
    g_disint_tx = 0;
    g_disint_ty = (rand() % 9);
    g_disint_score = 0;
    g_disint_lives = 3;
    snprintf(g_disint_status, sizeof(g_disint_status), "DISINTEGRATORS ON. Arrow Keys = Move, W/A/S/D = Fire blast.");
}

static void redraw_disintegrator_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "============================================================\r\n"
             "                 DISINTEGRATOR (Ahoy! Issue 20)             \r\n"
             "============================================================\r\n"
             "  Score: %d     |     Lives: %d\r\n"
             "============================================================\r\n\r\n",
             g_disint_score, g_disint_lives);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render 9x9 grid
    for (int y = 0; y < 9; y++) {
        char row_str[128] = "       ";
        for (int x = 0; x < 9; x++) {
            if (x == g_disint_px && y == g_disint_py) {
                strcat(row_str, "^ "); // Player
            } else if (x == g_disint_tx && y == g_disint_ty) {
                strcat(row_str, "* "); // Target/Enemy
            } else {
                strcat(row_str, ". ");
            }
        }
        strcat(row_str, "\r\n");
        lau_vram_write_string(g_vram, row_str, strlen(row_str));
    }

    lau_vram_write_string(g_vram, "\r\n============================================================\r\n", 64);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_disint_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: Arrows = Move, [W/A/S/D] = Shoot Blast, [ESC] Exit\r\n", 64);
}

static void handle_disintegrator_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (g_disint_lives <= 0) {
        snprintf(g_disint_status, sizeof(g_disint_status), "GAME OVER! Press [ESC] to exit.");
        redraw_disintegrator_screen();
        return;
    }

    // Move player using arrow controls mapped to custom chars (or W/A/S/D to shoot)
    // We will support simple char moves: W/A/S/D for firing, I/J/K/L for moving or handle arrow keys mapping
    // Let's check ch: 'i' = up, 'k' = down, 'j' = left, 'l' = right
    if (ch == 'i' || ch == 'I') {
        if (g_disint_py > 0) g_disint_py--;
    } else if (ch == 'k' || ch == 'K') {
        if (g_disint_py < 8) g_disint_py++;
    } else if (ch == 'j' || ch == 'J') {
        if (g_disint_px > 0) g_disint_px--;
    } else if (ch == 'l' || ch == 'L') {
        if (g_disint_px < 8) g_disint_px++;
    }

    // Shooting blasts: 'w' = up, 's' = down, 'a' = left, 'd' = right
    bool shot = false;
    if (ch == 'w' || ch == 'W') {
        shot = true;
        if (g_disint_tx == g_disint_px && g_disint_ty < g_disint_py) {
            g_disint_score += 10;
            snprintf(g_disint_status, sizeof(g_disint_status), "BOOM! Target vaporized!");
            g_disint_tx = (rand() % 9);
            g_disint_ty = (rand() % 2) ? 0 : 8;
        }
    } else if (ch == 's' || ch == 'S') {
        shot = true;
        if (g_disint_tx == g_disint_px && g_disint_ty > g_disint_py) {
            g_disint_score += 10;
            snprintf(g_disint_status, sizeof(g_disint_status), "BOOM! Target vaporized!");
            g_disint_tx = (rand() % 9);
            g_disint_ty = (rand() % 2) ? 0 : 8;
        }
    } else if (ch == 'a' || ch == 'A') {
        shot = true;
        if (g_disint_ty == g_disint_py && g_disint_tx < g_disint_px) {
            g_disint_score += 10;
            snprintf(g_disint_status, sizeof(g_disint_status), "BOOM! Target vaporized!");
            g_disint_tx = (rand() % 2) ? 0 : 8;
            g_disint_ty = (rand() % 9);
        }
    } else if (ch == 'd' || ch == 'D') {
        shot = true;
        if (g_disint_ty == g_disint_py && g_disint_tx > g_disint_px) {
            g_disint_score += 10;
            snprintf(g_disint_status, sizeof(g_disint_status), "BOOM! Target vaporized!");
            g_disint_tx = (rand() % 2) ? 0 : 8;
            g_disint_ty = (rand() % 9);
        }
    }

    if (shot && strcmp(g_disint_status, "BOOM! Target vaporized!") != 0) {
        snprintf(g_disint_status, sizeof(g_disint_status), "Missed! Target is at (%d, %d).", g_disint_tx, g_disint_ty);
    }

    // Move enemy closer to player
    if (rand() % 2) {
        if (g_disint_tx < g_disint_px) g_disint_tx++;
        else if (g_disint_tx > g_disint_px) g_disint_tx--;
        if (g_disint_ty < g_disint_py) g_disint_ty++;
        else if (g_disint_ty > g_disint_py) g_disint_ty--;
    }

    // Collision check
    if (g_disint_tx == g_disint_px && g_disint_ty == g_disint_py) {
        g_disint_lives--;
        if (g_disint_lives <= 0) {
            snprintf(g_disint_status, sizeof(g_disint_status), "CRASH! Game Over.");
        } else {
            snprintf(g_disint_status, sizeof(g_disint_status), "CRASH! Particle collision! Lost a life.");
            g_disint_tx = 0;
            g_disint_ty = (rand() % 9);
        }
    }

    redraw_disintegrator_screen();
}

static void init_fidgits(void) {
    g_fidgits_arr[0] = 'C';
    g_fidgits_arr[1] = 'A';
    g_fidgits_arr[2] = 'E';
    g_fidgits_arr[3] = 'B';
    g_fidgits_arr[4] = 'D';
    g_fidgits_cursor = 0;
    g_fidgits_selected = -1;
    g_fidgits_moves = 0;
    snprintf(g_fidgits_status, sizeof(g_fidgits_status), "Sort the Fidgits alphabetically. SPACE=Select/Swap.");
}

static void redraw_fidgits_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "============================================================\r\n"
             "                    FIDGITS (Ahoy! Issue 20)                \r\n"
             "============================================================\r\n"
             "  Moves: %d\r\n"
             "============================================================\r\n\r\n",
             g_fidgits_moves);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render the 5 Fidgits blocks
    lau_vram_write_string(g_vram, "         ", 9);
    for (int i = 0; i < 5; i++) {
        char block[32];
        if (i == g_fidgits_cursor) {
            snprintf(block, sizeof(block), (i == g_fidgits_selected) ? "[*%c*] " : "[ %c ] ", g_fidgits_arr[i]);
        } else {
            snprintf(block, sizeof(block), (i == g_fidgits_selected) ? "*%c*  " : " %c   ", g_fidgits_arr[i]);
        }
        lau_vram_write_string(g_vram, block, strlen(block));
    }
    lau_vram_write_string(g_vram, "\r\n\r\n============================================================\r\n", 66);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_fidgits_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [A] Left, [D] Right, [SPACE] Select/Swap, [ESC] Exit\r\n", 66);
}

static void handle_fidgits_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (ch == 'a' || ch == 'A') {
        if (g_fidgits_cursor > 0) g_fidgits_cursor--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_fidgits_cursor < 4) g_fidgits_cursor++;
    } else if (ch == ' ') {
        if (g_fidgits_selected == -1) {
            g_fidgits_selected = g_fidgits_cursor;
            snprintf(g_fidgits_status, sizeof(g_fidgits_status), "Selected Fidgit %c. Move cursor and press SPACE to swap.", g_fidgits_arr[g_fidgits_selected]);
        } else {
            int src = g_fidgits_selected;
            int tgt = g_fidgits_cursor;
            if (src != tgt) {
                char temp = g_fidgits_arr[src];
                g_fidgits_arr[src] = g_fidgits_arr[tgt];
                g_fidgits_arr[tgt] = temp;
                g_fidgits_moves++;
                snprintf(g_fidgits_status, sizeof(g_fidgits_status), "Swapped Fidgit %c with %c.", g_fidgits_arr[tgt], g_fidgits_arr[src]);
            }
            g_fidgits_selected = -1;

            // Check win condition
            if (g_fidgits_arr[0] == 'A' && g_fidgits_arr[1] == 'B' && g_fidgits_arr[2] == 'C' && g_fidgits_arr[3] == 'D' && g_fidgits_arr[4] == 'E') {
                snprintf(g_fidgits_status, sizeof(g_fidgits_status), "SUCCESS! Sorted all Fidgits in %d moves!", g_fidgits_moves);
            }
        }
    }

    redraw_fidgits_screen();
}

static void init_drum(void) {
    memset(g_drum_grid, 0, sizeof(g_drum_grid));
    // Pre-populate some classic patterns (e.g. Kick on 0 and 4, Snare on 2 and 6, Hi-hat on all even steps)
    g_drum_grid[0][0] = true;
    g_drum_grid[0][4] = true;
    g_drum_grid[1][2] = true;
    g_drum_grid[1][6] = true;
    g_drum_grid[2][0] = true;
    g_drum_grid[2][2] = true;
    g_drum_grid[2][4] = true;
    g_drum_grid[2][6] = true;

    g_drum_cursor_track = 0;
    g_drum_cursor_step = 0;
    g_drum_play_step = -1;
    g_drum_playing = false;
    g_drum_last_tick = 0;
    snprintf(g_drum_status, sizeof(g_drum_status), "Rhythmic Bits Loaded. [P] Play/Pause, [SPACE] Toggle Step, [ESC] Exit.");
}

static void redraw_drum_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[1024];
    snprintf(buf, sizeof(buf),
             "============================================================\r\n"
             "          RHYTHMIC BITS (Ahoy! Issue 23 Drum Sequencer)     \r\n"
             "============================================================\r\n"
             " INSTRUMENT |  0   1   2   3   4   5   6   7   | VISUALIZER\r\n"
             "============================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    const char* track_names[4] = { "KICK  (K)  ", "SNARE (S)  ", "H-HAT (H)  ", "TOM   (T)  " };

    for (int t = 0; t < 4; t++) {
        char line[256];
        int pos = 0;
        pos += snprintf(line + pos, sizeof(line) - pos, " %s |", track_names[t]);

        for (int s = 0; s < 8; s++) {
            bool current_play_pos = (g_drum_playing && g_drum_play_step == s);
            bool has_cursor = (g_drum_cursor_track == t && g_drum_cursor_step == s);

            char left_bracket = ' ';
            char right_bracket = ' ';

            if (current_play_pos) {
                left_bracket = '|';
                right_bracket = '|';
            } else if (has_cursor) {
                left_bracket = '[';
                right_bracket = ']';
            }

            char fill = g_drum_grid[t][s] ? 'X' : '.';
            pos += snprintf(line + pos, sizeof(line) - pos, "%c%c%c ", left_bracket, fill, right_bracket);
        }

        // Render mini sound visualizer column
        bool active_this_step = (g_drum_playing && g_drum_play_step >= 0 && g_drum_grid[t][g_drum_play_step]);
        if (active_this_step) {
            if (t == 0) pos += snprintf(line + pos, sizeof(line) - pos, "|  *BOOM*  ");
            else if (t == 1) pos += snprintf(line + pos, sizeof(line) - pos, "|  *CRACK* ");
            else if (t == 2) pos += snprintf(line + pos, sizeof(line) - pos, "|  *TICK*  ");
            else if (t == 3) pos += snprintf(line + pos, sizeof(line) - pos, "|  *THUD*  ");
        } else {
            pos += snprintf(line + pos, sizeof(line) - pos, "|          ");
        }

        pos += snprintf(line + pos, sizeof(line) - pos, "\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "============================================================\r\n", 62);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_drum_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [W/A/S/D] Navigate, [SPACE] Toggle Step, [C] Clear\r\n", 63);
    lau_vram_write_string(g_vram, "           [P] Play/Pause, [ESC] Exit\r\n", 39);
}

static void handle_drum_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_drum_playing = false;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (ch == 'w' || ch == 'W') {
        if (g_drum_cursor_track > 0) g_drum_cursor_track--;
    } else if (ch == 's' || ch == 'S') {
        if (g_drum_cursor_track < 3) g_drum_cursor_track++;
    } else if (ch == 'a' || ch == 'A') {
        if (g_drum_cursor_step > 0) g_drum_cursor_step--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_drum_cursor_step < 7) g_drum_cursor_step++;
    } else if (ch == ' ') {
        g_drum_grid[g_drum_cursor_track][g_drum_cursor_step] = !g_drum_grid[g_drum_cursor_track][g_drum_cursor_step];
    } else if (ch == 'c' || ch == 'C') {
        memset(g_drum_grid, 0, sizeof(g_drum_grid));
        snprintf(g_drum_status, sizeof(g_drum_status), "Cleared all sequence steps.");
    } else if (ch == 'p' || ch == 'P') {
        g_drum_playing = !g_drum_playing;
        if (g_drum_playing) {
            g_drum_play_step = 0;
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            g_drum_last_tick = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
            snprintf(g_drum_status, sizeof(g_drum_status), "Sequencer playing... [P] to pause.");
        } else {
            g_drum_play_step = -1;
            snprintf(g_drum_status, sizeof(g_drum_status), "Sequencer paused.");
        }
    }

    redraw_drum_screen();
}

static void update_drum_seq(uint32_t current_time) {
    if (!g_drum_playing) return;
    if (current_time - g_drum_last_tick >= 180) { // 180 ms per step
        g_drum_last_tick = current_time;
        g_drum_play_step = (g_drum_play_step + 1) % 8;
        redraw_drum_screen();
    }
}

static void init_jewel(void) {
    g_jewel_cx = 0; g_jewel_cy = 0;
    g_jewel_score = 0;
    // Populate grid
    const char gems[] = "ABCDE";
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 6; c++) {
            g_jewel_grid[r][c] = gems[rand() % 5];
        }
    }
    snprintf(g_jewel_status, sizeof(g_jewel_status), "Swap gems to align 3. [W/A/S/D] Move, [SPACE] Swap right.");
}

static void redraw_jewel_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================\r\n"
             "  JEWEL QUEST (Ahoy! Issue 24)      \r\n"
             "====================================\r\n"
             " Score: %d\r\n"
             "------------------------------------\r\n", g_jewel_score);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    for (int r = 0; r < 6; r++) {
        char line[128];
        int pos = 0;
        pos += snprintf(line + pos, sizeof(line) - pos, "   | ");
        for (int c = 0; c < 6; c++) {
            if (r == g_jewel_cy && c == g_jewel_cx) {
                pos += snprintf(line + pos, sizeof(line) - pos, "[%c] ", g_jewel_grid[r][c]);
            } else {
                pos += snprintf(line + pos, sizeof(line) - pos, " %c  ", g_jewel_grid[r][c]);
            }
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "|\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }
    lau_vram_write_string(g_vram, "====================================\r\n", 38);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_jewel_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [W/A/S/D] Move, [SPACE] Swap Right, [ESC] Exit\r\n", 58);
}

static void handle_jewel_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 'w' || ch == 'W') {
        if (g_jewel_cy > 0) g_jewel_cy--;
    } else if (ch == 's' || ch == 'S') {
        if (g_jewel_cy < 5) g_jewel_cy++;
    } else if (ch == 'a' || ch == 'A') {
        if (g_jewel_cx > 0) g_jewel_cx--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_jewel_cx < 5) g_jewel_cx++;
    } else if (ch == ' ') {
        if (g_jewel_cx < 5) {
            char tmp = g_jewel_grid[g_jewel_cy][g_jewel_cx];
            g_jewel_grid[g_jewel_cy][g_jewel_cx] = g_jewel_grid[g_jewel_cy][g_jewel_cx + 1];
            g_jewel_grid[g_jewel_cy][g_jewel_cx + 1] = tmp;
            
            // Basic check for 3 in a row
            bool matched = false;
            for (int r = 0; r < 6; r++) {
                for (int c = 0; c < 4; c++) {
                    if (g_jewel_grid[r][c] == g_jewel_grid[r][c+1] && g_jewel_grid[r][c] == g_jewel_grid[r][c+2]) {
                        g_jewel_score += 100;
                        const char gems[] = "ABCDE";
                        g_jewel_grid[r][c] = gems[rand() % 5];
                        g_jewel_grid[r][c+1] = gems[rand() % 5];
                        g_jewel_grid[r][c+2] = gems[rand() % 5];
                        matched = true;
                    }
                }
            }
            if (matched) {
                snprintf(g_jewel_status, sizeof(g_jewel_status), "MATCH! +100 Points!");
            } else {
                snprintf(g_jewel_status, sizeof(g_jewel_status), "Swapped gems.");
            }
        }
    }
    redraw_jewel_screen();
}

static void init_santa(void) {
    g_santa_x = 0;
    g_santa_dir = 1;
    g_present_y = -1;
    g_present_x = -1;
    g_santa_score = 0;
    g_chimney_x[0] = 5;
    g_chimney_x[1] = 15;
    g_chimney_x[2] = 25;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    g_santa_last_tick = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    snprintf(g_santa_status, sizeof(g_santa_status), "Drop presents on chimneys! [SPACE] Drop present.");
}

static void redraw_santa_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "   SANTA'S BUSY DAY (Ahoy! Issue 24 Holiday Game)   \r\n"
             "====================================================\r\n"
             " Score: %d\r\n"
             "----------------------------------------------------\r\n", g_santa_score);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Sky row with Santa
    char sky[64];
    memset(sky, ' ', 40);
    sky[40] = '\0';
    if (g_santa_x >= 0 && g_santa_x < 40) {
        sky[g_santa_x] = 'S'; // Santa
    }
    lau_vram_write_string(g_vram, " ", 1);
    lau_vram_write_string(g_vram, sky, strlen(sky));
    lau_vram_write_string(g_vram, "\r\n", 2);

    // Drop area rows
    for (int y = 0; y < 8; y++) {
        char row[64];
        memset(row, ' ', 40);
        row[40] = '\0';
        if (g_present_y == y && g_present_x >= 0 && g_present_x < 40) {
            row[g_present_x] = '*'; // Present
        }
        lau_vram_write_string(g_vram, " ", 1);
        lau_vram_write_string(g_vram, row, strlen(row));
        lau_vram_write_string(g_vram, "\r\n", 2);
    }

    // Houses and chimneys row
    char ground[64];
    memset(ground, '_', 40);
    ground[40] = '\0';
    for (int i = 0; i < 3; i++) {
        ground[g_chimney_x[i]] = 'H'; // Chimney
    }
    lau_vram_write_string(g_vram, " ", 1);
    lau_vram_write_string(g_vram, ground, strlen(ground));
    lau_vram_write_string(g_vram, "\r\n", 2);

    lau_vram_write_string(g_vram, "====================================================\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_santa_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [SPACE] Drop Present, [ESC] Exit\r\n", 45);
}

static void handle_santa_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == ' ') {
        if (g_present_y < 0) {
            g_present_x = g_santa_x;
            g_present_y = 0;
            snprintf(g_santa_status, sizeof(g_santa_status), "Present dropped!");
        }
    }
    redraw_santa_screen();
}

static void update_santa(uint32_t current_time) {
    if (current_time - g_santa_last_tick >= 100) {
        g_santa_last_tick = current_time;
        // Move Santa
        g_santa_x += g_santa_dir;
        if (g_santa_x >= 35) {
            g_santa_dir = -1;
        } else if (g_santa_x <= 0) {
            g_santa_dir = 1;
        }

        // Drop present
        if (g_present_y >= 0) {
            g_present_y++;
            if (g_present_y >= 8) {
                // Check chimney landing
                bool hit = false;
                for (int i = 0; i < 3; i++) {
                    if (g_present_x == g_chimney_x[i] || g_present_x == g_chimney_x[i] - 1 || g_present_x == g_chimney_x[i] + 1) {
                        hit = true;
                        break;
                    }
                }
                if (hit) {
                    g_santa_score += 150;
                    snprintf(g_santa_status, sizeof(g_santa_status), "DIRECT HIT! +150 Points!");
                } else {
                    snprintf(g_santa_status, sizeof(g_santa_status), "Missed the chimney!");
                }
                g_present_y = -1;
                g_present_x = -1;
            }
        }
        redraw_santa_screen();
    }
}

static void init_cloak(void) {
    g_cloak_room = 0;
    g_cloak_has_film = false;
    g_cloak_alarm = false;
    snprintf(g_cloak_status, sizeof(g_cloak_status), "You are inside the embassy. Avoid the guards. Find the microfilm.");
}

static void redraw_cloak_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "==================================================\r\n"
             "        CLOAK: Stealth Adventure (Ahoy! Issue 24) \r\n"
             "==================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    if (g_cloak_room == 0) {
        lau_vram_write_string(g_vram, " Room: EMBASSY ENTRANCE HALL\r\n Description: A quiet lobby with guards checking doors to the north.\r\n", 101);
    } else if (g_cloak_room == 1) {
        lau_vram_write_string(g_vram, " Room: SECURE ARCHIVE VAULT\r\n Description: Rows of safe deposit boxes. The microfilm drawer is here.\r\n", 104);
    } else if (g_cloak_room == 2) {
        lau_vram_write_string(g_vram, " Room: SECURITY CONTROL CENTRE\r\n Description: Server racks blinking with warning indicators.\r\n", 94);
    }

    lau_vram_write_string(g_vram, "--------------------------------------------------\r\n", 52);
    snprintf(buf, sizeof(buf), " Microfilm: %s | Alarm: %s\r\n", g_cloak_has_film ? "HELD" : "NONE", g_cloak_alarm ? "ACTIVE!" : "QUIET");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_cloak_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
