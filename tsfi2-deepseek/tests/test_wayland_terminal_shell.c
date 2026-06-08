#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <linux/input.h>
#include "lau_memory.h"
#include "tsfi_wire_firmware.h"
#include "lau_vram.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_staging.h"

// StagingBuffer is defined in tsfi_staging.h
// draw_debug_codepoint/draw_debug_text are defined in tsfi_staging.h

static struct wl_display *display = NULL;
static struct wl_registry *registry = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct wl_seat *seat = NULL;
static struct wl_keyboard *keyboard = NULL;

static struct wl_surface *surface = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static struct wl_buffer *wl_buffers[2] = {NULL, NULL};
static int current_buffer_idx = 0;

static int win_width = 1280;
static int win_height = 720;
static uint32_t *pixel_datas[2] = {NULL, NULL};
static uint32_t *back_buffer = NULL;
static LauVRAM *g_vram = NULL;
static TsfiZmmVmState vm;
static char cmd_buf[512] = {0};
static int cmd_len = 0;
static volatile bool running = true;
static bool configured = false;
static int pending_width = 0;
static int pending_height = 0;
static bool resize_pending = false;

// Registry listeners
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void)data; (void)version;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    }
}
static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove
};

// Keyboard listeners
static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)keyboard; (void)format; (void)size;
    close(fd);
}
static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    (void)data; (void)keyboard; (void)serial; (void)surface; (void)keys;
}
static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {
    (void)data; (void)keyboard; (void)serial; (void)surface;
}

extern uint32_t active_modifiers;
static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)data; (void)keyboard; (void)serial; (void)group; (void)mods_latched;
    if (mods_depressed & 1) active_modifiers |= 1;
    else active_modifiers &= ~1;
    if (mods_locked & 2) active_modifiers |= 16;
    else active_modifiers &= ~16;
}
static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard, int32_t rate, int32_t delay) {
    (void)data; (void)keyboard; (void)rate; (void)delay;
}
static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)keyboard; (void)serial; (void)time;
    if (state != 1) return; // Only key press

    extern uint32_t tsfi_input_map_to_utf32(uint32_t scancode);
    uint32_t utf32 = tsfi_input_map_to_utf32(key);

    if (key == KEY_ESC || key == 1) {
        running = false;
        return;
    }

    if (key == KEY_ENTER || key == 28) {
        lau_vram_write_string(g_vram, "\r\n", 2);
        if (cmd_len > 0) {
            cmd_buf[cmd_len] = '\0';
            if (strcmp(cmd_buf, "exit") == 0) {
                running = false;
                return;
            }
            
            // Redirect stdout/stderr of command to VRAM
            int stdout_pipe[2];
            if (pipe(stdout_pipe) == 0) {
                int old_stdout = dup(STDOUT_FILENO);
                int old_stderr = dup(STDERR_FILENO);
                dup2(stdout_pipe[1], STDOUT_FILENO);
                dup2(stdout_pipe[1], STDERR_FILENO);
                close(stdout_pipe[1]);

                // Determine if it is a VM command or Host command
                char cmd_copy[512];
                strncpy(cmd_copy, cmd_buf, sizeof(cmd_copy));
                cmd_copy[sizeof(cmd_copy) - 1] = '\0';
                char *first_word = strtok(cmd_copy, " \t");
                bool is_vm_cmd = false;
                if (first_word) {
                    if (strcmp(first_word, "YULINIT") == 0 ||
                        strcmp(first_word, "YULEXEC") == 0 ||
                        strcmp(first_word, "SWIFTLOAD") == 0 ||
                        strcmp(first_word, "REU") == 0 ||
                        strcmp(first_word, "CALC") == 0 ||
                        strcmp(first_word, "MEMDUMP") == 0 ||
                        strcmp(first_word, "SPRITE") == 0 ||
                        strcmp(first_word, "OMNICOMM") == 0) {
                        is_vm_cmd = true;
                    }
                }

                if (is_vm_cmd) {
                    tsfi_zmm_vm_exec(&vm, cmd_buf);
                } else {
                    int rc = system(cmd_buf);
                    (void)rc;
                }

                fflush(stdout);
                fflush(stderr);

                dup2(old_stdout, STDOUT_FILENO);
                dup2(old_stderr, STDERR_FILENO);
                close(old_stdout);
                close(old_stderr);

                int flags = fcntl(stdout_pipe[0], F_GETFL, 0);
                fcntl(stdout_pipe[0], F_SETFL, flags | O_NONBLOCK);
                char read_buf[4096];
                ssize_t n;
                while ((n = read(stdout_pipe[0], read_buf, sizeof(read_buf))) > 0) {
                    lau_vram_write_string(g_vram, read_buf, n);
                }
                close(stdout_pipe[0]);
            } else {
                tsfi_zmm_vm_exec(&vm, cmd_buf);
            }
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
    if (surface) {
        wl_surface_commit(surface);
    }
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

void render_terminal_display(void) {
    uint32_t bg_color = 0xFF0A0B10; // Obsidian dark background
    for (int i = 0; i < win_width * win_height; i++) {
        back_buffer[i] = bg_color;
    }

    StagingBuffer sb = {
        .magic = TSFI_STAGING_MAGIC,
        .width = win_width,
        .height = win_height,
        .stride = win_width * 4,
        .size = win_width * win_height * 4,
        .data = back_buffer
    };

    // Render title status bar area
    // Cyan status divider line at y=45
    if (win_height > 50) {
        for (int x = 0; x < win_width; x++) {
            back_buffer[45 * win_width + x] = 0xFF8BE9FD; // Pastel Cyan status divider
        }
    }

    draw_debug_text(&sb, 20, 15, "TSFI SOVEREIGN CPU TERMINAL", 0xFFBD93F9, true); // Pastel Purple
    draw_debug_text(&sb, win_width - 280, 15, "[ SYS: AUDITED ] [ VM: RUNNING ]", 0xFF50FA7B, true); // Pastel Green

    // Border around terminal panel
    if (win_height > 70) {
        // Left & Right borders
        for (int y = 55; y < win_height - 12; y++) {
            for (int dx = 0; dx < 2; dx++) {
                back_buffer[y * win_width + 10 + dx] = 0xFF6272A4; // Slate gray border
                back_buffer[y * win_width + (win_width - 12) + dx] = 0xFF6272A4;
            }
        }
        // Top & Bottom borders
        for (int x = 10; x < win_width - 10; x++) {
            for (int dy = 0; dy < 2; dy++) {
                back_buffer[(55 + dy) * win_width + x] = 0xFF6272A4;
                back_buffer[((win_height - 14) + dy) * win_width + x] = 0xFF6272A4;
            }
        }
    }

    // Dracula premium console palette
    static const uint32_t palette[16] = {
        0xFF000000, 0xFFFF5555, 0xFF50FA7B, 0xFFF1FA8C,
        0xFFBD93F9, 0xFFFF79C6, 0xFF8BE9FD, 0xFFF8F8F2,
        0xFF6272A4, 0xFFFF5555, 0xFF50FA7B, 0xFFF1FA8C,
        0xFFBD93F9, 0xFFFF79C6, 0xFF8BE9FD, 0xFFF8F8F2
    };

    int char_w = 10;
    int char_h = 18;
    int mon_x = 22;
    int mon_y = 67;

    int max_rows = (win_height - 80) / char_h;
    if (max_rows < 5) max_rows = 5;
    if (max_rows > 35) max_rows = 35;

    int start_y = 0;
    if (g_vram->cursor_y >= max_rows) {
        start_y = g_vram->cursor_y - max_rows + 1;
    }

    for (int y = 0; y < max_rows; y++) {
        int vram_y = start_y + y;
        if (vram_y >= LAU_VRAM_ROWS) break;
        
        for (int x = 0; x < 120; x++) {
            if (x >= LAU_VRAM_COLS) break;
            
            LauVRAMCell cell = g_vram->grid[vram_y][x];
            if (cell.character > 32) {
                uint32_t fg = palette[cell.fg_color & 0xF];
                if (cell.attributes & 1) fg = palette[(cell.fg_color & 0x7) + 8]; // Bold/Bright
                
                int px = mon_x + (x * char_w);
                int py = mon_y + (y * char_h);
                if (px >= 12 && px < win_width - 22 && py >= 57 && py < win_height - 32) {
                    draw_debug_codepoint(&sb, px, py, cell.character, fg);
                }
            }
        }
    }
    
    // Draw inverted green/amber cursor block
    int cy = g_vram->cursor_y - start_y;
    int cx = g_vram->cursor_x;
    if (cy >= 0 && cy < max_rows && cx >= 0 && cx < 120) {
        int px = mon_x + cx * char_w;
        int py = mon_y + cy * char_h;
        for (int dy = 0; dy < char_h - 2; dy++) {
            for (int dx = 0; dx < char_w; dx++) {
                int ty = py + dy;
                int tx = px + dx;
                if (tx >= 12 && tx < win_width - 22 && ty >= 57 && ty < win_height - 32) {
                    back_buffer[ty * win_width + tx] ^= 0xFF00FF00; // Electric green invert tint
                }
            }
        }
    }

    // Apply CRT horizontal scanlines
    for (int y = 0; y < win_height; y++) {
        if (y % 3 == 0) {
            for (int x = 0; x < win_width; x++) {
                uint32_t color = back_buffer[y * win_width + x];
                uint8_t r = ((color >> 16) & 0xFF) * 0.82f;
                uint8_t g = ((color >> 8) & 0xFF) * 0.82f;
                uint8_t b = (color & 0xFF) * 0.82f;
                back_buffer[y * win_width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }
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
                          "  YULINIT, YULEXEC, SWIFTLOAD, REU, CALC, MEMDUMP, SPRITE, OMNICOMM\r\n\r\n"
                          "zmm-vm> ";
    lau_vram_write_string(g_vram, welcome, strlen(welcome));

    int display_fd = wl_display_get_fd(display);
    printf("[TERMINAL] Entering event loop. Window should map on your Hyprland desktop now.\n");

    int f_count = 0;
    while (running) {
        wl_display_dispatch_pending(display);

        bool need_redraw = g_vram->is_dirty;

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

        struct pollfd fds[1] = {
            { .fd = display_fd, .events = POLLIN }
        };
        
        // Wait up to 16ms for display events (approx 60fps)
        int ret = poll(fds, 1, 16);
        if (ret > 0) {
            if (wl_display_read_events(display) < 0) {
                break;
            }
            wl_display_dispatch_pending(display);
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
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();

    printf("[TERMINAL] Exited gracefully.\n");
    return 0;
}
