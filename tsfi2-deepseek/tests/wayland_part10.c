        0x02, 0xC9, 0x03, 0xD0, 0x0C, 0xAD, 0x02, 0xD0, 0x18, 0x69, 0x03, 0x8D, 0x02, 0xD0, 0x4C, 0x56, 0x20, 
        0xAD, 0x02, 0xD0, 0x38, 0xE9, 0x03, 0x8D, 0x02, 0xD0, 
        0xA5, 0x03, 0xC9, 0x03, 0xD0, 0x0C, 0xAD, 0x03, 0xD0, 0x18, 0x69, 0x03, 0x8D, 0x03, 0xD0, 0x4C, 0x71, 0x20, 
        0xAD, 0x03, 0xD0, 0x38, 0xE9, 0x03, 0x8D, 0x03, 0xD0, 
        0xAD, 0x02, 0xD0, 0xC9, 0x28, 0xB0, 0x0B, 0xA2, 0x40, 0xA0, 0x10, 0x20, 0x1B, 0x21, 0xA9, 0x03, 0x85, 0x02, 
        0xAD, 0x02, 0xD0, 0xC9, 0xE0, 0x90, 0x0B, 0xA2, 0x40, 0xA0, 0x10, 0x20, 0x1B, 0x21, 0xA9, 0xFD, 0x85, 0x02, 
        0xAD, 0x03, 0xD0, 0xC9, 0x23, 0xB0, 0x0B, 0xA2, 0x40, 0xA0, 0x10, 0x20, 0x1B, 0x21, 0xA9, 0x03, 0x85, 0x03, 
        0xAD, 0x03, 0xD0, 0xC9, 0x9B, 0x90, 0x4D, 0xAD, 0x1E, 0xD0, 0x29, 0x03, 0xC9, 0x03, 0xD0, 0x17, 0xA2, 0x80, 0xA0, 0x20, 0x20, 0x1B, 0x21, 0xA9, 0xFD, 0x85, 0x03, 0xE6, 0x04, 
        0xA2, 0x2C, 0xA0, 0x21, 0x20, 0x14, 0x21, 0x4C, 0xF7, 0x20, 
        0xAD, 0x03, 0xD0, 0xC9, 0xAF, 0x90, 0x26, 0xA2, 0x10, 0xA0, 0x05, 0x20, 0x1B, 0x21, 0xA9, 0x80, 0x8D, 0x02, 0xD0, 0xA9, 0x3C, 0x8D, 0x03, 0xD0, 0xA9, 0x03, 0x85, 0x03, 
        0xA2, 0x31, 0xA0, 0x21, 0x20, 0x14, 0x21, 0xA5, 0x04, 0xF0, 0x02, 0xC6, 0x04, 
        0xA5, 0x04, 0x85, 0x03, 0x20, 0x01, 0x21, 0x00, 0x00, 0x00, 
        0xAD, 0x02, 0xD6, 0xD0, 0x0F, 0xAD, 0x27, 0x21, 0xC9, 0xFF, 0xF0, 0x08, 0x8D, 0x00, 0xD6, 0xEE, 0x07, 0x21, 0x60, 
        0x8E, 0x07, 0x21, 0x8C, 0x08, 0x21, 0x60, 
        0x8E, 0x00, 0xD4, 0x8C, 0x01, 0xD4, 0xA9, 0x21, 0x8D, 0x04, 0xD4, 0x60, 
        0x1B, 0x02, 0x18, 0x27, 0xFF, 
        0x0E, 0x39, 0x37, 0x1F, 0xFF, 
        0x0C, 0x0B, 0x1F, 0xFF
    };
    int prog_len = sizeof(pong_program);

    for (int i = 0; i < prog_len; i++) {
        vm_poke(&vm, 8192 + i, pong_program[i]);
    }

    vm_poke(&vm, 53248, 120);
    vm_poke(&vm, 53249, 155);
    vm_poke(&vm, 53250, 120);
    vm_poke(&vm, 53251, 60);
    vm_poke(&vm, 53269, 3);
    
    vm_poke(&vm, 2, 3);
    vm_poke(&vm, 3, 3);
    vm_poke(&vm, 4, 0);

    vm_poke64(&vm, 128, 0);
    vm_poke64(&vm, 129, 0);
    vm_poke64(&vm, 130, 0);
    vm_poke64(&vm, 131, 0xFF);
    vm_poke64(&vm, 132, 0x20);
    vm_poke64(&vm, 133, 8192);

    g_pong_loaded = true;
}

static void update_pong_game(void) {
    if (!g_pong_active) return;
    if (!g_pong_loaded) {
        init_pong_game();
    }

    static float g_paddle_x = 120.0f;
    if (g_key_up_pressed) {
        g_paddle_x -= 0.05f;
        if (g_paddle_x < 40.0f) g_paddle_x = 40.0f;
    }
    if (g_key_down_pressed) {
        g_paddle_x += 0.05f;
        if (g_paddle_x > 224.0f) g_paddle_x = 224.0f;
    }
    vm_poke(&vm, 53248, (int)g_paddle_x);

    static int speed_divider = 0;
    bool should_tick = (speed_divider++ % 60 == 0);

    if (should_tick) {
        // Reset PC to the start of the game loop ($200F / 8207) each tick
        vm_poke64(&vm, 133, 8207);

        char cmd[512];
        sprintf(cmd, "YULEXEC \"cpu6502\", \"c45b180800000000000000000000000000000000000000000000000000000000000003e8\"");
        vm.output_pos = 0;
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    int paddle_x = (int)vm_peek(&vm, 53248);
    int ball_x = (int)vm_peek(&vm, 53250);
    int ball_y = (int)vm_peek(&vm, 53251);
    int score = (int)vm_peek(&vm, 4);
    int pc = (int)vm_peek(&vm, 133);

    // Simulate Sprite-to-Sprite Collision register 53278 ($D01E)
    // Paddle is at Y=155. Ball checks collision when Y >= 155 ($9B)
    uint8_t collision_mask = 0;
    if (ball_y >= 150 && ball_y <= 160 && abs(ball_x - paddle_x) <= 24) {
        collision_mask = 3; // Sprite 0 and Sprite 1 collision
        printf("\x07");
        fflush(stdout);
    }
    vm_poke(&vm, 53278, collision_mask);

    static int print_counter = 0;
    if (print_counter++ % 60 == 0) {
        fprintf(stderr, "[PONG-TELEMETRY] PC: 0x%04X (dec %d), PaddleX: %d, BallX: %d, BallY: %d, Score: %d, Collide: %d\n", 
                pc, pc, paddle_x, ball_x, ball_y, score, collision_mask);
    }

    gfx_primitive_count = 0;

    int center_x = 640;
    int center_y = 360;

    int pad_x_scaled = center_x + (paddle_x - 132) * 4 - 22;
    int pad_y_scaled = center_y + (155 - 105) * 4 - 67; // Bottom position
    int ball_x_scaled = center_x + (ball_x - 132) * 4 - 22;
    int ball_y_scaled = center_y + (ball_y - 105) * 4 - 67;

    int y_top = center_y + (35 - 105) * 4 - 67;
    int y_bot = center_y + (175 - 105) * 4 - 67;
    int x_left = center_x + (40 - 132) * 4 - 22;
    int x_right = center_x + (224 - 132) * 4 - 22;

    // Draw Closed Left, Top, and Right walls
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_LINE; gp->x1 = x_left; gp->y1 = y_top; gp->x2 = x_right; gp->y2 = y_top; gp->color = 0xFF50FA7B; // Top Wall
    }
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_LINE; gp->x1 = x_left; gp->y1 = y_top; gp->x2 = x_left; gp->y2 = y_bot; gp->color = 0xFF50FA7B; // Left Wall
    }
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_LINE; gp->x1 = x_right; gp->y1 = y_top; gp->x2 = x_right; gp->y2 = y_bot; gp->color = 0xFF50FA7B; // Right Wall
    }

    // Draw Horizontal Paddle
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_LINE; gp->x1 = pad_x_scaled - 35; gp->y1 = pad_y_scaled; gp->x2 = pad_x_scaled + 35; gp->y2 = pad_y_scaled; gp->color = 0xFFFF79C6;
    }

    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_CIRCLE; gp->x1 = ball_x_scaled; gp->y1 = ball_y_scaled; gp->r = 10; gp->color = 0xFFF1FA8C;
    }

    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_TEXT; gp->x1 = center_x - 50 - 22; gp->y1 = y_top - 30; gp->color = 0xFF8BE9FD;
        sprintf(gp->text, "SCORE: %d", score);
    }
}

static void update_mercenary_yul_camera(void) {
    if (!g_mercenary_active) return;
    
    static bool map_loaded = false;
    if (!map_loaded) {
        int8_t vertices[17][3] = {
            {-20, -30, -20}, { 20, -30, -20}, { 20,  20, -20}, {-20,  20, -20},
            {-20, -30,  20}, { 20, -30,  20}, { 20,  20,  20}, {-20,  20,  20},
            {-80, -45, -80}, { 80, -45, -80}, { 80, -45,  80}, {-80, -45,  80},
            {  0,  60,   0}, {-15,  35, -15}, { 15,  35, -15}, { 15,  35,  15}, {-15,  35,  15}
        };
        uint8_t lines[24][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7},
            {8, 9}, {9, 10}, {10, 11}, {11, 8},
            {13, 14}, {14, 15}, {15, 16}, {16, 13},
            {12, 13}, {12, 14}, {12, 15}, {12, 16}
        };
        
        for (int i = 0; i < 17; i++) {
            vm_poke(&vm, 55280 + i * 3, (uint8_t)vertices[i][0]);
            vm_poke(&vm, 55281 + i * 3, (uint8_t)vertices[i][1]);
            vm_poke(&vm, 55282 + i * 3, (uint8_t)vertices[i][2]);
        }
        
        for (int j = 0; j < 24; j++) {
            vm_poke(&vm, 55536 + j * 2, lines[j][0]);
            vm_poke(&vm, 55537 + j * 2, lines[j][1]);
        }
        
        vm_poke(&vm, 54964, 17);
        vm_poke(&vm, 54965, 24);
        map_loaded = true;
    }
    
    int yaw = (g_frame_counter / 6) % 16;
    
    vm_poke(&vm, 54960, 1);
    vm_poke(&vm, 54961, yaw);
    vm_poke(&vm, 54962, 0);
    vm_poke(&vm, 54963, 120);
    
    vm_poke(&vm, 54966, 1);
    
    uint64_t status = vm_peek(&vm, 54967);
    if (status != 1) return;
    
    gfx_primitive_count = 0;
    
    int proj_x[17];
    int proj_y[17];
    for (int i = 0; i < 17; i++) {
        proj_x[i] = (int)vm_peek(&vm, 55800 + i * 2);
        proj_y[i] = (int)vm_peek(&vm, 55801 + i * 2);
    }
    
    int center_x = 640;
    int center_y = 360;
    
    static const uint8_t line_indices[24][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
        {8, 9}, {9, 10}, {10, 11}, {11, 8},
        {13, 14}, {14, 15}, {15, 16}, {16, 13},
        {12, 13}, {12, 14}, {12, 15}, {12, 16}
    };
    
    for (int j = 0; j < 24; j++) {
        int from = line_indices[j][0];
        int to = line_indices[j][1];
        
        int x1 = center_x + (proj_x[from] - 160) * 3 - 22;
        int y1 = center_y + (proj_y[from] - 100) * 3 - 67;
        int x2 = center_x + (proj_x[to] - 160) * 3 - 22;
        int y2 = center_y + (proj_y[to] - 100) * 3 - 67;
        
        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
            gp->type = GFX_LINE;
            gp->x1 = x1; gp->y1 = y1;
            gp->x2 = x2; gp->y2 = y2;
            if (j < 12) gp->color = 0xFF8BE9FD;       // Building (Pastel Blue)
            else if (j < 16) gp->color = 0xFF50FA7B;  // Grid (Pastel Green)
            else gp->color = 0xFFFF79C6;              // Spaceship (Pastel Pink)
        }
    }
    
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_CIRCLE;
        gp->x1 = center_x - 22;
        gp->y1 = center_y - 67;
        gp->r = 15;
        gp->color = 0xFFFF5555;
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    printf("[TERMINAL] Starting Wayland Yul VM Software-Rendered Terminal Emulator...\n");

    tsfi_wire_firmware_init();
    extern void tsfi_input_init(void);
    tsfi_input_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (!fw) {
        fprintf(stderr, "ERROR: Failed to initialize wire firmware.\n");
        return 1;
    }
    g_vram = &fw->vram;
    tsfi_zmm_vm_init(&vm);
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"diyat\", \"../solidity/bin/diyat.yul\", 858021");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"minder\", \"../solidity/bin/minder.yul\", 858030");
    vm_poke64(&vm, 54695, 0xd17a5); // Set diyatAddress to 0xd17a5

    // Pre-poke a retro space invader sprite demo into address 0x2000
    extern void vm_poke(TsfiZmmVmState *state, uint64_t addr, uint8_t val);
    uint8_t demo_sprite[63] = {
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x18, 0x00, 0x18,
        0x0c, 0x00, 0x30,
        0x1e, 0x00, 0x78,
        0x3f, 0x00, 0xfc,
        0x77, 0x00, 0xee,
        0x7f, 0xff, 0xfe,
        0x7f, 0xff, 0xfe,
        0x5f, 0xff, 0xfa,
        0x1f, 0xff, 0xf8,
        0x0f, 0xff, 0xf0,
        0x07, 0xff, 0xe0,
        0x03, 0xff, 0xc0,
        0x01, 0xff, 0x80,
        0x00, 0xff, 0x00,
        0x00, 0x7e, 0x00,
        0x00, 0x3c, 0x00,
        0x00, 0x18, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00
    };
    for (int i = 0; i < 63; i++) {
        vm_poke(&vm, 0x2000 + i, demo_sprite[i]);
    }

    display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "ERROR: Failed to connect to Wayland display.\n");
        return 1;
    }

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    if (!compositor || !shm || !xdg_wm_base) {
        fprintf(stderr, "ERROR: Missing Wayland globals.\n");
        return 1;
    }

    xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);

    if (seat) {
        wl_seat_add_listener(seat, &seat_listener, NULL);
    }

    surface = wl_compositor_create_surface(compositor);
    xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);

    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);
    xdg_toplevel_set_title(xdg_toplevel, "Yul CPU Terminal Emulator");
    
    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    wl_buffers[0] = create_shm_buffer(win_width, win_height, &pixel_datas[0]);
    wl_buffers[1] = create_shm_buffer(win_width, win_height, &pixel_datas[1]);
    if (!wl_buffers[0] || !wl_buffers[1]) {
        fprintf(stderr, "ERROR: Failed to create shm buffers.\n");
        return 1;
    }
    back_buffer = malloc(win_width * win_height * 4);
    if (!back_buffer) {
        fprintf(stderr, "ERROR: Failed to allocate back buffer.\n");
        return 1;
    }

    lau_vram_init(g_vram);
    const char *welcome = "=== TSFI YUL CPU TERMINAL EMULATOR ===\r\n"
                          "System 11 Audited. Active CPU: cpu6502\r\n"
                          "Available commands:\r\n"
                          "  YULINIT, YULEXEC, SWIFTLOAD, REU, CALC, MEMDUMP, SPRITE, RULE, OMNICOMM, FLOTSAM\r\n\r\n"
                          "zmm-vm> ";
    lau_vram_write_string(g_vram, welcome, strlen(welcome));
    execute_command("GO MENU");

    int stdin_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, stdin_flags | O_NONBLOCK);

    int display_fd = wl_display_get_fd(display);
    printf("[TERMINAL] Entering event loop. Window should map on your Hyprland desktop now.\n");

    int f_count = 0;
    while (running) {
        wl_display_dispatch_pending(display);

        // Check if Yul CPU has written automated test input character to MMIO slot 54592 ($D540)
        {
            extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len);
            uint8_t peek_cd[36] = {0};
            peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69; // peek(uint256) -> 0x7861d269
            uint64_t target_mmio = 54592;
            for (int k = 0; k < 8; k++) {
                peek_cd[4 + 31 - k] = (target_mmio >> (k * 8)) & 0xFF;
            }
            uint8_t peek_ret[32] = {0};
            size_t peek_ret_len = 32;
            if (lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len)) {
                uint8_t val = peek_ret[31];
                if (val != 0) {
                    char ch = (char)val;
                    if (g_editor_mode != MODE_TERMINAL) {
                        if (g_editor_mode == MODE_INSTACALC) {
                            handle_instacalc_input(ch);
                        } else if (g_editor_mode == MODE_YULBUILD) {
                            handle_yulbuild_input(ch);
                        } else if (g_editor_mode == MODE_APPLEPANIC) {
                            handle_applepanic_input(ch);
                        } else if (g_editor_mode == MODE_AIRASSAULT) {
                            handle_airassault_input(ch);
                        } else if (g_editor_mode == MODE_SLINKYBEAR) {
                            handle_slinkybear_input(ch);
                        } else if (g_editor_mode == MODE_SLINKYPANIC) {
                            handle_slinkypanic_input(ch);
                        } else if (g_editor_mode == MODE_ALPINER) {
                            handle_alpiner_input(ch);
                        } else if (g_editor_mode == MODE_CHECKLIST) {
                            handle_checklist_input(ch);
                        } else if (g_editor_mode == MODE_WHATSMYJOB) {
                            handle_job_input(ch);
                        } else if (g_editor_mode == MODE_PTE) {
                            handle_pte_input(ch);
                        } else if (g_editor_mode == MODE_SPACEPATROL) {
                            handle_spacepatrol_input(ch);
                        } else if (g_editor_mode == MODE_CONSTRUCTION_CO) {
                            handle_construction_co_input(ch);
                        } else if (g_editor_mode == MODE_STUDIO64) {
                            handle_studio64_input(ch);
                        } else if (g_editor_mode == MODE_MAGPIE) {
                            handle_magpie_input(ch);
                        } else if (g_editor_mode == MODE_ALICE) {
                            handle_alice_input(ch);
                        } else if (g_editor_mode == MODE_TOP) {
                            handle_top_input(ch);
                        } else if (g_editor_mode == MODE_FONTASIA) {
                            handle_fontasia_input(ch);
                        } else if (g_editor_mode == MODE_FLANKSPEED) {
                            handle_flankspeed_input(ch);
                        } else if (g_editor_mode == MODE_BOOTER) {
                            handle_booter_input(ch);
                        } else if (g_editor_mode == MODE_HOPAROUND) {
                            handle_hoparound_input(ch);
                        } else if (g_editor_mode == MODE_TOWERS) {
                            handle_towers_input(ch);
                        } else if (g_editor_mode == MODE_DISINTEGRATOR) {
                            handle_disintegrator_input(ch);
                        } else if (g_editor_mode == MODE_FIDGITS) {
                            handle_fidgits_input(ch);
                        } else if (g_editor_mode == MODE_MOXEY) {
                            handle_moxey_input(ch);
                        } else if (g_editor_mode == MODE_DRUM) {
                            handle_drum_input(ch);
                        } else if (g_editor_mode == MODE_JEWEL) {
                            handle_jewel_input(ch);
                        } else if (g_editor_mode == MODE_SANTA) {
                            handle_santa_input(ch);
                        } else if (g_editor_mode == MODE_CLOAK) {
                            handle_cloak_input(ch);
                        } else if (g_editor_mode == MODE_GYPSY) {
                            handle_gypsy_input(ch);
                        } else if (g_editor_mode == MODE_MARTIAN) {
                            handle_martian_input(ch);
                        } else if (g_editor_mode == MODE_HAUNTED) {
                            handle_haunted_input(ch);
                        } else if (g_editor_mode == MODE_INFRARAID) {
                            handle_infraraid_input(ch);
                        } else if (g_editor_mode == MODE_STREAMER) {
                            handle_streamer_input(ch);
                        } else if (g_editor_mode == MODE_KNOCKOUT) {
                            handle_knockout_input(ch);
                        } else if (g_editor_mode == MODE_ALARM) {
                            handle_alarm_input(ch);
                        } else if (g_editor_mode == MODE_MEMCHECK) {
                            handle_memcheck_input(ch);
                        } else if (g_editor_mode == MODE_ARENA) {
                            handle_arena_input(ch);
                        } else if (g_editor_mode == MODE_HEADTOHEAD) {
                            handle_headtohead_input(ch);
                        } else if (g_editor_mode == MODE_CRABFIGHT) {
                            handle_crabfight_input(ch);
                        } else if (g_editor_mode == MODE_TREASURE) {
                            handle_treasure_input(ch);
                        } else if (g_editor_mode == MODE_CHARDUMP) {
                            handle_chardump_input(ch);
                        } else if (g_editor_mode == MODE_CREATOR) {
                            handle_creator_input(ch);
                        } else if (ch == '\n' || ch == '\r') {
                            lau_vram_write_string(g_vram, "\r\n", 2);
                        } else if (ch == 127 || ch == '\b') {
                            lau_vram_write_char(g_vram, '\b');
                            lau_vram_write_char(g_vram, ' ');
                            lau_vram_write_char(g_vram, '\b');
                        } else if (ch >= 32 && ch < 127) {
                            if (g_editor_mode == MODE_DNATYPEWRITER) {
                                if (ch == 'A' || ch == 'a') {
                                    lau_vram_write_string(g_vram, "\x1b[32mA\x1b[0m", 9);
                                } else if (ch == 'T' || ch == 't') {
                                    lau_vram_write_string(g_vram, "\x1b[31mT\x1b[0m", 9);
                                } else if (ch == 'C' || ch == 'c') {
                                    lau_vram_write_string(g_vram, "\x1b[34mC\x1b[0m", 9);
                                } else if (ch == 'G' || ch == 'g') {
                                    lau_vram_write_string(g_vram, "\x1b[33mG\x1b[0m", 9);
                                } else {
                                    lau_vram_write_char(g_vram, ch);
                                }
                            } else {
                                lau_vram_write_char(g_vram, ch);
                            }
                        }
                    } else {
                        if (ch == '\n' || ch == '\r') {
                            lau_vram_write_string(g_vram, "\r\n", 2);
                            if (cmd_len > 0) {
                                cmd_buf[cmd_len] = '\0';
                                execute_command(cmd_buf);
                                cmd_len = 0;
                                cmd_buf[0] = '\0';
                            }
                            lau_vram_write_string(g_vram, "zmm-vm> ", 8);
                        } else if (ch == 127 || ch == '\b') {
                            if (cmd_len > 0) {
                                cmd_len--;
                                cmd_buf[cmd_len] = '\0';
                                lau_vram_write_char(g_vram, '\b');
                                lau_vram_write_char(g_vram, ' ');
                                lau_vram_write_char(g_vram, '\b');
                            }
                        } else if (ch >= 32 && ch < 127) {
                            char hook_cmd[64] = {0};
                            for (int i = 0; i < g_chrget_hooks_count; i++) {
                                if (g_chrget_hooks[i].key == ch) {
                                    snprintf(hook_cmd, sizeof(hook_cmd), "%s", g_chrget_hooks[i].cmd);
                                    break;
                                }
                            }
                            if (hook_cmd[0]) {
                                lau_vram_write_string(g_vram, "\r\n[CHRGET INTERCEPT]\r\n", 22);
                                execute_command(hook_cmd);
                                continue;
                            }
                            if (cmd_len < (int)sizeof(cmd_buf) - 2) {
                                cmd_buf[cmd_len++] = ch;
                                cmd_buf[cmd_len] = '\0';
                                lau_vram_write_char(g_vram, ch);
                            }
                        }
                    }
                    
                    // Clear register by poking 0 back
                    uint8_t poke_cd[4 + 32 + 32] = {0};
                    poke_cd[0] = 0x80; poke_cd[1] = 0x29; poke_cd[2] = 0xe7; poke_cd[3] = 0xc0; // poke(uint256,uint256)
                    for (int k = 0; k < 8; k++) {
                        poke_cd[4 + 31 - k] = (target_mmio >> (k * 8)) & 0xFF;
                    }
                    uint8_t poke_ret[32] = {0};
                    size_t poke_ret_len = 32;
                    lau_yul_thunk_execute("cpu6502", poke_cd, 4 + 32 + 32, poke_ret, &poke_ret_len);
                }
            }
        }

        bool need_redraw = g_vram->is_dirty || g_mercenary_active || g_pong_active || 
                           g_editor_mode == MODE_DRUM || g_editor_mode == MODE_JEWEL || 
                           g_editor_mode == MODE_SANTA || g_editor_mode == MODE_CLOAK || 
                           g_editor_mode == MODE_GYPSY || g_editor_mode == MODE_MARTIAN || 
                           g_editor_mode == MODE_HAUNTED || g_editor_mode == MODE_INFRARAID || 
                           g_editor_mode == MODE_STREAMER || g_editor_mode == MODE_KNOCKOUT || 
                           g_editor_mode == MODE_ALARM || g_editor_mode == MODE_MEMCHECK ||
                           g_editor_mode == MODE_ARENA || g_editor_mode == MODE_HEADTOHEAD ||
                           g_editor_mode == MODE_CRABFIGHT || g_editor_mode == MODE_TREASURE ||
                           g_editor_mode == MODE_CHARDUMP || g_editor_mode == MODE_TERM128 ||
                           g_editor_mode == MODE_ALCHEMIST || g_editor_mode == MODE_SKIFOLLY || 
                           g_editor_mode == MODE_DARTS || g_editor_mode == MODE_MAKEWAVE || 
                           g_editor_mode == MODE_PLOTWAVE;

        if (g_vram->is_dirty) {
            sync_vram_to_cpu();
        }

        if (resize_pending) {
            resize_pending = false;
            if (wl_buffers[0]) wl_buffer_destroy(wl_buffers[0]);
            if (wl_buffers[1]) wl_buffer_destroy(wl_buffers[1]);
            if (pixel_datas[0]) munmap(pixel_datas[0], win_width * win_height * 4);
            if (pixel_datas[1]) munmap(pixel_datas[1], win_width * win_height * 4);
            if (back_buffer) free(back_buffer);
            win_width = pending_width;
            win_height = pending_height;
            wl_buffers[0] = create_shm_buffer(win_width, win_height, &pixel_datas[0]);
            wl_buffers[1] = create_shm_buffer(win_width, win_height, &pixel_datas[1]);
            back_buffer = malloc(win_width * win_height * 4);
            printf("[TERMINAL] Resized to %dx%d\n", win_width, win_height);
            need_redraw = true;
        }

        if (configured && need_redraw) {
            g_vram->is_dirty = false;
            render_terminal_display();
            
            // Swap buffer indices to prevent writing to the buffer currently read by the compositor
            current_buffer_idx = 1 - current_buffer_idx;
            memcpy(pixel_datas[current_buffer_idx], back_buffer, win_width * win_height * 4);
            
            wl_surface_attach(surface, wl_buffers[current_buffer_idx], 0, 0);
            wl_surface_damage(surface, 0, 0, win_width, win_height);
            wl_surface_commit(surface);
            
            if (f_count % 60 == 0) {
                printf("[TERMINAL] Rendered frame %d\n", f_count);
            }
            f_count++;
        }

        while (wl_display_prepare_read(display) != 0) {
            wl_display_dispatch_pending(display);
        }
        wl_display_flush(display);

        struct pollfd fds[2] = {
            { .fd = display_fd, .events = POLLIN },
            { .fd = STDIN_FILENO, .events = POLLIN }
        };
        
        // Wait up to 16ms for display events (approx 60fps)
        int ret = poll(fds, 2, 16);
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                if (wl_display_read_events(display) < 0) {
                    break;
                }
                wl_display_dispatch_pending(display);
            } else {
                wl_display_cancel_read(display);
            }
            
            if (fds[1].revents & POLLIN) {
                char ch;
                ssize_t n_read;
                bool got_chars = false;
                while ((n_read = read(STDIN_FILENO, &ch, 1)) > 0) {
                    got_chars = true;
                    if (ch == 27) { // ESC key over STDIN
                        char next1, next2;
                        if (read(STDIN_FILENO, &next1, 1) == 1) {
                            if (next1 == '[') {
                                if (read(STDIN_FILENO, &next2, 1) == 1) {
                                    if (next2 == 'A') {
                                        if (g_editor_mode == MODE_CREATOR) {
                                            handle_creator_input('u');
                                        }
                                        continue;
                                    } else if (next2 == 'B') {
                                        if (g_editor_mode == MODE_CREATOR) {
                                            handle_creator_input('d');
                                        }
                                        continue;
                                    }
                                }
                            }
                        }
                        if (g_editor_mode != MODE_TERMINAL) {
                            if (g_editor_mode == MODE_CREATOR && g_creator_editing_value) {
                                g_creator_editing_value = false;
                                redraw_creator_screen();
                            } else {
                                g_editor_mode = MODE_TERMINAL;
                                g_mercenary_active = false;
                                g_pong_active = false;
                                g_applepanic_active = false;
                                g_airassault_active = false;
                                g_slinkybear_active = false;
                                g_slinkypanic_active = false;
                                g_alpiner_active = false;
                                g_checklist_active = false;
                                g_jobs_active = false;
                                execute_command("GO MENU");
                            }
                        }
                    } else if (g_editor_mode != MODE_TERMINAL) {
                        // In editor mode, just write characters directly to VRAM
                        if (g_editor_mode == MODE_INSTACALC) {
                            handle_instacalc_input(ch);
                        } else if (g_editor_mode == MODE_YULBUILD) {
                            handle_yulbuild_input(ch);
                        } else if (g_editor_mode == MODE_APPLEPANIC) {
                            handle_applepanic_input(ch);
                        } else if (g_editor_mode == MODE_AIRASSAULT) {
                            handle_airassault_input(ch);
                        } else if (g_editor_mode == MODE_SLINKYBEAR) {
                            handle_slinkybear_input(ch);
                        } else if (g_editor_mode == MODE_SLINKYPANIC) {
                            handle_slinkypanic_input(ch);
                        } else if (g_editor_mode == MODE_ALPINER) {
                            handle_alpiner_input(ch);
                        } else if (g_editor_mode == MODE_CHECKLIST) {
                            handle_checklist_input(ch);
                        } else if (g_editor_mode == MODE_WHATSMYJOB) {
                            handle_job_input(ch);
                        } else if (g_editor_mode == MODE_STUDIO64) {
                            handle_studio64_input(ch);
                        } else if (g_editor_mode == MODE_MAGPIE) {
                            handle_magpie_input(ch);
                        } else if (g_editor_mode == MODE_ALICE) {
                            handle_alice_input(ch);
                        } else if (g_editor_mode == MODE_TOP) {
                            handle_top_input(ch);
                        } else if (g_editor_mode == MODE_FONTASIA) {
                            handle_fontasia_input(ch);
                        } else if (g_editor_mode == MODE_FLANKSPEED) {
                            handle_flankspeed_input(ch);
                        } else if (g_editor_mode == MODE_BOOTER) {
                            handle_booter_input(ch);
                        } else if (g_editor_mode == MODE_HOPAROUND) {
                            handle_hoparound_input(ch);
                        } else if (g_editor_mode == MODE_TOWERS) {
                            handle_towers_input(ch);
                        } else if (g_editor_mode == MODE_DISINTEGRATOR) {
                            handle_disintegrator_input(ch);
                        } else if (g_editor_mode == MODE_FIDGITS) {
                            handle_fidgits_input(ch);
                        } else if (g_editor_mode == MODE_MOXEY) {
                            handle_moxey_input(ch);
                        } else if (g_editor_mode == MODE_DRUM) {
                            handle_drum_input(ch);
                        } else if (g_editor_mode == MODE_JEWEL) {
                            handle_jewel_input(ch);
                        } else if (g_editor_mode == MODE_SANTA) {
                            handle_santa_input(ch);
                        } else if (g_editor_mode == MODE_CLOAK) {
                            handle_cloak_input(ch);
                        } else if (g_editor_mode == MODE_GYPSY) {
                            handle_gypsy_input(ch);
                        } else if (g_editor_mode == MODE_MARTIAN) {
                            handle_martian_input(ch);
                        } else if (g_editor_mode == MODE_HAUNTED) {
                            handle_haunted_input(ch);
                        } else if (g_editor_mode == MODE_INFRARAID) {
                            handle_infraraid_input(ch);
                        } else if (g_editor_mode == MODE_STREAMER) {
                            handle_streamer_input(ch);
                        } else if (g_editor_mode == MODE_KNOCKOUT) {
                            handle_knockout_input(ch);
                        } else if (g_editor_mode == MODE_ALARM) {
                            handle_alarm_input(ch);
                        } else if (g_editor_mode == MODE_MEMCHECK) {
                            handle_memcheck_input(ch);
                        } else if (g_editor_mode == MODE_ARENA) {
                            handle_arena_input(ch);
                        } else if (g_editor_mode == MODE_HEADTOHEAD) {
                            handle_headtohead_input(ch);
                        } else if (g_editor_mode == MODE_CRABFIGHT) {
                            handle_crabfight_input(ch);
                        } else if (g_editor_mode == MODE_TREASURE) {
                            handle_treasure_input(ch);
                        } else if (g_editor_mode == MODE_CHARDUMP) {
                            handle_chardump_input(ch);
                        } else if (g_editor_mode == MODE_TERM128) {
                            handle_term128_input(ch);
                        } else if (g_editor_mode == MODE_ALCHEMIST) {
                            handle_alchemist_input(ch);
                        } else if (g_editor_mode == MODE_SKIFOLLY) {
                            handle_skifolly_input(ch);
                        } else if (g_editor_mode == MODE_DARTS) {
                            handle_darts_input(ch);
                        } else if (g_editor_mode == MODE_MAKEWAVE) {
                            handle_makewave_input(ch);
                        } else if (g_editor_mode == MODE_PLOTWAVE) {
                            handle_plotwave_input(ch);
                        } else if (g_editor_mode == MODE_CREATOR) {
                            handle_creator_input(ch);
                        } else if (ch == '\n' || ch == '\r') {
                            lau_vram_write_string(g_vram, "\r\n", 2);
                        } else if (ch == 127 || ch == '\b') {
                            lau_vram_write_char(g_vram, '\b');
                            lau_vram_write_char(g_vram, ' ');
                            lau_vram_write_char(g_vram, '\b');
                        } else if (ch >= 32 && ch < 127) {
                            if (g_editor_mode == MODE_DNATYPEWRITER) {
                                if (ch == 'A' || ch == 'a') {
                                    lau_vram_write_string(g_vram, "\x1b[32mA\x1b[0m", 9);
                                } else if (ch == 'T' || ch == 't') {
                                    lau_vram_write_string(g_vram, "\x1b[31mT\x1b[0m", 9);
                                } else if (ch == 'C' || ch == 'c') {
                                    lau_vram_write_string(g_vram, "\x1b[34mC\x1b[0m", 9);
                                } else if (ch == 'G' || ch == 'g') {
                                    lau_vram_write_string(g_vram, "\x1b[33mG\x1b[0m", 9);
                                } else {
                                    lau_vram_write_char(g_vram, ch);
                                }
                            } else {
                                lau_vram_write_char(g_vram, ch);
                            }
                        }
                    } else {
                        // In terminal mode, buffer commands and execute on enter
                        if (ch == '\n' || ch == '\r') {
                            lau_vram_write_string(g_vram, "\r\n", 2);
                            if (cmd_len > 0) {
                                cmd_buf[cmd_len] = '\0';
                                execute_command(cmd_buf);
                                cmd_len = 0;
                                cmd_buf[0] = '\0';
                            }
                            lau_vram_write_string(g_vram, "zmm-vm> ", 8);
                        } else if (ch == 127 || ch == '\b') {
                            if (cmd_len > 0) {
                                cmd_len--;
                                cmd_buf[cmd_len] = '\0';
                                lau_vram_write_char(g_vram, '\b');
                                lau_vram_write_char(g_vram, ' ');
                                lau_vram_write_char(g_vram, '\b');
                            }
                        } else if (ch >= 32 && ch < 127) {
                            char hook_cmd[64] = {0};
                            for (int i = 0; i < g_chrget_hooks_count; i++) {
                                if (g_chrget_hooks[i].key == ch) {
                                    snprintf(hook_cmd, sizeof(hook_cmd), "%s", g_chrget_hooks[i].cmd);
                                    break;
                                }
                            }
                            if (hook_cmd[0]) {
                                lau_vram_write_string(g_vram, "\r\n[CHRGET INTERCEPT]\r\n", 22);
                                execute_command(hook_cmd);
                                continue;
                            }
                            if (cmd_len < (int)sizeof(cmd_buf) - 2) {
                                cmd_buf[cmd_len++] = ch;
                                cmd_buf[cmd_len] = '\0';
                                lau_vram_write_char(g_vram, ch);
                            }
                        }
                    }
                }
                if (n_read == 0 && !got_chars) {
                    running = false;
                }
            }
        } else {
            wl_display_cancel_read(display);
        }
    }

    if (wl_buffers[0]) wl_buffer_destroy(wl_buffers[0]);
    if (wl_buffers[1]) wl_buffer_destroy(wl_buffers[1]);
    if (pixel_datas[0]) munmap(pixel_datas[0], win_width * win_height * 4);
    if (pixel_datas[1]) munmap(pixel_datas[1], win_width * win_height * 4);
    if (back_buffer) free(back_buffer);
    if (xdg_toplevel) xdg_toplevel_destroy(xdg_toplevel);
    if (xdg_surface) xdg_surface_destroy(xdg_surface);
    if (surface) wl_surface_destroy(surface);
    if (keyboard) wl_keyboard_destroy(keyboard);
    if (pointer) wl_pointer_destroy(pointer);
    if (seat) wl_seat_destroy(seat);
    if (xdg_wm_base) xdg_wm_base_destroy(xdg_wm_base);
    if (shm) wl_shm_destroy(shm);
    if (compositor) wl_compositor_destroy(compositor);
    if (registry) wl_registry_destroy(registry);
    if (display) wl_display_disconnect(display);

    tsfi_zmm_vm_destroy(&vm);
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_free_all_active(void);
    lau_free_all_active();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();

    printf("[TERMINAL] Exited gracefully.\n");
    return 0;
}
