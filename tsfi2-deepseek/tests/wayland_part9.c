        handle_term128_input(ch);
        return;
    }

    if (g_editor_mode == MODE_ALCHEMIST) {
        char ch = (char)utf32;
        if (key == KEY_ESC || key == 1) {
            ch = 27;
        }
        handle_alchemist_input(ch);
        return;
    }

    if (g_editor_mode == MODE_SKIFOLLY) {
        char ch = (char)utf32;
        if (key == KEY_ESC || key == 1) {
            ch = 27;
        }
        handle_skifolly_input(ch);
        return;
    }

    if (g_editor_mode == MODE_DARTS) {
        char ch = (char)utf32;
        if (key == KEY_ESC || key == 1) {
            ch = 27;
        }
        handle_darts_input(ch);
        return;
    }

    if (g_editor_mode == MODE_MAKEWAVE) {
        char ch = (char)utf32;
        if (key == KEY_ESC || key == 1) {
            ch = 27;
        } else if (key == 57 || key == KEY_SPACE) {
            ch = ' ';
        }
        handle_makewave_input(ch);
        return;
    }

    if (g_editor_mode == MODE_PLOTWAVE) {
        char ch = (char)utf32;
        if (key == KEY_ESC || key == 1) {
            ch = 27;
        } else if (key == 57 || key == KEY_SPACE) {
            ch = ' ';
        }
        handle_plotwave_input(ch);
        return;
    }

    if (g_editor_mode == MODE_CONSTRUCTION_CO) {
        char ch = (char)utf32;
        if (key == KEY_ESC || key == 1) {
            ch = 27;
        } else if (key == 17 || key == 103) { // W
            ch = 'w';
        } else if (key == 31 || key == 108) { // S
            ch = 's';
        } else if (key == 30 || key == 105) { // A
            ch = 'a';
        } else if (key == 32 || key == 106) { // D
            ch = 'd';
        } else if (key == 57 || key == KEY_SPACE) { // Spacebar
            ch = ' ';
        }
        handle_construction_co_input(ch);
        return;
    }

    if (g_editor_mode == MODE_SPACEPATROL) {
        char ch = (char)utf32;
        if (key == KEY_ESC || key == 1) {
            ch = 27;
        } else if (key == 103 || key == 17) { // W or UP -> pitch up (ch = 'i')
            ch = 'i';
        } else if (key == 108 || key == 31) { // S or DOWN -> pitch down (ch = 'k')
            ch = 'k';
        } else if (key == 105 || key == 30) { // A or LEFT -> yaw left (ch = 'j')
            ch = 'j';
        } else if (key == 106 || key == 32) { // D or RIGHT -> yaw right (ch = 'l')
            ch = 'l';
        }
        handle_spacepatrol_input(ch);
        return;
    }

    if (g_editor_mode == MODE_PTE) {
        char ch = (char)utf32;
        if (key == KEY_ENTER || key == 28) {
            ch = 13;
        } else if (key == KEY_BACKSPACE || key == 14) {
            ch = 8;
        } else if (key == KEY_ESC || key == 1) {
            ch = 27;
        } else if (key == 38) { // L
            ch = 12;
        } else if (key == 25) { // P
            ch = 16;
        } else if (key == 19) { // R
            ch = 18;
        }
        handle_pte_input(ch);
        return;
    }

    if (g_editor_mode == MODE_CHECKLIST) {
        char ch = (char)utf32;
        if (key == KEY_ENTER || key == 28) {
            ch = '\n';
        } else if (key == KEY_BACKSPACE || key == 14) {
            ch = '\b';
        } else if (key == KEY_ESC || key == 1) {
            ch = '\x1b';
        } else if (key == 103 || key == 17) {
            ch = 'w';
        } else if (key == 108 || key == 31) {
            ch = 's';
        }
        handle_checklist_input(ch);
        return;
    }

    if (g_editor_mode == MODE_CREATOR) {
        if (key == 103 || key == 17) { // UP or W
            handle_creator_input('u');
        } else if (key == 108 || key == 31) { // DOWN or S
            handle_creator_input('d');
        } else if (key == KEY_ENTER || key == 28) {
            handle_creator_input('\n');
        } else if (key == KEY_ESC || key == 1) {
            if (g_creator_editing_value) {
                handle_creator_input(27);
            } else {
                g_editor_mode = MODE_TERMINAL;
                g_mercenary_active = false;
                g_pong_active = false;
                execute_command("GO MENU");
            }
        } else if (key == KEY_BACKSPACE || key == 14) {
            handle_creator_input(127);
        } else if (utf32 >= 32 && utf32 < 127) {
            handle_creator_input((char)utf32);
        }
        return;
    }

    if (key == KEY_ESC || key == 1) {
        if (g_editor_mode != MODE_TERMINAL) {
            g_editor_mode = MODE_TERMINAL;
            g_mercenary_active = false;
            g_pong_active = false;
            execute_command("GO MENU");
            return;
        }
        running = false;
        return;
    }

    if (g_editor_mode != MODE_TERMINAL) {
        if (g_editor_mode == MODE_ZMACHINE) {
            if (key == KEY_ENTER || key == 28) {
                lau_vram_write_string(g_vram, "\r\n", 2);
                if (cmd_len > 0) {
                    cmd_buf[cmd_len] = '\0';
                    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len);
                    
                    if (strncasecmp(cmd_buf, "create ", 7) == 0) {
                        char *p = cmd_buf + 7;
                        uint32_t create_roomId = strtoul(p, &p, 10);
                        while (*p == ' ' || *p == '\t') p++;
                        unsigned int n = 0, s = 0, e = 0, w = 0;
                        sscanf(p, "%u,%u,%u,%u", &n, &s, &e, &w);
                        uint64_t create_exits = ((uint64_t)n << 24) | ((uint64_t)s << 16) | ((uint64_t)e << 8) | w;
                        
                        while (*p && *p != ' ' && *p != '\t') p++;
                        while (*p == ' ' || *p == '\t') p++;
                        const char *create_desc = p;
                        size_t create_desc_len = strlen(create_desc);
                        
                        uint8_t calldata[1024] = {0};
                        calldata[0] = 0xd6; calldata[1] = 0xc5; calldata[2] = 0x26; calldata[3] = 0x8c;
                        calldata[35] = create_roomId & 0xFF;
                        calldata[34] = (create_roomId >> 8) & 0xFF;
                        calldata[33] = (create_roomId >> 16) & 0xFF;
                        calldata[32] = (create_roomId >> 24) & 0xFF;
                        calldata[67] = 0x60; // offset 96
                        calldata[99] = create_exits & 0xFF;
                        calldata[98] = (create_exits >> 8) & 0xFF;
                        calldata[97] = (create_exits >> 16) & 0xFF;
                        calldata[96] = (create_exits >> 24) & 0xFF;
                        calldata[131] = create_desc_len & 0xFF;
                        calldata[130] = (create_desc_len >> 8) & 0xFF;
                        calldata[129] = (create_desc_len >> 16) & 0xFF;
                        calldata[128] = (create_desc_len >> 24) & 0xFF;
                        memcpy(&calldata[132], create_desc, create_desc_len);
                        
                        size_t string_padded_len = ((create_desc_len + 31) / 32) * 32;
                        size_t calldatasize = 4 + 32 + 32 + 32 + 32 + string_padded_len;
                        
                        uint8_t retval[32] = {0};
                        size_t retval_len = sizeof(retval);
                        if (run_zmachine_transaction(calldata, calldatasize, retval, &retval_len, true)) {
                            char ok_msg[128];
                            snprintf(ok_msg, sizeof(ok_msg), "  [Room %u created. Exits N=%u S=%u E=%u W=%u]\r\n", create_roomId, n, s, e, w);
                            lau_vram_write_string(g_vram, ok_msg, strlen(ok_msg));
                        } else {
                            lau_vram_write_string(g_vram, "  Error: Failed to register room on-chain.\r\n", 44);
                        }
                    } else {
                        // Setup calldata for parseCommand(address player, bytes cmd)
                        // selector: 0xf1ba03f9
                        uint8_t calldata[512] = {0};
                        calldata[0] = 0xf1; calldata[1] = 0xba; calldata[2] = 0x03; calldata[3] = 0xf9;
                        calldata[35] = 0x01; // player address
                        calldata[67] = 0x40; // offset
                        calldata[96] = (cmd_len >> 24) & 0xFF;
                        calldata[97] = (cmd_len >> 16) & 0xFF;
                        calldata[98] = (cmd_len >> 8) & 0xFF;
                        calldata[99] = cmd_len & 0xFF;
                        memcpy(&calldata[100], cmd_buf, cmd_len);
                        
                        size_t string_padded_len = ((cmd_len + 31) / 32) * 32;
                        size_t calldatasize = 4 + 32 + 32 + 32 + string_padded_len;
                        
                        uint8_t retval[4096] = {0};
                        size_t retval_len = sizeof(retval);
                        if (run_zmachine_transaction(calldata, calldatasize, retval, &retval_len, false)) {
                            if (retval_len >= 64) {
                                uint32_t str_len = (retval[60] << 24) | (retval[61] << 16) | (retval[62] << 8) | retval[63];
                                if (str_len > 511) str_len = 511;
                                char response[512] = {0};
                                memcpy(response, &retval[64], str_len);
                                response[str_len] = '\0';
                                
                                lau_vram_write_string(g_vram, "  ", 2);
                                lau_vram_write_string(g_vram, response, strlen(response));
                                lau_vram_write_string(g_vram, "\r\n", 2);

                                // On-chain persistence: if EVM is online and command is state-modifying, transact it
                                char test_res[128];
                                if (call_local_evm("net_version", "", test_res, sizeof(test_res))) {
                                    bool is_write_cmd = false;
                                    if (cmd_len >= 3) {
                                        if (strncmp(cmd_buf, "zap", 3) == 0) {
                                            is_write_cmd = true;
                                        }
                                    }
                                    if (cmd_len >= 4) {
                                        if (strncmp(cmd_buf, "nort", 4) == 0 ||
                                            strncmp(cmd_buf, "sout", 4) == 0 ||
                                            strncmp(cmd_buf, "east", 4) == 0 ||
                                            strncmp(cmd_buf, "west", 4) == 0 ||
                                            strncmp(cmd_buf, "take", 4) == 0 ||
                                            strncmp(cmd_buf, "aim ", 4) == 0 ||
                                            strncmp(cmd_buf, "shoo", 4) == 0 ||
                                            strncmp(cmd_buf, "star", 4) == 0 ||
                                            strncmp(cmd_buf, "stee", 4) == 0 ||
                                            strncmp(cmd_buf, "acce", 4) == 0 ||
                                            strncmp(cmd_buf, "fire", 4) == 0) {
                                            is_write_cmd = true;
                                        }
                                    }
                                    if (is_write_cmd) {
                                        size_t dummy_len = 0;
                                        run_zmachine_transaction(calldata, calldatasize, NULL, &dummy_len, true);
                                    }
                                }
                            } else {
                                lau_vram_write_string(g_vram, "  Error: Invalid VM return length.\r\n", 36);
                            }
                        } else {
                            lau_vram_write_string(g_vram, "  Error: Z-Machine Yul execution failed.\r\n", 42);
                        }
                    }
                    
                    cmd_len = 0;
                    cmd_buf[0] = '\0';
                }
                lau_vram_write_string(g_vram, "zmachine> ", 10);
            } else if (key == KEY_BACKSPACE || key == 14) {
                if (cmd_len > 0) {
                    cmd_len--;
                    cmd_buf[cmd_len] = '\0';
                    lau_vram_write_char(g_vram, '\b');
                    lau_vram_write_char(g_vram, ' ');
                    lau_vram_write_char(g_vram, '\b');
                }
            } else if (utf32 >= 32 && utf32 < 127 && cmd_len < 255) {
                char ch = (char)utf32;
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
                    return;
                }
                cmd_buf[cmd_len++] = ch;
                cmd_buf[cmd_len] = '\0';
                lau_vram_write_char(g_vram, ch);
            }
            return;
        }

        char ch = (char)utf32;
        if (key == 103) ch = 'w';
        else if (key == 108) ch = 's';
        else if (key == 105) ch = 'a';
        else if (key == 106) ch = 'd';

        if (g_editor_mode == MODE_INSTACALC) {
            handle_instacalc_input(ch);
            return;
        } else if (g_editor_mode == MODE_YULBUILD) {
            handle_yulbuild_input(ch);
            return;
        } else if (g_editor_mode == MODE_APPLEPANIC) {
            handle_applepanic_input(ch);
            return;
        } else if (g_editor_mode == MODE_AIRASSAULT) {
            handle_airassault_input(ch);
            return;
        } else if (g_editor_mode == MODE_SLINKYBEAR) {
            handle_slinkybear_input(ch);
            return;
        } else if (g_editor_mode == MODE_SLINKYPANIC) {
            handle_slinkypanic_input(ch);
            return;
        } else if (g_editor_mode == MODE_ALPINER) {
            handle_alpiner_input(ch);
            return;
        }

        if (key == KEY_ENTER || key == 28) {
            lau_vram_write_string(g_vram, "\r\n", 2);
        } else if (key == KEY_BACKSPACE || key == 14) {
            lau_vram_write_char(g_vram, '\b');
            lau_vram_write_char(g_vram, ' ');
            lau_vram_write_char(g_vram, '\b');
        } else if (utf32 >= 32 && utf32 < 127) {
            if (g_editor_mode == MODE_DNATYPEWRITER) {
                char c = (char)utf32;
                if (c == 'A' || c == 'a') {
                    lau_vram_write_string(g_vram, "\x1b[32mA\x1b[0m", 9);
                } else if (c == 'T' || c == 't') {
                    lau_vram_write_string(g_vram, "\x1b[31mT\x1b[0m", 9);
                } else if (c == 'C' || c == 'c') {
                    lau_vram_write_string(g_vram, "\x1b[34mC\x1b[0m", 9);
                } else if (c == 'G' || c == 'g') {
                    lau_vram_write_string(g_vram, "\x1b[33mG\x1b[0m", 9);
                } else {
                    lau_vram_write_char(g_vram, c);
                }
            } else {
                lau_vram_write_char(g_vram, (char)utf32);
            }
        }
        return;
    }

    if (g_editor_mode == MODE_TERMINAL && key >= 59 && key <= 66) {
        int idx = key - 59;
        const char *macro = g_fkey_macros[idx];
        for (int i = 0; macro[i] != '\0'; i++) {
            char ch = macro[i];
            if (ch == '\n' || ch == '\r') {
                lau_vram_write_string(g_vram, "\r\n", 2);
                if (cmd_len > 0) {
                    cmd_buf[cmd_len] = '\0';
                    execute_command(cmd_buf);
                    cmd_len = 0;
                    cmd_buf[0] = '\0';
                }
                lau_vram_write_string(g_vram, "zmm-vm> ", 8);
            } else {
                if (cmd_len < (int)sizeof(cmd_buf) - 2) {
                    cmd_buf[cmd_len++] = ch;
                    cmd_buf[cmd_len] = '\0';
                    lau_vram_write_char(g_vram, ch);
                }
            }
        }
        return;
    }

    if (key == KEY_ENTER || key == 28) {
        lau_vram_write_string(g_vram, "\r\n", 2);
        if (cmd_len > 0) {
            cmd_buf[cmd_len] = '\0';
            execute_command(cmd_buf);
            cmd_len = 0;
            cmd_buf[0] = '\0';
        }
        lau_vram_write_string(g_vram, "zmm-vm> ", 8);
    } else if (key == KEY_BACKSPACE || key == 14) {
        if (cmd_len > 0) {
            cmd_len--;
            cmd_buf[cmd_len] = '\0';
            lau_vram_write_char(g_vram, '\b');
            lau_vram_write_char(g_vram, ' ');
            lau_vram_write_char(g_vram, '\b');
        }
    } else if (utf32 >= 32 && utf32 < 127) {
        if (cmd_len < (int)sizeof(cmd_buf) - 2) {
            cmd_buf[cmd_len++] = (char)utf32;
            cmd_buf[cmd_len] = '\0';
            lau_vram_write_char(g_vram, (char)utf32);
        }
    }
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_handle_keymap,
    .enter = keyboard_handle_enter,
    .leave = keyboard_handle_leave,
    .key = keyboard_handle_key,
    .modifiers = keyboard_handle_modifiers,
    .repeat_info = keyboard_handle_repeat_info
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t caps) {
    (void)data;
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !keyboard) {
        keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    }
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer) {
        pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointer_listener, NULL);
    }
}
static void seat_handle_name(void *data, struct wl_seat *seat, const char *name) {
    (void)data; (void)seat; (void)name;
}
static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = seat_handle_name
};

// XDG shell listeners
static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(xdg_wm_base, serial);
}
static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surf, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xdg_surf, serial);
    configured = true;
}
static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure
};

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    (void)data; (void)xdg_toplevel; (void)states;
    if (width > 0 && height > 0) {
        pending_width = width;
        pending_height = height;
        resize_pending = true;
    }
}
static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    (void)data; (void)xdg_toplevel;
    running = false;
}
static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close
};

static struct wl_buffer *create_shm_buffer(int width, int height, uint32_t **out_pixels) {
    int stride = width * 4;
    int size = stride * height;
    
    int fd = memfd_create("shm-terminal", MFD_CLOEXEC);
    if (fd < 0) return NULL;
    if (ftruncate(fd, size) < 0) {
        close(fd);
        return NULL;
    }
    
    *out_pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (*out_pixels == MAP_FAILED) {
        close(fd);
        return NULL;
    }
    
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
    
    return buffer;
}

static void draw_line(uint32_t *buf, int width, int height, int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    
    while (1) {
        if (x1 >= 12 && x1 < width - 22 && y1 >= 57 && y1 < height - 32) {
            buf[y1 * width + x1] = color;
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

static void draw_circle(uint32_t *buf, int width, int height, int xc, int yc, int r, uint32_t color) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    
    while (y >= x) {
        int px[8] = { xc+x, xc-x, xc+x, xc-x, xc+y, xc-y, xc+y, xc-y };
        int py[8] = { yc+y, yc+y, yc-y, yc-y, yc+x, yc+x, yc-x, yc-x };
        for (int i = 0; i < 8; i++) {
            if (px[i] >= 12 && px[i] < width - 22 && py[i] >= 57 && py[i] < height - 32) {
                buf[py[i] * width + px[i]] = color;
            }
        }
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}
static void draw_fill_rect(uint32_t *buf, int width, int height, int x1, int y1, int x2, int y2, uint32_t color) {
    int start_x = x1 < x2 ? x1 : x2;
    int end_x = x1 < x2 ? x2 : x1;
    int start_y = y1 < y2 ? y1 : y2;
    int end_y = y1 < y2 ? y2 : y1;
    for (int y = start_y; y <= end_y; y++) {
        for (int x = start_x; x <= end_x; x++) {
            if (x >= 12 && x < width - 22 && y >= 57 && y < height - 32) {
                buf[y * width + x] = color;
            }
        }
    }
}

static void draw_fill_round_rect(uint32_t *buf, int width, int height, int x1, int y1, int x2, int y2, int r, uint32_t color) {
    int start_x = x1 < x2 ? x1 : x2;
    int end_x = x1 < x2 ? x2 : x1;
    int start_y = y1 < y2 ? y1 : y2;
    int end_y = y1 < y2 ? y2 : y1;
    if (r <= 0) {
        draw_fill_rect(buf, width, height, start_x, start_y, end_x, end_y, color);
        return;
    }
    int w = end_x - start_x + 1;
    int h = end_y - start_y + 1;
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    draw_fill_rect(buf, width, height, start_x + r, start_y, end_x - r, end_y, color);
    draw_fill_rect(buf, width, height, start_x, start_y + r, start_x + r - 1, end_y - r, color);
    draw_fill_rect(buf, width, height, end_x - r + 1, start_y + r, end_x, end_y - r, color);

    for (int dy = 0; dy < r; dy++) {
        for (int dx = 0; dx < r; dx++) {
            if (dx*dx + dy*dy <= r*r) {
                int tx1 = start_x + r - 1 - dx;
                int ty1 = start_y + r - 1 - dy;
                if (tx1 >= 12 && tx1 < width - 22 && ty1 >= 57 && ty1 < height - 32) {
                    buf[ty1 * width + tx1] = color;
                }
                int tx2 = end_x - r + 1 + dx;
                int ty2 = start_y + r - 1 - dy;
                if (tx2 >= 12 && tx2 < width - 22 && ty2 >= 57 && ty2 < height - 32) {
                    buf[ty2 * width + tx2] = color;
                }
                int tx3 = start_x + r - 1 - dx;
                int ty3 = end_y - r + 1 + dy;
                if (tx3 >= 12 && tx3 < width - 22 && ty3 >= 57 && ty3 < height - 32) {
                    buf[ty3 * width + tx3] = color;
                }
                int tx4 = end_x - r + 1 + dx;
                int ty4 = end_y - r + 1 + dy;
                if (tx4 >= 12 && tx4 < width - 22 && ty4 >= 57 && ty4 < height - 32) {
                    buf[ty4 * width + tx4] = color;
                }
            }
        }
    }
}

static void vm_poke(TsfiZmmVmState *vstate, uint64_t addr, uint8_t val);

static void sync_vram_to_cpu(void) {
    static uint8_t shadow_buffer[9600];
    static bool shadow_initialized = false;
    
    uint8_t current_buffer[9600];
    for (int y = 0; y < 60; y++) {
        for (int x = 0; x < 160; x++) {
            current_buffer[y * 160 + x] = (uint8_t)g_vram->grid[y][x].character;
        }
    }
    
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len);
    
    if (!shadow_initialized) {
        memcpy(shadow_buffer, current_buffer, 9600);
        shadow_initialized = true;
        
        static uint8_t calldata[9700];
        calldata[0] = 0xf7; calldata[1] = 0xe8; calldata[2] = 0xe8; calldata[3] = 0x1b;
        memset(&calldata[4], 0, 32);
        calldata[4 + 30] = 0x40;
        calldata[4 + 31] = 0x00;
        memset(&calldata[36], 0, 32);
        calldata[36 + 31] = 0x40;
        memset(&calldata[68], 0, 32);
        calldata[68 + 30] = 0x25;
        calldata[68 + 31] = 0x80;
        memcpy(&calldata[100], current_buffer, 9600);
        
        uint8_t retval[32];
        size_t retval_len = 32;
        lau_yul_thunk_execute("cpu6502", calldata, 9700, retval, &retval_len);
        return;
    }
    
    int diff_count = 0;
    for (int i = 0; i < 9600; i++) {
        if (current_buffer[i] != shadow_buffer[i]) {
            diff_count++;
        }
    }
    
    if (diff_count > 50) {
        memcpy(shadow_buffer, current_buffer, 9600);
        static uint8_t calldata[9700];
        calldata[0] = 0xf7; calldata[1] = 0xe8; calldata[2] = 0xe8; calldata[3] = 0x1b;
        memset(&calldata[4], 0, 32);
        calldata[4 + 30] = 0x40;
        calldata[4 + 31] = 0x00;
        memset(&calldata[36], 0, 32);
        calldata[36 + 31] = 0x40;
        memset(&calldata[68], 0, 32);
        calldata[68 + 30] = 0x25;
        calldata[68 + 31] = 0x80;
        memcpy(&calldata[100], current_buffer, 9600);
        
        uint8_t retval[32];
        size_t retval_len = 32;
        lau_yul_thunk_execute("cpu6502", calldata, 9700, retval, &retval_len);
    } else if (diff_count > 0) {
        for (int i = 0; i < 9600; i++) {
            if (current_buffer[i] != shadow_buffer[i]) {
                vm_poke(&vm, 0x4000 + i, current_buffer[i]);
                shadow_buffer[i] = current_buffer[i];
            }
        }
    }
}

#include "tsfi_vae_firmware.h"

// VAE presentation state
static LauVaeFirmware_State g_vae_present_state;
static float *g_vae_density_cache = NULL;
static int g_vae_density_w = 0;
static int g_vae_density_h = 0;

static void update_vae_presentation_field(int w, int h) {
    if (!g_vae_density_cache || g_vae_density_w != w || g_vae_density_h != h) {
        if (g_vae_density_cache) free(g_vae_density_cache);
        g_vae_density_cache = (float*)malloc(w * h * sizeof(float));
        g_vae_density_w = w;
        g_vae_density_h = h;
    }
    
    // Construct local NandTrapState using basic parameters to mock local fiber activation
    static NandTrapState nt;
    static bool nt_initialized = false;
    if (!nt_initialized) {
        memset(&nt, 0, sizeof(NandTrapState));
        nt.magic = NAND_TRAP_MAGIC;
        nt.version = NAND_TRAP_VERSION;
        // Seed 1024 fibers with default weights
        for (int i = 0; i < GRANS; i++) {
            nt.fibers[i].x = sinf((float)i * 0.05f) * 0.5f;
            nt.fibers[i].y = cosf((float)i * 0.05f) * 0.5f;
            nt.fibers[i].weight = 1.0f;
        }
        nt_initialized = true;
    }

    // Dynamic LTI Causal Discovery step to morph the VAE latent configuration smoothly (wilderness simulation)
    static NandTrapState next_nt;
    tsfi_nand_trap_causal_discovery(&nt, &next_nt, 0.005f);
    
    // Check for extreme expansion and keep fiber bounds normalized
    for (int i = 0; i < GRANS; i++) {
        if (next_nt.fibers[i].x > 2.0f || next_nt.fibers[i].x < -2.0f) next_nt.fibers[i].x = sinf((float)i * 0.05f) * 0.5f;
        if (next_nt.fibers[i].y > 2.0f || next_nt.fibers[i].y < -2.0f) next_nt.fibers[i].y = cosf((float)i * 0.05f) * 0.5f;
        if (next_nt.fibers[i].weight > 3.0f || next_nt.fibers[i].weight < 0.1f) next_nt.fibers[i].weight = 1.0f;
    }
    nt = next_nt;
    
    // Boot VAE directly off our dynamic NAND state representation
    tsfi_vae_firmware_boot(&g_vae_present_state, &nt);
    // Decode the 1024 NAND fibers to our screen-sized density field
    tsfi_vae_decode_to_field(&g_vae_present_state, g_vae_density_cache, w, h);
}

void render_terminal_display(void) {
    // 1. Update the VAE density field off current active NAND state
    update_vae_presentation_field(win_width, win_height);

    // 2. Render VAE Wilderness Background (Modulated by density cache)
    for (int y = 0; y < win_height; y++) {
        for (int x = 0; x < win_width; x++) {
            int idx = y * win_width + x;
            float density = g_vae_density_cache[idx];
            
            // Map SDF density values to forest moss greens and deep wild shadows
            if (density < 8.0f) {
                // High density region: Lush organic wilderness green
                float intensity = 1.0f - (density / 8.0f);
                if (intensity < 0.0f) intensity = 0.0f;
                uint8_t g = (uint8_t)(35.0f + intensity * 65.0f); // 35..100
                uint8_t r = (uint8_t)(15.0f + intensity * 25.0f); // 15..40
                uint8_t b = (uint8_t)(20.0f + intensity * 30.0f); // 20..50
                back_buffer[idx] = 0xFF000000 | (r << 16) | (g << 8) | b;
            } else {
                // Background: Obsidian dark void
                back_buffer[idx] = 0xFF0A0B10;
            }
        }
    }

    StagingBuffer sb = {
        .magic = TSFI_STAGING_MAGIC,
        .width = win_width,
        .height = win_height,
        .stride = win_width * 4,
        .data = back_buffer
    };

    // ANTIC Display List CPU Terminal Improvements
    typedef struct {
        int mode; // 2: Header, 4: Console/Text, 8: Diagnostics Graph
        int start_y; // Pixel Y coordinate
        int height; // Pixel height of this band
        int lms_offset; // VRAM row offset (LMS)
        int vscrol; // Vertical fine scroll offset (pixels, 0..17)
    } TerminalDisplayInstruction;

    // Static display list and smooth scroll state
    // Static display list and smooth scroll state
    static float s_smooth_scroll_y = 0.0f;
    static int s_last_target_y = 0;
    
    int char_w = 10;
    int char_h = 18;
    mon_x = 22;
    mon_y = 67;

    int max_rows = (win_height - 80) / char_h;
    if (max_rows < 5) max_rows = 5;
    if (max_rows > 35) max_rows = 35;

    int target_start_y = 0;
    if (g_vram->cursor_y >= max_rows) {
        target_start_y = g_vram->cursor_y - max_rows + 1;
    }

    // Query Folklore MMIO register space for overridden scrolling values:
    // FOLKLORE_LMS_ADDR (MMIO addr: 54624 -> $D560): Direct row index base (64-bit/uint256)
    // FOLKLORE_VSCROL   (MMIO addr: 54632 -> $D568): Fractional vertical scroll offset (32-bit fixed-point, 16.16 scaled by 65536)
    uint64_t folklore_lms_val = 0;
    uint64_t folklore_vscrol_val = 0;
    
    {
        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len);
        uint8_t peek_cd[36] = {0};
        peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69; // peek(uint256)
        
        // Peek LMS
        uint64_t mmio_lms = 54624;
        for (int k = 0; k < 8; k++) peek_cd[4 + 31 - k] = (mmio_lms >> (k * 8)) & 0xFF;
        uint8_t ret_lms[32] = {0};
        size_t ret_lms_len = 32;
        if (lau_yul_thunk_execute("cpu6502", peek_cd, 36, ret_lms, &ret_lms_len)) {
            for (int k = 0; k < 8; k++) folklore_lms_val = (folklore_lms_val << 8) | ret_lms[24 + k];
        }

        // Peek VSCROL
        uint64_t mmio_vscrol = 54632;
        for (int k = 0; k < 8; k++) peek_cd[4 + 31 - k] = (mmio_vscrol >> (k * 8)) & 0xFF;
        uint8_t ret_vscrol[32] = {0};
        size_t ret_vscrol_len = 32;
        if (lau_yul_thunk_execute("cpu6502", peek_cd, 36, ret_vscrol, &ret_vscrol_len)) {
            for (int k = 0; k < 8; k++) folklore_vscrol_val = (folklore_vscrol_val << 8) | ret_vscrol[24 + k];
        }
    }

    int lms_row_base = 0;
    int fine_vscrol = 0;

    if (folklore_lms_val != 0 || folklore_vscrol_val != 0) {
        // If MMIO registers have custom values, use them directly as hardware registers
        lms_row_base = (int)folklore_lms_val;
        // folklore_vscrol_val is 16.16 fixed point representation of fine scroll offset
        fine_vscrol = (int)(folklore_vscrol_val / 65536);
        if (fine_vscrol < 0) fine_vscrol = 0;
        if (fine_vscrol >= char_h) fine_vscrol = char_h - 1;
    } else {
        // Fallback to smooth interpolation loop
        if (s_last_target_y != target_start_y) {
            if (abs(target_start_y - s_last_target_y) > 10) {
                s_smooth_scroll_y = (float)target_start_y;
            }
            s_last_target_y = target_start_y;
        }
        float diff = (float)target_start_y - s_smooth_scroll_y;
        if (fabsf(diff) > 0.001f) {
            s_smooth_scroll_y += diff * 0.25f;
        } else {
            s_smooth_scroll_y = (float)target_start_y;
        }

        lms_row_base = (int)floorf(s_smooth_scroll_y);
        fine_vscrol = (int)roundf((s_smooth_scroll_y - (float)lms_row_base) * (float)char_h);
        if (fine_vscrol < 0) fine_vscrol = 0;
        if (fine_vscrol >= char_h) fine_vscrol = char_h - 1;
    }

    // Display List Instructions definitions
    int console_clip_y0 = 55;
    int console_clip_y1 = win_height - 32;

    TerminalDisplayInstruction display_list[] = {
        { .mode = 2, .start_y = 0, .height = 46, .lms_offset = 0, .vscrol = 0 },
        { .mode = 4, .start_y = 46, .height = (win_height - 32 - 46), .lms_offset = lms_row_base, .vscrol = fine_vscrol },
        { .mode = 8, .start_y = win_height - 31, .height = 31, .lms_offset = 0, .vscrol = 0 }
    };

    // Dracula premium console palette
    static const uint32_t palette[16] = {
        0xFF000000, 0xFFFF5555, 0xFF50FA7B, 0xFFF1FA8C,
        0xFFBD93F9, 0xFFFF79C6, 0xFF8BE9FD, 0xFFF8F8F2,
        0xFF6272A4, 0xFFFF5555, 0xFF50FA7B, 0xFFF1FA8C,
        0xFFBD93F9, 0xFFFF79C6, 0xFF8BE9FD, 0xFFF8F8F2
    };

    int display_cols = (win_width - 44) / char_w;
    if (display_cols < 10) display_cols = 10;

    if (g_superterm_mode) {
        // Auto scroll horizontally to track cursor
        if (g_vram->cursor_x >= g_superterm_scroll_x + display_cols) {
            g_superterm_scroll_x = g_vram->cursor_x - display_cols + 1;
        } else if (g_vram->cursor_x < g_superterm_scroll_x) {
            g_superterm_scroll_x = g_vram->cursor_x;
        }
        int max_scroll = g_superterm_cols - display_cols;
        if (max_scroll < 0) max_scroll = 0;
        if (g_superterm_scroll_x > max_scroll) g_superterm_scroll_x = max_scroll;
        if (g_superterm_scroll_x < 0) g_superterm_scroll_x = 0;
    } else {
        g_superterm_scroll_x = 0;
    }

    // Execute Display List
    for (size_t dl_idx = 0; dl_idx < sizeof(display_list)/sizeof(display_list[0]); dl_idx++) {
        TerminalDisplayInstruction inst = display_list[dl_idx];

        if (inst.mode == 2) {
            // Render Mode 2 Title/Status Bar
            if (win_height > 50) {
                for (int x = 0; x < win_width; x++) {
                    back_buffer[45 * win_width + x] = 0xFF8BE9FD; // Pastel Cyan status divider
                }
            }
            draw_debug_text(&sb, 20, 15, "TSFI SOVEREIGN CPU TERMINAL [ANTIC DL]", 0xFFBD93F9, true); // Pastel Purple
            draw_debug_text(&sb, win_width - 280, 15, "[ SYS: AUDITED ] [ VM: RUNNING ]", 0xFF50FA7B, true); // Pastel Green
        }
        else if (inst.mode == 4) {
            // Render Mode 4 Console Band with smooth scrolling
            // Border around terminal panel
            if (win_height > 70) {
                // Left & Right borders
                for (int y = console_clip_y0; y < console_clip_y1; y++) {
                    for (int dx = 0; dx < 2; dx++) {
                        back_buffer[y * win_width + 10 + dx] = 0xFF6272A4; // Slate gray border
                        back_buffer[y * win_width + (win_width - 12) + dx] = 0xFF6272A4;
                    }
                }
                // Top & Bottom borders
                for (int x = 10; x < win_width - 10; x++) {
                    for (int dy = 0; dy < 2; dy++) {
                        back_buffer[(console_clip_y0 + dy) * win_width + x] = 0xFF6272A4;
                        back_buffer[((console_clip_y1 - 2) + dy) * win_width + x] = 0xFF6272A4;
                    }
                }
            }

            // Draw text cells with vertical smooth scroll offset and clipping
            // Since we scroll down, we render extra rows (max_rows + 2) to pad smooth transition
            int scroll_offset_y = inst.vscrol;
            for (int y = 0; y < max_rows + 2; y++) {
                int vram_y = inst.lms_offset + y;
                if (vram_y >= LAU_VRAM_ROWS || vram_y < 0) continue;

                for (int x = 0; x < display_cols; x++) {
                    int vram_x = g_superterm_scroll_x + x;
                    if (vram_x >= LAU_VRAM_COLS) break;
                    if (g_superterm_mode && vram_x >= g_superterm_cols) break;

                    LauVRAMCell cell = g_vram->grid[vram_y][vram_x];
                    if (cell.character > 32) {
                        uint32_t fg = palette[cell.fg_color & 0xF];
                        if (cell.attributes & 1) fg = palette[(cell.fg_color & 0x7) + 8]; // Bold/Bright

                        int px = mon_x + (x * char_w);
                        int py = mon_y + (y * char_h) - scroll_offset_y;

                        bool in_selection = false;
                        if (select_start_x >= 0 && select_start_y >= 0 && select_end_x >= 0 && select_end_y >= 0) {
                            int sy = select_start_y, ey = select_end_y;
                            int sx = select_start_x, ex = select_end_x;
                            if (sy > ey || (sy == ey && sx > ex)) {
                                sy = select_end_y; ey = select_start_y;
                                sx = select_end_x; ex = select_start_x;
                            }
                            if (vram_y > sy && vram_y < ey) in_selection = true;
                            else if (vram_y == sy && vram_y == ey) in_selection = (vram_x >= sx && vram_x <= ex);
                            else if (vram_y == sy) in_selection = (vram_x >= sx);
                            else if (vram_y == ey) in_selection = (vram_x <= ex);
                        }

                        if (in_selection) {
                            for (int dy = 0; dy < char_h; dy++) {
                                for (int dx = 0; dx < char_w; dx++) {
                                    int ty = py + dy;
                                    int tx = px + dx;
                                    if (tx >= 12 && tx < win_width - 22 && ty >= console_clip_y0 && ty < console_clip_y1) {
                                        back_buffer[ty * win_width + tx] = 0xFF44475A; // Dracula selection bg
                                    }
                                }
                            }
                            draw_debug_codepoint_clipped(&sb, px, py, cell.character, 0xFFF8F8F2, console_clip_y0, console_clip_y1);
                        } else {
                            draw_debug_codepoint_clipped(&sb, px, py, cell.character, fg, console_clip_y0, console_clip_y1);
                        }
                    }
                }
            }

            // Draw inverted green/amber cursor block (with smooth vertical translation)
            int cy = g_vram->cursor_y - inst.lms_offset;
            int cx = g_vram->cursor_x - g_superterm_scroll_x;
            if (cy >= -1 && cy < max_rows + 2 && cx >= 0 && cx < display_cols) {
                int px = mon_x + cx * char_w;
                int py = mon_y + cy * char_h - scroll_offset_y;
                for (int dy = 0; dy < char_h - 2; dy++) {
                    for (int dx = 0; dx < char_w; dx++) {
                        int ty = py + dy;
                        int tx = px + dx;
                        if (tx >= 12 && tx < win_width - 22 && ty >= console_clip_y0 && ty < console_clip_y1) {
                            back_buffer[ty * win_width + tx] ^= 0xFF00FF00; // Electric green invert tint
                        }
                    }
                }
            }
        }
        else if (inst.mode == 8) {
            // Render Mode 8 JIT Diagnostics Bar / Graphics
            // Gray baseline divider
            for (int x = 10; x < win_width - 10; x++) {
                back_buffer[(win_height - 32) * win_width + x] = 0xFF6272A4;
            }
            // Draw a subtle diagnostic scrolling wave/histogram
            static int wave_offset = 0;
            wave_offset = (wave_offset + 1) % win_width;
            for (int x = 20; x < win_width - 20; x += 4) {
                float angle = (float)(x + wave_offset) * 0.05f;
                int height = (int)(10.0f + sinf(angle) * 8.0f);
                for (int y = 0; y < height; y++) {
                    int ty = win_height - 10 - y;
                    if (ty >= win_height - 31 && ty < win_height) {
                        back_buffer[ty * win_width + x] = 0xFF50FA7B; // Diagnostics Wave in green
                        back_buffer[ty * win_width + x + 1] = 0xFF50FA7B;
                    }
                }
            }
            draw_debug_text(&sb, 20, win_height - 25, "JIT_PERF: OPTIMIZED", 0xFFBD93F9, true);
        }
    }

    if (g_mercenary_active) {
        update_mercenary_yul_camera();
    } else if (g_pong_active) {
        update_pong_game();
    } else if (g_applepanic_active) {
        update_applepanic_game();
    } else if (g_airassault_active) {
        update_airassault_game();
    } else if (g_slinkybear_active) {
        update_slinkybear_game();
    } else if (g_slinkypanic_active) {
        update_slinkypanic_game();
    } else if (g_alpiner_active) {
        update_alpiner_game();
    } else if (g_editor_mode == MODE_SPACEPATROL) {
        update_spacepatrol_simulation();
        redraw_spacepatrol_screen();
    } else if (g_editor_mode == MODE_CONSTRUCTION_CO) {
        update_construction_co_simulation();
        redraw_construction_co_screen();
    } else if (g_editor_mode == MODE_DRUM) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_drum_seq(current_ms);
    } else if (g_editor_mode == MODE_SANTA) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_santa(current_ms);
    } else if (g_editor_mode == MODE_MARTIAN) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_martian(current_ms);
    } else if (g_editor_mode == MODE_KNOCKOUT) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_knockout(current_ms);
    } else if (g_editor_mode == MODE_MEMCHECK) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_memcheck(current_ms);
    } else if (g_editor_mode == MODE_ARENA) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_arena(current_ms);
    } else if (g_editor_mode == MODE_CHARDUMP) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_chardump(current_ms);
    } else if (g_editor_mode == MODE_TERM128) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_term128(current_ms);
    } else if (g_editor_mode == MODE_SKIFOLLY) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_skifolly(current_ms);
    } else if (g_editor_mode == MODE_DARTS) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_darts(current_ms);
    } else if (g_editor_mode == MODE_PLOTWAVE) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        update_plotwave(current_ms);
    } else if (g_editor_mode == MODE_ALARM) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint32_t current_ms = (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        static uint32_t last_time_sec = 0;
        if (current_ms - last_time_sec >= 1000) {
            last_time_sec = current_ms;
            g_alarm_s++;
            if (g_alarm_s >= 60) {
                g_alarm_s = 0;
                g_alarm_m++;
                if (g_alarm_m >= 60) {
                    g_alarm_m = 0;
                    g_alarm_h = (g_alarm_h % 24) + 1;
                }
            }
            if (g_alarm_h == g_alarm_set_h && g_alarm_m == g_alarm_set_m) {
                g_alarm_triggered = true;
            }
            redraw_alarm_screen();
        }
    }
    // Draw VIDTEX graphics overlay
    for (int i = 0; i < gfx_primitive_count; i++) {
        GfxPrimitive gp = gfx_primitives[i];
        if (gp.type == GFX_LINE) {
            draw_line(back_buffer, win_width, win_height, mon_x + gp.x1, mon_y + gp.y1, mon_x + gp.x2, mon_y + gp.y2, gp.color);
        } else if (gp.type == GFX_CIRCLE) {
            draw_circle(back_buffer, win_width, win_height, mon_x + gp.x1, mon_y + gp.y1, gp.r, gp.color);
        } else if (gp.type == GFX_POINT) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int tx = mon_x + gp.x1 + dx;
                    int ty = mon_y + gp.y1 + dy;
                    if (tx >= 12 && tx < win_width - 22 && ty >= 57 && ty < win_height - 32) {
                        back_buffer[ty * win_width + tx] = gp.color;
                    }
                }
            }
        } else if (gp.type == GFX_TEXT) {
            int text_len = strlen(gp.text);
            int box_w = text_len * 9 + 4;
            int box_h = 16;
            int start_x = mon_x + gp.x1 - 2;
            int start_y = mon_y + gp.y1 - 2;
            for (int dy = 0; dy < box_h; dy++) {
                for (int dx = 0; dx < box_w; dx++) {
                    int tx = start_x + dx;
                    int ty = start_y + dy;
                    if (tx >= 12 && tx < win_width - 22 && ty >= 57 && ty < win_height - 32) {
                        uint32_t orig = back_buffer[ty * win_width + tx];
                        uint8_t r = ((orig >> 16) & 0xFF) * 0.3f;
                        uint8_t g = ((orig >> 8) & 0xFF) * 0.3f;
                        uint8_t b = (orig & 0xFF) * 0.3f;
                        back_buffer[ty * win_width + tx] = 0xFF000000 | (r << 16) | (g << 8) | b;
                    }
                }
            }
            draw_debug_text(&sb, mon_x + gp.x1, mon_y + gp.y1, gp.text, gp.color, true);
        } else if (gp.type == GFX_STUFFED_3D) {
            draw_3d_stuffed_animal(back_buffer, win_width, win_height, mon_x + gp.x1, mon_y + gp.y1, gp.r, gp.query, gp.frame);
        } else if (gp.type == GFX_FILL_RECT) {
            draw_fill_rect(back_buffer, win_width, win_height, mon_x + gp.x1, mon_y + gp.y1, mon_x + gp.x2, mon_y + gp.y2, gp.color);
        } else if (gp.type == GFX_ROUND_RECT) {
            int px1 = mon_x + gp.x1;
            int py1 = mon_y + gp.y1;
            int px2 = mon_x + gp.x2;
            int py2 = mon_y + gp.y2;
            bool hover = (mouse_px >= px1 && mouse_px <= px2 && mouse_py >= py1 && mouse_py <= py2);
            uint32_t color = gp.color;
            if (hover) {
                // Brighter/highlight color for interactive buttons
                color = 0xFF8BE9FD;
            }
            draw_fill_round_rect(back_buffer, win_width, win_height, px1, py1, px2, py2, gp.r, color);
        } else if (gp.type == GFX_PMG_PLAYER) {
            int pidx = gp.r;
            int dx = 0, dy = 0;
            const char *q_name = "teddy_bear";
            if (pidx == 0) {
                dx = (int)vm_peek(&vm, 55051);
                dy = (int)vm_peek(&vm, 55052);
                q_name = "teddy_bear";
            } else if (pidx == 1) {
                dx = (int)vm_peek(&vm, 55055);
                dy = (int)vm_peek(&vm, 55056);
                q_name = "crow";
            } else if (pidx == 2) {
                dx = (int)vm_peek(&vm, 55057);
                dy = (int)vm_peek(&vm, 55058);
                q_name = "cat";
            } else if (pidx == 3) {
                dx = (int)vm_peek(&vm, 55070);
                dy = (int)vm_peek(&vm, 55071);
                q_name = "fish";
            }
            int px = mon_x + gp.x1 + dx;
            int py = mon_y + gp.y1 + dy;
            draw_3d_stuffed_animal(back_buffer, win_width, win_height, px, py, 24, q_name, g_frame_counter);
        }
    }
    // Draw the Telemetry HUD Sidebar
    LauTelemetryState *t = lau_telemetry_get_state();
    if (t) {
        t->render_telemetry.frames_rendered++;
        static struct timespec last_ts;
        static bool first = true;
        struct timespec current_ts;
        clock_gettime(CLOCK_MONOTONIC, &current_ts);
        if (!first) {
            double elapsed = (current_ts.tv_sec - last_ts.tv_sec) * 1000.0 + (current_ts.tv_nsec - last_ts.tv_nsec) / 1000000.0;
            t->render_telemetry.avg_ms_per_frame = elapsed;
        }
        first = false;
        last_ts = current_ts;
        
        if (win_width >= 1280) {
            int sidebar_x = 1230;
            // Draw divider line
            for (int y = 55; y < win_height - 12; y++) {
                back_buffer[y * win_width + 1222] = 0xFF6272A4; // Slate divider
            }
            char buf[128];
            uint32_t title_color = 0xFF8BE9FD; // Cyan
            uint32_t val_color = 0xFFF1FA8C;   // Yellow
            
            draw_debug_text(&sb, sidebar_x, 70, "SYSTEM HUD", title_color, true);
            
            sprintf(buf, "ALLOCS: %lu", (unsigned long)t->total_allocs);
            draw_debug_text(&sb, sidebar_x, 100, buf, val_color, true);
            
            sprintf(buf, "FREES:  %lu", (unsigned long)t->total_frees);
            draw_debug_text(&sb, sidebar_x, 120, buf, val_color, true);
            
            sprintf(buf, "ACTIVE: %lu", (unsigned long)t->active_allocs);
            draw_debug_text(&sb, sidebar_x, 140, buf, val_color, true);
            
            sprintf(buf, "STEPS:  %lu", (unsigned long)t->exec_steps);
            draw_debug_text(&sb, sidebar_x, 180, buf, val_color, true);
            
            draw_debug_text(&sb, sidebar_x, 220, "RENDER HUD", title_color, true);
            
            sprintf(buf, "FRAMES: %lu", (unsigned long)t->render_telemetry.frames_rendered);
            draw_debug_text(&sb, sidebar_x, 250, buf, val_color, true);
            
            sprintf(buf, "LATENCY: %.2f ms", t->render_telemetry.avg_ms_per_frame);
            draw_debug_text(&sb, sidebar_x, 270, buf, val_color, true);
        }
    }

    sync_vram_to_cpu();
    g_frame_counter++;

}



extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len);

static void vm_poke(TsfiZmmVmState *vstate, uint64_t addr, uint8_t val) {
    (void)vstate;
    uint8_t poke_cd[68] = {0};
    poke_cd[0] = 0x80; poke_cd[1] = 0x29; poke_cd[2] = 0xe7; poke_cd[3] = 0xc0;
    for (int i = 0; i < 8; i++) {
        poke_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
    }
    poke_cd[36 + 31] = val;
    
    uint8_t ret[32];
    size_t ret_len = 32;
    lau_yul_thunk_execute("cpu6502", poke_cd, 68, ret, &ret_len);
}

static uint64_t vm_peek(TsfiZmmVmState *vstate, uint64_t addr) {
    (void)vstate;
    uint8_t peek_cd[36] = {0};
    peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
    for (int i = 0; i < 8; i++) {
        peek_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
    }
    
    uint8_t peek_ret[32] = {0};
    size_t peek_ret_len = 32;
    uint64_t res = 0;
    if (lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len)) {
        for (int i = 0; i < 8; i++) {
            res |= ((uint64_t)peek_ret[31 - i]) << (i * 8);
        }
    }
    return res;
}

static void vm_poke64(TsfiZmmVmState *vstate, uint64_t addr, uint64_t val) {
    (void)vstate;
    uint8_t poke_cd[68] = {0};
    poke_cd[0] = 0x80; poke_cd[1] = 0x29; poke_cd[2] = 0xe7; poke_cd[3] = 0xc0;
    for (int i = 0; i < 8; i++) {
        poke_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
    }
    for (int i = 0; i < 8; i++) {
        poke_cd[36 + 31 - i] = (val >> (i * 8)) & 0xFF;
    }
    
    uint8_t ret[32];
    size_t ret_len = 32;
    lau_yul_thunk_execute("cpu6502", poke_cd, 68, ret, &ret_len);
}

static void init_pong_game(void) {
    uint8_t pong_program[] = {
        0xA5, 0x08, 0xD0, 0x0F, 0xA2, 0x27, 0xA0, 0x21, 0x20, 0x14, 0x21, 0xA9, 
        0x01, 0x85, 0x08, 0xAD, 0x00, 0xDC, 0x29, 0x04, 0xD0, 0x0F, 0xAD, 0x00, 
        0xD0, 0x38, 0xE9, 0x04, 0xC9, 0x28, 0xB0, 0x02, 0xA9, 0x28, 0x8D, 0x00, 
        0xD0, 0xAD, 0x00, 0xDC, 0x29, 0x08, 0xD0, 0x0F, 0xAD, 0x00, 0xD0, 0x18, 
        0x69, 0x04, 0xC9, 0xE0, 0x90, 0x02, 0xA9, 0xE0, 0x8D, 0x00, 0xD0, 0xA5, 
