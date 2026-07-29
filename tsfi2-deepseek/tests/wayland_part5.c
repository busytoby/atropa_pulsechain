
    if (g_magpie_graph_mode == 0) {
        // Table list mode
        lau_vram_write_string(g_vram, "   ID | EMPLOYEE NAME     | DEPARTMENT       | MONTHLY SALARY  \r\n", 65);
        lau_vram_write_string(g_vram, "   ---|-------------------|------------------|-----------------\r\n", 65);
        for (int i = 0; i < g_magpie_count; i++) {
            // Apply query filter if set
            if (g_magpie_query[0] != '\0') {
                if (strcasecmp(g_magpie_db[i].dept, g_magpie_query) != 0 &&
                    strcasecmp(g_magpie_db[i].name, g_magpie_query) != 0) {
                    continue;
                }
            }
            snprintf(buf, sizeof(buf), "   %2d | %-17.31s | %-16.31s | $%.2f\r\n",
                     i + 1, g_magpie_db[i].name, g_magpie_db[i].dept, g_magpie_db[i].salary);
            lau_vram_write_string(g_vram, buf, strlen(buf));
        }
    } else {
        // Horizontal bar chart salary mode
        lau_vram_write_string(g_vram, "   EMPLOYEE NAME     | SALARY BAR CHART (1 block = $1000)\r\n", 58);
        lau_vram_write_string(g_vram, "   ------------------|---------------------------------------------\r\n", 67);
        for (int i = 0; i < g_magpie_count; i++) {
            char bar[32] = "";
            int blocks = (int)(g_magpie_db[i].salary / 1000.0);
            if (blocks > 30) blocks = 30;
            for (int b = 0; b < blocks; b++) {
                bar[b] = '#';
            }
            bar[blocks] = '\0';
            snprintf(buf, sizeof(buf), "   %-17.31s | %-30.31s ($%.2f)\r\n",
                     g_magpie_db[i].name, bar, g_magpie_db[i].salary);
            lau_vram_write_string(g_vram, buf, strlen(buf));
        }
    }

    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_magpie_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [A] Add Record [D] Delete Last [S] Search Dept [G] Graph [ESC] Exit\r\n", 81);
}

static void handle_magpie_input(char ch) {
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
        if (g_magpie_count < 10) {
            static const char *names[4] = { "Eve", "Frank", "Grace", "Heidi" };
            static const char *depts[2] = { "Engineering", "Marketing" };
            int idx = g_magpie_count;
            snprintf(g_magpie_db[idx].name, 32, "%s", names[idx % 4]);
            snprintf(g_magpie_db[idx].dept, 32, "%s", depts[idx % 2]);
            g_magpie_db[idx].salary = 4000.0 + (idx * 500.0);
            g_magpie_count++;
            snprintf(g_magpie_status, sizeof(g_magpie_status), "Added employee %s to %s.", g_magpie_db[idx].name, g_magpie_db[idx].dept);
        } else {
            snprintf(g_magpie_status, sizeof(g_magpie_status), "Database full! Maximum 10 records allowed.");
        }
    } else if (ch == 'd' || ch == 'D') {
        if (g_magpie_count > 0) {
            g_magpie_count--;
            snprintf(g_magpie_status, sizeof(g_magpie_status), "Deleted last database record.");
        } else {
            snprintf(g_magpie_status, sizeof(g_magpie_status), "Database is already empty.");
        }
    } else if (ch == 's' || ch == 'S') {
        // Toggle/Cycle query search filter: All -> Engineering -> Sales -> All
        if (g_magpie_query[0] == '\0') {
            strncpy(g_magpie_query, "Engineering", sizeof(g_magpie_query) - 1);
            snprintf(g_magpie_status, sizeof(g_magpie_status), "Filter query: 'Engineering'.");
        } else if (strcmp(g_magpie_query, "Engineering") == 0) {
            strncpy(g_magpie_query, "Sales", sizeof(g_magpie_query) - 1);
            snprintf(g_magpie_status, sizeof(g_magpie_status), "Filter query: 'Sales'.");
        } else {
            g_magpie_query[0] = '\0';
            snprintf(g_magpie_status, sizeof(g_magpie_status), "Search filter cleared.");
        }
    } else if (ch == 'g' || ch == 'G') {
        g_magpie_graph_mode = 1 - g_magpie_graph_mode;
        snprintf(g_magpie_status, sizeof(g_magpie_status), "Switched view mode.");
    }
    redraw_magpie_screen();
}

static void init_studio64(void) {
    g_s64_cursor_step = 0;
    g_s64_cursor_pitch = 4;
    for (int i = 0; i < 8; i++) {
        g_s64_sequence[i] = -1;
    }
    strncpy(g_s64_status, "Ready to compose. Use keyboard to enter notes.", sizeof(g_s64_status) - 1);
}

static void redraw_studio64_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        ENTECH SOFTWARE: STUDIO 64 MUSIC WORD PROCESSOR              \r\n"
             "====================================================================\r\n"
             "  [STEP : %d / 8]                 [PITCH : %s]\r\n"
             "  [COMPOSITION WORKSPACE: C64 SID MONOPHONIC ENVELOPE GENERATION]\r\n"
             "====================================================================\r\n",
             g_s64_cursor_step + 1,
             g_s64_cursor_pitch == 0 ? "C4" :
             g_s64_cursor_pitch == 1 ? "D4" :
             g_s64_cursor_pitch == 2 ? "E4" :
             g_s64_cursor_pitch == 3 ? "F4" :
             g_s64_cursor_pitch == 4 ? "G4" :
             g_s64_cursor_pitch == 5 ? "A4" :
             g_s64_cursor_pitch == 6 ? "B4" : "C5");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    const char *pitches[8] = { "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5" };

    // Render staff from top (C5) to bottom (C4)
    for (int p = 7; p >= 0; p--) {
        char line_buf[128];
        int pos = 0;
        pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "  %s |", pitches[p]);
        for (int s = 0; s < 8; s++) {
            int is_cursor = (g_s64_cursor_step == s && g_s64_cursor_pitch == p);
            int is_note = (g_s64_sequence[s] == p);

            if (is_cursor && is_note) {
                line_buf[pos++] = '[';
                line_buf[pos++] = 'X';
                line_buf[pos++] = ']';
            } else if (is_cursor) {
                line_buf[pos++] = '[';
                line_buf[pos++] = '_';
                line_buf[pos++] = ']';
            } else if (is_note) {
                line_buf[pos++] = ' ';
                line_buf[pos++] = 'O';
                line_buf[pos++] = ' ';
            } else {
                line_buf[pos++] = ' ';
                line_buf[pos++] = '-';
                line_buf[pos++] = ' ';
            }
            line_buf[pos++] = '|';
        }
        line_buf[pos++] = '\r';
        line_buf[pos++] = '\n';
        line_buf[pos] = '\0';
        lau_vram_write_string(g_vram, line_buf, strlen(line_buf));
    }

    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_s64_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [W/S] Change Pitch [A/D] Change Step [SPACE] Toggle note [P] Play [ESC] Exit\r\n", 90);
}

static void handle_studio64_input(char ch) {
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
        if (g_s64_cursor_step > 0) g_s64_cursor_step--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_s64_cursor_step < 7) g_s64_cursor_step++;
    } else if (ch == 'w' || ch == 'W') {
        if (g_s64_cursor_pitch < 7) g_s64_cursor_pitch++;
    } else if (ch == 's' || ch == 'S') {
        if (g_s64_cursor_pitch > 0) g_s64_cursor_pitch--;
    } else if (ch == ' ') {
        if (g_s64_sequence[g_s64_cursor_step] == g_s64_cursor_pitch) {
            g_s64_sequence[g_s64_cursor_step] = -1; // Clear note
            strncpy(g_s64_status, "Note cleared at step.", sizeof(g_s64_status) - 1);
        } else {
            g_s64_sequence[g_s64_cursor_step] = g_s64_cursor_pitch; // Set note
            strncpy(g_s64_status, "Note placed on staff.", sizeof(g_s64_status) - 1);
        }
    } else if (ch == 'p' || ch == 'P') {
        // Build playback status string listing register values
        char play_buf[256] = "PLAYBACK SID: ";
        int pos = strlen(play_buf);
        int notes_played = 0;
        for (int i = 0; i < 8; i++) {
            if (g_s64_sequence[i] != -1) {
                int freq = 261 + (g_s64_sequence[i] * 30); // Approximate C4 to C5 Hz
                uint32_t val = (uint32_t)(freq * 16.40277);
                uint8_t hi = (val >> 8) & 0xFF;
                uint8_t lo = val & 0xFF;
                pos += snprintf(play_buf + pos, sizeof(play_buf) - pos, "S%d(F:%d/H:%02X,L:%02X) ", i + 1, freq, hi, lo);
                notes_played++;
            }
        }
        if (notes_played == 0) {
            snprintf(g_s64_status, sizeof(g_s64_status), "Nothing to play! Sequence is empty.");
        } else {
            snprintf(g_s64_status, sizeof(g_s64_status), "%s", play_buf);
            // Fire short system sound effect beep
            printf("\x07");
            fflush(stdout);
        }
    }
    redraw_studio64_screen();
}

static void init_construction_co(void) {
    g_cc_crane_x = 5.0;
    g_cc_crane_y = 1.0;
    g_cc_velocity_x = 0.0;
    g_cc_velocity_y = 0.0;
    g_cc_has_cargo = 0;
    g_cc_score = 0;
    g_cc_fuel = 100.0;
    strncpy(g_cc_status, "Drone flight initialized. Load cargo blocks.", sizeof(g_cc_status) - 1);
    
    // Clear and build setup
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            g_cc_grid[y][x] = 0;
        }
    }
    // Set targets for construction scaffolding at the bottom rows
    g_cc_grid[9][3] = 2; // Target 1
    g_cc_grid[9][4] = 2; // Target 2
    g_cc_grid[9][5] = 2; // Target 3
    g_cc_grid[9][6] = 2; // Target 4
    g_cc_grid[8][4] = 2; // Target 5
    g_cc_grid[8][5] = 2; // Target 6
}

static void redraw_construction_co_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        AHOY! CONSTRUCTION CO: CARGO DRONE FLIGHT SIMULATOR          \r\n"
             "====================================================================\r\n"
             "   [POSITION  : X:%3.1f Y:%3.1f]     [VELOCITY  : VX:%4.2f VY:%4.2f]\r\n"
             "   [CARGO LOAD: %s]             [DRONE FUEL: %5.1f %%]\r\n"
             "   [SCAFFOLDS : %d / 6 BUILT]        [BUILD SCORE: %d PTS]\r\n"
             "====================================================================\r\n",
             g_cc_crane_x, g_cc_crane_y, g_cc_velocity_x, g_cc_velocity_y,
             g_cc_has_cargo ? "HEAVY BLOCK" : "EMPTY      ", g_cc_fuel, g_cc_score, g_cc_score * 100);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render 10x10 Building Grid
    for (int y = 0; y < 10; y++) {
        char grid_line[64];
        int pos = 0;
        grid_line[pos++] = ' ';
        grid_line[pos++] = ' ';
        grid_line[pos++] = '|';
        for (int x = 0; x < 10; x++) {
            int cx = (int)(g_cc_crane_x + 0.5);
            int cy = (int)(g_cc_crane_y + 0.5);
            
            if (y == cy && x == cx) {
                grid_line[pos++] = ' ';
                grid_line[pos++] = g_cc_has_cargo ? 'H' : 'Y'; // Drone representation
                grid_line[pos++] = ' ';
            } else {
                int cell = g_cc_grid[y][x];
                if (cell == 1) {
                    grid_line[pos++] = '[';
                    grid_line[pos++] = '#'; // Built scaffold
                    grid_line[pos++] = ']';
                } else if (cell == 2) {
                    grid_line[pos++] = ' ';
                    grid_line[pos++] = '?'; // Targeted scaffolding spot
                    grid_line[pos++] = ' ';
                } else if (y == 2 && x == 1) {
                    grid_line[pos++] = '(';
                    grid_line[pos++] = 'L'; // Loading Pad
                    grid_line[pos++] = ')';
                } else {
                    grid_line[pos++] = ' ';
                    grid_line[pos++] = '.';
                    grid_line[pos++] = ' ';
                }
            }
        }
        grid_line[pos++] = '|';
        grid_line[pos++] = '\r';
        grid_line[pos++] = '\n';
        grid_line[pos] = '\0';
        lau_vram_write_string(g_vram, grid_line, strlen(grid_line));
    }
    
    lau_vram_write_string(g_vram, "====================================================================\r\n", 70);
    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_cc_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [W/S] Vertical Lift [A/D] Horizontal Move [SPACE] Grab/Drop [ESC] Exit\r\n", 84);
}

static void handle_construction_co_input(char ch) {
    if (ch == 27) { // ESC -> Exit to terminal mode
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    if (ch == 'w' || ch == 'W') {
        g_cc_velocity_y -= 0.6; // Thrust up (grid goes 0 to 9 down, so negative is up!)
        strncpy(g_cc_status, "Drone vertical engine burn.", sizeof(g_cc_status) - 1);
    } else if (ch == 's' || ch == 'S') {
        g_cc_velocity_y += 0.6; // Thrust down
        strncpy(g_cc_status, "Descending thrusters activated.", sizeof(g_cc_status) - 1);
    } else if (ch == 'a' || ch == 'A') {
        g_cc_velocity_x -= 0.6; // Thrust left
        strncpy(g_cc_status, "Left steering exhaust active.", sizeof(g_cc_status) - 1);
    } else if (ch == 'd' || ch == 'D') {
        g_cc_velocity_x += 0.6; // Thrust right
        strncpy(g_cc_status, "Right steering exhaust active.", sizeof(g_cc_status) - 1);
    } else if (ch == ' ') {
        int cx = (int)(g_cc_crane_x + 0.5);
        int cy = (int)(g_cc_crane_y + 0.5);
        
        if (g_cc_has_cargo) {
            // Drop cargo
            if (cy >= 0 && cy < 10 && cx >= 0 && cx < 10) {
                if (g_cc_grid[cy][cx] == 2) {
                    g_cc_grid[cy][cx] = 1; // Scaffold successfully built
                    g_cc_score++;
                    g_cc_has_cargo = 0;
                    strncpy(g_cc_status, "SUCCESS: Scaffold block dropped at target coordinate!", sizeof(g_cc_status) - 1);
                } else {
                    strncpy(g_cc_status, "WARNING: Block dropped in wrong location! Lost cargo.", sizeof(g_cc_status) - 1);
                    g_cc_has_cargo = 0;
                }
            }
        } else {
            // Grab cargo at Loading Pad (y:2, x:1)
            if (cy == 2 && cx == 1) {
                g_cc_has_cargo = 1;
                strncpy(g_cc_status, "CARGO LOADED: Heavy block attached to flight harness.", sizeof(g_cc_status) - 1);
            } else {
                strncpy(g_cc_status, "ERROR: No cargo blocks found here. Fly to Loading Pad (L).", sizeof(g_cc_status) - 1);
            }
        }
    }
    redraw_construction_co_screen();
}

static void update_construction_co_simulation(void) {
    if (g_cc_fuel > 0.0) {
        // Thrust consumes fuel
        double consumption = 0.01;
        if (g_cc_has_cargo) consumption *= 1.8; // Carrying cargo burns more fuel!
        g_cc_fuel -= consumption;
        if (g_cc_fuel < 0.0) g_cc_fuel = 0.0;
    } else {
        g_cc_velocity_x = 0.0;
        g_cc_velocity_y = 0.0;
        strncpy(g_cc_status, "WARNING: OUT OF FUEL! Cargo drone grounded.", sizeof(g_cc_status) - 1);
    }

    // Apply gravity
    g_cc_velocity_y += 0.08; // Gravity drift downwards

    // Carry mass increases momentum
    double drag = g_cc_has_cargo ? 0.95 : 0.9;
    g_cc_velocity_x *= drag;
    g_cc_velocity_y *= drag;

    g_cc_crane_x += g_cc_velocity_x;
    g_cc_crane_y += g_cc_velocity_y;

    // Boundary constraints
    if (g_cc_crane_x < 0.0) { g_cc_crane_x = 0.0; g_cc_velocity_x = 0.0; }
    if (g_cc_crane_x > 9.0) { g_cc_crane_x = 9.0; g_cc_velocity_x = 0.0; }
    if (g_cc_crane_y < 0.0) { g_cc_crane_y = 0.0; g_cc_velocity_y = 0.0; }
    if (g_cc_crane_y > 9.0) {
        g_cc_crane_y = 9.0;
        g_cc_velocity_y = 0.0;
        if (g_cc_velocity_y > 1.5) {
            strncpy(g_cc_status, "CRASH: Hard landing! Hull integrity damaged.", sizeof(g_cc_status) - 1);
        }
    }
}

static void init_spacepatrol(void) {
    g_sp_alt = 10000.0;
    g_sp_speed = 350.0;
    g_sp_pitch = 0.0;
    g_sp_yaw = 0.0;
    g_sp_throttle = 60;
    g_sp_fuel = 95.5;
    g_sp_shields = 100.0;
    g_sp_target_dist = 5000.0;
    strncpy(g_sp_status, "Orbit entry stabilized. Patrol route active.", sizeof(g_sp_status) - 1);
}

static void redraw_spacepatrol_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[2048];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        AHOY! SPACE PATROL: FLIGHT DYNAMICS PILOT DECK              \r\n"
             "====================================================================\r\n"
             "   [ALTITUDE : %8.1f M]            [AIRSPEED : %5.1f MPS]\r\n"
             "   [PITCH    : %8.2f DEG]          [YAW      : %5.2f DEG]\r\n"
             "   [THROTTLE : %8d %%]            [FUEL     : %5.1f GAL]\r\n"
             "   [SHIELDS  : %8.1f %%]            [TARGET   : %5.1f M]\r\n"
             "====================================================================\r\n",
             g_sp_alt, g_sp_speed, g_sp_pitch, g_sp_yaw, g_sp_throttle, g_sp_fuel, g_sp_shields, g_sp_target_dist);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Draw simple ascii wireframe cockpit view
    lau_vram_write_string(g_vram, 
         "  +--------------------------------------------------------------+\r\n"
         "  |   *                .               *             .        *  |\r\n"
         "  |         .                 *               *           .      |\r\n", 217);

    // Draw dynamic crosshair based on pitch and yaw offsets
    char cockpit_row[128];
    int cursor_col = 32 + (int)(g_sp_yaw * 1.5);
    if (cursor_col < 4) cursor_col = 4;
    if (cursor_col > 60) cursor_col = 60;

    int cursor_row = 3 + (int)(g_sp_pitch * 0.5);
    if (cursor_row < 1) cursor_row = 1;
    if (cursor_row > 5) cursor_row = 5;

    for (int r = 1; r <= 5; r++) {
        int pos = 0;
        cockpit_row[pos++] = ' ';
        cockpit_row[pos++] = ' ';
        cockpit_row[pos++] = '|';
        for (int c = 1; c <= 62; c++) {
            if (r == cursor_row && c == cursor_col) {
                cockpit_row[pos++] = '+'; // crosshair center
            } else if (r == cursor_row && (c == cursor_col - 1 || c == cursor_col + 1)) {
                cockpit_row[pos++] = '-';
            } else if (c == cursor_col && (r == cursor_row - 1 || r == cursor_row + 1)) {
                cockpit_row[pos++] = '|';
            } else if ((r + c) % 17 == 0) {
                cockpit_row[pos++] = '*'; // dynamic starfield drift
            } else {
                cockpit_row[pos++] = ' ';
            }
        }
        cockpit_row[pos++] = '|';
        cockpit_row[pos++] = '\r';
        cockpit_row[pos++] = '\n';
        cockpit_row[pos] = '\0';
        lau_vram_write_string(g_vram, cockpit_row, strlen(cockpit_row));
    }

    lau_vram_write_string(g_vram, 
         "  |     *           .                  .              *      .   |\r\n"
         "  +-----\\____________________[HUD COCKPIT]____________________/--+\r\n", 151);

    snprintf(buf, sizeof(buf), "  Status: %s\r\n", g_sp_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, "  Controls: [I/K] Pitch [J/L] Yaw [W/S] Throttle [SPACE] Fire blasters [ESC] Exit\r\n", 84);
}

static void handle_spacepatrol_input(char ch) {
    if (ch == 27) { // ESC -> Exit to terminal mode
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }

    // Input handlers
    if (ch == 'w' || ch == 'W') {
        if (g_sp_throttle < 100) g_sp_throttle += 5;
        strncpy(g_sp_status, "Thrusters burn increased.", sizeof(g_sp_status) - 1);
    } else if (ch == 's' || ch == 'S') {
        if (g_sp_throttle > 0) g_sp_throttle -= 5;
        strncpy(g_sp_status, "Thrusters burn decreased.", sizeof(g_sp_status) - 1);
    } else if (ch == 'i' || ch == 'I') {
        g_sp_pitch += 1.0;
        if (g_sp_pitch > 15.0) g_sp_pitch = 15.0;
        strncpy(g_sp_status, "Pitching up.", sizeof(g_sp_status) - 1);
    } else if (ch == 'k' || ch == 'K') {
        g_sp_pitch -= 1.0;
        if (g_sp_pitch < -15.0) g_sp_pitch = -15.0;
        strncpy(g_sp_status, "Pitching down.", sizeof(g_sp_status) - 1);
    } else if (ch == 'j' || ch == 'J') {
        g_sp_yaw -= 2.0;
        if (g_sp_yaw < -20.0) g_sp_yaw = -20.0;
        strncpy(g_sp_status, "Yawing left.", sizeof(g_sp_status) - 1);
    } else if (ch == 'l' || ch == 'L') {
        g_sp_yaw += 2.0;
        if (g_sp_yaw > 20.0) g_sp_yaw = 20.0;
        strncpy(g_sp_status, "Yawing right.", sizeof(g_sp_status) - 1);
    } else if (ch == ' ') {
        strncpy(g_sp_status, "BLASTERS FIRED! Plasma bursts traveling to target.", sizeof(g_sp_status) - 1);
        if (g_sp_target_dist > 100.0) {
            g_sp_target_dist -= 250.0;
            if (g_sp_target_dist < 0.0) g_sp_target_dist = 0.0;
        }
    }
    redraw_spacepatrol_screen();
}

static void update_spacepatrol_simulation(void) {
    // Dynamic updates
    if (g_sp_fuel > 0.0) {
        g_sp_fuel -= (g_sp_throttle * 0.002);
        if (g_sp_fuel < 0.0) g_sp_fuel = 0.0;
    } else {
        g_sp_throttle = 0;
        strncpy(g_sp_status, "WARNING: FUEL DEPLETED! Free drift mode active.", sizeof(g_sp_status) - 1);
    }

    // Altitude depends on pitch and speed
    g_sp_alt += (g_sp_speed * sin(g_sp_pitch * M_PI / 180.0) * 0.05);
    if (g_sp_alt < 0.0) {
        g_sp_alt = 0.0;
        g_sp_speed = 0.0;
        g_sp_shields = 0.0;
        strncpy(g_sp_status, "CRITICAL: CRASHED ON PLANETARY SURFACE!", sizeof(g_sp_status) - 1);
    }

    // Speed depends on throttle and pitching gravity drag
    double target_speed = g_sp_throttle * 6.0;
    g_sp_speed += (target_speed - g_sp_speed) * 0.1;

    // Slowly reduce target distance if heading towards it
    if (g_sp_speed > 0.0 && g_sp_target_dist > 0.0) {
        g_sp_target_dist -= (g_sp_speed * 0.05);
        if (g_sp_target_dist <= 0.0) {
            g_sp_target_dist = 8000.0; // spawn next patrol marker
            strncpy(g_sp_status, "Target marker reached! Locating next waypoint...", sizeof(g_sp_status) - 1);
        }
    }
}

static void init_pte(void) {
    g_pte_line_count = 1;
    g_pte_cursor_x = g_pte_left_margin;
    g_pte_cursor_y = 0;
    g_pte_edit_margin_mode = 0;
    for (int i = 0; i < PTE_MAX_LINES; i++) {
        memset(g_pte_lines[i], ' ', PTE_LINE_WIDTH);
        g_pte_lines[i][PTE_LINE_WIDTH - 1] = '\0';
    }
    strncpy(g_pte_status_msg, "Welcome to PTE! [ESC] Exit [Ctrl+M] Margins [Ctrl+P] Save", sizeof(g_pte_status_msg) - 1);
}

static void redraw_pte_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);

    char buf[512];
    snprintf(buf, sizeof(buf),
             "====================================================================\r\n"
             "        PTE WORD PROCESSOR (Ahoy! Issue 11 Layout Engine)            \r\n"
             "  Left Margin: %d  |  Right Margin: %d  | Mode: %s\r\n"
             "====================================================================\r\n\r\n",
             g_pte_left_margin, g_pte_right_margin, 
             g_pte_edit_margin_mode == 1 ? "EDIT LEFT MARGIN" : (g_pte_edit_margin_mode == 2 ? "EDIT RIGHT MARGIN" : "TEXT ENTRY"));
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Print text lines showing margin offsets
    for (int y = 0; y < 15; y++) {
        char display_line[256];
        int pos = 0;
        
        // Render left margin spacing
        for (int m = 0; m < g_pte_left_margin; m++) {
            display_line[pos++] = (y == g_pte_cursor_y && m == g_pte_cursor_x) ? '_' : '.';
        }
        
        // Copy text characters between margins
        for (int x = g_pte_left_margin; x <= g_pte_right_margin; x++) {
            char ch = g_pte_lines[y][x];
            if (y == g_pte_cursor_y && x == g_pte_cursor_x) {
                display_line[pos++] = '_'; // Cursor marker
            } else {
                display_line[pos++] = ch;
            }
        }
        
        // Render right margin padding
        display_line[pos++] = '|';
        display_line[pos++] = '\r';
        display_line[pos++] = '\n';
        display_line[pos] = '\0';
        lau_vram_write_string(g_vram, display_line, strlen(display_line));
    }

    snprintf(buf, sizeof(buf), "\r\nStatus: %s\r\n", g_pte_status_msg);
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void handle_pte_input(char ch) {
    if (ch == 27) { // ESC -> Exit to terminal mode
        g_editor_mode = MODE_TERMINAL;
        g_vram->cursor_x = 0;
        g_vram->cursor_y = 0;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "Returned to terminal shell.\r\n", 29);
        return;
    }
    
    // Toggle Margins Edit Mode (Ctrl+M is character code 13 - wait, Enter is 13 too. Let's use 'M' or control key check)
    // In many terminals, Ctrl+M is received as 13. To prevent clash with Return/Enter, let's use 'm' for toggle when in margin modes.
    if (g_pte_edit_margin_mode > 0) {
        if (ch >= '0' && ch <= '9') {
            int val = ch - '0';
            if (g_pte_edit_margin_mode == 1) {
                g_pte_left_margin = val * 5;
                if (g_pte_left_margin >= g_pte_right_margin) g_pte_left_margin = g_pte_right_margin - 5;
                g_pte_cursor_x = g_pte_left_margin;
            } else {
                g_pte_right_margin = 40 + val * 5;
                if (g_pte_right_margin >= PTE_LINE_WIDTH - 2) g_pte_right_margin = PTE_LINE_WIDTH - 3;
                if (g_pte_right_margin <= g_pte_left_margin) g_pte_right_margin = g_pte_left_margin + 5;
            }
            g_pte_edit_margin_mode = 0;
            strncpy(g_pte_status_msg, "Margin updated.", sizeof(g_pte_status_msg) - 1);
        }
        redraw_pte_screen();
        return;
    }

    if (ch == 13) { // Return -> newline
        if (g_pte_cursor_y < PTE_MAX_LINES - 1) {
            g_pte_cursor_y++;
            g_pte_cursor_x = g_pte_left_margin;
            if (g_pte_cursor_y >= g_pte_line_count) {
                g_pte_line_count = g_pte_cursor_y + 1;
            }
        }
    } else if (ch == 127 || ch == 8) { // Backspace
        if (g_pte_cursor_x > g_pte_left_margin) {
            g_pte_cursor_x--;
            g_pte_lines[g_pte_cursor_y][g_pte_cursor_x] = ' ';
        } else if (g_pte_cursor_y > 0) {
            g_pte_cursor_y--;
            g_pte_cursor_x = g_pte_right_margin;
        }
    } else if (ch == 16) { // Ctrl+P -> Save text buffer
        FILE *f = fopen("pte_document.txt", "w");
        if (f) {
            for (int y = 0; y < g_pte_line_count; y++) {
                // Trim trailing space
                int end = g_pte_right_margin;
                while (end >= g_pte_left_margin && g_pte_lines[y][end] == ' ') end--;
                for (int x = g_pte_left_margin; x <= end; x++) {
                    fputc(g_pte_lines[y][x], f);
                }
                fputc('\n', f);
            }
            fclose(f);
            strncpy(g_pte_status_msg, "File pte_document.txt saved successfully.", sizeof(g_pte_status_msg) - 1);
        } else {
            strncpy(g_pte_status_msg, "Error writing to file.", sizeof(g_pte_status_msg) - 1);
        }
    } else if (ch == 12) { // Ctrl+L -> Change margins
        // Let's use Ctrl+L to trigger margins config to avoid conflict with Return (13)
        g_pte_edit_margin_mode = 1;
        strncpy(g_pte_status_msg, "Press [1-9] to set Left Margin multiplier.", sizeof(g_pte_status_msg) - 1);
    } else if (ch == 18) { // Ctrl+R -> Right margin config
        g_pte_edit_margin_mode = 2;
        strncpy(g_pte_status_msg, "Press [1-9] to set Right Margin multiplier.", sizeof(g_pte_status_msg) - 1);
    } else if (ch >= 32 && ch < 127) { // Text insertion
        if (g_pte_cursor_x <= g_pte_right_margin) {
            g_pte_lines[g_pte_cursor_y][g_pte_cursor_x] = ch;
            g_pte_cursor_x++;
            
            // Auto wrap boundary check (Ahoy! Layout rule)
            if (g_pte_cursor_x > g_pte_right_margin) {
                if (g_pte_cursor_y < PTE_MAX_LINES - 1) {
                    g_pte_cursor_y++;
                    g_pte_cursor_x = g_pte_left_margin;
                    if (g_pte_cursor_y >= g_pte_line_count) {
                        g_pte_line_count = g_pte_cursor_y + 1;
                    }
                }
            }
        }
    }
    redraw_pte_screen();
}

static void init_checklist(void) {
    g_checklist_count = 0;
    g_checklist_cursor = 0;
    g_checklist_entry_mode = 0;
    g_checklist_entry_len = 0;
    g_checklist_entry_buf[0] = '\0';
    strncpy(g_checklist_status_msg, "Welcome to Checklist! [a] Add [Space] Toggle [d] Delete [s] Save [l] Load", sizeof(g_checklist_status_msg) - 1);
    
    g_checklist_items[0].checked = false;
    strcpy(g_checklist_items[0].category, "General");
    strcpy(g_checklist_items[0].text, "Learn C64 Machine Language");
    
    g_checklist_items[1].checked = true;
    strcpy(g_checklist_items[1].category, "General");
    strcpy(g_checklist_items[1].text, "Read Ahoy! Magazine Issue 7");
    
    g_checklist_items[2].checked = false;
    strcpy(g_checklist_items[2].category, "Work");
    strcpy(g_checklist_items[2].text, "Build Checklist database system");
    
    g_checklist_count = 3;
}

static void redraw_checklist_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "==============================================================\r\n"
        "      CHECKLIST (Ahoy! Issue 7 / Bob Spirko Database)         \r\n"
        "==============================================================\r\n\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    if (g_checklist_count == 0) {
        lau_vram_write_string(g_vram, "  <No items in checklist. Press 'a' to add a new item.>\r\n\r\n", 58);
    } else {
        for (int i = 0; i < g_checklist_count; i++) {
            const char *cur = (i == g_checklist_cursor) ? " -> " : "    ";
            const char *box = g_checklist_items[i].checked ? "[\x1b[32mX\x1b[0m]" : "[\x1b[31m \x1b[0m]";
            
            if (i == g_checklist_cursor) {
                snprintf(buf, sizeof(buf), "\x1b[33m%s%s <%s> %s\x1b[0m\r\n", 
                         cur, box, g_checklist_items[i].category, g_checklist_items[i].text);
            } else {
                snprintf(buf, sizeof(buf), "%s%s <%s> %s\r\n", 
                         cur, box, g_checklist_items[i].category, g_checklist_items[i].text);
            }
            lau_vram_write_string(g_vram, buf, strlen(buf));
        }
        lau_vram_write_string(g_vram, "\r\n", 2);
    }
    
    lau_vram_write_string(g_vram, "==============================================================\r\n", 64);
    
    if (g_checklist_entry_mode == 1) {
        snprintf(buf, sizeof(buf), "  [NEW ITEM DESCRIPTION]: %s_\r\n", g_checklist_entry_buf);
        lau_vram_write_string(g_vram, buf, strlen(buf));
    } else if (g_checklist_entry_mode == 2) {
        snprintf(buf, sizeof(buf), "  [NEW ITEM CATEGORY]:    %s_\r\n", g_checklist_entry_buf);
        lau_vram_write_string(g_vram, buf, strlen(buf));
    } else if (g_checklist_entry_mode == 3) {
        snprintf(buf, sizeof(buf), "  [EDIT DESCRIPTION]:     %s_\r\n", g_checklist_entry_buf);
        lau_vram_write_string(g_vram, buf, strlen(buf));
    } else if (g_checklist_entry_mode == 4) {
        snprintf(buf, sizeof(buf), "  [EDIT CATEGORY]:        %s_\r\n", g_checklist_entry_buf);
        lau_vram_write_string(g_vram, buf, strlen(buf));
    } else {
        snprintf(buf, sizeof(buf), "  Status: \x1b[1;32m%s\x1b[0m\r\n", g_checklist_status_msg);
        lau_vram_write_string(g_vram, buf, strlen(buf));
    }
    
    lau_vram_write_string(g_vram, "==============================================================\r\n", 64);
    if (g_checklist_entry_mode != 0) {
        lau_vram_write_string(g_vram, " [Type text and press ENTER | ESC to cancel]\r\n", 45);
    } else {
        lau_vram_write_string(g_vram, " [UP/DOWN or W/S: Navigate | SPACE: Toggle | A: Add | E: Edit ]\r\n"
                                      " [D: Delete | C: Clear | S: Save | L: Load | ESC: Return to Terminal]\r\n", 137);
    }
}

static void handle_checklist_input(char ch) {
    if (g_checklist_entry_mode != 0) {
        if (ch == '\x1b') {
            g_checklist_entry_mode = 0;
            strncpy(g_checklist_status_msg, "Action cancelled.", sizeof(g_checklist_status_msg) - 1);
            redraw_checklist_screen();
            return;
        }
        
        if (ch == '\n' || ch == '\r') {
            if (g_checklist_entry_mode == 1) {
                if (g_checklist_count < 30) {
                    snprintf(g_checklist_items[g_checklist_count].text, sizeof(g_checklist_items[g_checklist_count].text), "%s", g_checklist_entry_buf);
                    g_checklist_items[g_checklist_count].checked = false;
                }
                g_checklist_entry_mode = 2;
                g_checklist_entry_buf[0] = '\0';
                g_checklist_entry_len = 0;
            } else if (g_checklist_entry_mode == 2) {
                if (g_checklist_count < 30) {
                    if (g_checklist_entry_len == 0) {
                        strcpy(g_checklist_items[g_checklist_count].category, "General");
                    } else {
                        snprintf(g_checklist_items[g_checklist_count].category, sizeof(g_checklist_items[g_checklist_count].category), "%s", g_checklist_entry_buf);
                    }
                    g_checklist_count++;
                    g_checklist_cursor = g_checklist_count - 1;
                    strncpy(g_checklist_status_msg, "Item added.", sizeof(g_checklist_status_msg) - 1);
                }
                g_checklist_entry_mode = 0;
            } else if (g_checklist_entry_mode == 3) {
                if (g_checklist_cursor < g_checklist_count) {
                    snprintf(g_checklist_items[g_checklist_cursor].text, sizeof(g_checklist_items[g_checklist_cursor].text), "%s", g_checklist_entry_buf);
                    
                    snprintf(g_checklist_entry_buf, sizeof(g_checklist_entry_buf), "%s", g_checklist_items[g_checklist_cursor].category);
                    g_checklist_entry_len = strlen(g_checklist_entry_buf);
                    g_checklist_entry_mode = 4;
                } else {
                    g_checklist_entry_mode = 0;
                }
            } else if (g_checklist_entry_mode == 4) {
                if (g_checklist_cursor < g_checklist_count) {
                    if (g_checklist_entry_len == 0) {
                        strcpy(g_checklist_items[g_checklist_cursor].category, "General");
                    } else {
                        snprintf(g_checklist_items[g_checklist_cursor].category, sizeof(g_checklist_items[g_checklist_cursor].category), "%s", g_checklist_entry_buf);
                    }
                    strncpy(g_checklist_status_msg, "Item updated.", sizeof(g_checklist_status_msg) - 1);
                }
                g_checklist_entry_mode = 0;
            }
            redraw_checklist_screen();
            return;
        }
        
        if (ch == '\b' || ch == 127) {
            if (g_checklist_entry_len > 0) {
                g_checklist_entry_len--;
                g_checklist_entry_buf[g_checklist_entry_len] = '\0';
            }
            redraw_checklist_screen();
            return;
        }
        
        if (ch >= 32 && ch < 127 && g_checklist_entry_len < 60) {
            g_checklist_entry_buf[g_checklist_entry_len++] = ch;
            g_checklist_entry_buf[g_checklist_entry_len] = '\0';
            redraw_checklist_screen();
        }
        return;
    }
    
    if (ch == '\x1b') {
        g_checklist_active = false;
        g_editor_mode = MODE_TERMINAL;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "CIS: CHECKLIST database exited.\r\n", 33);
        return;
    }
    
    if (ch == 'w' || ch == 'W') {
        if (g_checklist_cursor > 0) {
            g_checklist_cursor--;
        }
        redraw_checklist_screen();
    } else if (ch == 's' || ch == 'S') {
        if (g_checklist_cursor < g_checklist_count - 1) {
            g_checklist_cursor++;
        }
        redraw_checklist_screen();
    } else if (ch == ' ') {
        if (g_checklist_cursor >= 0 && g_checklist_cursor < g_checklist_count) {
            g_checklist_items[g_checklist_cursor].checked = !g_checklist_items[g_checklist_cursor].checked;
        }
        redraw_checklist_screen();
    } else if (ch == 'a' || ch == 'A') {
        if (g_checklist_count >= 30) {
            strncpy(g_checklist_status_msg, "Checklist full (max 30 items).", sizeof(g_checklist_status_msg) - 1);
        } else {
            g_checklist_entry_mode = 1;
            g_checklist_entry_len = 0;
            g_checklist_entry_buf[0] = '\0';
        }
        redraw_checklist_screen();
    } else if (ch == 'e' || ch == 'E') {
        if (g_checklist_cursor >= 0 && g_checklist_cursor < g_checklist_count) {
            g_checklist_entry_mode = 3;
            strncpy(g_checklist_entry_buf, g_checklist_items[g_checklist_cursor].text, sizeof(g_checklist_entry_buf) - 1);
            g_checklist_entry_buf[sizeof(g_checklist_entry_buf) - 1] = '\0';
            g_checklist_entry_len = strlen(g_checklist_entry_buf);
        } else {
            strncpy(g_checklist_status_msg, "No item selected.", sizeof(g_checklist_status_msg) - 1);
        }
        redraw_checklist_screen();
    } else if (ch == 'd' || ch == 'D') {
        if (g_checklist_cursor >= 0 && g_checklist_cursor < g_checklist_count) {
            for (int i = g_checklist_cursor; i < g_checklist_count - 1; i++) {
                g_checklist_items[i] = g_checklist_items[i + 1];
            }
            g_checklist_count--;
            if (g_checklist_cursor >= g_checklist_count && g_checklist_cursor > 0) {
                g_checklist_cursor = g_checklist_count - 1;
            }
            strncpy(g_checklist_status_msg, "Item deleted.", sizeof(g_checklist_status_msg) - 1);
        }
        redraw_checklist_screen();
    } else if (ch == 'c' || ch == 'C') {
        g_checklist_count = 0;
        g_checklist_cursor = 0;
        strncpy(g_checklist_status_msg, "Checklist cleared.", sizeof(g_checklist_status_msg) - 1);
        redraw_checklist_screen();
    } else if (ch == 's' || ch == 'S') {
        save_checklist();
        redraw_checklist_screen();
    } else if (ch == 'l' || ch == 'L') {
        load_checklist();
        redraw_checklist_screen();
    }
}

static void save_job_tree(void) {
    FILE *f = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/job_tree.txt", "w");
    if (!f) return;
    for (int i = 0; i < g_job_nodes_count; i++) {
        fprintf(f, "%d|%d|%s\n", g_job_nodes[i].yes_child, g_job_nodes[i].no_child, g_job_nodes[i].text);
    }
    fclose(f);
}

static void load_job_tree(void) {
    FILE *f = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/job_tree.txt", "r");
    if (!f) {
        g_job_nodes[0].yes_child = 1;
        g_job_nodes[0].no_child = 2;
        strcpy(g_job_nodes[0].text, "Do you work indoors?");
        
        g_job_nodes[1].yes_child = -1;
        g_job_nodes[1].no_child = -1;
        strcpy(g_job_nodes[1].text, "Doctor");
        
        g_job_nodes[2].yes_child = -1;
        g_job_nodes[2].no_child = -1;
        strcpy(g_job_nodes[2].text, "Farmer");
        
        g_job_nodes_count = 3;
        return;
    }
    g_job_nodes_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), f) && g_job_nodes_count < 50) {
        line[strcspn(line, "\r\n")] = '\0';
        char *bar1 = strchr(line, '|');
        if (!bar1) continue;
        *bar1 = '\0';
        char *bar2 = strchr(bar1 + 1, '|');
        if (!bar2) continue;
        *bar2 = '\0';
        
        g_job_nodes[g_job_nodes_count].yes_child = atoi(line);
        g_job_nodes[g_job_nodes_count].no_child = atoi(bar1 + 1);
        snprintf(g_job_nodes[g_job_nodes_count].text, sizeof(g_job_nodes[g_job_nodes_count].text), "%s", bar2 + 1);
        g_job_nodes_count++;
    }
    fclose(f);
}

static void init_job_game(void) {
    load_job_tree();
    g_job_current_node = 0;
    g_job_prev_node = -1;
    g_job_last_answer_was_yes = false;
    g_job_submode = 0;
    g_job_new_name[0] = '\0';
    g_job_new_question[0] = '\0';
}

static void redraw_job_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "==============================================================\r\n"
        "      WHAT'S MY JOB? (Ahoy! Issue 7 / B.W. Behling Guessing)  \r\n"
        "==============================================================\r\n\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    
    if (g_job_submode == 0) {
        snprintf(buf, sizeof(buf), "  Question: \x1b[1;36m%s\x1b[0m\r\n\r\n", g_job_nodes[g_job_current_node].text);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        lau_vram_write_string(g_vram, "  [Press Y for YES | N for NO]\r\n", 30);
    } else if (g_job_submode == 1) {
        snprintf(buf, sizeof(buf), "  My guess is: \x1b[1;33m%s\x1b[0m!\r\n\r\n", g_job_nodes[g_job_current_node].text);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        lau_vram_write_string(g_vram, "  Is this correct? [Press Y for YES | N for NO]\r\n", 48);
    } else if (g_job_submode == 2) {
        snprintf(buf, sizeof(buf), "  I give up! What is the job? \r\n");
        lau_vram_write_string(g_vram, buf, strlen(buf));
        snprintf(buf, sizeof(buf), "  > %s_\r\n", g_job_new_name);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        lau_vram_write_string(g_vram, "\r\n  [Type job name and press ENTER]\r\n", 37);
    } else if (g_job_submode == 3) {
        snprintf(buf, sizeof(buf), "  Help me learn! Type a question that distinguishes a\r\n"
                                   "  \x1b[1;32m%s\x1b[0m from a \x1b[1;31m%s\x1b[0m:\r\n",
                                   g_job_new_name, g_job_nodes[g_job_current_node].text);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        snprintf(buf, sizeof(buf), "  > %s_\r\n", g_job_new_question);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        lau_vram_write_string(g_vram, "\r\n  [Type question and press ENTER]\r\n", 37);
    } else if (g_job_submode == 4) {
        snprintf(buf, sizeof(buf), "  For a \x1b[1;32m%s\x1b[0m, what is the answer to:\r\n"
                                   "  \"%s\"?\r\n\r\n",
                                   g_job_new_name, g_job_new_question);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        lau_vram_write_string(g_vram, "  [Press Y for YES | N for NO]\r\n", 30);
    }
    
    lau_vram_write_string(g_vram, "\r\n==============================================================\r\n", 66);
    lau_vram_write_string(g_vram, " [ESC: Exit back to Terminal Menu]\r\n", 36);
}

static void handle_job_input(char ch) {
    if (ch == '\x1b') {
        g_jobs_active = false;
        g_editor_mode = MODE_TERMINAL;
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        lau_vram_write_string(g_vram, "CIS: WHAT'S MY JOB? database exited.\r\n", 38);
        return;
    }
    
    if (g_job_submode == 0) {
        if (ch == 'y' || ch == 'Y') {
            int next = g_job_nodes[g_job_current_node].yes_child;
            if (next == -1) {
                g_job_submode = 1;
            } else {
                g_job_prev_node = g_job_current_node;
                g_job_last_answer_was_yes = true;
                g_job_current_node = next;
            }
            redraw_job_screen();
        } else if (ch == 'n' || ch == 'N') {
            int next = g_job_nodes[g_job_current_node].no_child;
            if (next == -1) {
                g_job_submode = 1;
            } else {
                g_job_prev_node = g_job_current_node;
                g_job_last_answer_was_yes = false;
                g_job_current_node = next;
            }
            redraw_job_screen();
        }
    } else if (g_job_submode == 1) {
        if (ch == 'y' || ch == 'Y') {
            g_job_submode = 0;
            g_job_current_node = 0;
            g_job_prev_node = -1;
            redraw_job_screen();
            lau_vram_write_string(g_vram, "\r\n  I knew it! Let's play again.\r\n", 35);
        } else if (ch == 'n' || ch == 'N') {
            g_job_submode = 2;
            g_job_new_name[0] = '\0';
            redraw_job_screen();
        }
    } else if (g_job_submode == 2) {
        if (ch == '\n' || ch == '\r') {
            if (strlen(g_job_new_name) > 0) {
                g_job_submode = 3;
                g_job_new_question[0] = '\0';
            }
            redraw_job_screen();
            return;
        }
        if (ch == '\b' || ch == 127) {
            int len = strlen(g_job_new_name);
            if (len > 0) {
                g_job_new_name[len - 1] = '\0';
            }
            redraw_job_screen();
            return;
        }
        int len = strlen(g_job_new_name);
        if (ch >= 32 && ch < 127 && len < 60) {
            g_job_new_name[len] = ch;
            g_job_new_name[len + 1] = '\0';
            redraw_job_screen();
        }
    } else if (g_job_submode == 3) {
        if (ch == '\n' || ch == '\r') {
            if (strlen(g_job_new_question) > 0) {
                g_job_submode = 4;
            }
            redraw_job_screen();
            return;
        }
        if (ch == '\b' || ch == 127) {
            int len = strlen(g_job_new_question);
            if (len > 0) {
                g_job_new_question[len - 1] = '\0';
            }
            redraw_job_screen();
            return;
        }
        int len = strlen(g_job_new_question);
        if (ch >= 32 && ch < 127 && len < 120) {
            g_job_new_question[len] = ch;
            g_job_new_question[len + 1] = '\0';
            redraw_job_screen();
        }
    } else if (g_job_submode == 4) {
        if (ch == 'y' || ch == 'Y' || ch == 'n' || ch == 'N') {
            bool yes_for_new = (ch == 'y' || ch == 'Y');
            
            if (g_job_nodes_count < 46) {
                int old_node_idx = g_job_current_node;
                int new_job_idx = g_job_nodes_count;
                int old_leaf_idx = g_job_nodes_count + 1;
                g_job_nodes_count += 2;
                
                g_job_nodes[new_job_idx].yes_child = -1;
                g_job_nodes[new_job_idx].no_child = -1;
                snprintf(g_job_nodes[new_job_idx].text, sizeof(g_job_nodes[new_job_idx].text), "%s", g_job_new_name);
                
                g_job_nodes[old_leaf_idx] = g_job_nodes[old_node_idx];
                
                snprintf(g_job_nodes[old_node_idx].text, sizeof(g_job_nodes[old_node_idx].text), "%s", g_job_new_question);
                if (yes_for_new) {
                    g_job_nodes[old_node_idx].yes_child = new_job_idx;
                    g_job_nodes[old_node_idx].no_child = old_leaf_idx;
                } else {
                    g_job_nodes[old_node_idx].yes_child = old_leaf_idx;
                    g_job_nodes[old_node_idx].no_child = new_job_idx;
                }
                
                save_job_tree();
            }
            
            g_job_submode = 0;
            g_job_current_node = 0;
            g_job_prev_node = -1;
            redraw_job_screen();
        }
    }
}



static void redraw_slinkypanic_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "==================================================\r\n"
        "      SLINKY PANIC - HYBRID RESCUE / PLATFORM     \r\n"
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
            } else if (g_slinky_monster_row == r && g_slinky_col == c) {
                snprintf(val_str, sizeof(val_str), "/\x1b[31mM\x1b[0m\\");
            } else if (g_slinky_hole[r][c] > 0) {
                snprintf(val_str, sizeof(val_str), "/_\\");
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
        " [I/K/J/L to jump, SPACE to dig holes]            \r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void update_slinkypanic_game(void) {
    if (!g_slinkypanic_active) return;
    
    static int monster_tick = 0;
    monster_tick++;
    if (monster_tick % 25 == 0) {
        for (int r = 0; r < 5; r++) {
            for (int c = 0; c <= r; c++) {
                if (g_slinky_hole[r][c] > 0) {
                    g_slinky_hole[r][c]--;
                }
            }
        }
        
        if (g_slinky_monster_stuck > 0) {
            g_slinky_monster_stuck--;
            if (g_slinky_monster_stuck == 0) {
                g_slinky_hole[g_slinky_monster_row][g_slinky_monster_col] = 0;
            }
        } else {
            if (g_slinky_monster_row < g_slinky_row) {
                g_slinky_monster_row++;
                if (g_slinky_monster_col < g_slinky_col) g_slinky_monster_col++;
            } else if (g_slinky_monster_row > g_slinky_row) {
                g_slinky_monster_row--;
                if (g_slinky_monster_col > g_slinky_col) g_slinky_monster_col--;
            } else {
                if (g_slinky_monster_col < g_slinky_col) g_slinky_monster_col++;
                else if (g_slinky_monster_col > g_slinky_col) g_slinky_monster_col--;
            }
            
            if (g_slinky_hole[g_slinky_monster_row][g_slinky_monster_col] > 0) {
                g_slinky_monster_stuck = 15;
            }
            
            if (g_slinky_monster_row == g_slinky_row && g_slinky_monster_col == g_slinky_col) {
                g_slinky_lives--;
                if (g_slinky_lives <= 0) {
                    g_slinky_lives = 3;
                    g_slinky_score = 0;
                    memset(g_slinky_blocks, 0, sizeof(g_slinky_blocks));
                }
                g_slinky_row = 0;
                g_slinky_col = 0;
                g_slinky_monster_row = 4;
                g_slinky_monster_col = 4;
                g_slinky_monster_stuck = 0;
            }
        }
        redraw_slinkypanic_screen();
    }
}

static void handle_slinkypanic_input(char ch) {
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
    } else if (ch == ' ') {
        g_slinky_hole[g_slinky_row][g_slinky_col] = 40;
        printf("\x07");
        fflush(stdout);
        redraw_slinkypanic_screen();
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
