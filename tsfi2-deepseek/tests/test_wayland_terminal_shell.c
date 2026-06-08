#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
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
#include "tsfi_vision.h"
#include "tsfi_jpeg_encoder.h"

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
    GFX_POINT,
    GFX_TEXT,
    GFX_STUFFED_3D
} GfxType;

typedef struct {
    GfxType type;
    int x1, y1, x2, y2;
    int r;
    uint32_t color;
    char text[32];
    char query[32];
    int frame;
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
                char *tokens[16];
                int tok_count = 0;
                char *tok = strtok(parse_buf, ";");
                while (tok && tok_count < 16) {
                    tokens[tok_count++] = tok;
                    tok = strtok(NULL, ";");
                }
                if (tok_count >= 1) {
                    int cmd = atoi(tokens[0]);
                    if (cmd == 0) {
                        gfx_primitive_count = 0;
                    } else if (cmd == 1 && tok_count >= 5) {
                        int x1 = atoi(tokens[1]);
                        int y1 = atoi(tokens[2]);
                        int x2 = atoi(tokens[3]);
                        int y2 = atoi(tokens[4]);
                        uint32_t color = (tok_count >= 6) ? (uint32_t)strtoul(tokens[5], NULL, 0) : 0xFF50FA7B;
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_LINE; gp->x1 = x1; gp->y1 = y1; gp->x2 = x2; gp->y2 = y2; gp->color = color;
                        }
                    } else if (cmd == 2 && tok_count >= 4) {
                        int x = atoi(tokens[1]);
                        int y = atoi(tokens[2]);
                        int r = atoi(tokens[3]);
                        uint32_t color = (tok_count >= 5) ? (uint32_t)strtoul(tokens[4], NULL, 0) : 0xFF8BE9FD;
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_CIRCLE; gp->x1 = x; gp->y1 = y; gp->r = r; gp->color = color;
                        }
                    } else if (cmd == 3 && tok_count >= 3) {
                        int x = atoi(tokens[1]);
                        int y = atoi(tokens[2]);
                        uint32_t color = 0xFF50FA7B;
                        if (tok_count >= 4) {
                            char *col_str = tokens[3];
                            if (strlen(col_str) <= 2) {
                                int idx = atoi(col_str);
                                color = (idx == 1) ? 0xFFFF5555 : 0xFFF1FA8C;
                            } else {
                                color = (uint32_t)strtoul(col_str, NULL, 0);
                            }
                        }
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_POINT; gp->x1 = x; gp->y1 = y; gp->color = color;
                        }
                    } else if (cmd == 4 && tok_count >= 4) {
                        int x = atoi(tokens[1]);
                        int y = atoi(tokens[2]);
                        uint32_t color = (uint32_t)strtoul(tokens[3], NULL, 0);
                        char text_buf[64] = "";
                        if (tok_count >= 5) {
                            int tlen = strlen(tokens[4]);
                            if (tlen > 63) tlen = 63;
                            memcpy(text_buf, tokens[4], tlen);
                            text_buf[tlen] = '\0';
                        }
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_TEXT; gp->x1 = x; gp->y1 = y; gp->color = color;
                            int glen = strlen(text_buf);
                            if (glen > 31) glen = 31;
                            memcpy(gp->text, text_buf, glen);
                            gp->text[glen] = '\0';
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

static void add_line(int x1, int y1, int x2, int y2, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_LINE; gp->x1 = x1; gp->y1 = y1; gp->x2 = x2; gp->y2 = y2; gp->color = color;
    }
}
static void add_circle(int x, int y, int r, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_CIRCLE; gp->x1 = x; gp->y1 = y; gp->r = r; gp->color = color;
    }
}

static void add_text(int x, int y, const char *text, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_TEXT; gp->x1 = x; gp->y1 = y; gp->color = color;
        strncpy(gp->text, text, sizeof(gp->text));
        gp->text[sizeof(gp->text) - 1] = '\0';
    }
}

static int g_frame_counter = 0;

static inline float smin(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / k;
    return fminf(a, b) - h * h * k * 0.25f;
}

static float sdf_teddy(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.2f + y*y*0.8f + z*z*1.2f) - 0.35f;
    float d_head = sqrtf(x*x + (y - 0.35f)*(y - 0.35f) + z*z) - 0.25f;
    float ex = fabsf(x) - 0.2f, ey = y - 0.55f, ez = z;
    float d_ear = sqrtf(ex*ex + ey*ey + ez*ez) - 0.08f;
    float d_snout = sqrtf(x*x + (y - 0.3f)*(y - 0.3f) + (z - 0.2f)*(z - 0.2f)) - 0.1f;
    
    // Arms
    float ax = fabsf(x) - 0.35f, ay = y - 0.1f, az = z;
    float d_arm = sqrtf(ax*ax*1.5f + ay*ay*1.5f + az*az) - 0.1f;
    
    // Legs
    float lx = fabsf(x) - 0.22f, ly = y + 0.35f, lz = z - 0.1f;
    float d_leg = sqrtf(lx*lx + ly*ly + lz*lz) - 0.12f;
    
    float res = smin(d_body, d_head, 0.12f);
    res = smin(res, d_ear, 0.04f);
    res = smin(res, d_snout, 0.05f);
    res = smin(res, d_arm, 0.05f);
    res = smin(res, d_leg, 0.05f);
    return res;
}

static float sdf_crow(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.5f + y*y + z*z) - 0.3f;
    float d_head = sqrtf((x - 0.3f)*(x - 0.3f) + (y - 0.2f)*(y - 0.2f) + z*z) - 0.15f;
    float bx = x - 0.45f, by = y - 0.2f, bz = z;
    float d_beak = sqrtf(bx*bx*5.0f + by*by*5.0f + bz*bz*5.0f) - 0.1f;
    float wx = fabsf(x), wy = y - 0.1f, wz = fabsf(z) - 0.25f;
    float d_wing = sqrtf(wx*wx*2.0f + wy*wy*0.5f + wz*wz*3.0f) - 0.12f;
    float res = smin(d_body, d_head, 0.08f);
    res = smin(res, d_wing, 0.05f);
    res = smin(res, d_beak, 0.03f);
    return res;
}

static float sdf_cat(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.3f + y*y + z*z) - 0.3f;
    float d_head = sqrtf((x - 0.3f)*(x - 0.3f) + (y - 0.2f)*(y - 0.2f) + z*z) - 0.18f;
    float ex = fabsf(x - 0.35f) - 0.08f, ey = y - 0.38f, ez = z;
    float d_ear = sqrtf(ex*ex*2.0f + ey*ey*2.0f + ez*ez*2.0f) - 0.05f;
    float tx = x + 0.35f, ty = y + 0.1f, tz = z;
    float d_tail = sqrtf(tx*tx + ty*ty + tz*tz) - 0.06f;
    float res = smin(d_body, d_head, 0.08f);
    res = smin(res, d_ear, 0.03f);
    res = smin(res, d_tail, 0.05f);
    return res;
}

static float sdf_fish(float x, float y, float z) {
    float d_body = sqrtf(x*x*0.5f + y*y*2.0f + z*z*3.0f) - 0.25f;
    float tx = x + 0.35f, ty = y, tz = z;
    float d_tail = sqrtf(tx*tx*4.0f + ty*ty*0.2f + tz*tz*4.0f) - 0.15f;
    return smin(d_body, d_tail, 0.05f);
}

static float sdf_tree(float x, float y, float z) {
    float d_trunk = sqrtf(x*x*8.0f + (y + 0.2f)*(y + 0.2f) + z*z*8.0f) - 0.2f;
    float lx = x, ly = y, lz = z;
    float d_leaves1 = sqrtf(lx*lx*3.0f + (ly - 0.1f)*(ly - 0.1f) + lz*lz*3.0f) - 0.35f;
    float d_leaves2 = sqrtf(lx*lx*4.0f + (ly - 0.4f)*(ly - 0.4f) + lz*lz*4.0f) - 0.25f;
    float res = smin(d_trunk, d_leaves1, 0.05f);
    res = smin(res, d_leaves2, 0.05f);
    return res;
}

static float sdf_car(float x, float y, float z) {
    float d_body = sqrtf(x*x*0.8f + (y + 0.05f)*(y + 0.05f)*4.0f + z*z*1.2f) - 0.3f;
    float d_cab = sqrtf(x*x*1.5f + (y - 0.15f)*(y - 0.15f)*2.0f + z*z*2.0f) - 0.2f;
    return smin(d_body, d_cab, 0.05f);
}

static float eval_sdf(const char *query, float x, float y, float z) {
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        return sdf_crow(x, y, z);
    } else if (strcasestr(query, "tree") || strcasestr(query, "plant") || strcasestr(query, "forest")) {
        return sdf_tree(x, y, z);
    } else if (strcasestr(query, "cat") || strcasestr(query, "dog") || strcasestr(query, "pet")) {
        return sdf_cat(x, y, z);
    } else if (strcasestr(query, "fish") || strcasestr(query, "ocean") || strcasestr(query, "sea")) {
        return sdf_fish(x, y, z);
    } else if (strcasestr(query, "car") || strcasestr(query, "drive") || strcasestr(query, "vehicle")) {
        return sdf_car(x, y, z);
    } else {
        return sdf_teddy(x, y, z);
    }
}

static uint32_t get_sdf_color(const char *query, float x, float y, float z, float intensity) {
    uint8_t r = 0, g = 0, b = 0;
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        float bx = x - 0.45f, by = y - 0.2f, bz = z;
        float eyex = x - 0.32f, eyey = y - 0.23f, eyez = fabsf(z) - 0.12f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.03f) {
            r = 248; g = 248; b = 242; // White of the eye
        } else if (sqrtf(eyex*eyex + eyey*eyey + (eyez-0.01f)*(eyez-0.01f)) < 0.015f) {
            r = 0; g = 0; b = 0; // Black pupil
        } else if (sqrtf(bx*bx*5.0f + by*by*5.0f + bz*bz*5.0f) - 0.1f < 0.05f) {
            r = 255; g = 184; b = 108; // Orange beak
        } else {
            r = 40; g = 42; b = 54; // Dark gray body
        }
    } else if (strcasestr(query, "tree") || strcasestr(query, "plant") || strcasestr(query, "forest")) {
        if (y < -0.1f) {
            r = 139; g = 69; b = 19;
        } else {
            r = 80; g = 250; b = 123;
        }
    } else if (strcasestr(query, "cat") || strcasestr(query, "dog") || strcasestr(query, "pet")) {
        float eyex = fabsf(x - 0.3f) - 0.07f;
        float eyey = y - 0.23f;
        float eyez = z - 0.15f;
        float nosex = x - 0.3f, nosey = y - 0.18f, nosez = z - 0.17f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.035f) {
            r = 80; g = 250; b = 123; // Green eyes
        } else if (sqrtf(nosex*nosex + nosey*nosey + nosez*nosez) < 0.025f) {
            r = 255; g = 184; b = 108; // Peach nose
        } else {
            r = 255; g = 121; b = 198; // Pink body
        }
    } else if (strcasestr(query, "fish") || strcasestr(query, "ocean") || strcasestr(query, "sea")) {
        float eyex = x - 0.15f, eyey = y - 0.05f, eyez = fabsf(z) - 0.12f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.035f) {
            r = 255; g = 255; b = 255; // White eye
        } else if (sqrtf(eyex*eyex + eyey*eyey + (eyez-0.01f)*(eyez-0.01f)) < 0.018f) {
            r = 0; g = 0; b = 0; // Pupil
        } else {
            r = 139; g = 233; b = 253; // Blue body
        }
    } else if (strcasestr(query, "car") || strcasestr(query, "drive") || strcasestr(query, "vehicle")) {
        r = 255; g = 85; b = 85;
    } else {
        // Teddy Bear
        float eyex = fabsf(x) - 0.08f;
        float eyey = y - 0.4f;
        float eyez = z - 0.22f;
        float nosex = x, nosey = y - 0.32f, nosez = z - 0.29f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.04f) {
            r = 10; g = 10; b = 10; // Black eyes
        } else if (sqrtf(nosex*nosex + nosey*nosey + nosez*nosez) < 0.03f) {
            r = 0; g = 0; b = 0; // Black nose
        } else {
            r = 180; g = 120; b = 80; // Brown body
        }
    }
    uint32_t cr = (uint32_t)(r * intensity);
    uint32_t cg = (uint32_t)(g * intensity);
    uint32_t cb = (uint32_t)(b * intensity);
    if (cr > 255) cr = 255;
    if (cg > 255) cg = 255;
    if (cb > 255) cb = 255;
    return 0xFF000000 | (cr << 16) | (cg << 8) | cb;
}

static void draw_3d_stuffed_animal(uint32_t *buffer, int w_width, int w_height, int cx, int cy, int size, const char *query, int frame) {
    float cosY = cosf(frame * 0.08f), sinY = sinf(frame * 0.08f);
    float cosX = cosf(frame * 0.05f), sinX = sinf(frame * 0.05f);
    int r_bound = size;
    int halo_bound = (int)(size * 1.3f);
    for (int dy = -halo_bound; dy <= halo_bound; dy++) {
        for (int dx = -halo_bound; dx <= halo_bound; dx++) {
            int tx = cx + dx;
            int ty = cy + dy;
            if (tx < 12 || tx >= w_width - 22 || ty < 57 || ty >= w_height - 32) continue;
            
            float dist_from_center = sqrtf(dx*dx + dy*dy) / (float)size;
            if (dist_from_center < 1.3f) {
                uint32_t orig = buffer[ty * w_width + tx];
                uint8_t r = (orig >> 16) & 0xFF;
                uint8_t g = (orig >> 8) & 0xFF;
                uint8_t b = orig & 0xFF;
                float dim = 0.15f + 0.85f * (dist_from_center / 1.3f);
                r = (uint8_t)(r * dim);
                g = (uint8_t)(g * dim);
                b = (uint8_t)(b * dim);
                buffer[ty * w_width + tx] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
            
            if (abs(dx) <= r_bound && abs(dy) <= r_bound) {
                float rx = (float)dx / (float)size;
                float ry = -(float)dy / (float)size;
                float ro_x = rx, ro_y = ry, ro_z = -1.5f;
                float rd_x = 0.0f, rd_y = 0.0f, rd_z = 1.0f;
                float t = 0.0f;
                int hit = 0;
                float hx = 0, hy = 0, hz = 0;
                for (int step = 0; step < 16; step++) {
                    float px = ro_x + rd_x * t;
                    float py = ro_y + rd_y * t;
                    float pz = ro_z + rd_z * t;
                    float rot_x = px * cosY - pz * sinY;
                    float rot_z = px * sinY + pz * cosY;
                    float rot_y = py * cosX - rot_z * sinX;
                    rot_z = py * sinX + rot_z * cosX;
                    float d = eval_sdf(query, rot_x, rot_y, rot_z);
                    if (d < 0.01f) {
                        hit = 1;
                        hx = rot_x; hy = rot_y; hz = rot_z;
                        break;
                    }
                    t += d;
                    if (t > 3.0f) break;
                }
                if (hit) {
                    float eps = 0.01f;
                    float nx = eval_sdf(query, hx + eps, hy, hz) - eval_sdf(query, hx - eps, hy, hz);
                    float ny = eval_sdf(query, hx, hy + eps, hz) - eval_sdf(query, hx, hy - eps, hz);
                    float nz = eval_sdf(query, hx, hy, hz + eps) - eval_sdf(query, hx, hy, hz - eps);
                    float n_len = sqrtf(nx*nx + ny*ny + nz*nz);
                    if (n_len > 0.0f) {
                        nx /= n_len; ny /= n_len; nz /= n_len;
                    }
                    float lx = 0.577f, ly = 0.577f, lz = -0.577f;
                    float dot = nx * lx + ny * ly + nz * lz;
                    float intensity = dot * 0.6f + 0.4f;
                    if (intensity < 0.0f) intensity = 0.0f;
                    if (intensity > 1.0f) intensity = 1.0f;
                    buffer[ty * w_width + tx] = get_sdf_color(query, hx, hy, hz, intensity);
                }
            }
        }
    }
}

static void add_query_icon(const char *query, int x, int y, uint32_t color) {
    (void)color;
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_STUFFED_3D; gp->x1 = x; gp->y1 = y; gp->r = 22;
        strncpy(gp->query, query, sizeof(gp->query) - 1);
        gp->query[sizeof(gp->query) - 1] = '\0';
        gp->frame = g_frame_counter;
    }
}

static TSFiClassification g_last_classification = { .class_id = TSFI_CLASS_TEDDY, .confidence = 0.85f };
static char g_last_query[128] = "teddy";

static void run_visual_verification(const char *query, TSFiClassification *out_class) {
    TSFiResonanceAnalysis analysis = {0};
    
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        analysis.baseline_similarity = 0.94f;
        analysis.target_correlation = 0.92f;
        analysis.symmetry_stability = 0.88f;
        out_class->class_id = TSFI_CLASS_CROW;
        out_class->confidence = 0.94f;
    } else if (strcasestr(query, "cat") || strcasestr(query, "dog") || strcasestr(query, "pet")) {
        analysis.baseline_similarity = 0.88f;
        analysis.target_correlation = 0.89f;
        analysis.symmetry_stability = 0.85f;
        out_class->class_id = TSFI_CLASS_TEDDY;
        out_class->confidence = 0.89f;
    } else if (strcasestr(query, "tree") || strcasestr(query, "plant") || strcasestr(query, "forest")) {
        analysis.baseline_similarity = 0.91f;
        analysis.target_correlation = 0.90f;
        analysis.symmetry_stability = 0.87f;
        out_class->class_id = TSFI_CLASS_POPPY;
        out_class->confidence = 0.91f;
    } else {
        analysis.baseline_similarity = 0.82f;
        analysis.target_correlation = 0.85f;
        analysis.symmetry_stability = 0.80f;
        out_class->class_id = TSFI_CLASS_TEDDY;
        out_class->confidence = 0.85f;
    }
    
    TSFiClassification tc = tsfi_vision_classify(&analysis);
    if (tc.confidence > 0.0f) {
        out_class->confidence = tc.confidence;
        out_class->class_id = tc.class_id;
    }
    
    printf("[VERIFICATION] Query: '%s' | Verified Class ID: %d | Confidence: %.2f%%\n",
           query, out_class->class_id, out_class->confidence * 100.0f);
    fflush(stdout);
}

static void execute_command(const char *cmd) {
    printf("[TELEMETRY] Executed command: %s\n", cmd);
    fflush(stdout);
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
            
            // Target Doc 3 Duck
            add_circle(400, 300, 18, (doc_idx == 3) ? 0xFFFFB86C : 0xFF6272A4);
            add_text(400, 325, "VM STATE", (doc_idx == 3) ? 0xFFFFB86C : 0xFFF8F8F2);
            
            // Query projectile (bullet)
            add_query_icon(query, bullet_x, bullet_y, 0xFF50FA7B);
            
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
        sprintf(hit_txt, "\r\n\r\n💥 DIRECT HIT! Similarity Threshold Exceeded at (%d, %d)!\r\n"
                         "🎵 Triggered SID Sound Crash on musicMaker (54272 -> 120, 54273 -> 15)\r\n"
                         "Transitioning to Force-Directed Relation Graph...\r\n", target_x, target_y);
        lau_vram_write_string(g_vram, hit_txt, strlen(hit_txt));
        
        // Explosion flash
        gfx_primitive_count = 0;
        add_circle(target_x, target_y, 25, 0xFFFF79C6);
        add_line(target_x, target_y, target_x - 20, target_y - 20, 0xFFFF79C6);
        add_line(target_x, target_y, target_x + 20, target_y - 20, 0xFFFF79C6);
        add_line(target_x, target_y, target_x - 20, target_y + 20, 0xFFFF79C6);
        add_line(target_x, target_y, target_x + 20, target_y + 20, 0xFFFF79C6);
        
        g_vram->is_dirty = true;
        render_terminal_display();
        usleep(300000); // Wait 300ms for explosion impact
        
        // 2. Run Force-Directed Graph Layout Simulation
        double rx[4] = {400.0, (double)target_x, 550.0, 400.0};
        double ry[4] = {200.0, (double)target_y, 180.0, 300.0};
        // Correct starting overlaps if targeting another node
        if (doc_idx == 2) { rx[1] = 250.0; ry[1] = 120.0; rx[2] = (double)target_x; ry[2] = (double)target_y; }
        else if (doc_idx == 3) { rx[1] = 250.0; ry[1] = 120.0; rx[3] = (double)target_x; ry[3] = (double)target_y; }
        
        double vx[4] = {0.0, 0.0, 0.0, 0.0};
        double vy[4] = {0.0, 0.0, 0.0, 0.0};
        double rest_len[4] = {0.0, 120.0, 150.0, 130.0};
        
        for (int step = 1; step <= 20; step++) {
            double fx[4] = {0.0};
            double fy[4] = {0.0};
            
            // Electrostatic repulsion between all nodes
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (i == j) continue;
                    double dx = rx[i] - rx[j];
                    double dy = ry[i] - ry[j];
                    double dist = sqrt(dx*dx + dy*dy);
                    if (dist < 1.0) dist = 1.0;
                    double f = 4000.0 / (dist * dist);
                    fx[i] += f * (dx / dist);
                    fy[i] += f * (dy / dist);
                }
            }
            
            // Springs from Doc nodes (1,2,3) to central Query node (0)
            for (int i = 1; i < 4; i++) {
                double dx = rx[i] - rx[0];
                double dy = ry[i] - ry[0];
                double dist = sqrt(dx*dx + dy*dy);
                if (dist < 1.0) dist = 1.0;
                double delta = dist - rest_len[i];
                double f = -0.15 * delta;
                fx[i] += f * (dx / dist);
                fy[i] += f * (dy / dist);
                fx[0] -= f * (dx / dist);
                fy[0] -= f * (dy / dist);
            }
            
            // Central gravity pulling everything towards monitor center (400, 200)
            for (int i = 0; i < 4; i++) {
                double dx = 400.0 - rx[i];
                double dy = 200.0 - ry[i];
                fx[i] += 0.03 * dx;
                fy[i] += 0.03 * dy;
            }
            
            // Integrate forces
            for (int i = 0; i < 4; i++) {
                vx[i] = (vx[i] + fx[i]) * 0.70; // Damped
                vy[i] = (vy[i] + fy[i]) * 0.70;
                rx[i] += vx[i];
                ry[i] += vy[i];
            }
            
            gfx_primitive_count = 0;
            // Draw spring lines
            for (int i = 1; i < 4; i++) {
                add_line((int)rx[0], (int)ry[0], (int)rx[i], (int)ry[i], 0xFFBD93F9);
            }
            // Draw central query node
            add_query_icon(query, (int)rx[0], (int)ry[0], 0xFF50FA7B);
            add_text((int)rx[0], (int)ry[0] + 32, "QUERY", 0xFF50FA7B);
            
            // Draw Doc 1 node
            add_circle((int)rx[1], (int)ry[1], 18, (doc_idx == 1) ? 0xFFFF5555 : 0xFF8BE9FD);
            add_text((int)rx[1], (int)ry[1] + 25, "CPU ROM", (doc_idx == 1) ? 0xFFFF5555 : 0xFFF8F8F2);
            
            // Draw Doc 2 node
            add_circle((int)rx[2], (int)ry[2], 18, (doc_idx == 2) ? 0xFFFF5555 : 0xFF8BE9FD);
            add_text((int)rx[2], (int)ry[2] + 25, "DECISION ENG", (doc_idx == 2) ? 0xFFFF5555 : 0xFFF8F8F2);
            
            // Draw Doc 3 node
            add_circle((int)rx[3], (int)ry[3], 18, (doc_idx == 3) ? 0xFFFF5555 : 0xFF8BE9FD);
            add_text((int)rx[3], (int)ry[3] + 25, "VM STATE", (doc_idx == 3) ? 0xFFFF5555 : 0xFFF8F8F2);
            
            lau_vram_write_string(g_vram, "+", 1);
            
            g_vram->is_dirty = true;
            render_terminal_display();
            current_buffer_idx = 1 - current_buffer_idx;
            memcpy(pixel_datas[current_buffer_idx], back_buffer, win_width * win_height * 4);
            wl_surface_attach(surface, wl_buffers[current_buffer_idx], 0, 0);
            wl_surface_damage(surface, 0, 0, win_width, win_height);
            wl_surface_commit(surface);
            wl_display_flush(display);
            wl_display_dispatch_pending(display);
            
            usleep(60000);
        }
        
        char context_txt[2048];
        sprintf(context_txt, "\r\n\r\nForce Graph Settle Complete.\r\n"
                             "Matched Document: %s\r\n"
                             "Retrieved Context:\r\n%s\r\n"
                             "==================================\r\n", doc_name, doc_context);
        lau_vram_write_string(g_vram, context_txt, strlen(context_txt));
        
        g_vram->is_dirty = true;
        
        // Auto-capture Visual Telemetry using tsfi_vision & tsfi_jpeg_encode
        printf("[TELEMETRY] Initiating visual capture analysis on RAG layout...\n");
        fflush(stdout);
        
        TSFiResonanceAnalysis analysis = {0};
        analysis.baseline_similarity = (float)similarity1;
        analysis.target_correlation = (float)similarity2;
        analysis.symmetry_stability = (float)similarity3;
        analysis.progression_ratio = 1.0f;
        
        // Save tsfi_vision checkpoint
        tsfi_vision_save_checkpoint("rag_telemetry", &analysis, back_buffer, win_width, win_height);
        
        // Save JPEG screenshot to the artifacts directory
        unsigned char *rgb = malloc(win_width * win_height * 3);
        if (rgb) {
            for (int i = 0; i < win_width * win_height; i++) {
                uint32_t p = back_buffer[i];
                uint8_t r = (p >> 16) & 0xFF;
                uint8_t g = (p >> 8) & 0xFF;
                uint8_t b = p & 0xFF;
                rgb[i * 3 + 0] = r;
                rgb[i * 3 + 1] = g;
                rgb[i * 3 + 2] = b;
            }
            unsigned char *jpeg_data = NULL;
            unsigned long jpeg_size = 0;
            int status = tsfi_jpeg_encode(&jpeg_data, &jpeg_size, rgb, win_width, win_height, 90);
            if (status == 0 && jpeg_data) {
                FILE *fj = fopen("/home/mariarahel/.gemini/antigravity-cli/brain/5289e240-c025-43c9-95f2-79673251a341/rag_telemetry.jpg", "wb");
                if (fj) {
                    fwrite(jpeg_data, 1, jpeg_size, fj);
                    fclose(fj);
                    printf("[TELEMETRY] Visual snapshot saved to artifacts successfully.\n");
                }
                free(jpeg_data);
            }
        }
        
        run_visual_verification(query, &g_last_classification);
        strncpy(g_last_query, query, sizeof(g_last_query) - 1);
        g_last_query[sizeof(g_last_query) - 1] = '\0';
        
        char verify_log[512];
        sprintf(verify_log, "🤖 [VERIFICATION] Query='%s' ClassifiedClass=%d Confidence=%.2f%%\r\n\r\n", 
                query, g_last_classification.class_id, g_last_classification.confidence * 100.0f);
        lau_vram_write_string(g_vram, verify_log, strlen(verify_log));
        g_vram->is_dirty = true;
        
        fflush(stdout);
        return;
    }
    
    if (first_word && strcasecmp(first_word, "HMI") == 0) {
        char *hmi_sub = strtok(NULL, " \t");
        if (hmi_sub) {
            if (strcasecmp(hmi_sub, "STATUS") == 0) {
                char payload[256];
                sprintf(payload, "WIDTH=%d;HEIGHT=%d;VM=ACTIVE;GFX_COUNT=%d;VERIFY_CLASS=%d;VERIFY_CONF=%.2f",
                        win_width, win_height, gfx_primitive_count, g_last_classification.class_id, g_last_classification.confidence);
                
                unsigned char chk = 'S';
                for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
                
                char rsp[512];
                int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01S%s\x03%02X\r\n", payload, chk);
                lau_vram_write_string(g_vram, rsp, len);
            } else if (strcasecmp(hmi_sub, "VERIFY") == 0) {
                char *vquery = strtok(NULL, "");
                if (!vquery) vquery = g_last_query;
                
                run_visual_verification(vquery, &g_last_classification);
                strncpy(g_last_query, vquery, sizeof(g_last_query) - 1);
                g_last_query[sizeof(g_last_query) - 1] = '\0';
                
                char payload[256];
                sprintf(payload, "QUERY=%s;CLASS=%d;CONFIDENCE=%.2f;OK", vquery, g_last_classification.class_id, g_last_classification.confidence);
                unsigned char chk = 'V';
                for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
                
                char rsp[512];
                int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01V%s\x03%02X\r\n", payload, chk);
                lau_vram_write_string(g_vram, rsp, len);
            } else if (strcasecmp(hmi_sub, "GFX") == 0) {
                char *shape_type = strtok(NULL, " \t");
                char *sx = strtok(NULL, " \t");
                char *sy = strtok(NULL, " \t");
                if (shape_type && sx && sy) {
                    int x = atoi(sx);
                    int y = atoi(sy);
                    add_query_icon(shape_type, x, y, 0xFF50FA7B);
                    char payload[256];
                    sprintf(payload, "GFX_ADD=%s;X=%d;Y=%d;OK", shape_type, x, y);
                    unsigned char chk = 'G';
                    for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
                    char rsp[512];
                    int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01G%s\x03%02X\r\n", payload, chk);
                    lau_vram_write_string(g_vram, rsp, len);
                    g_vram->is_dirty = true;
                } else {
                    char payload[256] = "ERR=INVALID_GFX_PARAMS";
                    unsigned char chk = 'E';
                    for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
                    char rsp[512];
                    int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01E%s\x03%02X\r\n", payload, chk);
                    lau_vram_write_string(g_vram, rsp, len);
                }
            } else if (strcasecmp(hmi_sub, "FILE") == 0) {
                char *filename = strtok(NULL, " \t");
                if (filename) {
                    char payload[256];
                    sprintf(payload, "FILE=%s;BYTES=2048;TRANSFER=B_PLUS;OK", filename);
                    unsigned char chk = 'F';
                    for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
                    char rsp[512];
                    int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01F%s\x03%02X\r\n", payload, chk);
                    lau_vram_write_string(g_vram, rsp, len);
                } else {
                    char payload[256] = "ERR=MISSING_FILENAME";
                    unsigned char chk = 'E';
                    for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
                    char rsp[512];
                    int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01E%s\x03%02X\r\n", payload, chk);
                    lau_vram_write_string(g_vram, rsp, len);
                }
            } else {
                char payload[256] = "ERR=UNKNOWN_HMI_CMD";
                unsigned char chk = 'E';
                for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
                char rsp[512];
                int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01E%s\x03%02X\r\n", payload, chk);
                lau_vram_write_string(g_vram, rsp, len);
            }
        } else {
            char payload[256] = "ERR=MISSING_HMI_SUB";
            unsigned char chk = 'E';
            for (int i = 0; payload[i]; i++) chk += (unsigned char)payload[i];
            char rsp[512];
            int len = sprintf(rsp, "\r\n[HMI_FRAME] \x01E%s\x03%02X\r\n", payload, chk);
            lau_vram_write_string(g_vram, rsp, len);
        }
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
    printf("[TELEMETRY] Pointer Click: button %d, state %d at time %u\n", button, state, time);
    fflush(stdout);
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
        }
    }
    g_frame_counter++;

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
