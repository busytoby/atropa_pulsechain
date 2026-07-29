            // Draw crosshairs
            if (g_darts_state == 0 && x == g_darts_aim_x) {
                line[x] = '|';
            } else if (g_darts_state == 1 && y == g_darts_aim_y) {
                line[x] = '-';
            }
        }
        
        // Show current throw pointer
        if (y == g_darts_aim_y && g_darts_state == 2) {
            line[g_darts_aim_x] = '*';
        }
        lau_vram_write_string(g_vram, line, strlen(line));
    }
}

static void handle_darts_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == ' ') {
        if (g_darts_state == 0) {
            g_darts_state = 1;
        } else if (g_darts_state == 1) {
            g_darts_state = 2;
            // Calculate throw result
            float dx = (g_darts_aim_x - 40) * 0.5f;
            float dy = (g_darts_aim_y - 7) * 1.0f;
            float dist = sqrtf(dx*dx + dy*dy);
            
            if (dist < 1.5f) {
                g_darts_score += 50; // Bullseye!
            } else if (dist < 4.0f) {
                g_darts_score += 25;
            } else if (dist < 8.0f) {
                g_darts_score += 10;
            }
            g_darts_throws--;
        } else if (g_darts_state == 2) {
            if (g_darts_throws <= 0) {
                init_darts();
            } else {
                g_darts_state = 0;
            }
        }
        redraw_darts_screen();
    }
}

static void update_darts(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 50) return;
    last_tick = ms;

    if (g_darts_state == 0) {
        g_darts_aim_x += g_darts_aim_dir * 2;
        if (g_darts_aim_x < 20 || g_darts_aim_x > 60) {
            g_darts_aim_dir = -g_darts_aim_dir;
        }
        redraw_darts_screen();
    } else if (g_darts_state == 1) {
        g_darts_aim_y += g_darts_aim_dir;
        if (g_darts_aim_y < 2 || g_darts_aim_y > 12) {
            g_darts_aim_dir = -g_darts_aim_dir;
        }
        redraw_darts_screen();
    }
}

// ----------------------------------------------------
// MAKEWAVE & PLOTWAVE Simulators
// ----------------------------------------------------
static int g_plotwave_offset = 0;
static bool g_plotwave_paused = false;

static void generate_active_wave(void) {
    float pi_val = 3.14159265f;
    for (int i = 0; i < 256; i++) {
        float t = (float)i / 256.0f;
        float phase = t * g_makewave_frequency;
        float val = 0.0f;
        switch (g_makewave_type) {
            case 0: // Sine
                val = sinf(2.0f * pi_val * phase);
                break;
            case 1: // Square
                val = ((phase - floorf(phase)) < g_makewave_duty) ? 1.0f : -1.0f;
                break;
            case 2: // Triangle
                val = 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
                break;
            case 3: // Sawtooth
                val = 2.0f * (phase - floorf(phase)) - 1.0f;
                break;
            case 4: // Noise
                val = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                break;
        }
        g_makewave_table[i] = val * g_makewave_amplitude;
    }
}

static void init_makewave(void) {
    g_makewave_type = 0;
    g_makewave_frequency = 4.0f;
    g_makewave_amplitude = 0.8f;
    g_makewave_duty = 0.5f;
    snprintf(g_makewave_status, sizeof(g_makewave_status), "Press [G] to generate, [ESC] to exit.");
    generate_active_wave();
}

static void redraw_makewave_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char hud[1024];
    snprintf(hud, sizeof(hud),
             "================================================================================\r\n"
             "        AHOY! MAKEWAVE | WAVEFORM GENERATOR UTILITY\r\n"
             "================================================================================\r\n"
             " [1] Waveform: %s\r\n"
             " [2/3] Frequency: %.1f Hz\r\n"
             " [4/5] Amplitude: %.2f\r\n"
             " [G] Generate Waveform Table\r\n"
             " [ESC] Exit\r\n"
             "--------------------------------------------------------------------------------\r\n"
             " Status: %s\r\n"
             "================================================================================\r\n",
             g_makewave_type == 0 ? "SINE" : (g_makewave_type == 1 ? "SQUARE" : (g_makewave_type == 2 ? "TRIANGLE" : (g_makewave_type == 3 ? "SAWTOOTH" : "NOISE"))),
             g_makewave_frequency, g_makewave_amplitude, g_makewave_status);
    lau_vram_write_string(g_vram, hud, strlen(hud));

    // Show a mini horizontal preview of the wave data
    char preview[128];
    strcpy(preview, "Preview: ");
    for (int i = 0; i < 40; i++) {
        int idx = (i * 256) / 40;
        float val = g_makewave_table[idx];
        if (val > 0.5f) strcat(preview, "^");
        else if (val < -0.5f) strcat(preview, "_");
        else strcat(preview, "-");
    }
    strcat(preview, "\r\nREADY.\r\n");
    lau_vram_write_string(g_vram, preview, strlen(preview));
}

static void handle_makewave_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        lau_vram_write_string(g_vram, "\r\nExited MAKEWAVE.\r\nREADY.\r\n", 28);
        return;
    }
    if (ch == '1') {
        g_makewave_type = (g_makewave_type + 1) % 5;
        snprintf(g_makewave_status, sizeof(g_makewave_status), "Selected %s waveform.", 
                 g_makewave_type == 0 ? "SINE" : (g_makewave_type == 1 ? "SQUARE" : (g_makewave_type == 2 ? "TRIANGLE" : (g_makewave_type == 3 ? "SAWTOOTH" : "NOISE"))));
    } else if (ch == '2') {
        g_makewave_frequency -= 0.5f;
        if (g_makewave_frequency < 0.5f) g_makewave_frequency = 0.5f;
    } else if (ch == '3') {
        g_makewave_frequency += 0.5f;
        if (g_makewave_frequency > 20.0f) g_makewave_frequency = 20.0f;
    } else if (ch == '4') {
        g_makewave_amplitude -= 0.1f;
        if (g_makewave_amplitude < 0.1f) g_makewave_amplitude = 0.1f;
    } else if (ch == '5') {
        g_makewave_amplitude += 0.1f;
        if (g_makewave_amplitude > 1.0f) g_makewave_amplitude = 1.0f;
    } else if (ch == 'g' || ch == 'G') {
        generate_active_wave();
        snprintf(g_makewave_status, sizeof(g_makewave_status), "Waveform table successfully written to RAM!");
    }
    redraw_makewave_screen();
}

static void init_plotwave(void) {
    g_plotwave_offset = 0;
    g_plotwave_paused = false;
}

static void redraw_plotwave_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char hud[512];
    snprintf(hud, sizeof(hud),
             "================================================================================\r\n"
             "        AHOY! PLOTWAVE | REAL-TIME OSCILLOSCOPE ANIMATOR\r\n"
             "================================================================================\r\n"
             " [SPACE] Pause/Play | [ESC] Exit\r\n"
             "--------------------------------------------------------------------------------\r\n");
    lau_vram_write_string(g_vram, hud, strlen(hud));

    // Render 15 lines of oscilloscope
    for (int r = 0; r < 15; r++) {
        char line[82];
        memset(line, ' ', 80);
        line[80] = '\r';
        line[81] = '\n';
        
        // Target amplitude for this row
        float row_amp = 1.0f - (2.0f * (float)r / 14.0f);

        for (int c = 0; c < 80; c++) {
            int table_idx = (c + g_plotwave_offset) % 256;
            float val = g_makewave_table[table_idx];
            // If the value is close to this row's target amplitude, draw a pixel
            if (fabsf(val - row_amp) < 0.08f) {
                line[c] = '*';
            }
            // Draw axis line in center
            if (r == 7 && line[c] == ' ') {
                line[c] = '-';
            }
        }
        lau_vram_write_string(g_vram, line, 82);
    }
    lau_vram_write_string(g_vram, "================================================================================\r\n", 82);
}

static void handle_plotwave_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        lau_vram_write_string(g_vram, "\r\nExited PLOTWAVE.\r\nREADY.\r\n", 28);
        return;
    }
    if (ch == ' ') {
        g_plotwave_paused = !g_plotwave_paused;
    }
    redraw_plotwave_screen();
}

static void update_plotwave(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 60) return;
    last_tick = ms;

    if (!g_plotwave_paused) {
        g_plotwave_offset = (g_plotwave_offset + 2) % 256;
        redraw_plotwave_screen();
    }
}

static void init_moxey(void) {
    g_moxey_room = 0;
    g_moxey_has_key = false;
    g_moxey_has_lantern = false;
    g_moxey_lantern_lit = false;
    g_moxey_chest_unlocked = false;
    snprintf(g_moxey_status, sizeof(g_moxey_status), "Welcome to Moxey's Porch! Type N/S/E/W to navigate, T to Take.");
}

static void redraw_moxey_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[1024];
    snprintf(buf, sizeof(buf),
             "============================================================\r\n"
             "                MOXEY'S PORCH (Ahoy! Issue 21)              \r\n"
             "============================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Room Description
    if (g_moxey_room == 0) {
        lau_vram_write_string(g_vram, 
             "Location: FRONT PORCH\r\n\r\n"
             "You stand on Moxey's ancient wooden porch. The wind whistles\r\n"
             "through the trees. A locked front door lies north.\r\n", 165);
        if (!g_moxey_has_key) {
            lau_vram_write_string(g_vram, "There is a brass [KEY] resting on the window sill.\r\n", 52);
        }
    } else if (g_moxey_room == 1) {
        lau_vram_write_string(g_vram,
             "Location: LIVING ROOM\r\n\r\n"
             "Inside the house, dust lies thick on the furniture.\r\n"
             "The exit lies south. A doorway to the kitchen lies north.\r\n", 163);
        if (!g_moxey_has_lantern) {
            lau_vram_write_string(g_vram, "An unlit [LANTERN] sits on the fireplace mantel.\r\n", 49);
        }
    } else if (g_moxey_room == 2) {
        lau_vram_write_string(g_vram,
             "Location: KITCHEN\r\n\r\n"
             "Old copper pots hang on the walls. A dark doorway leads\r\n"
             "south to the living room. Stairs lead DOWN to the basement.\r\n", 168);
    } else if (g_moxey_room == 3) {
        if (!g_moxey_lantern_lit) {
            lau_vram_write_string(g_vram,
                 "Location: BASEMENT\r\n\r\n"
                 "It is pitch black! You feel a freezing chill. Without a light\r\n"
                 "source, you cannot see anything and must go UP.\r\n", 161);
        } else {
            lau_vram_write_string(g_vram,
                 "Location: BASEMENT (ILLUMINATED)\r\n\r\n"
                 "Your lantern illuminates the damp stone basement. On a wooden\r\n"
                 "crate sits Moxey's mysterious heavy [CHEST]. Stairs lead UP.\r\n", 182);
        }
    }

    // Inventory status
    lau_vram_write_string(g_vram, "\r\n============================================================\r\n", 64);
    snprintf(buf, sizeof(buf), "Inventory: [ %s ] [ %s ]\r\n",
             g_moxey_has_key ? "Key" : "Empty",
             g_moxey_has_lantern ? (g_moxey_lantern_lit ? "Lit Lantern" : "Lantern") : "Empty");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    snprintf(buf, sizeof(buf), "Status: %s\r\n", g_moxey_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    lau_vram_write_string(g_vram, "============================================================\r\n", 62);
    lau_vram_write_string(g_vram, "Controls: [N/S] Go North/South, [U/D] Go Up/Down\r\n", 50);
    lau_vram_write_string(g_vram, "          [T] Take, [L] Light Lantern, [O] Open Chest, [ESC] Exit\r\n", 66);
}

static void handle_moxey_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (g_moxey_chest_unlocked) {
        snprintf(g_moxey_status, sizeof(g_moxey_status), "You've won! Press [ESC] to exit.");
        redraw_moxey_screen();
        return;
    }

    if (ch == 'n' || ch == 'N') {
        if (g_moxey_room == 0) {
            if (g_moxey_has_key) {
                g_moxey_room = 1;
                snprintf(g_moxey_status, sizeof(g_moxey_status), "You unlock the door and enter the Living Room.");
            } else {
                snprintf(g_moxey_status, sizeof(g_moxey_status), "The door is locked! You need a Key to open it.");
            }
        } else if (g_moxey_room == 1) {
            g_moxey_room = 2;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You walk into the Kitchen.");
        } else {
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You cannot go North from here.");
        }
    } else if (ch == 's' || ch == 'S') {
        if (g_moxey_room == 1) {
            g_moxey_room = 0;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You step back onto the Front Porch.");
        } else if (g_moxey_room == 2) {
            g_moxey_room = 1;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You step back into the Living Room.");
        } else {
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You cannot go South from here.");
        }
    } else if (ch == 'd' || ch == 'D') {
        if (g_moxey_room == 2) {
            g_moxey_room = 3;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You walk down the dark cellar stairs.");
        } else {
            snprintf(g_moxey_status, sizeof(g_moxey_status), "There are no stairs leading down here.");
        }
    } else if (ch == 'u' || ch == 'U') {
        if (g_moxey_room == 3) {
            g_moxey_room = 2;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You walk up the stairs back to the Kitchen.");
        } else {
            snprintf(g_moxey_status, sizeof(g_moxey_status), "There are no stairs leading up here.");
        }
    } else if (ch == 't' || ch == 'T') {
        if (g_moxey_room == 0 && !g_moxey_has_key) {
            g_moxey_has_key = true;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You picked up the brass Key!");
        } else if (g_moxey_room == 1 && !g_moxey_has_lantern) {
            g_moxey_has_lantern = true;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You picked up the iron Lantern!");
        } else {
            snprintf(g_moxey_status, sizeof(g_moxey_status), "Nothing to take here.");
        }
    } else if (ch == 'l' || ch == 'L') {
        if (g_moxey_has_lantern) {
            g_moxey_lantern_lit = true;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You light the Lantern. It shines brightly.");
        } else {
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You don't have a Lantern to light!");
        }
    } else if (ch == 'o' || ch == 'O') {
        if (g_moxey_room == 3 && g_moxey_lantern_lit) {
            g_moxey_chest_unlocked = true;
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You open Moxey's chest and discover his gold! YOU WIN!");
        } else {
            snprintf(g_moxey_status, sizeof(g_moxey_status), "You don't see any chest to open!");
        }
    }

    redraw_moxey_screen();
}

static void init_hoparound(void) {
    g_hoparound_x = 4;
    g_hoparound_y = 4;
    g_hoparound_score = 0;
    g_hoparound_moves = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            g_hoparound_grid[r][c] = 0;
        }
    }
    g_hoparound_grid[4][4] = 1; // Starting cell is visited
    snprintf(g_hoparound_status, sizeof(g_hoparound_status), "Chirp! Lead the Craw bird to gather all seeds.");
}

static void redraw_hoparound_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "============================================================\r\n"
             "                 HOP AROUND (Ahoy! Issue 16)                \r\n"
             "============================================================\r\n"
             "  Moves: %d   |   Score: %d / 630\r\n"
             "============================================================\r\n\r\n",
             g_hoparound_moves, g_hoparound_score);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    for (int r = 0; r < 8; r++) {
        lau_vram_write_string(g_vram, "       ", 7);
        for (int c = 0; c < 8; c++) {
            if (r == g_hoparound_y && c == g_hoparound_x) {
                // The main character is a Craw bird
                lau_vram_write_string(g_vram, ">v<", 3);
            } else if (g_hoparound_grid[r][c] == 1) {
                lau_vram_write_string(g_vram, " . ", 3);
            } else {
                lau_vram_write_string(g_vram, "[o]", 3);
            }
        }
        lau_vram_write_string(g_vram, "\r\n\r\n", 4);
    }

    lau_vram_write_string(g_vram, "============================================================\r\n", 62);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_hoparound_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [W/A/S/D] Hop around the board [ESC] Exit\r\n", 55);
}

static void handle_hoparound_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    int target_x = g_hoparound_x;
    int target_y = g_hoparound_y;
    bool moved = false;

    if (ch == 'w' || ch == 'W') {
        target_y--;
        moved = true;
    } else if (ch == 's' || ch == 'S') {
        target_y++;
        moved = true;
    } else if (ch == 'a' || ch == 'A') {
        target_x--;
        moved = true;
    } else if (ch == 'd' || ch == 'D') {
        target_x++;
        moved = true;
    }

    if (moved) {
        if (target_x >= 0 && target_x < 8 && target_y >= 0 && target_y < 8) {
            g_hoparound_x = target_x;
            g_hoparound_y = target_y;
            g_hoparound_moves++;

            if (g_hoparound_grid[target_y][target_x] == 0) {
                g_hoparound_grid[target_y][target_x] = 1;
                g_hoparound_score += 10;
                snprintf(g_hoparound_status, sizeof(g_hoparound_status), "Chirp! Craw gathered a seed (+10 pts).");
            } else {
                snprintf(g_hoparound_status, sizeof(g_hoparound_status), "Hop! Already visited this spot.");
            }

            // Check if all spots are visited
            bool win = true;
            for (int r = 0; r < 8; r++) {
                for (int c = 0; c < 8; c++) {
                    if (g_hoparound_grid[r][c] == 0) win = false;
                }
            }
            if (win) {
                snprintf(g_hoparound_status, sizeof(g_hoparound_status), "CONGRATULATIONS! Craw gathered all seeds!");
            }
        } else {
            snprintf(g_hoparound_status, sizeof(g_hoparound_status), "Ouch! Hit the fence!");
        }
    }

    redraw_hoparound_screen();
}

static void init_booter(void) {
    g_booter_cursor = 0;
    snprintf(g_booter_status, sizeof(g_booter_status), "Use W/S to navigate. Press ENTER to boot program.");
}

static void redraw_booter_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        BOOTER v1.0 (Ahoy! Issue 16 C64 Disk Menu Auto-Loader)      \r\n"
             "====================================================================\r\n"
             "  Please choose a program / utility to load and run:\r\n"
             "====================================================================\r\n\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    for (int i = 0; i < g_booter_count; i++) {
        char line[128];
        if (i == g_booter_cursor) {
            snprintf(line, sizeof(line), "   -->  [ %s ]  <--\r\n\r\n", g_booter_entries[i]);
        } else {
            snprintf(line, sizeof(line), "        ( %s )\r\n\r\n", g_booter_entries[i]);
        }
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_booter_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [W/S] Move Up/Down [ENTER] Boot Program [ESC] Shell\r\n", 65);
}

static void handle_booter_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (ch == 'w' || ch == 'W') {
        if (g_booter_cursor > 0) g_booter_cursor--;
    } else if (ch == 's' || ch == 'S') {
        if (g_booter_cursor < g_booter_count - 1) g_booter_cursor++;
    } else if (ch == '\n' || ch == '\r') {
        g_editor_mode = MODE_TERMINAL;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        switch (g_booter_cursor) {
            case 0: execute_command("ALICE"); break;
            case 1: execute_command("TOP"); break;
            case 2: execute_command("FONTASIA"); break;
            case 3: execute_command("FLANKSPEED"); break;
            case 4: execute_command("CHECKLIST"); break;
            case 5: execute_command("YULBUILD"); break;
            case 6: execute_command("HOPAROUND"); break;
            case 7: execute_command("TOWERS"); break;
            case 8: execute_command("DISINTEGRATOR"); break;
            case 9: execute_command("FIDGITS"); break;
            case 10: execute_command("MOXEY"); break;
            case 11: execute_command("DRUM"); break;
            case 12: execute_command("JEWEL"); break;
            case 13: execute_command("SANTA"); break;
            case 14: execute_command("CLOAK"); break;
            case 15: execute_command("GYPSY"); break;
        }
        return;
    }

    redraw_booter_screen();
}

static void init_flankspeed(void) {
    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 8; c++) {
            strcpy(g_flankspeed_buffer[r][c], "00");
        }
    }
    g_flankspeed_cursor_row = 0;
    g_flankspeed_cursor_col = 0;
    g_flankspeed_char_idx = 0;
    g_flankspeed_assembling = false;
    g_flankspeed_asm_buf[0] = '\0';
    g_flankspeed_asm_len = 0;
    snprintf(g_flankspeed_status, sizeof(g_flankspeed_status), "Flankspeed active. Base: $%04X. Type hex bytes.", g_flankspeed_start_addr);
}

static void redraw_flankspeed_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        FLANKSPEED (Ahoy! Issue 14 Machine Language Entry System)    \r\n"
             "====================================================================\r\n"
             "  ADDR  |  B0  B1  B2  B3  B4  B5  B6  B7  |  LINE CHECKSUM\r\n"
             "====================================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    for (int r = 0; r < 16; r++) {
        char line[256];
        int pos = 0;
        uint16_t addr = g_flankspeed_start_addr + r * 8;
        
        pos += snprintf(line + pos, sizeof(line) - pos, "  $%04X | ", addr);
        
        uint32_t checksum = 0;
        for (int c = 0; c < 8; c++) {
            uint32_t val = (uint32_t)strtoul(g_flankspeed_buffer[r][c], NULL, 16);
            checksum += val;

            if (r == g_flankspeed_cursor_row && c == g_flankspeed_cursor_col) {
                // Highlight cursor position
                if (g_flankspeed_char_idx == 0) {
                    pos += snprintf(line + pos, sizeof(line) - pos, "[%c]%c ", g_flankspeed_buffer[r][c][0], g_flankspeed_buffer[r][c][1]);
                } else {
                    pos += snprintf(line + pos, sizeof(line) - pos, "%c[%c] ", g_flankspeed_buffer[r][c][0], g_flankspeed_buffer[r][c][1]);
                }
            } else {
                pos += snprintf(line + pos, sizeof(line) - pos, " %s  ", g_flankspeed_buffer[r][c]);
            }
        }
        
        pos += snprintf(line + pos, sizeof(line) - pos, "|  $00%02X\r\n", checksum & 0xFF);
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_flankspeed_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    if (g_flankspeed_assembling) {
        snprintf(buf, sizeof(buf), "  Assemble 6502 instruction: %s_\r\n", g_flankspeed_asm_buf);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        lau_vram_write_string(g_vram, "  Controls: Type instruction (e.g. LDA #$01) [ENTER] Compile [ESC] Cancel\r\n", 74);
    } else {
        lau_vram_write_string(g_vram, "  Controls: [W/A/S/D] Move [0-9, A-F] Write hex [:] Solve/Assemble [ESC] Exit\r\n", 79);
    }
}

static bool flankspeed_assemble(const char *instr) {
    uint8_t bytes[16];
    int len = 0;
    
    char cmd[32] = {0};
    char arg[32] = {0};
    int n = sscanf(instr, "%31s %31s", cmd, arg);
    if (n <= 0) return false;
    
    // Integrated Symbol lookup helper aliases
    if (strcasecmp(arg, "START") == 0) {
        strcpy(arg, "$0300");
    } else if (strcasecmp(arg, "LOOP") == 0) {
        strcpy(arg, "$0308");
    } else if (strcasecmp(arg, "DATA") == 0) {
        strcpy(arg, "$0320");
    } else {
        // Dynamic Symbol Dictionary check
        for (int i = 0; i < g_flankspeed_symbol_count; i++) {
            if (strcasecmp(arg, g_flankspeed_symbols[i].name) == 0) {
                strncpy(arg, g_flankspeed_symbols[i].value, sizeof(arg) - 1);
                arg[sizeof(arg) - 1] = '\0';
                break;
            }
        }
    }
    
    if (strcasecmp(cmd, "NOP") == 0) {
        bytes[0] = 0xEA; len = 1;
    } else if (strcasecmp(cmd, "RTS") == 0) {
        bytes[0] = 0x60; len = 1;
    } else if (strcasecmp(cmd, "CLC") == 0) {
        bytes[0] = 0x18; len = 1;
    } else if (strcasecmp(cmd, "SEC") == 0) {
        bytes[0] = 0x38; len = 1;
    } else if (strcasecmp(cmd, "TAX") == 0) {
        bytes[0] = 0xAA; len = 1;
    } else if (strcasecmp(cmd, "TAY") == 0) {
        bytes[0] = 0xA8; len = 1;
    } else if (strcasecmp(cmd, "TXA") == 0) {
        bytes[0] = 0x8A; len = 1;
    } else if (strcasecmp(cmd, "TYA") == 0) {
        bytes[0] = 0x98; len = 1;
    } else if (strcasecmp(cmd, "PHA") == 0) {
        bytes[0] = 0x48; len = 1;
    } else if (strcasecmp(cmd, "PLA") == 0) {
        bytes[0] = 0x68; len = 1;
    } else if (strcasecmp(cmd, "LDA") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0xA9; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "LDX") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0xA2; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "LDY") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0xA0; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "ADC") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0x69; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "SBC") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0xE9; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "CMP") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0xC9; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "CPX") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0xE0; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "CPY") == 0 && arg[0] == '#' && arg[1] == '$') {
        unsigned int val = 0;
        sscanf(arg + 2, "%x", &val);
        bytes[0] = 0xC0; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "STA") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0x8D; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; len = 3;
    } else if (strcasecmp(cmd, "STX") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0x8E; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; len = 3;
    } else if (strcasecmp(cmd, "STY") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0x8C; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; len = 3;
    } else if (strcasecmp(cmd, "INC") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0xEE; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; len = 3;
    } else if (strcasecmp(cmd, "DEC") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0xCE; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; len = 3;
    } else if (strcasecmp(cmd, "JMP") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0x4C; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; len = 3;
    } else if (strcasecmp(cmd, "JSR") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0x20; bytes[1] = val & 0xFF; bytes[2] = (val >> 8) & 0xFF; len = 3;
    } else if (strcasecmp(cmd, "BNE") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0xD0; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "BEQ") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0xF0; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "BPL") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0x10; bytes[1] = val & 0xFF; len = 2;
    } else if (strcasecmp(cmd, "BMI") == 0 && arg[0] == '$') {
        unsigned int val = 0;
        sscanf(arg + 1, "%x", &val);
        bytes[0] = 0x30; bytes[1] = val & 0xFF; len = 2;
    }

    if (len > 0) {
        for (int i = 0; i < len; i++) {
            snprintf(g_flankspeed_buffer[g_flankspeed_cursor_row][g_flankspeed_cursor_col], 3, "%02X", bytes[i]);
            g_flankspeed_cursor_col++;
            if (g_flankspeed_cursor_col >= 8) {
                g_flankspeed_cursor_col = 0;
                g_flankspeed_cursor_row++;
                if (g_flankspeed_cursor_row >= 16) {
                    g_flankspeed_cursor_row = 15;
                    g_flankspeed_cursor_col = 7;
                }
            }
        }
        snprintf(g_flankspeed_status, sizeof(g_flankspeed_status), "Assembled: %s", instr);
        redraw_flankspeed_screen();
        return true;
    } else {
        snprintf(g_flankspeed_status, sizeof(g_flankspeed_status), "Unknown instruction format.");
        redraw_flankspeed_screen();
        return false;
    }
}

static void handle_flankspeed_input(char ch) {
    if (g_flankspeed_assembling) {
        if (ch == 27) { // ESC
            g_flankspeed_assembling = false;
            snprintf(g_flankspeed_status, sizeof(g_flankspeed_status), "Assembly mode cancelled.");
        } else if (ch == '\n' || ch == '\r') {
            flankspeed_assemble(g_flankspeed_asm_buf);
            g_flankspeed_assembling = false;
        } else if (ch == 127 || ch == '\b') {
            if (g_flankspeed_asm_len > 0) {
                g_flankspeed_asm_len--;
                g_flankspeed_asm_buf[g_flankspeed_asm_len] = '\0';
            }
        } else if (ch >= 32 && ch < 127) {
            if (g_flankspeed_asm_len < 63) {
                g_flankspeed_asm_buf[g_flankspeed_asm_len++] = ch;
                g_flankspeed_asm_buf[g_flankspeed_asm_len] = '\0';
            }
        }
        redraw_flankspeed_screen();
        return;
    }

    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    // Navigation
    if (ch == 'w' || ch == 'W') {
        if (g_flankspeed_cursor_row > 0) g_flankspeed_cursor_row--;
        g_flankspeed_char_idx = 0;
    } else if (ch == 's' || ch == 'S') {
        if (g_flankspeed_cursor_row < 15) g_flankspeed_cursor_row++;
        g_flankspeed_char_idx = 0;
    } else if (ch == 'a' || ch == 'A') {
        if (g_flankspeed_cursor_col > 0) {
            g_flankspeed_cursor_col--;
        } else if (g_flankspeed_cursor_row > 0) {
            g_flankspeed_cursor_row--;
            g_flankspeed_cursor_col = 7;
        }
        g_flankspeed_char_idx = 0;
    } else if (ch == 'd' || ch == 'D') {
        if (g_flankspeed_cursor_col < 7) {
            g_flankspeed_cursor_col++;
        } else if (g_flankspeed_cursor_row < 15) {
            g_flankspeed_cursor_row++;
            g_flankspeed_cursor_col = 0;
        }
        g_flankspeed_char_idx = 0;
    } else if (ch == 'p' || ch == 'P') {
        snprintf(g_flankspeed_status, sizeof(g_flankspeed_status), "BASIC Loader: 10 FOR I = %d TO %d: READ V: POKE I, V: NEXT",
                 g_flankspeed_start_addr, g_flankspeed_start_addr + 16 * 8 - 1);
    } else if (ch == ':') {
        g_flankspeed_assembling = true;
        g_flankspeed_asm_buf[0] = '\0';
        g_flankspeed_asm_len = 0;
        snprintf(g_flankspeed_status, sizeof(g_flankspeed_status), "Entering Assembly Solve Mode.");
    } else {
        // Hex character processing
        char uc = ch;
        if (uc >= 'a' && uc <= 'z') uc -= 32;
        if ((uc >= '0' && uc <= '9') || (uc >= 'A' && uc <= 'F')) {
            int r = g_flankspeed_cursor_row;
            int c = g_flankspeed_cursor_col;
            if (g_flankspeed_char_idx == 0) {
                g_flankspeed_buffer[r][c][0] = uc;
                g_flankspeed_char_idx = 1;
            } else {
                g_flankspeed_buffer[r][c][1] = uc;
                // Move cursor right
                if (g_flankspeed_cursor_col < 7) {
                    g_flankspeed_cursor_col++;
                } else if (g_flankspeed_cursor_row < 15) {
                    g_flankspeed_cursor_row++;
                    g_flankspeed_cursor_col = 0;
                }
                g_flankspeed_char_idx = 0;
            }
            snprintf(g_flankspeed_status, sizeof(g_flankspeed_status), "Updated address $%04X byte.", g_flankspeed_start_addr + r * 8 + c);
        }
    }

    redraw_flankspeed_screen();
}

static void init_fontasia(void) {
    memset(g_fontasia_grid, 0, sizeof(g_fontasia_grid));
    g_fontasia_cursor_x = 0;
    g_fontasia_cursor_y = 0;
    snprintf(g_fontasia_status, sizeof(g_fontasia_status), "Fontasia character designer initialized.");
}

static void redraw_fontasia_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        FONTASIA (Ahoy! Issue 14 Custom C64 Font Generator)         \r\n"
             "====================================================================\r\n"
             "  [GRID : 8 x 8 PIXELS]                     [PREVIEW / DATA VALUES]\r\n"
             "====================================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render 8 rows of grid and data
    uint8_t row_bytes[8] = {0};
    for (int y = 0; y < 8; y++) {
        char line[256];
        int pos = 0;
        
        // Grid display
        pos += snprintf(line + pos, sizeof(line) - pos, "   Row %d: |", y + 1);
        for (int x = 0; x < 8; x++) {
            char pixel = g_fontasia_grid[y][x] ? '#' : '.';
            if (x == g_fontasia_cursor_x && y == g_fontasia_cursor_y) {
                pos += snprintf(line + pos, sizeof(line) - pos, "[%c]", pixel);
            } else {
                pos += snprintf(line + pos, sizeof(line) - pos, " %c ", pixel);
            }
            // Accumulate row bits
            if (g_fontasia_grid[y][x]) {
                row_bytes[y] |= (1 << (7 - x));
            }
        }
        
        // Magnified preview and decimal byte printout
        pos += snprintf(line + pos, sizeof(line) - pos, "|  -->  Byte: %3d (Hex: $%02X)  [",
                        row_bytes[y], row_bytes[y]);
        
        for (int x = 0; x < 8; x++) {
            pos += snprintf(line + pos, sizeof(line) - pos, "%s", g_fontasia_grid[y][x] ? "[]" : "  ");
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "]\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_fontasia_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [W/A/S/D] Move [SPACE] Toggle [C] Clear [P] Export [ESC] Exit\r\n", 75);
}

static void handle_fontasia_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (ch == 'w' || ch == 'W') {
        if (g_fontasia_cursor_y > 0) g_fontasia_cursor_y--;
    } else if (ch == 's' || ch == 'S') {
        if (g_fontasia_cursor_y < 7) g_fontasia_cursor_y++;
    } else if (ch == 'a' || ch == 'A') {
        if (g_fontasia_cursor_x > 0) g_fontasia_cursor_x--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_fontasia_cursor_x < 7) g_fontasia_cursor_x++;
    } else if (ch == ' ') {
        g_fontasia_grid[g_fontasia_cursor_y][g_fontasia_cursor_x] = 1 - g_fontasia_grid[g_fontasia_cursor_y][g_fontasia_cursor_x];
        snprintf(g_fontasia_status, sizeof(g_fontasia_status), "Toggled pixel at (%d, %d).", g_fontasia_cursor_x + 1, g_fontasia_cursor_y + 1);
    } else if (ch == 'c' || ch == 'C') {
        memset(g_fontasia_grid, 0, sizeof(g_fontasia_grid));
        snprintf(g_fontasia_status, sizeof(g_fontasia_status), "Cleared character grid.");
    } else if (ch == 'p' || ch == 'P') {
        uint8_t row_bytes[8] = {0};
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (g_fontasia_grid[y][x]) {
                    row_bytes[y] |= (1 << (7 - x));
                }
            }
        }
        snprintf(g_fontasia_status, sizeof(g_fontasia_status),
                 "BASIC DATA: DATA %d,%d,%d,%d,%d,%d,%d,%d",
                 row_bytes[0], row_bytes[1], row_bytes[2], row_bytes[3],
                 row_bytes[4], row_bytes[5], row_bytes[6], row_bytes[7]);
    }

    redraw_fontasia_screen();
}

static void init_alice(void) {
    g_alice_room = 0;
    g_alice_has_key = 0;
    g_alice_door_unlocked = 0;
    g_alice_input_len = 0;
    g_alice_input_buf[0] = '\0';
    snprintf(g_alice_status, sizeof(g_alice_status), "Welcome to Alice in Adventureland! Type HELP for commands.");
}

static void redraw_alice_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "      ALICE IN ADVENTURELAND (Ahoy! Issue 13 Text Adventure)        \r\n"
             "====================================================================\r\n"
             " [ROOM : %s]           [INVENTORY : %s]\r\n"
             "====================================================================\r\n",
             g_alice_room == 0 ? "DEEP FOREST" : (g_alice_room == 1 ? "RABBIT HOLE" : "WONDERLAND HALL"),
             g_alice_has_key ? "GOLDEN KEY" : "EMPTY");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    if (g_alice_room == 0) {
        lau_vram_write_string(g_vram, " You are standing in a dense, magical forest. Tall ancient trees stretch\r\n", 76);
        lau_vram_write_string(g_vram, " high above. A narrow path leads NORTH deeper into the woods.\r\n\r\n", 65);
    } else if (g_alice_room == 1) {
        lau_vram_write_string(g_vram, " You are next to a giant oak tree. Below the roots lies a dark, gaping\r\n", 73);
        lau_vram_write_string(g_vram, " RABBIT HOLE. A path leads SOUTH back to the forest. You can GO DOWN.\r\n\r\n", 73);
    } else if (g_alice_room == 2) {
        lau_vram_write_string(g_vram, " You stand in a grand hallway of Wonderland. In front of you is a huge\r\n", 73);
        if (g_alice_door_unlocked) {
            lau_vram_write_string(g_vram, " GOLDEN DOOR (which is now UNLOCKED). A ladder goes UP to the surface.\r\n\r\n", 74);
        } else {
            lau_vram_write_string(g_vram, " locked GOLDEN DOOR. A ladder goes UP to the surface.\r\n", 55);
            if (!g_alice_has_key) {
                lau_vram_write_string(g_vram, " A tiny golden KEY lies on a small glass table in the corner.\r\n\r\n", 64);
            } else {
                lau_vram_write_string(g_vram, "\r\n", 2);
            }
        }
    }

    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_alice_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    snprintf(buf, sizeof(buf), " Command > %s_\r\n", g_alice_input_buf);
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void handle_alice_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (ch == '\n' || ch == '\r') {
        g_alice_input_buf[g_alice_input_len] = '\0';
        char cmd[64];
        // Clean trailing spaces / uppercase conversion
        int cp = 0;
        for (int i = 0; i < g_alice_input_len; i++) {
            char c = g_alice_input_buf[i];
            if (c >= 'a' && c <= 'z') c -= 32;
            if (cp < 63) cmd[cp++] = c;
        }
        cmd[cp] = '\0';

        // Command routing
        if (strcmp(cmd, "HELP") == 0) {
            snprintf(g_alice_status, sizeof(g_alice_status), "Commands: LOOK, GO NORTH, GO SOUTH, GO DOWN, GO UP, TAKE KEY, UNLOCK DOOR.");
        } else if (strcmp(cmd, "LOOK") == 0) {
            snprintf(g_alice_status, sizeof(g_alice_status), "You look around carefully.");
        } else if (strcmp(cmd, "GO NORTH") == 0 || strcmp(cmd, "NORTH") == 0) {
            if (g_alice_room == 0) {
                g_alice_room = 1;
                snprintf(g_alice_status, sizeof(g_alice_status), "You walked North to the Rabbit Hole tree.");
            } else {
                snprintf(g_alice_status, sizeof(g_alice_status), "You cannot go North from here.");
            }
        } else if (strcmp(cmd, "GO SOUTH") == 0 || strcmp(cmd, "SOUTH") == 0) {
            if (g_alice_room == 1) {
                g_alice_room = 0;
                snprintf(g_alice_status, sizeof(g_alice_status), "You walked South back to the forest.");
            } else {
                snprintf(g_alice_status, sizeof(g_alice_status), "You cannot go South from here.");
            }
        } else if (strcmp(cmd, "GO DOWN") == 0 || strcmp(cmd, "DOWN") == 0) {
            if (g_alice_room == 1) {
                g_alice_room = 2;
                snprintf(g_alice_status, sizeof(g_alice_status), "You fell down the Rabbit Hole into Wonderland Hall!");
            } else {
                snprintf(g_alice_status, sizeof(g_alice_status), "There is nothing to go down here.");
            }
        } else if (strcmp(cmd, "GO UP") == 0 || strcmp(cmd, "UP") == 0) {
            if (g_alice_room == 2) {
                g_alice_room = 1;
                snprintf(g_alice_status, sizeof(g_alice_status), "You climbed the ladder to the surface.");
            } else {
                snprintf(g_alice_status, sizeof(g_alice_status), "You cannot go Up from here.");
            }
        } else if (strcmp(cmd, "TAKE KEY") == 0 || strcmp(cmd, "GET KEY") == 0) {
            if (g_alice_room == 2 && !g_alice_has_key) {
                g_alice_has_key = 1;
                snprintf(g_alice_status, sizeof(g_alice_status), "You picked up the tiny Golden Key.");
            } else {
                snprintf(g_alice_status, sizeof(g_alice_status), "There is no key to take here.");
            }
        } else if (strcmp(cmd, "UNLOCK DOOR") == 0 || strcmp(cmd, "UNLOCK") == 0) {
            if (g_alice_room == 2) {
                if (g_alice_has_key) {
                    g_alice_door_unlocked = 1;
                    snprintf(g_alice_status, sizeof(g_alice_status), "YOU DID IT! The Golden Door clicks open. You escape! [ESC] to Exit.");
                } else {
                    snprintf(g_alice_status, sizeof(g_alice_status), "The door is locked solid. You need a key.");
                }
            } else {
                snprintf(g_alice_status, sizeof(g_alice_status), "There is no door to unlock here.");
            }
        } else {
            snprintf(g_alice_status, sizeof(g_alice_status), "Unknown command '%s'. Type HELP.", cmd);
        }

        g_alice_input_len = 0;
        g_alice_input_buf[0] = '\0';
    } else if (ch == '\b' || ch == 127) {
        if (g_alice_input_len > 0) {
            g_alice_input_len--;
            g_alice_input_buf[g_alice_input_len] = '\0';
        }
    } else if (ch >= 32 && ch < 127) {
        if (g_alice_input_len < 63) {
            g_alice_input_buf[g_alice_input_len++] = ch;
            g_alice_input_buf[g_alice_input_len] = '\0';
        }
    }

    redraw_alice_screen();
}

static void init_top(void) {
    g_top_player_x = 15;
    g_top_player_y = 9;
    g_top_height = 0;
    g_top_lives = 3;
    g_top_score = 0;
    g_top_rock_x = 20;
    g_top_rock_y = 1;
    snprintf(g_top_status, sizeof(g_top_status), "Climb to the top! Use A/D to move, W to jump/climb.");
}

static void update_top_simulation(void) {
    // Move rock leftwards or downwards
    g_top_rock_x--;
    if (g_top_rock_x <= 0) {
        g_top_rock_x = 29;
        g_top_rock_y = 1 + (rand() % 8);
    }

    // Check collisions
    if (g_top_player_x == g_top_rock_x && g_top_player_y == g_top_rock_y) {
        g_top_lives--;
        g_top_rock_x = 29;
        if (g_top_lives <= 0) {
            snprintf(g_top_status, sizeof(g_top_status), "GAME OVER! Final Score: %d. Press ESC to exit.", g_top_score);
        } else {
            snprintf(g_top_status, sizeof(g_top_status), "Ouch! Hit by a rock. Lives left: %d", g_top_lives);
        }
    }
}

static void redraw_top_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "            TO THE TOP (Ahoy! Issue 13 Climbing Game)              \r\n"
             "====================================================================\r\n"
             "  [SCORE : %d]            [HEIGHT : %d m]          [LIVES : %d]\r\n"
             "====================================================================\r\n",
             g_top_score, g_top_height, g_top_lives);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render Tower (10 lines x 30 characters wide)
    // Row 1 to 9 represent climbing rows
    for (int y = 0; y < 10; y++) {
        char line[128];
        int pos = 0;
        pos += snprintf(line + pos, sizeof(line) - pos, "      |");
        for (int x = 0; x < 30; x++) {
            char ch = ' ';
            // Platforms every odd row
            if (y % 2 == 1) {
                ch = '=';
            }
            // Ladders at specific columns
            if (x == 15 || x == 5 || x == 25) {
                ch = '|';
            }
            // Draw player
            if (x == g_top_player_x && y == g_top_player_y) {
                ch = '*'; // Player character
            }
            // Draw rock
            if (x == g_top_rock_x && y == g_top_rock_y) {
                ch = 'o'; // Rolling rock
            }
            line[pos++] = ch;
        }
        pos += snprintf(line + pos, sizeof(line) - pos, "|\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_top_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void handle_top_input(char ch) {
    if (ch == 27) { // ESC -> Exit
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (g_top_lives <= 0) return;

    if (ch == 'a' || ch == 'A') {
        if (g_top_player_x > 0) g_top_player_x--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_top_player_x < 29) g_top_player_x++;
    } else if (ch == 'w' || ch == 'W') {
        // Jump/Climb platform
        if (g_top_player_y > 0) {
            g_top_player_y--;
            g_top_height += 10;
            g_top_score += 50;
            if (g_top_player_y == 0) { // Reached the top!
                g_top_player_y = 9; // Reset to bottom for next stage
                g_top_score += 500;
                snprintf(g_top_status, sizeof(g_top_status), "STAGE COMPLETED! Level Reset.");
            }
        }
    }

    update_top_simulation();
    redraw_top_screen();
}

static void init_magpie(void) {
    g_magpie_count = 0;
    g_magpie_graph_mode = 0;
    g_magpie_query[0] = '\0';
    
    // Seed initial records
    strncpy(g_magpie_db[0].name, "Alice", 31);
    strncpy(g_magpie_db[0].dept, "Engineering", 31);
    g_magpie_db[0].salary = 8500.0;
    
    strncpy(g_magpie_db[1].name, "Bob", 31);
    strncpy(g_magpie_db[1].dept, "Sales", 31);
    g_magpie_db[1].salary = 6200.0;
    
    strncpy(g_magpie_db[2].name, "Charlie", 31);
    strncpy(g_magpie_db[2].dept, "Marketing", 31);
    g_magpie_db[2].salary = 5400.0;
    
    strncpy(g_magpie_db[3].name, "Dave", 31);
    strncpy(g_magpie_db[3].dept, "Engineering", 31);
    g_magpie_db[3].salary = 7900.0;
    
    g_magpie_count = 4;
    snprintf(g_magpie_status, sizeof(g_magpie_status), "Magpie database initialized with %d records.", g_magpie_count);
}

static void redraw_magpie_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        AUDIOGENIC: MICRO MAGPIE CARTRIDGE DATABASE MANAGER         \r\n"
             "====================================================================\r\n"
             "  [RECORDS : %d / 10]              [FILTER QUERY : %s]\r\n"
             "  [VIEW MODE : %s]           [SYS DEVICE : C64 DRIVE #8]\r\n"
             "====================================================================\r\n",
             g_magpie_count,
             g_magpie_query[0] ? g_magpie_query : "ALL RECORDS",
             g_magpie_graph_mode ? "GRAPH (BAR CHART)" : "TABLE RECORD LIST");
    lau_vram_write_string(g_vram, buf, strlen(buf));
