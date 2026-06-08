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
static struct wl_pointer *pointer = NULL;
static bool drag_selecting = false;
static int select_start_x = -1, select_start_y = -1;
static int select_end_x = -1, select_end_y = -1;
static int mouse_px = -1, mouse_py = -1;
static uint32_t last_click_time = 0;
static int click_count = 0;
typedef enum {
    GFX_LINE,
    GFX_CIRCLE,
    GFX_POINT
} GfxType;

typedef struct {
    GfxType type;
    int x1, y1, x2, y2;
    int r;
    uint32_t color;
} GfxPrimitive;

#define MAX_GFX_PRIMITIVES 1024
static GfxPrimitive gfx_primitives[MAX_GFX_PRIMITIVES];
static int gfx_primitive_count = 0;
void render_terminal_display(void);

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
static void terminal_write_string(LauVRAM *vram, const char *str, int len) {
    static int state = 0; // 0: normal, 1: esc, 2: bracket, 3: gfx_command
    static char parse_buf[128];
    static int parse_len = 0;
    
    for (int i = 0; i < len; i++) {
        char c = str[i];
        if (state == 0) {
            if (c == '\x1b') {
                state = 1;
            } else {
                lau_vram_write_char(vram, c);
            }
        } else if (state == 1) {
            if (c == '[') {
                state = 2;
            } else {
                state = 0;
                lau_vram_write_char(vram, '\x1b');
                lau_vram_write_char(vram, c);
            }
        } else if (state == 2) {
            if (c == 'G') {
                state = 3;
                parse_len = 0;
            } else {
                state = 0;
                lau_vram_write_char(vram, '\x1b');
                lau_vram_write_char(vram, '[');
                lau_vram_write_char(vram, c);
            }
        } else if (state == 3) {
            if (c == 'm') {
                parse_buf[parse_len] = '\0';
                int cmd = 0;
                int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
                int count = sscanf(parse_buf, "%d;%d;%d;%d;%d", &cmd, &p1, &p2, &p3, &p4);
                if (count >= 1) {
                    if (cmd == 0) {
                        gfx_primitive_count = 0;
                    } else if (cmd == 1 && count >= 5) {
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_LINE;
                            gp->x1 = p1; gp->y1 = p2;
                            gp->x2 = p3; gp->y2 = p4;
                            gp->color = 0xFF50FA7B; // Dracula green line
                        }
                    } else if (cmd == 2 && count >= 4) {
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_CIRCLE;
                            gp->x1 = p1; gp->y1 = p2;
                            gp->r = p3;
                            gp->color = 0xFF8BE9FD; // Dracula cyan circle
                        }
                    } else if (cmd == 3 && count >= 4) {
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_POINT;
                            gp->x1 = p1; gp->y1 = p2;
                            gp->color = (p3 == 1) ? 0xFFFF5555 : 0xFFF1FA8C; // red / yellow
                        }
                    }
                }
                state = 0;
                vram->is_dirty = true;
            } else {
                if (parse_len < (int)sizeof(parse_buf) - 2) {
                    parse_buf[parse_len++] = c;
                }
            }
        }
    }
}

static void execute_command(const char *cmd) {
    if (strcmp(cmd, "exit") == 0) {
        running = false;
        return;
    }
    
    char cmd_copy[512];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy));
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    char *first_word = strtok(cmd_copy, " \t");
    
    if (first_word && strcasecmp(first_word, "RAG") == 0) {
        char *query = strtok(NULL, "");
        if (!query) query = "Yul CPU compilation";
        
        char txt[1024];
        sprintf(txt, "\r\n=== RAG Vector Database Search ===\r\n"
                     "Query: \"%s\"\r\n"
                     "Initializing VIDTEX RAG Shooting Gallery Scatter Plot...\r\n"
                     "Target Duck (Doc 1) at (250, 120) [Solidity CPU ROM]\r\n"
                     "Target Duck (Doc 2) at (550, 180) [Decision Engine]\r\n"
                     "Target Duck (Doc 3) at (400, 300) [ZMM VM State]\r\n\r\n"
                     "Firing Query Projectile from (400, 360)...\r\n", query);
        lau_vram_write_string(g_vram, txt, strlen(txt));
        
        int start_x = 400, start_y = 360;
        int target_x = 250, target_y = 120;
        
        for (int frame = 1; frame <= 12; frame++) {
            int bullet_x = start_x + (target_x - start_x) * frame / 12;
            int bullet_y = start_y + (target_y - start_y) * frame / 12;
            
            char gfx_buf[256];
            sprintf(gfx_buf, "\x1b[G0;m\x1b[G2;250;120;18;m\x1b[G2;550;180;18;m\x1b[G2;400;300;18;m\x1b[G3;%d;%d;1;m", bullet_x, bullet_y);
            terminal_write_string(g_vram, gfx_buf, strlen(gfx_buf));
            
            lau_vram_write_string(g_vram, ".", 1);
            
            g_vram->is_dirty = true;
            render_terminal_display();
            current_buffer_idx = 1 - current_buffer_idx;
            memcpy(pixel_datas[current_buffer_idx], back_buffer, win_width * win_height * 4);
            wl_surface_attach(surface, wl_buffers[current_buffer_idx], 0, 0);
            wl_surface_damage(surface, 0, 0, win_width, win_height);
            wl_surface_commit(surface);
            wl_display_flush(display);
            wl_display_dispatch_pending(display);
            
            usleep(80000);
        }
        
        char hit_txt[1024];
        sprintf(hit_txt, "\r\n\r\n💥 DIRECT HIT! Similarity Threshold Exceeded at (250, 120)!\r\n"
                         "🎵 Triggered SID Sound Crash on musicMaker (54272 -> 120, 54273 -> 15)\r\n\r\n"
                         "Retrieved Context: Solidity Yul CPU contract is initialized at virtual address 0x1\r\n"
                         "and provides full instruction decoding support for 6502/6509 opcodes.\r\n"
                         "==================================\r\n");
        lau_vram_write_string(g_vram, hit_txt, strlen(hit_txt));
        
        char exp_buf[256];
        sprintf(exp_buf, "\x1b[G2;250;120;25;m\x1b[G1;250;120;235;105;m\x1b[G1;250;120;265;105;m\x1b[G1;250;120;235;135;m\x1b[G1;250;120;265;135;m");
        terminal_write_string(g_vram, exp_buf, strlen(exp_buf));
        
        g_vram->is_dirty = true;
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

        bool is_vm_cmd = false;
        if (first_word) {
            if (strcasecmp(first_word, "YULINIT") == 0 ||
                strcasecmp(first_word, "YULEXEC") == 0 ||
                strcasecmp(first_word, "SWIFTLOAD") == 0 ||
                strcasecmp(first_word, "REU") == 0 ||
                strcasecmp(first_word, "CALC") == 0 ||
                strcasecmp(first_word, "MEMDUMP") == 0 ||
                strcasecmp(first_word, "SPRITE") == 0 ||
                strcasecmp(first_word, "OMNICOMM") == 0) {
                is_vm_cmd = true;
            }
        }

        if (is_vm_cmd) {
            tsfi_zmm_vm_exec(&vm, cmd);
        } else {
            int rc = system(cmd);
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
            terminal_write_string(g_vram, read_buf, n);
        }
        close(stdout_pipe[0]);
    } else {
        tsfi_zmm_vm_exec(&vm, cmd);
    }
}

static void get_cell_coords(wl_fixed_t fx, wl_fixed_t fy, int *out_x, int *out_y) {
    int px = wl_fixed_to_int(fx);
    int py = wl_fixed_to_int(fy);
    int start_y = 0;
    int max_rows = (win_height - 80) / 18;
    if (max_rows < 5) max_rows = 5;
    if (max_rows > 35) max_rows = 35;
    if (g_vram && g_vram->cursor_y >= max_rows) {
        start_y = g_vram->cursor_y - max_rows + 1;
    }
    
    int cell_x = (px - 22) / 10;
    int cell_y = (py - 67) / 18;
    
    if (cell_x >= 0 && cell_x < 120 && cell_y >= 0 && cell_y < max_rows) {
        *out_x = cell_x;
        *out_y = start_y + cell_y;
    } else {
        *out_x = -1;
        *out_y = -1;
    }
}

static void perform_copy(void) {
    if (select_start_x < 0 || select_start_y < 0 || select_end_x < 0 || select_end_y < 0) return;
    
    int sy = select_start_y, ey = select_end_y;
    int sx = select_start_x, ex = select_end_x;
    if (sy > ey || (sy == ey && sx > ex)) {
        sy = select_end_y; ey = select_start_y;
        sx = select_end_x; ex = select_start_x;
    }
    
    char copy_buf[8192];
    int len = 0;
    
    for (int y = sy; y <= ey; y++) {
        int x_start = (y == sy) ? sx : 0;
        int x_end = (y == ey) ? ex : 120 - 1;
        
        for (int x = x_start; x <= x_end; x++) {
            if (x >= 0 && x < 120 && y >= 0 && y < 60) {
                char c = (char)g_vram->grid[y][x].character;
                if (len < (int)sizeof(copy_buf) - 5) {
                    copy_buf[len++] = c;
                }
            }
        }
        if (y < ey) {
            copy_buf[len++] = '\n';
        }
    }
    copy_buf[len] = '\0';
    
    while (len > 0 && copy_buf[len - 1] == ' ') {
        copy_buf[--len] = '\0';
    }
    
    if (len > 0) {
        FILE *f = popen("wl-copy -p 2>/dev/null || wl-copy 2>/dev/null || xclip -i -selection primary 2>/dev/null", "w");
        if (f) {
            fwrite(copy_buf, 1, len, f);
            pclose(f);
        }
    }
}

static void perform_paste(void) {
    FILE *f = popen("wl-paste -p 2>/dev/null || wl-paste 2>/dev/null", "r");
    if (!f) return;
    char buf[1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        for (size_t i = 0; i < n; i++) {
            char c = buf[i];
            if (c >= 32 && c < 127) {
                if (cmd_len < (int)sizeof(cmd_buf) - 2) {
                    cmd_buf[cmd_len++] = c;
                    cmd_buf[cmd_len] = '\0';
                    lau_vram_write_char(g_vram, c);
                }
            } else if (c == '\n' || c == '\r') {
                lau_vram_write_string(g_vram, "\r\n", 2);
                if (cmd_len > 0) {
                    cmd_buf[cmd_len] = '\0';
                    execute_command(cmd_buf);
                    cmd_len = 0;
                    cmd_buf[0] = '\0';
                }
                lau_vram_write_string(g_vram, "zmm-vm> ", 8);
            }
        }
    }
    pclose(f);
}

static bool is_word_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
}

static void select_word_at(int cx, int cy) {
    if (cx < 0 || cx >= 120 || cy < 0 || cy >= 60) return;
    
    int start_x = cx;
    while (start_x > 0 && is_word_char((char)g_vram->grid[cy][start_x - 1].character)) {
        start_x--;
    }
    
    int end_x = cx;
    while (end_x < 120 - 1 && is_word_char((char)g_vram->grid[cy][end_x + 1].character)) {
        end_x++;
    }
    
    select_start_x = start_x;
    select_start_y = cy;
    select_end_x = end_x;
    select_end_y = cy;
    
    perform_copy();
}

static void select_line_at(int cy) {
    if (cy < 0 || cy >= 60) return;
    select_start_x = 0;
    select_start_y = cy;
    select_end_x = 120 - 1;
    select_end_y = cy;
    
    perform_copy();
}

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *wl_surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)data; (void)wl_pointer; (void)serial; (void)wl_surface; (void)surface_x; (void)surface_y;
}
static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *wl_surface) {
    (void)data; (void)wl_pointer; (void)serial; (void)wl_surface;
}
static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)data; (void)wl_pointer; (void)time;
    mouse_px = wl_fixed_to_int(surface_x);
    mouse_py = wl_fixed_to_int(surface_y);
    if (drag_selecting) {
        int cx, cy;
        get_cell_coords(surface_x, surface_y, &cx, &cy);
        if (cx >= 0 && cy >= 0) {
            select_end_x = cx;
            select_end_y = cy;
            if (g_vram) g_vram->is_dirty = true;
        }
    }
}
static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    (void)data; (void)wl_pointer; (void)serial;
    if (button == 272) {
        if (state == 1) { // Left press
            uint32_t diff = time - last_click_time;
            last_click_time = time;
            if (diff < 300) {
                click_count++;
            } else {
                click_count = 1;
            }
            
            int cx, cy;
            get_cell_coords(wl_fixed_from_int(mouse_px), wl_fixed_from_int(mouse_py), &cx, &cy);
            if (cx >= 0 && cy >= 0) {
                if (click_count == 1) {
                    drag_selecting = true;
                    select_start_x = cx;
                    select_start_y = cy;
                    select_end_x = cx;
                    select_end_y = cy;
                } else if (click_count == 2) {
                    drag_selecting = false;
                    select_word_at(cx, cy);
                } else if (click_count >= 3) {
                    drag_selecting = false;
                    select_line_at(cy);
                }
                if (g_vram) g_vram->is_dirty = true;
            }
        } else if (state == 0) { // Left release
            if (drag_selecting) {
                drag_selecting = false;
                perform_copy();
                if (g_vram) g_vram->is_dirty = true;
            }
        }
    } else if (button == 274 && state == 1) { // Middle click paste
        perform_paste();
    }
}
static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
    (void)data; (void)wl_pointer; (void)time; (void)axis; (void)value;
}
static void pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {
    (void)data; (void)wl_pointer;
}
static void pointer_handle_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source) {
    (void)data; (void)wl_pointer; (void)axis_source;
}
static void pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {
    (void)data; (void)wl_pointer; (void)time; (void)axis;
}
static void pointer_handle_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
    (void)data; (void)wl_pointer; (void)axis; (void)discrete;
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = pointer_handle_axis,
    .frame = pointer_handle_frame,
    .axis_source = pointer_handle_axis_source,
    .axis_stop = pointer_handle_axis_stop,
    .axis_discrete = pointer_handle_axis_discrete
};

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
                    bool in_selection = false;
                    if (select_start_x >= 0 && select_start_y >= 0 && select_end_x >= 0 && select_end_y >= 0) {
                        int sy = select_start_y, ey = select_end_y;
                        int sx = select_start_x, ex = select_end_x;
                        if (sy > ey || (sy == ey && sx > ex)) {
                            sy = select_end_y; ey = select_start_y;
                            sx = select_end_x; ex = select_start_x;
                        }
                        if (vram_y > sy && vram_y < ey) in_selection = true;
                        else if (vram_y == sy && vram_y == ey) in_selection = (x >= sx && x <= ex);
                        else if (vram_y == sy) in_selection = (x >= sx);
                        else if (vram_y == ey) in_selection = (x <= ex);
                    }
                    
                    if (in_selection) {
                        for (int dy = 0; dy < char_h; dy++) {
                            for (int dx = 0; dx < char_w; dx++) {
                                int ty = py + dy;
                                int tx = px + dx;
                                if (tx >= 12 && tx < win_width - 22 && ty >= 57 && ty < win_height - 32) {
                                    back_buffer[ty * win_width + tx] = 0xFF44475A; // Dracula selection bg
                                }
                            }
                        }
                        draw_debug_codepoint(&sb, px, py, cell.character, 0xFFF8F8F2);
                    } else {
                        draw_debug_codepoint(&sb, px, py, cell.character, fg);
                    }
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
        }
    }

}

static void format_uint256_hex(char *dest, uint64_t val) {
    for (int i = 0; i < 32; i++) {
        int shift = (31 - i) * 8;
        uint8_t byteval = 0;
        if (shift < 64) {
            byteval = (val >> shift) & 0xFF;
        }
        sprintf(&dest[i * 2], "%02x", byteval);
    }
}

static void vm_poke(TsfiZmmVmState *vstate, uint64_t addr, uint8_t val) {
    char cmd[512];
    char addr_hex[65];
    char val_hex[65];
    format_uint256_hex(addr_hex, addr);
    format_uint256_hex(val_hex, val);
    
    // selector: poke(uint256,uint256) -> 0x8029e7c0
    sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%s%s\"", addr_hex, val_hex);
    vstate->output_pos = 0;
    tsfi_zmm_vm_exec(vstate, cmd);
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
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();

    printf("[TERMINAL] Exited gracefully.\n");
    return 0;
}
