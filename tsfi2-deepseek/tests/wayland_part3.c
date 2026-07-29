    lau_vram_write_string(g_vram, " Actions: [N/S/E/W] Move, [T] Take Film, [C] Cut Power, [ESC] Exit\r\n", 67);
}

static void handle_cloak_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 'n' || ch == 'N') {
        if (g_cloak_room == 0) {
            g_cloak_room = 1;
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Moved to Vault Room.");
        } else {
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Cannot go North here.");
        }
    } else if (ch == 's' || ch == 'S') {
        if (g_cloak_room == 1) {
            g_cloak_room = 0;
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Moved to Entrance.");
        } else {
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Cannot go South here.");
        }
    } else if (ch == 'e' || ch == 'E') {
        if (g_cloak_room == 0) {
            g_cloak_room = 2;
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Moved to Security Room.");
        } else {
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Cannot go East here.");
        }
    } else if (ch == 'w' || ch == 'W') {
        if (g_cloak_room == 2) {
            g_cloak_room = 0;
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Moved to Entrance.");
        } else {
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Cannot go West here.");
        }
    } else if (ch == 't' || ch == 'T') {
        if (g_cloak_room == 1) {
            g_cloak_has_film = true;
            snprintf(g_cloak_status, sizeof(g_cloak_status), "MICROFILM SECURED! Escape to Entrance now.");
        } else {
            snprintf(g_cloak_status, sizeof(g_cloak_status), "No film here.");
        }
    } else if (ch == 'c' || ch == 'C') {
        if (g_cloak_room == 2) {
            g_cloak_alarm = false;
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Cut control power. Security offline.");
        } else {
            snprintf(g_cloak_status, sizeof(g_cloak_status), "Cannot cut power from here.");
        }
    }
    redraw_cloak_screen();
}

static void init_gypsy(void) {
    g_gypsy_fuel = 100;
    g_gypsy_credits = 500;
    g_gypsy_cargo = 0;
    g_gypsy_sector = 1;
    snprintf(g_gypsy_status, sizeof(g_gypsy_status), "Awaiting commands at Sector Starport.");
}

static void redraw_gypsy_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "   GYPSY STARSHIP: Space Trader (Ahoy! Issue 24)    \r\n"
             "====================================================\r\n"
             " Sector: %d | Fuel: %d/100 | Credits: %d | Cargo: %d\r\n"
             "----------------------------------------------------\r\n",
             g_gypsy_sector, g_gypsy_fuel, g_gypsy_credits, g_gypsy_cargo);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    lau_vram_write_string(g_vram, " Local Market Prices:\r\n  - Ore Minerals: $50\r\n  - Deuterium Gas: $150\r\n", 76);
    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_gypsy_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [B] Buy Mineral, [S] Sell Cargo, [H] Hyperjump, [ESC] Exit\r\n", 70);
}

static void handle_gypsy_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 'b' || ch == 'B') {
        if (g_gypsy_credits >= 50) {
            g_gypsy_credits -= 50;
            g_gypsy_cargo++;
            snprintf(g_gypsy_status, sizeof(g_gypsy_status), "Bought 1 mineral unit.");
        } else {
            snprintf(g_gypsy_status, sizeof(g_gypsy_status), "Insufficient credits!");
        }
    } else if (ch == 's' || ch == 'S') {
        if (g_gypsy_cargo > 0) {
            g_gypsy_cargo--;
            g_gypsy_credits += 75; // Profit
            snprintf(g_gypsy_status, sizeof(g_gypsy_status), "Sold 1 cargo unit at local premium.");
        } else {
            snprintf(g_gypsy_status, sizeof(g_gypsy_status), "No cargo to sell!");
        }
    } else if (ch == 'h' || ch == 'H') {
        if (g_gypsy_fuel >= 20) {
            g_gypsy_fuel -= 20;
            g_gypsy_sector = (g_gypsy_sector % 5) + 1;
            snprintf(g_gypsy_status, sizeof(g_gypsy_status), "Jumped sectors! Fuel consumed.");
        } else {
            snprintf(g_gypsy_status, sizeof(g_gypsy_status), "Low fuel! Refuel ship.");
        }
    }
    redraw_gypsy_screen();
}

// ----------------------------------------------------
// Issue 25: Martian Monsters
// ----------------------------------------------------
static void init_martian(void) {
    g_martian_ship_x = 20;
    g_martian_monster_x = 10;
    g_martian_monster_y = 2;
    g_martian_score = 0;
    g_martian_lives = 3;
    snprintf(g_martian_status, sizeof(g_martian_status), "Defend Earth from descending Martians!");
}

static void redraw_martian_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[2048];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    MARTIAN MONSTERS: Space Defender (Ahoy! Issue 25)\r\n"
             "====================================================\r\n"
             " Lives: %d | Score: %d\r\n"
             "----------------------------------------------------\r\n",
             g_martian_lives, g_martian_score);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render 10 lines of space
    for (int y = 0; y < 10; y++) {
        char line[64];
        memset(line, ' ', 50);
        line[0] = '|';
        line[49] = '|';
        line[50] = '\r';
        line[51] = '\n';
        line[52] = '\0';

        if (y == g_martian_monster_y) {
            line[g_martian_monster_x] = 'M';
        }
        if (y == 8) {
            line[g_martian_ship_x] = 'A';
            line[g_martian_ship_x - 1] = '/';
            line[g_martian_ship_x + 1] = '\\';
        }
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_martian_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [A] Move Left, [D] Move Right, [SPACE] Fire, [ESC] Exit\r\n", 67);
}

static void handle_martian_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (g_martian_lives <= 0) {
        if (ch == ' ' || ch == '\r') {
            init_martian();
        }
        return;
    }
    if (ch == 'a' || ch == 'A') {
        if (g_martian_ship_x > 2) g_martian_ship_x--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_martian_ship_x < 47) g_martian_ship_x++;
    } else if (ch == ' ') {
        // Laser fire! Check x alignment
        if (abs(g_martian_ship_x - g_martian_monster_x) <= 1) {
            g_martian_score += 100;
            g_martian_monster_y = 1;
            g_martian_monster_x = (rand() % 40) + 5;
            snprintf(g_martian_status, sizeof(g_martian_status), "Direct Hit! Monster destroyed.");
        } else {
            snprintf(g_martian_status, sizeof(g_martian_status), "Missed! Target is off-center.");
        }
    }
    redraw_martian_screen();
}

static void update_martian(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 400) return;
    last_tick = ms;

    if (g_martian_lives <= 0) return;

    g_martian_monster_y++;
    if (g_martian_monster_y >= 8) {
        // Hit!
        g_martian_lives--;
        g_martian_monster_y = 1;
        g_martian_monster_x = (rand() % 40) + 5;
        if (g_martian_lives <= 0) {
            snprintf(g_martian_status, sizeof(g_martian_status), "GAME OVER. Press SPACE to Restart.");
        } else {
            snprintf(g_martian_status, sizeof(g_martian_status), "Ship hit! Lives remaining: %d", g_martian_lives);
        }
    }
    redraw_martian_screen();
}

// ----------------------------------------------------
// Issue 25: The Haunted Castle
// ----------------------------------------------------
static void init_haunted(void) {
    g_haunted_x = 1;
    g_haunted_y = 1;
    g_haunted_gold = 0;
    g_haunted_ghost_x = 7;
    g_haunted_ghost_y = 6;
    snprintf(g_haunted_status, sizeof(g_haunted_status), "Escape with the castle treasures!");
}

static void redraw_haunted_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[2048];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    THE HAUNTED CASTLE: Gothic Labyrinth (Issue 25)  \r\n"
             "====================================================\r\n"
             " Gold Collected: %d GP\r\n"
             "----------------------------------------------------\r\n",
             g_haunted_gold);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // 8x8 Labyrinth
    const char* maze[8] = {
        "########",
        "#..#...#",
        "#.##.#.#",
        "#....#.#",
        "###.##.#",
        "#...#..#",
        "#.#.G..#",
        "########"
    };

    for (int y = 0; y < 8; y++) {
        char line[64];
        snprintf(line, sizeof(line), "  ");
        for (int x = 0; x < 8; x++) {
            char cell = maze[y][x];
            if (x == g_haunted_x && y == g_haunted_y) {
                cell = '@'; // Player
            } else if (x == g_haunted_ghost_x && y == g_haunted_ghost_y) {
                cell = 'G'; // Ghost
            } else if (x == 5 && y == 1 && g_haunted_gold == 0) {
                cell = '$'; // Gold Chest
            }
            char block[4];
            snprintf(block, sizeof(block), "%c ", cell);
            strcat(line, block);
        }
        strcat(line, "\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_haunted_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [W/A/S/D] Move Hero, [ESC] Exit Labyrinth\r\n", 54);
}

static void handle_haunted_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }

    int next_x = g_haunted_x;
    int next_y = g_haunted_y;

    if (ch == 'w' || ch == 'W') next_y--;
    else if (ch == 's' || ch == 'S') next_y++;
    else if (ch == 'a' || ch == 'A') next_x--;
    else if (ch == 'd' || ch == 'D') next_x++;

    const char* maze[8] = {
        "########",
        "#..#...#",
        "#.##.#.#",
        "#....#.#",
        "###.##.#",
        "#...#..#",
        "#.#.G..#",
        "########"
    };

    if (next_x >= 0 && next_x < 8 && next_y >= 0 && next_y < 8) {
        if (maze[next_y][next_x] != '#') {
            g_haunted_x = next_x;
            g_haunted_y = next_y;
        }
    }

    // Check Chest
    if (g_haunted_x == 5 && g_haunted_y == 1 && g_haunted_gold == 0) {
        g_haunted_gold += 250;
        snprintf(g_haunted_status, sizeof(g_haunted_status), "Opened Gold Chest! Earned +250 GP.");
    }

    // Check Ghost contact
    if (g_haunted_x == g_haunted_ghost_x && g_haunted_y == g_haunted_ghost_y) {
        g_haunted_gold = 0;
        snprintf(g_haunted_status, sizeof(g_haunted_status), "Spooked by the Ghost! Dropped all gold!");
    }

    redraw_haunted_screen();
}

// ----------------------------------------------------
// Issue 25: Infraraid
// ----------------------------------------------------
static void init_infraraid(void) {
    g_infraraid_angle = 90;
    g_infraraid_sweep = 0;
    g_infraraid_targets = 5;
    snprintf(g_infraraid_status, sizeof(g_infraraid_status), "Calibrating radar sweeping grid...");
}

static void redraw_infraraid_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    INFRARAID: Defensive Radar Shield (Issue 25)    \r\n"
             "====================================================\r\n"
             " Defense Angle: %d Deg | Active Targets: %d\r\n"
             "----------------------------------------------------\r\n",
             g_infraraid_angle, g_infraraid_targets);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Radar screen simulation
    lau_vram_write_string(g_vram, "   \\                 |                 /\r\n", 44);
    lau_vram_write_string(g_vram, "    \\                |                /\r\n", 42);
    lau_vram_write_string(g_vram, "     \\   *           |               /\r\n", 40);
    lau_vram_write_string(g_vram, "      \\              |              /\r\n", 38);
    lau_vram_write_string(g_vram, "       \\_____________|_____________/\r\n", 38);

    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_infraraid_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [A] Sweep Left, [D] Sweep Right, [SPACE] Intercept, [ESC] Exit\r\n", 75);
}

static void handle_infraraid_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 'a' || ch == 'A') {
        g_infraraid_angle = (g_infraraid_angle + 350) % 360;
        snprintf(g_infraraid_status, sizeof(g_infraraid_status), "Sweeping grid left...");
    } else if (ch == 'd' || ch == 'D') {
        g_infraraid_angle = (g_infraraid_angle + 10) % 360;
        snprintf(g_infraraid_status, sizeof(g_infraraid_status), "Sweeping grid right...");
    } else if (ch == ' ') {
        if (g_infraraid_targets > 0) {
            g_infraraid_targets--;
            snprintf(g_infraraid_status, sizeof(g_infraraid_status), "Laser Intercept Fired! Target neutralized.");
        } else {
            snprintf(g_infraraid_status, sizeof(g_infraraid_status), "Grid secure. No active threats.");
        }
    }
    redraw_infraraid_screen();
}

// ----------------------------------------------------
// Issue 25: Streamer Font
// ----------------------------------------------------
static void init_streamer(void) {
    g_streamer_cursor_x = 0;
    g_streamer_cursor_y = 0;
    snprintf(g_streamer_status, sizeof(g_streamer_status), "Use W/A/S/D to select pixel, SPACE to toggle.");
}

static void redraw_streamer_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    STREAMER FONT: Custom Character Editor (Issue 25)\r\n"
             "====================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Draw 8x8 grid representation
    for (int y = 0; y < 8; y++) {
        char line[128];
        snprintf(line, sizeof(line), "  Row %d:  [ ", y);
        for (int x = 0; x < 8; x++) {
            bool pixel = (g_streamer_char[y] & (1 << (7 - x))) != 0;
            char cell_char = pixel ? 'X' : '.';
            if (x == g_streamer_cursor_x && y == g_streamer_cursor_y) {
                char item[8];
                snprintf(item, sizeof(item), "<%c>", cell_char);
                strcat(line, item);
            } else {
                char item[8];
                snprintf(item, sizeof(item), " %c ", cell_char);
                strcat(line, item);
            }
        }
        strcat(line, " ]\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_streamer_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [W/A/S/D] Move, [SPACE] Toggle, [S] Save Font, [ESC] Exit\r\n", 70);
}

static void handle_streamer_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 'w' || ch == 'W') {
        if (g_streamer_cursor_y > 0) g_streamer_cursor_y--;
    } else if (ch == 's' || ch == 'S') {
        if (g_streamer_cursor_y < 7) g_streamer_cursor_y++;
    } else if (ch == 'a' || ch == 'A') {
        if (g_streamer_cursor_x > 0) g_streamer_cursor_x--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_streamer_cursor_x < 7) g_streamer_cursor_x++;
    } else if (ch == ' ') {
        // Toggle bit
        g_streamer_char[g_streamer_cursor_y] ^= (1 << (7 - g_streamer_cursor_x));
        snprintf(g_streamer_status, sizeof(g_streamer_status), "Toggled pixel at (%d, %d).", g_streamer_cursor_x, g_streamer_cursor_y);
    }
    redraw_streamer_screen();
}

// ----------------------------------------------------
// Issue 25: Knockout!
// ----------------------------------------------------
static void init_knockout(void) {
    g_knockout_paddle_x = 16;
    g_knockout_ball_x = 10;
    g_knockout_ball_y = 6;
    g_knockout_ball_dx = 1;
    g_knockout_ball_dy = 1;
    g_knockout_score = 0;
    // Fill bricks
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 20; x++) {
            g_knockout_blocks[y][x] = 1;
        }
    }
}

static void redraw_knockout_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[2048];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    KNOCKOUT!: Brick Breaker Simulation (Issue 25)  \r\n"
             "====================================================\r\n"
             " Score: %d\r\n"
             "----------------------------------------------------\r\n",
             g_knockout_score);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render bricks and ball
    for (int y = 0; y < 10; y++) {
        char line[64];
        memset(line, ' ', 40);
        line[0] = '|';
        line[39] = '|';
        line[40] = '\r';
        line[41] = '\n';
        line[42] = '\0';

        if (y < 3) {
            for (int x = 1; x < 39; x++) {
                int bx = (x - 1) / 2;
                if (g_knockout_blocks[y][bx]) {
                    line[x] = '=';
                }
            }
        }

        if (y == g_knockout_ball_y) {
            line[g_knockout_ball_x] = 'O';
        }

        if (y == 9) {
            // Paddle
            for (int p = 0; p < 6; p++) {
                if (g_knockout_paddle_x + p < 39) {
                    line[g_knockout_paddle_x + p] = '_';
                }
            }
        }

        lau_vram_write_string(g_vram, line, strlen(line));
    }

    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    lau_vram_write_string(g_vram, " Controls: [A] Paddle Left, [D] Paddle Right, [ESC] Exit\r\n", 57);
}

static void handle_knockout_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 'a' || ch == 'A') {
        if (g_knockout_paddle_x > 1) g_knockout_paddle_x -= 2;
    } else if (ch == 'd' || ch == 'D') {
        if (g_knockout_paddle_x < 32) g_knockout_paddle_x += 2;
    }
    redraw_knockout_screen();
}

static void update_knockout(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 300) return;
    last_tick = ms;

    g_knockout_ball_x += g_knockout_ball_dx;
    g_knockout_ball_y += g_knockout_ball_dy;

    // Bounce walls
    if (g_knockout_ball_x <= 1 || g_knockout_ball_x >= 38) {
        g_knockout_ball_dx = -g_knockout_ball_dx;
    }
    if (g_knockout_ball_y <= 0) {
        g_knockout_ball_dy = -g_knockout_ball_dy;
    }

    // Bounce bricks
    if (g_knockout_ball_y < 3) {
        int bx = (g_knockout_ball_x - 1) / 2;
        if (g_knockout_blocks[g_knockout_ball_y][bx]) {
            g_knockout_blocks[g_knockout_ball_y][bx] = 0;
            g_knockout_ball_dy = -g_knockout_ball_dy;
            g_knockout_score += 50;
        }
    }

    // Paddle collision
    if (g_knockout_ball_y == 8) {
        if (g_knockout_ball_x >= g_knockout_paddle_x && g_knockout_ball_x <= g_knockout_paddle_x + 6) {
            g_knockout_ball_dy = -g_knockout_ball_dy;
        }
    }

    // Fall out
    if (g_knockout_ball_y >= 10) {
        g_knockout_ball_x = 10;
        g_knockout_ball_y = 4;
        g_knockout_ball_dy = 1;
    }

    redraw_knockout_screen();
}

// ----------------------------------------------------
// Issue 25: Alarm Clock
// ----------------------------------------------------
static void init_alarm(void) {
    g_alarm_h = 12;
    g_alarm_m = 0;
    g_alarm_s = 0;
    g_alarm_set_h = 12;
    g_alarm_set_m = 2;
    g_alarm_triggered = false;
    snprintf(g_alarm_status, sizeof(g_alarm_status), "Alarm Clock configured.");
}

static void redraw_alarm_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    ALARM CLOCK: CIA Timer Simulator (Issue 25)     \r\n"
             "====================================================\r\n"
             " Simulated Time: %02d:%02d:%02d\r\n"
             " Alarm Set for:  %02d:%02d\r\n"
             "----------------------------------------------------\r\n"
             " Alarm Triggered: %s\r\n"
             "====================================================\r\n",
             g_alarm_h, g_alarm_m, g_alarm_s,
             g_alarm_set_h, g_alarm_set_m,
             g_alarm_triggered ? "RINGING! *BEEP* *BEEP*" : "STANDBY");
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [H] Adjust Alarm Hr, [M] Adjust Alarm Min, [ESC] Exit\r\n", 65);
}

static void handle_alarm_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 'h' || ch == 'H') {
        g_alarm_set_h = (g_alarm_set_h % 24) + 1;
    } else if (ch == 'm' || ch == 'M') {
        g_alarm_set_m = (g_alarm_set_m + 5) % 60;
    }
    redraw_alarm_screen();
}

// ----------------------------------------------------
// Issue 25: Memory Check
// ----------------------------------------------------
static void init_memcheck(void) {
    g_memcheck_addr = 0x0800;
    g_memcheck_errors = 0;
    snprintf(g_memcheck_status, sizeof(g_memcheck_status), "Scanning system RAM block by block...");
}

static void redraw_memcheck_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    MEMORY CHECK: System RAM Diagnostic (Issue 25)  \r\n"
             "====================================================\r\n"
             " Current Address Scan: $%04X / $FFFF\r\n"
             " Integrity Failures:   %d bad bytes\r\n"
             "----------------------------------------------------\r\n"
             " Scanner Status: %s\r\n"
             "====================================================\r\n",
             g_memcheck_addr, g_memcheck_errors, g_memcheck_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [ESC] Abort Diagnostics Scanner\r\n", 44);
}

static void handle_memcheck_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
}

static void update_memcheck(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 100) return;
    last_tick = ms;

    if (g_memcheck_addr < 0xFFFF) {
        g_memcheck_addr += 256;
        if (rand() % 500 == 0) {
            g_memcheck_errors++;
            snprintf(g_memcheck_status, sizeof(g_memcheck_status), "ALERT: Bad cell parity detected at $%04X!", g_memcheck_addr);
        }
    } else {
        snprintf(g_memcheck_status, sizeof(g_memcheck_status), "Scan Complete. RAM fully verified.");
    }
    redraw_memcheck_screen();
}

// ----------------------------------------------------
// Issue 26: Arena
// ----------------------------------------------------
static void init_arena(void) {
    g_arena_x = 2;
    g_arena_y = 2;
    g_arena_enemy_x = 8;
    g_arena_enemy_y = 6;
    g_arena_score = 0;
    snprintf(g_arena_status, sizeof(g_arena_status), "Survival Arena! Evade the hunter.");
}

static void redraw_arena_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[2048];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    ARENA: Tactical Grid Combat (Ahoy! Issue 26)   \r\n"
             "====================================================\r\n"
             " Score: %d | Status: %s\r\n"
             "----------------------------------------------------\r\n",
             g_arena_score, g_arena_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    for (int y = 0; y < 8; y++) {
        char line[64] = "  ";
        for (int x = 0; x < 10; x++) {
            char cell = '.';
            if (x == g_arena_x && y == g_arena_y) cell = '@';
            else if (x == g_arena_enemy_x && y == g_arena_enemy_y) cell = 'E';
            else if ((x == 4 && y == 3) || (x == 5 && y == 4)) cell = '#'; // obstacle

            char cell_str[4];
            snprintf(cell_str, sizeof(cell_str), "%c ", cell);
            strcat(line, cell_str);
        }
        strcat(line, "\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }
    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    lau_vram_write_string(g_vram, " Controls: [W/A/S/D] Move, [SPACE] Attack, [ESC] Exit\r\n", 54);
}

static void handle_arena_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    int next_x = g_arena_x;
    int next_y = g_arena_y;
    if (ch == 'w' || ch == 'W') next_y--;
    else if (ch == 's' || ch == 'S') next_y++;
    else if (ch == 'a' || ch == 'A') next_x--;
    else if (ch == 'd' || ch == 'D') next_x++;
    else if (ch == ' ') {
        if (abs(g_arena_x - g_arena_enemy_x) <= 1 && abs(g_arena_y - g_arena_enemy_y) <= 1) {
            g_arena_score += 100;
            g_arena_enemy_x = rand() % 10;
            g_arena_enemy_y = rand() % 8;
            snprintf(g_arena_status, sizeof(g_arena_status), "Hit! Enemy sent packing.");
        } else {
            snprintf(g_arena_status, sizeof(g_arena_status), "Missed strike.");
        }
    }

    if (next_x >= 0 && next_x < 10 && next_y >= 0 && next_y < 8) {
        if (!((next_x == 4 && next_y == 3) || (next_x == 5 && next_y == 4))) {
            g_arena_x = next_x;
            g_arena_y = next_y;
        }
    }
    redraw_arena_screen();
}

static void update_arena(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 600) return;
    last_tick = ms;

    // Enemy chase logic
    if (g_arena_enemy_x < g_arena_x) g_arena_enemy_x++;
    else if (g_arena_enemy_x > g_arena_x) g_arena_enemy_x--;

    if (g_arena_enemy_y < g_arena_y) g_arena_enemy_y++;
    else if (g_arena_enemy_y > g_arena_y) g_arena_enemy_y--;

    if (g_arena_x == g_arena_enemy_x && g_arena_y == g_arena_enemy_y) {
        g_arena_score = (g_arena_score > 50) ? (g_arena_score - 50) : 0;
        snprintf(g_arena_status, sizeof(g_arena_status), "Caught by Hunter! Score reduced.");
    }
    redraw_arena_screen();
}

// ----------------------------------------------------
// Issue 26: Head to Head
// ----------------------------------------------------
static void init_headtohead(void) {
    g_h2h_p1_x = 0;
    g_h2h_p1_y = 0;
    g_h2h_p2_x = 9;
    g_h2h_p2_y = 9;
    g_h2h_p1_score = 0;
    g_h2h_p2_score = 0;
    memset(g_h2h_grid, 0, sizeof(g_h2h_grid));
    g_h2h_grid[0][0] = 1;
    g_h2h_grid[9][9] = 2;
    snprintf(g_h2h_status, sizeof(g_h2h_status), "Claim territory by occupying cells!");
}

static void redraw_headtohead_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    HEAD TO HEAD: Grid Alignment Game (Issue 26)    \r\n"
             "====================================================\r\n"
             " Player 1 (Blue): %d | Player 2 (Red): %d\r\n"
             "----------------------------------------------------\r\n",
             g_h2h_p1_score, g_h2h_p2_score);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    for (int y = 0; y < 10; y++) {
        char line[128] = "  ";
        for (int x = 0; x < 10; x++) {
            char cell = '.';
            if (x == g_h2h_p1_x && y == g_h2h_p1_y) cell = '1';
            else if (x == g_h2h_p2_x && y == g_h2h_p2_y) cell = '2';
            else if (g_h2h_grid[y][x] == 1) cell = 'o';
            else if (g_h2h_grid[y][x] == 2) cell = 'x';

            char cell_str[8];
            snprintf(cell_str, sizeof(cell_str), "%c ", cell);
            strcat(line, cell_str);
        }
        strcat(line, "\r\n");
        lau_vram_write_string(g_vram, line, strlen(line));
    }
    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    lau_vram_write_string(g_vram, " Controls: [W/A/S/D] Player 1, [I/J/K/L] Player 2, [ESC] Exit\r\n", 62);
}

static void handle_headtohead_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }

    // P1 movement (WASD)
    if (ch == 'w' || ch == 'W') { if (g_h2h_p1_y > 0) g_h2h_p1_y--; }
    else if (ch == 's' || ch == 'S') { if (g_h2h_p1_y < 9) g_h2h_p1_y++; }
    else if (ch == 'a' || ch == 'A') { if (g_h2h_p1_x > 0) g_h2h_p1_x--; }
    else if (ch == 'd' || ch == 'D') { if (g_h2h_p1_x < 9) g_h2h_p1_x++; }

    // P2 movement (IJKL)
    if (ch == 'i' || ch == 'I') { if (g_h2h_p2_y > 0) g_h2h_p2_y--; }
    else if (ch == 'k' || ch == 'K') { if (g_h2h_p2_y < 9) g_h2h_p2_y++; }
    else if (ch == 'j' || ch == 'J') { if (g_h2h_p2_x > 0) g_h2h_p2_x--; }
    else if (ch == 'l' || ch == 'L') { if (g_h2h_p2_x < 9) g_h2h_p2_x++; }

    // Claim cells
    if (g_h2h_grid[g_h2h_p1_y][g_h2h_p1_x] == 0) {
        g_h2h_grid[g_h2h_p1_y][g_h2h_p1_x] = 1;
        g_h2h_p1_score++;
    }
    if (g_h2h_grid[g_h2h_p2_y][g_h2h_p2_x] == 0) {
        g_h2h_grid[g_h2h_p2_y][g_h2h_p2_x] = 2;
        g_h2h_p2_score++;
    }

    redraw_headtohead_screen();
}

// ----------------------------------------------------
// Issue 26: Crabfight
// ----------------------------------------------------
static void init_crabfight(void) {
    g_crab_x = 10;
    g_crab_opp_x = 30;
    g_crab_hp = 100;
    g_crab_opp_hp = 100;
    snprintf(g_crab_status, sizeof(g_crab_status), "Shoreline Dueling! Move close and PINCH.");
}

static void redraw_crabfight_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    CRABFIGHT: Crustacean Shoreline Duel (Issue 26) \r\n"
             "====================================================\r\n"
             " Player HP: %d/100 | Opponent HP: %d/100\r\n"
             "----------------------------------------------------\r\n",
             g_crab_hp, g_crab_opp_hp);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    // Render beach line
    char beach[64];
    memset(beach, '~', 40);
    beach[40] = '\0';

    if (g_crab_x >= 0 && g_crab_x < 40) beach[g_crab_x] = 'C';
    if (g_crab_opp_x >= 0 && g_crab_opp_x < 40) beach[g_crab_opp_x] = 'O';

    snprintf(buf, sizeof(buf), " [Sea]  %s  [Dunes]\r\n", beach);
    lau_vram_write_string(g_vram, buf, strlen(buf));

    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_crab_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [A] Left, [D] Right, [SPACE] Pinch Attack, [ESC] Exit\r\n", 66);
}

static void handle_crabfight_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }

    if (ch == 'a' || ch == 'A') {
        if (g_crab_x > 1) g_crab_x--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_crab_x < g_crab_opp_x - 1) g_crab_x++;
    } else if (ch == ' ') {
        // Pinch!
        if (g_crab_opp_x - g_crab_x <= 2) {
            g_crab_opp_hp -= 15;
            if (g_crab_opp_hp <= 0) {
                g_crab_opp_hp = 0;
                snprintf(g_crab_status, sizeof(g_crab_status), "VICTORY! Opponent crab retreated.");
            } else {
                snprintf(g_crab_status, sizeof(g_crab_status), "Snip! Opponent takes damage.");
            }
        } else {
            snprintf(g_crab_status, sizeof(g_crab_status), "Too far away to pinch!");
        }
    }
    redraw_crabfight_screen();
}

// ----------------------------------------------------
// Issue 26: Treasure Wheel
// ----------------------------------------------------
static void init_treasure(void) {
    g_wheel_score = 0;
    g_wheel_spin_val = 150;
    strcpy(g_wheel_guessed, "____ ________");
    snprintf(g_wheel_status, sizeof(g_wheel_status), "Spin the wheel and solve the retro puzzle!");
}

static void redraw_treasure_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    TREASURE WHEEL: Puzzle Word Game (Issue 26)     \r\n"
             "====================================================\r\n"
             " Balance: $%d | Current Wheel Value: $%d\r\n"
             "----------------------------------------------------\r\n"
             " Puzzle:  %s\r\n"
             "----------------------------------------------------\r\n",
             g_wheel_score, g_wheel_spin_val, g_wheel_guessed);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_wheel_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
    lau_vram_write_string(g_vram, " Controls: [S] Spin Wheel, [A-Z] Guess Letter, [ESC] Exit\r\n", 58);
}

static void handle_treasure_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == 's' || ch == 'S') {
        g_wheel_spin_val = ((rand() % 5) + 1) * 100;
        snprintf(g_wheel_status, sizeof(g_wheel_status), "Spun wheel to $%d! Guess a letter.", g_wheel_spin_val);
    } else if (ch >= 'a' && ch <= 'z') {
        ch -= 32; // upper case
    }
    if (ch >= 'A' && ch <= 'Z') {
        bool correct = false;
        for (size_t i = 0; i < strlen(g_wheel_phrase); i++) {
            if (g_wheel_phrase[i] == ch && g_wheel_guessed[i] == '_') {
                g_wheel_guessed[i] = ch;
                g_wheel_score += g_wheel_spin_val;
                correct = true;
            }
        }
        if (correct) {
            snprintf(g_wheel_status, sizeof(g_wheel_status), "Correct guess! Balance updated.");
        } else {
            snprintf(g_wheel_status, sizeof(g_wheel_status), "Letter '%c' is not in the phrase.", ch);
        }
    }
    redraw_treasure_screen();
}

// ----------------------------------------------------
// Issue 26: Character Dump
// ----------------------------------------------------
static void init_chardump(void) {
    g_chardump_offset = 0xD000;
    snprintf(g_chardump_status, sizeof(g_chardump_status), "Press ESC to return. Dump address block starting at $D000.");
}

static void redraw_chardump_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char buf[1024];
    snprintf(buf, sizeof(buf),
             "====================================================\r\n"
             "    CHARACTER DUMP: System RAM Hex Scan (Issue 26)   \r\n"
             "====================================================\r\n");
    lau_vram_write_string(g_vram, buf, strlen(buf));

    for (int row = 0; row < 8; row++) {
        uint16_t addr = g_chardump_offset + row * 8;
        char hex[128];
        snprintf(hex, sizeof(hex), "  $%04X:  %02X %02X %02X %02X %02X %02X %02X %02X  |  ",
                 addr, rand() % 256, rand() % 256, rand() % 256, rand() % 256,
                 rand() % 256, rand() % 256, rand() % 256, rand() % 256);
        lau_vram_write_string(g_vram, hex, strlen(hex));

        char chars[16];
        snprintf(chars, sizeof(chars), "........\r\n");
        lau_vram_write_string(g_vram, chars, strlen(chars));
    }
    lau_vram_write_string(g_vram, "----------------------------------------------------\r\n", 54);
    snprintf(buf, sizeof(buf), " Status: %s\r\n", g_chardump_status);
    lau_vram_write_string(g_vram, buf, strlen(buf));
}

static void handle_chardump_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
}

static void update_chardump(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 800) return;
    last_tick = ms;

    g_chardump_offset += 64;
    if (g_chardump_offset > 0xE800) {
        g_chardump_offset = 0xD000;
    }
    redraw_chardump_screen();
}

// ----------------------------------------------------
// Term 128 Simulation
// ----------------------------------------------------
static void init_term128(void) {
    g_term128_state = 0;
    g_term128_connect_time = 0;
    strcpy(g_term128_buffer, "ATDT 555-4679\r\nDIALING...\r\n");
}

static void redraw_term128_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char header[512];
    snprintf(header, sizeof(header),
             "================================================================================\r\n"
             "    AHOY! TERM 128: MOS 8563 80-Column Telecommunications Simulator (Issue 27)   \r\n"
             "================================================================================\r\n"
             " Status: %s | Press [ESC] to hang up & exit\r\n"
             "--------------------------------------------------------------------------------\r\n",
             g_term128_state == 0 ? "DIALING BBS..." : "CONNECTED [2400 BAUD]");
    lau_vram_write_string(g_vram, header, strlen(header));

    // Print buffer
    lau_vram_write_string(g_vram, g_term128_buffer, strlen(g_term128_buffer));
}

static void handle_term128_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (g_term128_state == 1) {
        // Echo input characters
        size_t len = strlen(g_term128_buffer);
        if (len < sizeof(g_term128_buffer) - 2) {
            g_term128_buffer[len] = ch;
            g_term128_buffer[len + 1] = '\0';
        }
        redraw_term128_screen();
    }
}

static void update_term128(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 1000) return;
    last_tick = ms;

    if (g_term128_state == 0) {
        g_term128_connect_time++;
        if (g_term128_connect_time == 2) {
            strcat(g_term128_buffer, "CONNECT 2400\r\n");
            redraw_term128_screen();
        } else if (g_term128_connect_time >= 4) {
            g_term128_state = 1;
            strcat(g_term128_buffer,
                   "\r\n"
                   "****************************************\r\n"
                   "*        WELCOME TO ATROPA BBS         *\r\n"
                   "*   The premier 80-column C128 node    *\r\n"
                   "****************************************\r\n"
                   "READY FOR INPUT:\r\n> ");
            redraw_term128_screen();
        }
    }
}
// ----------------------------------------------------
// Alchemist's Apprentice Simulation
// ----------------------------------------------------
static void init_alchemist(void) {
    g_alchemist_has_flask = false;
    g_alchemist_has_powder = false;
    g_alchemist_potion_mixed = false;
    g_alchemist_input_len = 0;
    g_alchemist_input[0] = '\0';
    strcpy(g_alchemist_log, "You stand in the Alchemist's dusty lab. There is a crystal FLASK and a pouch of blue POWDER.\r\n");
}

static void redraw_alchemist_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    char header[512];
    snprintf(header, sizeof(header),
             "================================================================================\r\n"
             "                 AHOY! ALCHEMIST'S APPRENTICE (Issue 29 Simulator)              \r\n"
             "================================================================================\r\n"
             " Commands: LOOK, TAKE FLASK, TAKE POWDER, MIX, DRINK, ESC to exit\r\n"
             "--------------------------------------------------------------------------------\r\n\r\n");
    lau_vram_write_string(g_vram, header, strlen(header));
    lau_vram_write_string(g_vram, g_alchemist_log, strlen(g_alchemist_log));
    char prompt[128];
    snprintf(prompt, sizeof(prompt), "\r\n> %s", g_alchemist_input);
    lau_vram_write_string(g_vram, prompt, strlen(prompt));
}

static void handle_alchemist_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (ch == '\n' || ch == '\r') {
        g_alchemist_input[g_alchemist_input_len] = '\0';
        char cmd[64];
        strcpy(cmd, g_alchemist_input);
        g_alchemist_input_len = 0;
        g_alchemist_input[0] = '\0';

        // Verb-Noun Parsing
        if (strcasecmp(cmd, "look") == 0) {
            strcpy(g_alchemist_log, "The room smells of sulfur. You see a FLASK and blue POWDER. A warm mortar sits on the table.\r\n");
        } else if (strcasecmp(cmd, "take flask") == 0) {
            g_alchemist_has_flask = true;
            strcpy(g_alchemist_log, "You pick up the fragile crystal flask.\r\n");
        } else if (strcasecmp(cmd, "take powder") == 0) {
            g_alchemist_has_powder = true;
            strcpy(g_alchemist_log, "You carefully pack the blue powder in your pouch.\r\n");
        } else if (strcasecmp(cmd, "mix") == 0) {
            if (g_alchemist_has_flask && g_alchemist_has_powder) {
                g_alchemist_potion_mixed = true;
                strcpy(g_alchemist_log, "You combine the powder inside the flask. The mixture glows emerald green!\r\n");
            } else {
                strcpy(g_alchemist_log, "You need the flask and powder first!\r\n");
            }
        } else if (strcasecmp(cmd, "drink") == 0) {
            if (g_alchemist_potion_mixed) {
                strcpy(g_alchemist_log, "A rush of cosmic energy flows through you! YOU HAVE COMPLETED THE APPRENTICESHIP!\r\n");
            } else {
                strcpy(g_alchemist_log, "Drink what? You haven't mixed anything.\r\n");
            }
        } else {
            strcpy(g_alchemist_log, "I don't understand that command.\r\n");
        }
        redraw_alchemist_screen();
        return;
    }

    if (ch == 127 || ch == '\b') {
        if (g_alchemist_input_len > 0) {
            g_alchemist_input_len--;
            g_alchemist_input[g_alchemist_input_len] = '\0';
        }
    } else if (g_alchemist_input_len < 60 && ch >= 32) {
        g_alchemist_input[g_alchemist_input_len++] = ch;
        g_alchemist_input[g_alchemist_input_len] = '\0';
    }
    redraw_alchemist_screen();
}

// ----------------------------------------------------
// Ski Folly Simulation
// ----------------------------------------------------
static void init_skifolly(void) {
    g_skifolly_player_x = 40;
    g_skifolly_score = 0;
    g_skifolly_gate_x = 35;
    g_skifolly_gate_w = 14;
    g_skifolly_row_counter = 0;
    g_skifolly_crashed = false;
}

static void redraw_skifolly_screen(void) {
    // We write game lines row by row
    char line[128];
    memset(line, ' ', 80);
    line[80] = '\r';
    line[81] = '\n';
    line[82] = '\0';

    // Put gates
    for (int i = 0; i < 80; i++) {
        if (i < g_skifolly_gate_x || i > g_skifolly_gate_x + g_skifolly_gate_w) {
            if (i % 8 == 0) line[i] = '^'; // trees
        }
    }
    // Player
    line[g_skifolly_player_x] = g_skifolly_crashed ? '*' : 'Y';

    char status[128];
    snprintf(status, sizeof(status), "SKI FOLLY | SCORE: %d | Status: %s | (A/D to move, ESC to exit)\r\n", 
             g_skifolly_score, g_skifolly_crashed ? "CRASHED! Press SPACE to restart" : "Slalom descent");
    
    lau_vram_write_string(g_vram, status, strlen(status));
    lau_vram_write_string(g_vram, line, strlen(line));
}

static void handle_skifolly_input(char ch) {
    if (ch == 27) {
        g_editor_mode = MODE_TERMINAL;
        return;
    }
    if (g_skifolly_crashed) {
        if (ch == ' ') {
            init_skifolly();
            redraw_skifolly_screen();
        }
        return;
    }
    if (ch == 'a' || ch == 'A') {
        if (g_skifolly_player_x > 2) g_skifolly_player_x--;
    } else if (ch == 'd' || ch == 'D') {
        if (g_skifolly_player_x < 77) g_skifolly_player_x++;
    }
    redraw_skifolly_screen();
}

static void update_skifolly(uint32_t ms) {
    static uint32_t last_tick = 0;
    if (ms - last_tick < 150) return;
    last_tick = ms;

    if (g_skifolly_crashed) return;

    // Shift gate left or right randomly
    int shift = (rand() % 3) - 1;
    g_skifolly_gate_x += shift;
    if (g_skifolly_gate_x < 10) g_skifolly_gate_x = 10;
    if (g_skifolly_gate_x > 60) g_skifolly_gate_x = 60;

    // Check collision
    if (g_skifolly_player_x < g_skifolly_gate_x || g_skifolly_player_x > g_skifolly_gate_x + g_skifolly_gate_w) {
        g_skifolly_crashed = true;
    } else {
        g_skifolly_score += 10;
    }
    redraw_skifolly_screen();
}

// ----------------------------------------------------
// English Darts Simulation
// ----------------------------------------------------
static void init_darts(void) {
    g_darts_aim_x = 40;
    g_darts_aim_y = 12;
    g_darts_score = 0;
    g_darts_throws = 3;
    g_darts_state = 0;
    g_darts_aim_dir = 1;
}

static void redraw_darts_screen(void) {
    const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
    lau_vram_write_string(g_vram, clear_seq, 3);
    
    char hud[512];
    snprintf(hud, sizeof(hud), 
             "================================================================================\r\n"
             "        AHOY! ENGLISH DARTS | THROWS LEFT: %d | SCORE: %d\r\n"
             "================================================================================\r\n"
             " Status: %s | Press [SPACE] to lock aim, [ESC] to exit\r\n"
             "--------------------------------------------------------------------------------\r\n",
             g_darts_throws, g_darts_score,
             g_darts_state == 0 ? "AIMING HORIZONTAL (X)" : (g_darts_state == 1 ? "AIMING VERTICAL (Y)" : "THROWN!"));
    lau_vram_write_string(g_vram, hud, strlen(hud));

    // Render simple ASCII dartboard
    for (int y = 0; y < 15; y++) {
        char line[128];
        memset(line, ' ', 80);
        line[80] = '\r';
        line[81] = '\n';
        line[82] = '\0';
        
        for (int x = 0; x < 80; x++) {
            // Draw Target Rings
            float dx = (x - 40) * 0.5f;
            float dy = (y - 7) * 1.0f;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 1.5f) line[x] = 'X';      // Bullseye
            else if (dist < 4.0f) line[x] = 'O'; // Inner Ring
            else if (dist < 8.0f) line[x] = '#'; // Outer Ring
            
