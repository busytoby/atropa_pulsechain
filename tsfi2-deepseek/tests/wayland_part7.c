         const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
         lau_vram_write_string(g_vram, clear_seq, 3);
         const char *header = 
             "==================================================\r\n"
             "        INSTA-CALC C64 SPREADSHEET (CIMARRON)     \r\n"
             "==================================================\r\n"
             "  - CARTRIDGE ACTIVE - CELLS AVAILABLE: 256       \r\n"
             "  - MODE: ENTRY MODE  - RETRIEVAL PORT: $D630     \r\n"
             "==================================================\r\n"
             " [Press ESC to return to Terminal Menu]           \r\n\r\n"
             "    A         B         C         D         E     \r\n"
             " 1  [100.00]  [50.00]   [150.00]  [0.00]    [ ]   \r\n"
             " 2  [20.00]   [30.00]   [50.00]   [0.00]    [ ]   \r\n"
             " 3  [120.00]  [80.00]   [200.00]  [0.00]    [ ]   \r\n"
             " 4  [ ]       [ ]       [ ]       [ ]       [ ]   \r\n"
             " 5  [ ]       [ ]       [ ]       [ ]       [ ]   \r\n\r\n"
             " Entry: A1 = ";
         lau_vram_write_string(g_vram, header, strlen(header));
         log_telemetry("Rendered Insta-Calc Screen");
         return;
    }
    
    if (first_word && (strcasecmp(first_word, "PANIC") == 0 || strcasecmp(first_word, "APPLEPANIC") == 0)) {
         g_editor_mode = MODE_APPLEPANIC;
         g_mercenary_active = false;
         g_pong_active = false;
         g_applepanic_active = true;
         g_panic_player_x = 2;
         g_panic_player_y = 3;
         g_panic_monster_x = 25;
         g_panic_monster_y = 3;
         g_panic_monster_stuck_ticks = 0;
         g_panic_score = 0;
         g_panic_lives = 3;
         memset(g_panic_dig_ticks, 0, sizeof(g_panic_dig_ticks));
         redraw_applepanic_screen();
         log_telemetry("Rendered Apple Panic Screen");
         return;
    }
    
    if (first_word && (strcasecmp(first_word, "SLINKY") == 0 || strcasecmp(first_word, "SLINKYBEAR") == 0)) {
         g_editor_mode = MODE_SLINKYBEAR;
         g_mercenary_active = false;
         g_pong_active = false;
         g_slinkybear_active = true;
         g_slinky_row = 0;
         g_slinky_col = 0;
         g_slinky_score = 0;
         g_slinky_lives = 3;
         memset(g_slinky_blocks, 0, sizeof(g_slinky_blocks));
         redraw_slinkybear_screen();
         log_telemetry("Rendered Slinky the Bear Screen");
    }
    
    if (first_word && strcasecmp(first_word, "SLINKYPANIC") == 0) {
         g_editor_mode = MODE_SLINKYPANIC;
         g_mercenary_active = false;
         g_pong_active = false;
         g_slinkypanic_active = true;
         g_slinky_row = 0;
         g_slinky_col = 0;
         g_slinky_monster_row = 4;
         g_slinky_monster_col = 4;
         g_slinky_monster_stuck = 0;
         g_slinky_score = 0;
         g_slinky_lives = 3;
         memset(g_slinky_blocks, 0, sizeof(g_slinky_blocks));
         memset(g_slinky_hole, 0, sizeof(g_slinky_hole));
         redraw_slinkypanic_screen();
         log_telemetry("Rendered Slinky Panic Screen");
         return;
    }
    
    if (first_word && strcasecmp(first_word, "ALPINER") == 0 && !g_aitest_active) {
         g_editor_mode = MODE_ALPINER;
         g_mercenary_active = false;
         g_pong_active = false;
         g_alpiner_active = true;
         g_alpiner_player_x = 22;
         g_alpiner_player_y = 12;
         g_alpiner_yeti_x = 18;
         g_alpiner_yeti_y = 6;
         g_alpiner_rock_x = 22;
         g_alpiner_rock_y = 4;
         g_alpiner_score = 0;
         g_alpiner_lives = 3;
         g_alpiner_mountain = 1;
         trigger_alpiner_speech("ALPINER!");
         redraw_alpiner_screen();
         log_telemetry("Rendered Alpiner Screen");
         return;
    }

    if (first_word && (strcasecmp(first_word, "WHATSMYJOB") == 0 || strcasecmp(first_word, "JOBS") == 0)) {
         g_editor_mode = MODE_WHATSMYJOB;
         g_mercenary_active = false;
         g_pong_active = false;
         g_jobs_active = true;
         init_job_game();
         redraw_job_screen();
         log_telemetry("Started What's My Job");
         return;
    }

    if (first_word && strcasecmp(first_word, "BIND") == 0) {
         char *p = cmd_buf + 4;
         while (*p == ' ' || *p == '\t') p++;
         if (*p && *(p+1) == ' ') {
             char k = *p;
             char *c = p + 2;
             while (*c == ' ' || *c == '\t') c++;
             if (*c) {
                 bool found = false;
                 for (int i = 0; i < g_chrget_hooks_count; i++) {
                     if (g_chrget_hooks[i].key == k) {
                         snprintf(g_chrget_hooks[i].cmd, sizeof(g_chrget_hooks[i].cmd), "%s", c);
                         found = true;
                         break;
                     }
                 }
                 if (!found && g_chrget_hooks_count < 16) {
                     g_chrget_hooks[g_chrget_hooks_count].key = k;
                     snprintf(g_chrget_hooks[g_chrget_hooks_count].cmd, sizeof(g_chrget_hooks[g_chrget_hooks_count].cmd), "%s", c);
                     g_chrget_hooks_count++;
                 }
                 char msg[128];
                 snprintf(msg, sizeof(msg), "CHRGET: Bound key '%c' to command '%s'\r\n", k, c);
                 lau_vram_write_string(g_vram, msg, strlen(msg));
                 return;
             }
         }
         lau_vram_write_string(g_vram, "Usage: BIND <char> <command>\r\n", 29);
         return;
    }

    if (first_word && strcasecmp(first_word, "NEW") == 0) {
        FILE *src = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/basic_program.txt", "r");
        if (src) {
            FILE *bak = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/basic_program_bak.txt", "w");
            if (bak) {
                char ch;
                while ((ch = fgetc(src)) != EOF) {
                    fputc(ch, bak);
                }
                fclose(bak);
            }
            fclose(src);
        }
        FILE *empty = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/basic_program.txt", "w");
        if (empty) {
            fclose(empty);
        }
        lau_vram_write_string(g_vram, "\r\nREADY.\r\n", 10);
        return;
    }

    if (first_word && strcasecmp(first_word, "OLD") == 0) {
        FILE *bak = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/basic_program_bak.txt", "r");
        if (bak) {
            FILE *dest = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/basic_program.txt", "w");
            if (dest) {
                char ch;
                while ((ch = fgetc(bak)) != EOF) {
                    fputc(ch, dest);
                }
                fclose(dest);
                lau_vram_write_string(g_vram, "\r\nBASIC program restored successfully.\r\nREADY.\r\n", 49);
            } else {
                lau_vram_write_string(g_vram, "\r\nError: Could not write file.\r\nREADY.\r\n", 40);
            }
            fclose(bak);
        } else {
            lau_vram_write_string(g_vram, "\r\nError: No deleted BASIC program backup found to restore.\r\nREADY.\r\n", 68);
        }
        return;
    }

    if (first_word && strcasecmp(first_word, "ASM_FILE") == 0) {
        char *path = cmd_buf + 8;
        while (*path == ' ' || *path == '\t') path++;
        if (strlen(path) == 0) {
            lau_vram_write_string(g_vram, "Usage: ASM_FILE <path_to_assembly_script>\r\n", 43);
            return;
        }
        FILE *af = fopen(path, "r");
        if (!af) {
            lau_vram_write_string(g_vram, "Error: Could not open assembly script file.\r\n", 45);
            return;
        }
        char line[128];
        int l_count = 0;
        while (fgets(line, sizeof(line), af)) {
            // Prune trailing newlines
            size_t len = strlen(line);
            while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
                line[len-1] = '\0';
                len--;
            }
            // Skip comments or empty lines
            if (line[0] == ';' || line[0] == '\0') continue;
            
            // Check for symbol definition directive (e.g. .define KEY $E000)
            if (line[0] == '.') {
                char dir_name[32] = {0};
                char sym_name[32] = {0};
                char sym_val[32] = {0};
                if (sscanf(line, "%31s %31s %31s", dir_name, sym_name, sym_val) == 3) {
                    if (strcasecmp(dir_name, ".define") == 0 && g_flankspeed_symbol_count < 128) {
                        snprintf(g_flankspeed_symbols[g_flankspeed_symbol_count].name, sizeof(g_flankspeed_symbols[g_flankspeed_symbol_count].name), "%s", sym_name);
                        snprintf(g_flankspeed_symbols[g_flankspeed_symbol_count].value, sizeof(g_flankspeed_symbols[g_flankspeed_symbol_count].value), "%s", sym_val);
                        g_flankspeed_symbol_count++;
                    }
                }
                continue;
            }
            
            int line_num = l_count + 1;
            if (!flankspeed_assemble(line)) {
                char err_msg[128];
                snprintf(err_msg, sizeof(err_msg), "❌ ASM_FILE Error: Invalid instruction syntax on line %d: '%s'\r\n", line_num, line);
                lau_vram_write_string(g_vram, err_msg, strlen(err_msg));
                fclose(af);
                return;
            }
            l_count++;
        }
        fclose(af);
        char ret_msg[128];
        snprintf(ret_msg, sizeof(ret_msg), "ASM_FILE: Successfully assembled %d instructions to RAM (Active dynamic symbols: %d).\r\n", l_count, g_flankspeed_symbol_count);
        lau_vram_write_string(g_vram, ret_msg, strlen(ret_msg));
        return;
    }

    if (first_word && strcasecmp(first_word, "RENUMBER") == 0) {
         char *p = cmd_buf + 8;
         while (*p == ' ' || *p == '\t') p++;
         execute_renumber(p);
         return;
    }

    if (first_word && strcasecmp(first_word, "MINDER") == 0) {
         char *p = cmd_buf + 6;
         while (*p == ' ' || *p == '\t') p++;
         execute_minder(p);
         return;
    }
    if (first_word && strcasecmp(first_word, "BASE") == 0) {
         char *p = cmd_buf + 4;
         while (*p == ' ' || *p == '\t') p++;
         execute_base(p);
         return;
    }
    if (first_word && strcasecmp(first_word, "COLS") == 0) {
         char *p = cmd_buf + 4;
         while (*p == ' ' || *p == '\t') p++;
         execute_cols(p);
         return;
    }

    if (first_word && strcasecmp(first_word, "BASECONVERSIONS") == 0) {
         execute_command("node ../scripts/test_baseconversions.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "SALVAGEDIVER") == 0) {
         execute_command("node ../scripts/test_salvagediver.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "DOS") == 0) {
         execute_command("node ../scripts/test_dos.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "SOUNDEXPLORER") == 0) {
         execute_command("node ../scripts/test_soundexplorer.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "CASTLEDARKNESS") == 0) {
         execute_command("node ../scripts/test_castledarkness.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "LAWNJOB") == 0) {
         execute_command("node ../scripts/test_lawnjob.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "EMERALDELEPHANT") == 0) {
         execute_command("node ../scripts/test_emeraldelephant.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "VIC40OS") == 0) {
         execute_command("node ../scripts/test_vic40os.js");
         return;
    }
    if (first_word && strcasecmp(first_word, "BAMREADPRINT") == 0) {
         execute_command("node ../scripts/test_bamreadprint.js");
         return;
    }

    if (first_word && strcasecmp(first_word, "PTE") == 0) {
         g_editor_mode = MODE_PTE;
         g_mercenary_active = false;
         g_pong_active = false;
         init_pte();
         redraw_pte_screen();
         log_telemetry("Rendered PTE Word Processor Screen");
         return;
    }

    if (first_word && strcasecmp(first_word, "SPACEPATROL") == 0) {
         g_editor_mode = MODE_SPACEPATROL;
         g_mercenary_active = false;
         g_pong_active = false;
         init_spacepatrol();
         redraw_spacepatrol_screen();
         log_telemetry("Rendered Space Patrol Screen");
         return;
    }

    if (first_word && strcasecmp(first_word, "CONSTRUCTIONCO") == 0) {
         g_editor_mode = MODE_CONSTRUCTION_CO;
         g_mercenary_active = false;
         g_pong_active = false;
         init_construction_co();
         redraw_construction_co_screen();
         log_telemetry("Rendered Construction Co Screen");
         return;
    }

    if (first_word && strcasecmp(first_word, "STUDIO64") == 0) {
         g_editor_mode = MODE_STUDIO64;
         g_mercenary_active = false;
         g_pong_active = false;
         init_studio64();
         redraw_studio64_screen();
         log_telemetry("Rendered Studio 64 Screen");
         return;
    }

    if (first_word && strcasecmp(first_word, "MAGPIE") == 0) {
         g_editor_mode = MODE_MAGPIE;
         g_mercenary_active = false;
         g_pong_active = false;
         init_magpie();
         redraw_magpie_screen();
         log_telemetry("Rendered Magpie Database Screen");
         return;
    }

    if (first_word && strcasecmp(first_word, "ALICE") == 0) {
         g_editor_mode = MODE_ALICE;
         g_mercenary_active = false;
         g_pong_active = false;
         init_alice();
         redraw_alice_screen();
         log_telemetry("Started Alice in Adventureland");
         return;
    }

    if (first_word && (strcasecmp(first_word, "TOP") == 0 || strcasecmp(first_word, "TOTHETOP") == 0)) {
         g_editor_mode = MODE_TOP;
         g_mercenary_active = false;
         g_pong_active = false;
         init_top();
         redraw_top_screen();
         log_telemetry("Started To the Top");
         return;
    }

    if (first_word && strcasecmp(first_word, "FONTASIA") == 0) {
         g_editor_mode = MODE_FONTASIA;
         g_mercenary_active = false;
         g_pong_active = false;
         init_fontasia();
         redraw_fontasia_screen();
         log_telemetry("Started Fontasia Character Editor");
         return;
    }

    if (first_word && strcasecmp(first_word, "FLANKSPEED") == 0) {
         g_editor_mode = MODE_FLANKSPEED;
         g_mercenary_active = false;
         g_pong_active = false;
         init_flankspeed();
         redraw_flankspeed_screen();
         log_telemetry("Started Flankspeed ML editor");
         return;
    }

    if (first_word && strcasecmp(first_word, "BOOTER") == 0) {
         g_editor_mode = MODE_BOOTER;
         g_mercenary_active = false;
         g_pong_active = false;
         init_booter();
         redraw_booter_screen();
         log_telemetry("Started Booter Disk Menu Auto-Loader");
         return;
    }

    if (first_word && strcasecmp(first_word, "HOPAROUND") == 0) {
         g_editor_mode = MODE_HOPAROUND;
         g_mercenary_active = false;
         g_pong_active = false;
         init_hoparound();
         redraw_hoparound_screen();
         log_telemetry("Started Hop Around game");
         return;
    }

    if (first_word && strcasecmp(first_word, "TOWERS") == 0) {
         g_editor_mode = MODE_TOWERS;
         g_mercenary_active = false;
         g_pong_active = false;
         init_towers();
         redraw_towers_screen();
         log_telemetry("Started Towers of Hanoi game");
         return;
    }

    if (first_word && strcasecmp(first_word, "DISINTEGRATOR") == 0) {
         g_editor_mode = MODE_DISINTEGRATOR;
         g_mercenary_active = false;
         g_pong_active = false;
         init_disintegrator();
         redraw_disintegrator_screen();
         log_telemetry("Started Disintegrator game");
         return;
    }

    if (first_word && strcasecmp(first_word, "FIDGITS") == 0) {
         g_editor_mode = MODE_FIDGITS;
         g_mercenary_active = false;
         g_pong_active = false;
         init_fidgits();
         redraw_fidgits_screen();
         log_telemetry("Started Fidgits game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "MOXEY") == 0 || strcasecmp(first_word, "MOXEYSPORCH") == 0)) {
         g_editor_mode = MODE_MOXEY;
         g_mercenary_active = false;
         g_pong_active = false;
         init_moxey();
         redraw_moxey_screen();
         log_telemetry("Started Moxey's Porch game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "DRUM") == 0 || strcasecmp(first_word, "RHYTHMIC") == 0)) {
         g_editor_mode = MODE_DRUM;
         g_mercenary_active = false;
         g_pong_active = false;
         init_drum();
         redraw_drum_screen();
         log_telemetry("Started Rhythmic Bits drum sequencer");
         return;
    }

    if (first_word && (strcasecmp(first_word, "JEWEL") == 0 || strcasecmp(first_word, "JEWELQUEST") == 0)) {
         g_editor_mode = MODE_JEWEL;
         g_mercenary_active = false;
         g_pong_active = false;
         init_jewel();
         redraw_jewel_screen();
         log_telemetry("Started Jewel Quest game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "SANTA") == 0 || strcasecmp(first_word, "SANTASBUSYDAY") == 0)) {
         g_editor_mode = MODE_SANTA;
         g_mercenary_active = false;
         g_pong_active = false;
         init_santa();
         redraw_santa_screen();
         log_telemetry("Started Santa's Busy Day game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "CLOAK") == 0)) {
         g_editor_mode = MODE_CLOAK;
         g_mercenary_active = false;
         g_pong_active = false;
         init_cloak();
         redraw_cloak_screen();
         log_telemetry("Started Cloak game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "GYPSY") == 0 || strcasecmp(first_word, "GYPSYSTARSHIP") == 0)) {
         g_editor_mode = MODE_GYPSY;
         g_mercenary_active = false;
         g_pong_active = false;
         init_gypsy();
         redraw_gypsy_screen();
         log_telemetry("Started Gypsy Starship game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "MARTIAN") == 0 || strcasecmp(first_word, "MARTIANMONSTERS") == 0)) {
         g_editor_mode = MODE_MARTIAN;
         g_mercenary_active = false;
         g_pong_active = false;
         init_martian();
         redraw_martian_screen();
         log_telemetry("Started Martian Monsters game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "HAUNTED") == 0 || strcasecmp(first_word, "HAUNTEDCASTLE") == 0)) {
         g_editor_mode = MODE_HAUNTED;
         g_mercenary_active = false;
         g_pong_active = false;
         init_haunted();
         redraw_haunted_screen();
         log_telemetry("Started Haunted Castle game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "INFRARAID") == 0)) {
         g_editor_mode = MODE_INFRARAID;
         g_mercenary_active = false;
         g_pong_active = false;
         init_infraraid();
         redraw_infraraid_screen();
         log_telemetry("Started Infraraid simulation");
         return;
    }

    if (first_word && (strcasecmp(first_word, "STREAMER") == 0 || strcasecmp(first_word, "STREAMERFONT") == 0)) {
         g_editor_mode = MODE_STREAMER;
         g_mercenary_active = false;
         g_pong_active = false;
         init_streamer();
         redraw_streamer_screen();
         log_telemetry("Started Streamer Font editor");
         return;
    }

    if (first_word && (strcasecmp(first_word, "KNOCKOUT") == 0)) {
         g_editor_mode = MODE_KNOCKOUT;
         g_mercenary_active = false;
         g_pong_active = false;
         init_knockout();
         redraw_knockout_screen();
         log_telemetry("Started Knockout game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "ALARM") == 0 || strcasecmp(first_word, "ALARMCLOCK") == 0)) {
         g_editor_mode = MODE_ALARM;
         g_mercenary_active = false;
         g_pong_active = false;
         init_alarm();
         redraw_alarm_screen();
         log_telemetry("Started Alarm Clock utility");
         return;
    }

    if (first_word && (strcasecmp(first_word, "MEMCHECK") == 0 || strcasecmp(first_word, "MEMORYCHECK") == 0)) {
         g_editor_mode = MODE_MEMCHECK;
         g_mercenary_active = false;
         g_pong_active = false;
         init_memcheck();
         redraw_memcheck_screen();
         log_telemetry("Started Memory Check utility");
         return;
    }

    if (first_word && (strcasecmp(first_word, "ARENA") == 0)) {
         g_editor_mode = MODE_ARENA;
         g_mercenary_active = false;
         g_pong_active = false;
         init_arena();
         redraw_arena_screen();
         log_telemetry("Started Arena game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "HEADTOHEAD") == 0 || strcasecmp(first_word, "H2H") == 0)) {
         g_editor_mode = MODE_HEADTOHEAD;
         g_mercenary_active = false;
         g_pong_active = false;
         init_headtohead();
         redraw_headtohead_screen();
         log_telemetry("Started Head to Head game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "CRABFIGHT") == 0)) {
         g_editor_mode = MODE_CRABFIGHT;
         g_mercenary_active = false;
         g_pong_active = false;
         init_crabfight();
         redraw_crabfight_screen();
         log_telemetry("Started Crabfight game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "TREASURE") == 0 || strcasecmp(first_word, "TREASUREWHEEL") == 0)) {
         g_editor_mode = MODE_TREASURE;
         g_mercenary_active = false;
         g_pong_active = false;
         init_treasure();
         redraw_treasure_screen();
         log_telemetry("Started Treasure Wheel game");
         return;
    }

    if (first_word && (strcasecmp(first_word, "CHARDUMP") == 0 || strcasecmp(first_word, "CHARACTERDUMP") == 0)) {
         g_editor_mode = MODE_CHARDUMP;
         g_mercenary_active = false;
         g_pong_active = false;
         init_chardump();
         redraw_chardump_screen();
         log_telemetry("Started Character Dump utility");
         return;
    }

    if (first_word && (strcasecmp(first_word, "TERM128") == 0 || strcasecmp(first_word, "TERM") == 0)) {
         g_editor_mode = MODE_TERM128;
         g_mercenary_active = false;
         g_pong_active = false;
         init_term128();
         redraw_term128_screen();
         log_telemetry("Started Term 128 simulator");
         return;
    }

    if (first_word && (strcasecmp(first_word, "ALCHEMIST") == 0 || strcasecmp(first_word, "APPRENTICE") == 0)) {
         g_editor_mode = MODE_ALCHEMIST;
         g_mercenary_active = false;
         g_pong_active = false;
         init_alchemist();
         redraw_alchemist_screen();
         log_telemetry("Started Alchemist's Apprentice");
         return;
    }

    if (first_word && (strcasecmp(first_word, "SKIFOLLY") == 0 || strcasecmp(first_word, "SKI") == 0)) {
         g_editor_mode = MODE_SKIFOLLY;
         g_mercenary_active = false;
         g_pong_active = false;
         init_skifolly();
         redraw_skifolly_screen();
         log_telemetry("Started Ski Folly");
         return;
    }

    if (first_word && (strcasecmp(first_word, "DARTS") == 0 || strcasecmp(first_word, "DART") == 0)) {
         g_editor_mode = MODE_DARTS;
         g_mercenary_active = false;
         g_pong_active = false;
         init_darts();
         redraw_darts_screen();
         log_telemetry("Started English Darts");
         return;
    }

    if (first_word && strcasecmp(first_word, "MAKEWAVE") == 0) {
         g_editor_mode = MODE_MAKEWAVE;
         g_mercenary_active = false;
         g_pong_active = false;
         init_makewave();
         redraw_makewave_screen();
         log_telemetry("Started MAKEWAVE audio table tool");
         return;
    }

    if (first_word && strcasecmp(first_word, "PLOTWAVE") == 0) {
         g_editor_mode = MODE_PLOTWAVE;
         g_mercenary_active = false;
         g_pong_active = false;
         init_plotwave();
         redraw_plotwave_screen();
         log_telemetry("Started PLOTWAVE oscilloscope utility");
         return;
    }

    if (first_word && (strcasecmp(first_word, "FASTER64") == 0 || strcasecmp(first_word, "FAST64") == 0)) {
         g_faster64_active = !g_faster64_active;
         if (g_faster64_active) {
             lau_vram_write_string(g_vram, "\r\nFaster 64! system accelerator is now ENABLED.\r\nREADY.\r\n", 56);
         } else {
             lau_vram_write_string(g_vram, "\r\nFaster 64! system accelerator is now DISABLED.\r\nREADY.\r\n", 57);
         }
         return;
    }

    if (first_word && strcasecmp(first_word, "SIM6510") == 0) {
        lau_vram_write_string(g_vram, "\r\n--- 6510 PROVISIONING & BANK-SWITCHING SIMULATION PROOF ---\r\n", 63);
        lau_vram_write_string(g_vram, "Configuring Data Direction Register ($0000) with $07...\r\n", 57);
        
        uint8_t ram[65536];
        uint8_t rom[65536];
        memset(ram, 0, sizeof(ram));
        memset(rom, 0, sizeof(rom));
        
        rom[0xE000] = 0xAA; // KERNAL ROM ID Signature
        ram[0xE000] = 0x00;
        
        uint8_t reg_port = 0x07; // Default ROMs mapped in
        char buf[256];
        snprintf(buf, sizeof(buf), "Initial Port register ($0001) state: $%02X (ROM enabled)\r\n", reg_port);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        
        uint8_t val1 = (reg_port & 0x01) ? rom[0xE000] : ram[0xE000];
        snprintf(buf, sizeof(buf), "Read $E000 -> Value: $%02X (ROM Bank)\r\n", val1);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        
        lau_vram_write_string(g_vram, "Poking $0001 with $06 to switch ROM off (RAM visible)...\r\n", 58);
        reg_port = 0x06;
        
        lau_vram_write_string(g_vram, "Poking $E000 with $BB...\r\n", 26);
        ram[0xE000] = 0xBB;
        
        uint8_t val2 = (reg_port & 0x01) ? rom[0xE000] : ram[0xE000];
        snprintf(buf, sizeof(buf), "Read $E000 -> Value: $%02X (RAM Bank)\r\n", val2);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        
        lau_vram_write_string(g_vram, "Poking $0001 with $07 to switch ROM bank back on...\r\n", 53);
        reg_port = 0x07;
        
        uint8_t val3 = (reg_port & 0x01) ? rom[0xE000] : ram[0xE000];
        snprintf(buf, sizeof(buf), "Read $E000 -> Value: $%02X (ROM Bank restored)\r\n", val3);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        
        if (val1 == 0xAA && val2 == 0xBB && val3 == 0xAA) {
            lau_vram_write_string(g_vram, "\r\nSTATUS: 6510 BANK-SWITCHING VERIFIED SUCCESS!\r\nREADY.\r\n", 57);
        } else {
            lau_vram_write_string(g_vram, "\r\nSTATUS: SIMULATION FAILED.\r\nREADY.\r\n", 37);
        }
        return;
    }

    if (first_word && strcasecmp(first_word, "DIR") == 0) {
        DIR *d = opendir(".");
        if (!d) {
            lau_vram_write_string(g_vram, "\r\nError: Could not open directory.\r\nREADY.\r\n", 44);
            return;
        }
        
        lau_vram_write_string(g_vram, "\r\n0 \"WORKSPACE\" 2A\r\n", 21);
        struct dirent *dir;
        char file_list[128][64];
        long file_sizes[128];
        int f_count = 0;
        
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] == '.') continue; // Skip dotfiles
            struct stat st;
            if (stat(dir->d_name, &st) == 0) {
                if (S_ISREG(st.st_mode)) {
                    snprintf(file_list[f_count], sizeof(file_list[f_count]), "%.63s", dir->d_name);
                    file_sizes[f_count] = st.st_size;
                    f_count++;
                    if (f_count >= 128) break;
                }
            }
        }
        closedir(d);
        
        // Print files in two columns
        for (int i = 0; i < f_count; i += 2) {
            char line[128];
            char col1[64] = "";
            char col2[64] = "";
            
            // Format first column: e.g. '12 "MYFILE" PRG' (using blocks of 254 bytes)
            long blocks1 = (file_sizes[i] + 253) / 254;
            snprintf(col1, sizeof(col1), "%-4ld \"%-16.16s\" PRG", blocks1, file_list[i]);
            
            if (i + 1 < f_count) {
                long blocks2 = (file_sizes[i+1] + 253) / 254;
                snprintf(col2, sizeof(col2), "%-4ld \"%-16.16s\" PRG", blocks2, file_list[i+1]);
            }
            
            snprintf(line, sizeof(line), "%-32s    %s\r\n", col1, col2);
            lau_vram_write_string(g_vram, line, strlen(line));
        }
        
        lau_vram_write_string(g_vram, "READY.\r\n", 8);
        return;
    }

    if (first_word && (strcasecmp(first_word, "COPY") == 0)) {
        char *src = strtok(NULL, " \t");
        char *dst = strtok(NULL, " \t");
        if (!src || !dst) {
            lau_vram_write_string(g_vram, "\r\nUsage: COPY <source> <destination>\r\nREADY.\r\n", 46);
            return;
        }
        FILE *sf = fopen(src, "rb");
        if (!sf) {
            lau_vram_write_string(g_vram, "\r\nError: Source file not found.\r\nREADY.\r\n", 40);
            return;
        }
        FILE *df = fopen(dst, "wb");
        if (!df) {
            fclose(sf);
            lau_vram_write_string(g_vram, "\r\nError: Could not open destination.\r\nREADY.\r\n", 46);
            return;
        }
        char copy_buf[1024];
        size_t bytes;
        while ((bytes = fread(copy_buf, 1, sizeof(copy_buf), sf)) > 0) {
            fwrite(copy_buf, 1, bytes, df);
        }
        fclose(sf);
        fclose(df);
        lau_vram_write_string(g_vram, "\r\nFile copied successfully.\r\nREADY.\r\n", 37);
        return;
    }

    if (first_word && (strcasecmp(first_word, "RENAME") == 0 || strcasecmp(first_word, "REN") == 0)) {
        char *oldname = strtok(NULL, " \t");
        char *newname = strtok(NULL, " \t");
        if (!oldname || !newname) {
            lau_vram_write_string(g_vram, "\r\nUsage: RENAME <oldname> <newname>\r\nREADY.\r\n", 45);
            return;
        }
        if (rename(oldname, newname) == 0) {
            lau_vram_write_string(g_vram, "\r\nFile renamed successfully.\r\nREADY.\r\n", 38);
        } else {
            lau_vram_write_string(g_vram, "\r\nError: Rename failed.\r\nREADY.\r\n", 33);
        }
        return;
    }

    if (first_word && (strcasecmp(first_word, "DELETE") == 0 || strcasecmp(first_word, "DEL") == 0)) {
        char *filename = strtok(NULL, " \t");
        if (!filename) {
            lau_vram_write_string(g_vram, "\r\nUsage: DELETE <filename>\r\nREADY.\r\n", 37);
            return;
        }
        if (unlink(filename) == 0) {
            lau_vram_write_string(g_vram, "\r\nFile deleted successfully.\r\nREADY.\r\n", 38);
        } else {
            lau_vram_write_string(g_vram, "\r\nError: Delete failed.\r\nREADY.\r\n", 33);
        }
        return;
    }

    if (first_word && strcasecmp(first_word, "CHECKLIST") == 0) {
         g_editor_mode = MODE_CHECKLIST;
         g_mercenary_active = false;
         g_pong_active = false;
         g_checklist_active = true;
         init_checklist();
         redraw_checklist_screen();
         log_telemetry("Rendered Checklist Screen");
         return;
    }

    if (first_word && (strcasecmp(first_word, "ADVENTURE") == 0 || strcasecmp(first_word, "ZMACHINE") == 0)) {
        g_editor_mode = MODE_ZMACHINE;
        g_mercenary_active = false;
        g_pong_active = false;
        
        // Initialize the zmachine Yul contract
        tsfi_zmm_vm_exec(&vm, "YULINIT \"zmachine\", \"../solidity/bin/zmachine.yul\", 5");
        
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        const char *header = 
            "==================================================\r\n"
            "       ON-CHAIN Z-MACHINE TEXT ADVENTURE          \r\n"
            "==================================================\r\n"
            " [Press ESC to return to Terminal Menu]          \r\n\r\n"
            "  You are standing in the lobby.\r\n"
            "zmachine> ";
        lau_vram_write_string(g_vram, header, strlen(header));
        log_telemetry("Started On-Chain Z-Machine Text Adventure");
        return;
    }
    
    if (first_word && strcasecmp(first_word, "GO") == 0) {
        char *target = strtok(NULL, " \t");
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        
        if (g_aitest_active && target) {
            if (strcmp(target, "1") == 0) { first_word = "CHOPLIFTER"; cmd = "CHOPLIFTER"; target = NULL; }
            else if (strcmp(target, "2") == 0) { first_word = "FORTAPOCALYPSE"; cmd = "FORTAPOCALYPSE"; target = NULL; }
            else if (strcmp(target, "3") == 0) { first_word = "HOMEWORD"; cmd = "HOMEWORD"; target = NULL; }
            else if (strcmp(target, "4") == 0) { first_word = "HOMETAX"; cmd = "HOMETAX"; target = NULL; }
            else if (strcmp(target, "5") == 0) { first_word = "GTIACOL"; cmd = "GTIACOL"; target = NULL; }
            else if (strcmp(target, "6") == 0) { first_word = "SEGAVDP"; cmd = "SEGAVDP"; target = NULL; }
            else if (strcmp(target, "7") == 0) { first_word = "SATURNVDP"; cmd = "SATURNVDP"; target = NULL; }
            else if (strcmp(target, "8") == 0) { first_word = "WORDPAC"; cmd = "WORDPAC"; target = NULL; }
            else if (strcmp(target, "9") == 0) { first_word = "DATAPAC"; cmd = "DATAPAC"; target = NULL; }
            else if (strcmp(target, "10") == 0) { first_word = "PROTECTO"; cmd = "PROTECTO"; target = NULL; }
            else if (strcmp(target, "11") == 0) { first_word = "MICROMINDER"; cmd = "MICROMINDER"; target = NULL; }
            else if (strcmp(target, "12") == 0) { first_word = "SALVAGEDIVER"; cmd = "SALVAGEDIVER"; target = NULL; }
            else if (strcmp(target, "13") == 0) { first_word = "DOS"; cmd = "DOS"; target = NULL; }
            else if (strcmp(target, "14") == 0) { first_word = "SOUNDEXPLORER"; cmd = "SOUNDEXPLORER"; target = NULL; }
            else if (strcmp(target, "15") == 0) { first_word = "CASTLEDARKNESS"; cmd = "CASTLEDARKNESS"; target = NULL; }
            else if (strcmp(target, "16") == 0) { first_word = "BASECONVERSIONS"; cmd = "BASECONVERSIONS"; target = NULL; }
            else if (strcmp(target, "17") == 0) { first_word = "LAWNJOB"; cmd = "LAWNJOB"; target = NULL; }
            else if (strcmp(target, "18") == 0) { first_word = "EMERALDELEPHANT"; cmd = "EMERALDELEPHANT"; target = NULL; }
            else if (strcmp(target, "19") == 0) { first_word = "VIC40OS"; cmd = "VIC40OS"; target = NULL; }
            else if (strcmp(target, "20") == 0) { first_word = "BAMREADPRINT"; cmd = "BAMREADPRINT"; target = NULL; }
            else if (strcmp(target, "21") == 0) { first_word = "TUNNEL"; cmd = "TUNNEL"; target = NULL; }
            else if (strcmp(target, "22") == 0) { first_word = "PTE"; cmd = "PTE"; target = NULL; }
            else if (strcmp(target, "23") == 0) { first_word = "BLOCKEDIT"; cmd = "BLOCKEDIT"; target = NULL; }
            else if (strcmp(target, "24") == 0) { first_word = "CHARSET"; cmd = "CHARSET"; target = NULL; }
            else if (strcmp(target, "25") == 0) { first_word = "TESTALL"; cmd = "TESTALL"; target = NULL; }
            else if (strcmp(target, "26") == 0) { first_word = "GYPSY"; cmd = "GYPSY"; target = NULL; }
            else if (strcmp(target, "27") == 0) { target = "MENU"; }
        } else if (g_dashboard_active && target) {
            if (strcmp(target, "1") == 0) { first_word = "CHOPLIFTER"; cmd = "CHOPLIFTER"; target = NULL; }
            else if (strcmp(target, "2") == 0) { first_word = "FORTAPOCALYPSE"; cmd = "FORTAPOCALYPSE"; target = NULL; }
            else if (strcmp(target, "3") == 0) { first_word = "HOMEWORD"; cmd = "HOMEWORD"; target = NULL; }
            else if (strcmp(target, "4") == 0) { first_word = "HOMETAX"; cmd = "HOMETAX"; target = NULL; }
            else if (strcmp(target, "5") == 0) { first_word = "GTIACOL"; cmd = "GTIACOL"; target = NULL; }
            else if (strcmp(target, "6") == 0) { first_word = "SEGAVDP"; cmd = "SEGAVDP"; target = NULL; }
            else if (strcmp(target, "7") == 0) { first_word = "SATURNVDP"; cmd = "SATURNVDP"; target = NULL; }
            else if (strcmp(target, "8") == 0) { first_word = "WORDPAC"; cmd = "WORDPAC"; target = NULL; }
            else if (strcmp(target, "9") == 0) { first_word = "DATAPAC"; cmd = "DATAPAC"; target = NULL; }
            else if (strcmp(target, "10") == 0) { first_word = "PROTECTO"; cmd = "PROTECTO"; target = NULL; }
            else if (strcmp(target, "11") == 0) { first_word = "MICROMINDER"; cmd = "MICROMINDER"; target = NULL; }
            else if (strcmp(target, "12") == 0) { first_word = "SALVAGEDIVER"; cmd = "SALVAGEDIVER"; target = NULL; }
            else if (strcmp(target, "13") == 0) { first_word = "DOS"; cmd = "DOS"; target = NULL; }
            else if (strcmp(target, "14") == 0) { first_word = "SOUNDEXPLORER"; cmd = "SOUNDEXPLORER"; target = NULL; }
            else if (strcmp(target, "15") == 0) { first_word = "CASTLEDARKNESS"; cmd = "CASTLEDARKNESS"; target = NULL; }
            else if (strcmp(target, "16") == 0) { first_word = "BASECONVERSIONS"; cmd = "BASECONVERSIONS"; target = NULL; }
            else if (strcmp(target, "17") == 0) { first_word = "LAWNJOB"; cmd = "LAWNJOB"; target = NULL; }
            else if (strcmp(target, "18") == 0) { first_word = "EMERALDELEPHANT"; cmd = "EMERALDELEPHANT"; target = NULL; }
            else if (strcmp(target, "19") == 0) { first_word = "VIC40OS"; cmd = "VIC40OS"; target = NULL; }
            else if (strcmp(target, "20") == 0) { first_word = "BAMREADPRINT"; cmd = "BAMREADPRINT"; target = NULL; }
            else if (strcmp(target, "21") == 0) { first_word = "TUNNEL"; cmd = "TUNNEL"; target = NULL; }
            else if (strcmp(target, "22") == 0) { first_word = "PTE"; cmd = "PTE"; target = NULL; }
            else if (strcmp(target, "23") == 0) { first_word = "BLOCKEDIT"; cmd = "BLOCKEDIT"; target = NULL; }
            else if (strcmp(target, "24") == 0) { first_word = "CHARSET"; cmd = "CHARSET"; target = NULL; }
            else if (strcmp(target, "25") == 0) { first_word = "TESTALL"; cmd = "TESTALL"; target = NULL; }
            else if (strcmp(target, "26") == 0) { first_word = "GYPSY"; cmd = "GYPSY"; target = NULL; }
            else if (strcmp(target, "27") == 0) { target = "MENU"; }
        }

        if (first_word && strcasecmp(first_word, "GO") == 0) {
            if (!target || strcasecmp(target, "MENU") == 0) {
                g_dashboard_active = false;
                g_aitest_active = false;
                lau_vram_write_string(g_vram, clear_seq, 3);
                const char *menu = 
                    "\r\n"
                    "      CompuServe Information Service      \r\n"
                    "==========================================\r\n"
                    "  1 GO VM         - Inspect Yul CPU VM State  \r\n"
                    "  2 GO RAG        - Vector DB RAG Gallery     \r\n"
                    "  3 WORDCRAFT     - Wordcraft 80 Ultra Demo   \r\n"
                    "  4 EASYSCRIPT    - EasyScript 64 Demo        \r\n"
                    "  5 DNATYPEWRITER - DNA Vector Typewriter     \r\n"
                    "  6 GO DASHBOARD  - Unified System Dashboard  \r\n"
                    "  7 GO AITEST     - AI Exploratory Test Suite \r\n"
                    "  8 EXIT          - Close Terminal Emulator   \r\n"
                    "  9 GO ADVENTURE  - Play On-Chain Adventure   \r\n"
                    "==========================================\r\n"
                    "Enter option name or GO target: \r\n";
                lau_vram_write_string(g_vram, menu, strlen(menu));
                log_telemetry("Rendered Main Menu");
            } else if (strcasecmp(target, "6") == 0 || strcasecmp(target, "DASHBOARD") == 0) {
                g_dashboard_active = true;
                g_aitest_active = false;
                lau_vram_write_string(g_vram, clear_seq, 3);
                const char *dashboard_menu =
                    "\r\n"
                    "--- CompuServe CIS: Unified Terminal Dashboard ---\r\n"
                    "==================================================\r\n"
                    "  1 CHOPLIFTER     - Run Choplifter Verification\r\n"
                    "  2 FORTAPOCALYPSE - Run Fort Apocalypse Verification\r\n"
                    "  3 HOMEWORD       - Run Homeword Verification\r\n"
                    "  4 HOMETAX        - Run Hometax Verification\r\n"
                    "  5 GTIACOL        - Run GTIA Collision Verification\r\n"
                    "  6 SEGAVDP        - Run Sega VDP Verification\r\n"
                    "  7 SATURNVDP      - Run Saturn VDP1 Verification\r\n"
                    "  8 WORDPAC        - Run Protecto WordPac Verification\r\n"
                    "  9 DATAPAC        - Run Datasoft DataPac Verification\r\n"
                    " 10 PROTECTO       - Run Protecto Mail-Order Verification\r\n"
                    " 11 MICROMINDER    - Run Micro-Minder Verification\r\n"
                    " 12 SALVAGEDIVER   - Run Salvage Diver Verification\r\n"
                    " 13 DOS            - Run DOS Command Verification\r\n"
                    " 14 SOUNDEXPLORER  - Run Sound Explorer Verification\r\n"
                    " 15 CASTLEDARKNESS - Run Castle of Darkness Verification\r\n"
                    " 16 BASECONVERSIONS- Run Base Conversions Verification\r\n"
                    " 17 LAWNJOB        - Run Lawn Job Verification\r\n"
                    " 18 EMERALDELEPHANT- Run Emerald Elephant Verification\r\n"
                    " 19 VIC40OS        - Run VIC 40 Column OS Verification\r\n"
                    " 20 BAMREADPRINT   - Run BAM Read & Print Verification\r\n"
                    " 21 TUNNEL         - Run Tunnel of Tomachon Verification\r\n"
                    " 22 PTE            - Run PTE Word Processor Verification\r\n"
                    " 23 BLOCKEDIT      - Run Block/Sector Editor Verification\r\n"
                    " 24 CHARSET        - Run Alternate Character Set Verification\r\n"
                    " 25 TESTALL        - Run automated tests on ALL systems\r\n"
                    " 26 GYPSY          - Run Gypsy Starship Space Simulation\r\n"
                    " 27 GO MENU        - Return to Main Menu\r\n"
                    "==================================================\r\n"
                    "Enter system name, option number, or GO target: \r\n";
                lau_vram_write_string(g_vram, dashboard_menu, strlen(dashboard_menu));
                log_telemetry("Rendered Dashboard");
            } else if (strcasecmp(target, "7") == 0 || strcasecmp(target, "AITEST") == 0) {
                g_aitest_active = true;
                g_dashboard_active = false;
                render_aitest_dashboard();
            } else if (strcasecmp(target, "1") == 0 || strcasecmp(target, "VM") == 0) {
                lau_vram_write_string(g_vram, clear_seq, 3);
                const char *vm_info =
                    "\r\n"
                    "--- CompuServe CIS: VM Status Room ---\r\n"
                    "Active CPU: Yul cpu6502 core\r\n"
                    "Storage MMIO Test Injection Register: $D540 (54592)\r\n"
                    "To return to menu, type GO MENU\r\n";
                lau_vram_write_string(g_vram, vm_info, strlen(vm_info));
                log_telemetry("Rendered VM Screen");
            } else if (strcasecmp(target, "2") == 0 || strcasecmp(target, "RAG") == 0) {
                lau_vram_write_string(g_vram, clear_seq, 3);
                const char *rag_info =
                    "\r\n"
                    "--- CompuServe CIS: RAG Shooting Gallery ---\r\n"
                    "To run a RAG search simulation, type:\r\n"
                    "  RAG <query>\r\n"
                    "e.g., RAG crow\r\n"
                    "This will save visual snapshots as both JPG and GIF!\r\n"
                    "To return to menu, type GO MENU\r\n";
                lau_vram_write_string(g_vram, rag_info, strlen(rag_info));
            } else if (strcasecmp(target, "3") == 0 || strcasecmp(target, "HELP") == 0) {
                lau_vram_write_string(g_vram, clear_seq, 3);
                const char *help_info =
                    "\r\n"
                    "--- CompuServe CIS: Escape Parser Utilities ---\r\n"
                    "Our terminal translates standard ANSI/Vidtex sequences:\r\n"
                    "  - ESC ESC d           : Clears screen & cursor home\r\n"
                    "  - ESC ESC I <col> <row>: Positions cursor (offset 32)\r\n"
                    "  - ESC [ <params> m    : Sets ANSI SGR colors\r\n"
                    "To return to menu, type GO MENU\r\n";
                lau_vram_write_string(g_vram, help_info, strlen(help_info));
            } else if (strcasecmp(target, "9") == 0 || strcasecmp(target, "ADVENTURE") == 0 || strcasecmp(target, "ZMACHINE") == 0) {
                g_editor_mode = MODE_ZMACHINE;
                g_mercenary_active = false;
                g_pong_active = false;
                tsfi_zmm_vm_exec(&vm, "YULINIT \"zmachine\", \"../solidity/bin/zmachine.yul\", 5");
                lau_vram_write_string(g_vram, clear_seq, 3);
                const char *header = 
                    "==================================================\r\n"
                    "       ON-CHAIN Z-MACHINE TEXT ADVENTURE          \r\n"
                    "==================================================\r\n"
                    " [Press ESC to return to Terminal Menu]          \r\n\r\n"
                    "  You are standing in the lobby.\r\n"
                    "zmachine> ";
                lau_vram_write_string(g_vram, header, strlen(header));
                log_telemetry("Started On-Chain Z-Machine Text Adventure");
                return;
            } else if (strcasecmp(target, "8") == 0) {
                running = false;
            } else {
                char error[256];
                sprintf(error, "\r\nInvalid GO target: \"%s\". Type GO MENU for options.\r\n", target);
                lau_vram_write_string(g_vram, error, strlen(error));
            }
            return;
        }
    }

    if (first_word && (strcasecmp(first_word, "SODARO") == 0 || strcasecmp(first_word, "MERCENARY") == 0)) {
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        const char *advice = 
            "\r\n"
            "--- Bob Sodaro's Novagen MERCENARY Map Advisor ---\r\n"
            "\"Navigating Targ requires mapping the wireframe city.\r\n"
            " Photocopy the grid map or draw it by hand!\" - RUN Mag\r\n"
            "==================================================\r\n"
            "Drawing Yul-emulated wireframe city in real-time...\r\n\r\n";
        lau_vram_write_string(g_vram, advice, strlen(advice));
        
        g_mercenary_active = true;
        g_pong_active = false;
        g_vram->is_dirty = true;
        return;
    }

    if (first_word && strcasecmp(first_word, "COMTERM") == 0) {
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        lau_vram_write_string(g_vram, 
            "=====================================================\r\n"
            "       BYTEC-COMTERM: BILINGUAL ARABIC SHAPER        \r\n"
            "=====================================================\r\n"
            " Initializing Comterm Arabic/English Terminal VDT...\r\n"
            " Shaper status: ACTIVE (Model: 1984 Hyperion/Comterm)\r\n\r\n"
            " Input Unicode Sequence (العربية):\r\n"
            " U+0627 U+0644 U+0639 U+0631 U+0628 U+064A U+0629\r\n\r\n", 335);

        uint32_t cps[] = {0x0627, 0x0644, 0x0639, 0x0631, 0x0628, 0x064A, 0x0629};
        tsfi_ottype_t ot;
        memset(&ot, 0, sizeof(ot));
        tsfi_shaped_glyph_t shaped[16];
        int out_count = tsfi_ottype_shape(&ot, cps, 7, shaped, 16, NULL, 0);
        
        char buf[256];
        snprintf(buf, sizeof(buf), " Output Glyphs: %d (Shaped into joining forms!)\r\n", out_count);
        lau_vram_write_string(g_vram, buf, strlen(buf));
        
        for (int i = 0; i < out_count && i < 16; i++) {
            snprintf(buf, sizeof(buf), "   Glyph [%d]: ID=%u, DX=%d\r\n", i, shaped[i].glyph_index, (int)shaped[i].advance_dx);
            lau_vram_write_string(g_vram, buf, strlen(buf));
        }
        
        lau_vram_write_string(g_vram, "\r\n =====================================================\r\n", 58);
        return;
    }

    if (first_word && strcasecmp(first_word, "AMTYPE") == 0) {
        char *arg = strtok(NULL, " \t");
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        if (!arg) {
            lau_vram_write_string(g_vram, 
                "=====================================================\r\n"
                "    AMTYPE CORPORATION: MAGAZINE TRANSCRIPTION SERVICE\r\n"
                "=====================================================\r\n"
                " Archival utility simulating the 1984 transcription service.\r\n"
                " Usage: AMTYPE [1 | 2 | 3]\r\n\r\n"
                " Available program listings to type:\r\n"
                "   1 - A.J. Kwitowski's Sound Concept (Ahoy!)\r\n"
                "   2 - Marshall F. Smith's Production Line (Ahoy!)\r\n"
                "   3 - Hurwood's Vector Graphics Maze\r\n"
                "=====================================================\r\n", 516);
            return;
        }
        
        int choice = atoi(arg);
        if (choice == 1) {
            lau_vram_write_string(g_vram, " [AMTYPE] Typing 'Sound Concept' listing... OK.\r\n", 49);
            execute_command("HURWOOD CONCEPT STAGE");
        } else if (choice == 2) {
            lau_vram_write_string(g_vram, " [AMTYPE] Typing 'Production Line' listing... OK.\r\n", 52);
            execute_command("HURWOOD SMITH STAGE");
        } else if (choice == 3) {
            lau_vram_write_string(g_vram, " [AMTYPE] Typing 'Vector Maze' listing... OK.\r\n", 48);
            execute_command("HURWOOD MAZE STAGE");
        } else {
            lau_vram_write_string(g_vram, " [AMTYPE] Invalid listing selection.\r\n", 38);
        }
        return;
    }

    if (first_word && strcasecmp(first_word, "INVISICLUE") == 0) {
        char *arg = strtok(NULL, " \t");
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        if (!arg) {
            lau_vram_write_string(g_vram, 
                "=====================================================\r\n"
                "          INFOCOM ON-CHAIN INVISICLUES SYSTEM        \r\n"
                "=====================================================\r\n"
                " Usage: INVISICLUE [1 | 2 | 3 | 4]\r\n\r\n"
                " Available Invisiclue Hint Sheets:\r\n"
                "   1 - Opening the locked lobby door\r\n"
                "   2 - Locating the hidden Gold Token\r\n"
                "   3 - Defeating the Cylon Raider\r\n"
                "   4 - Discovering the ancient magic word\r\n"
                "=====================================================\r\n", 497);
            return;
        }
        
        int hint_id = atoi(arg);
        lau_vram_write_string(g_vram, " Decrypting Invisiclue via on-chain keySystem...\r\n", 49);
        
        const char *hints[4] = {
            "Use the keycard found in the lobby.",
            "Search the desk in the office.",
            "Aim at coordinate 20, 15 and shoot.",
            "The ancient magic word is XYZZY."
        };
        
        if (hint_id >= 1 && hint_id <= 4) {
            char buf[256];
            snprintf(buf, sizeof(buf), "\r\n [REVEALED HINT]: \"%s\"\r\n", hints[hint_id - 1]);
            lau_vram_write_string(g_vram, buf, strlen(buf));
        } else {
            lau_vram_write_string(g_vram, "\r\n [ERROR]: Invalid hint sheet ID.\r\n", 37);
        }
        
        lau_vram_write_string(g_vram, "\r\n =====================================================\r\n", 58);
        return;
    }

    if (first_word && (strcasecmp(first_word, "SUBLOGIC") == 0 || strcasecmp(first_word, "LANDER") == 0)) {
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        lau_vram_write_string(g_vram, 
            "=====================================================\r\n"
            "   SUBLOGIC CORPORATION: FLIGHT SIMULATOR II TRIBUTE \r\n"
            "=====================================================\r\n"
            " Simulation core initialized (Model: 1983 Apple II/C64)\r\n"
            " Control controls: Standard Flight Instrumentation Panel\r\n\r\n"
            " INSTRUMENTS:\r\n"
            "   ALTITUDE   : 2500 FT    AIRSPEED   : 120 KTS\r\n"
            "   PITCH      : +002 DEG   ROLL       : +000 DEG\r\n"
            "   THROTTLE   : 85%        FUEL       : 45 GAL\r\n"
            "   GEAR       : DOWN       FLAPS      : 0%\r\n\r\n"
            " Navigating over target coordinates: TARG / LAKE MEAD\r\n"
            " [Vector flight dynamics running successfully in background!]\r\n"
            "=====================================================\r\n", 647);
        return;
    }

    if (first_word && strcasecmp(first_word, "PONG") == 0) {
        const char clear_seq[] = { '\x1b', '\x1b', 'd', '\0' };
        lau_vram_write_string(g_vram, clear_seq, 3);
        
        const char *advice = 
            "\r\n"
            "--- C64 Interactive Pong on ZMM VM Yul ---\r\n"
            "Control left paddle using W (Up) and S (Down).\r\n"
            "Keep the ball inside the neon arena!\r\n"
            "==========================================\r\n"
            "Running Pong emulator core in real-time...\r\n\r\n";
        lau_vram_write_string(g_vram, advice, strlen(advice));
        
        g_pong_active = true;
        g_mercenary_active = false;
        g_pong_loaded = false;
        g_vram->is_dirty = true;
        return;
    }
    
    if (first_word && strcasecmp(first_word, "RAG") == 0) {
        char *query = strtok(NULL, "");
        if (!query) query = "Yul CPU compilation";
        
        double similarity1 = 0.40;
        double similarity2 = 0.40;
        double similarity3 = 0.40;
        
        if (strcasestr(query, "crow") || strcasestr(query, "allow") || strcasestr(query, "ballet")) {
            similarity1 = 0.94;
            similarity2 = 0.45;
            similarity3 = 0.32;
        } else if (strcasestr(query, "calc") || strcasestr(query, "math") || strcasestr(query, "engine")) {
            similarity1 = 0.38;
            similarity2 = 0.91;
            similarity3 = 0.52;
        } else if (strcasestr(query, "yul") || strcasestr(query, "vm") || strcasestr(query, "state")) {
            similarity1 = 0.29;
            similarity2 = 0.48;
            similarity3 = 0.96;
        } else {
            unsigned int h = 0;
            for (int i = 0; query[i] != '\0'; i++) h = h * 31 + query[i];
            similarity1 = 0.4 + (h % 20) / 100.0;
            similarity2 = 0.4 + ((h >> 5) % 20) / 100.0;
            similarity3 = 0.4 + ((h >> 10) % 20) / 100.0;
        }
        
        int target_x = 250, target_y = 120;
        int doc_idx = 1;
        const char *doc_name = "solidity/bin/cpu6502.yul [Solidity CPU ROM]";
        const char *doc_context = "Solidity Yul CPU contract is initialized at virtual address 0x1\r\nand provides full instruction decoding support for 6502/6509 opcodes.";
        
        if (similarity2 > similarity1 && similarity2 > similarity3) {
            target_x = 550; target_y = 180;
            doc_idx = 2;
            doc_name = "benchmarks/docs/TSFI_DECISION_ENGINE.md [Decision Engine]";
            doc_context = "Decision Engine runs fast-path branching algorithms\r\nand optimizes matrix coprocessor inputs.";
        } else if (similarity3 > similarity1 && similarity3 > similarity2) {
            target_x = 400; target_y = 300;
            doc_idx = 3;
            doc_name = "src/tsfi_zmm_vm.c [ZMM VM State]";
            doc_context = "ZMM VM provides sandboxed, high-performance CPU simulation\r\nwith registered memory banks and page translation tables.";
        }
        
        char txt[1024];
        sprintf(txt, "\r\n=== RAG Vector Database Search ===\r\n"
                     "Query: \"%s\"\r\n"
                     "Initializing VIDTEX RAG Shooting Gallery Scatter Plot...\r\n"
                     "Target Duck (Doc 1) at (250, 120) [Solidity CPU ROM] Similarity: %.2f\r\n"
                     "Target Duck (Doc 2) at (550, 180) [Decision Engine] Similarity: %.2f\r\n"
                     "Target Duck (Doc 3) at (400, 300) [ZMM VM State] Similarity: %.2f\r\n\r\n"
                     "Firing Query Projectile towards closest cluster (Doc %d)...\r\n", 
                     query, similarity1, similarity2, similarity3, doc_idx);
        lau_vram_write_string(g_vram, txt, strlen(txt));
        
        int start_x = 400, start_y = 360;
        
        // 1. Run Shooting Gallery Scatter Plot Simulation
        for (int frame = 1; frame <= 12; frame++) {
            int bullet_x = start_x + (target_x - start_x) * frame / 12;
            int bullet_y = start_y + (target_y - start_y) * frame / 12;
            
            gfx_primitive_count = 0;
            // Target Doc 1 Duck
            add_circle(250, 120, 18, (doc_idx == 1) ? 0xFFFF5555 : 0xFF6272A4);
            add_text(250, 145, "CPU ROM", (doc_idx == 1) ? 0xFFFF5555 : 0xFFF8F8F2);
            
            // Target Doc 2 Duck
            add_circle(550, 180, 18, (doc_idx == 2) ? 0xFF8BE9FD : 0xFF6272A4);
            add_text(550, 205, "DECISION ENG", (doc_idx == 2) ? 0xFF8BE9FD : 0xFFF8F8F2);
