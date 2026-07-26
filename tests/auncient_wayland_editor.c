#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "../src/auncient_timeline_autodin.h"

// Include global state data component
#include "auncient_wayland_data.c"

static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_seat *seat = NULL;
static struct wl_keyboard *keyboard = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct wl_surface *surface = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static struct wl_buffer *wl_buffers[2] = {NULL, NULL};
static int current_buffer_idx = 0;
static bool running = true;



#include "auncient_wayland_audio.c"

// Pre-computed Morphological character frames (simulating pre-baked sprite assets)
typedef struct {
    uint16_t dilated[16];
    uint16_t eroded[16];
    uint16_t glints[16];
} MorphFrame;

static MorphFrame morph_cache[6][5];

static int current_water_idx = 0;
static int raymarch_mode = 0;
static int material_variant = 0;

static char key_history[3] = {'\0', '\0', '\0'};

typedef struct {
    uint32_t active_model;
    uint32_t material_variant;
    float rotation_angle;
    float camera_y;
    uint32_t starfield_count;
} USDStageRecord;

typedef struct {
    float rotation_angle;
    float camera_y;
    uint32_t active_model;
    float raster_intensity;
    uint32_t material_variant; // USD Material VariantSet selection!
    float padding[11];
} CoaxialUBO;

static CoaxialUBO active_ubo;

typedef struct {
    void (*render_scene)(uint32_t *pixels, int w, int h, const CoaxialUBO *ubo);
} HydraRenderDelegate;
#include "auncient_wayland_graphics.c"

typedef struct {
    int x;
    int y;
    char glyph;
    uint32_t color;
} MultiplexSprite;

static MultiplexSprite sprites[5] = {
    {100, 150, 'A', 0xFFFF00FF},
    {200, 250, 'B', 0xFF00FFFF},
    {300, 100, 'C', 0xFFFFFF00},
    {150, 300, 'D', 0xFF00FF00},
    {250, 80,  'E', 0xFFFF5555}
};

static int sorted_sprite_indices[5] = {0, 1, 2, 3, 4};

typedef struct {
    float x;
    float y;
    float z;
    char glyph;
} Star;

static Star starfield[15];

typedef struct {
    float x;
    int y;
    float speed;
    char glyph;
    uint32_t color;
} ScrollStar;

static ScrollStar scroll_stars[30];

typedef struct {
    uint32_t transaction_id;
    uint32_t state_code;
    char action;
    uint32_t hash;
} EditorHistoryRecord;

static struct wl_buffer *create_shm_buffer(int width, int height, uint32_t **out_pixels) {
    int stride = width * 4;
    int size = stride * height;
    
    int fd = memfd_create("shm-editor", MFD_CLOEXEC);
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

static const uint8_t water_remove_raw[40][5] = {
    {96, 25, 0, 16, 255},
    {97, 25, 192, 15, 3},
    {98, 25, 192, 15, 3},
    {99, 25, 240, 15, 15},
    {100, 25, 252, 15, 63},
    {101, 25, 252, 15, 63},
    {110, 25, 0, 14, 255},
    {111, 25, 192, 13, 3},
    {168, 26, 192, 13, 3},
    {169, 26, 240, 13, 15},
    {170, 26, 252, 13, 63},
    {179, 26, 0, 12, 255},
    {180, 26, 192, 11, 3},
    {181, 26, 240, 11, 15},
    {182, 26, 252, 11, 63},
    {191, 26, 0, 10, 255},
    {248, 27, 192, 9, 3},
    {249, 27, 240, 9, 15},
    {2, 28, 0, 8, 255},
    {3, 28, 0, 8, 255},
    {4, 28, 0, 8, 255},
    {5, 28, 0, 8, 255},
    {6, 28, 0, 8, 255},
    {7, 28, 0, 8, 255},
    {64, 29, 0, 8, 255},
    {65, 29, 0, 8, 255},
    {66, 29, 0, 8, 255},
    {67, 29, 0, 8, 255},
    {68, 29, 0, 8, 255},
    {69, 29, 0, 8, 255},
    {70, 29, 0, 8, 255},
    {71, 29, 0, 8, 255},
    {128, 30, 0, 8, 255},
    {129, 30, 0, 8, 255},
    {130, 30, 0, 8, 255},
    {131, 30, 0, 8, 255},
    {132, 30, 0, 8, 255},
    {133, 30, 0, 8, 255},
    {134, 30, 0, 8, 255},
    {135, 30, 0, 8, 255}
};

// Simulated Self-Modifying Code (SMC) line remover and ZMM-style backup mirroring
static void simulate_smc_remove_next_line(uint32_t *active, uint32_t *backup, int win_w, int win_h, int ground, float flood_height) {
    if (flood_height <= 0.0f) return;
    
    // Determine table lookup index based on flood height
    int table_idx = (int)(flood_height * 1.5f);
    if (table_idx < 0) table_idx = 0;
    if (table_idx >= 40) table_idx = 39;
    current_water_idx = table_idx;
    
    // Extract parameters from raw water remove table configuration data
    uint8_t count = water_remove_raw[table_idx][3];
    uint32_t start_mask = ((uint32_t)water_remove_raw[table_idx][2] << 24) | 0x00FFFFFF;
    uint32_t end_mask = ((uint32_t)water_remove_raw[table_idx][4] << 24) | 0x00FFFFFF;
    
    int row_displacement = ground + 30 - (int)flood_height;
    if (row_displacement < 0 || row_displacement >= win_h) return;
    
    // Clear elements on both active screen and backup cache using simulated SMC loops and dynamic ripples
    for (int col = 0; col < win_w; col++) {
        // Calculate sinusoidal water ripple wave coordinates dynamically
        int ripple_y = row_displacement + (int)(sinf(retro_time * 6.0f + col * 0.06f) * 4.0f);
        if (ripple_y < 0) ripple_y = 0;
        if (ripple_y >= win_h) continue;
        
        int idx_boundary = ripple_y * win_w + col;
        
        if (col < 10) {
            active[idx_boundary] &= start_mask;
            backup[idx_boundary] &= start_mask;
        } else if (col < 10 + count * 8) {
            active[idx_boundary] = 0xFF051224; // Blue clear color at surface boundary
            backup[idx_boundary] = 0xFF051224;
        } else if (col < 10 + count * 8 + 10) {
            active[idx_boundary] &= end_mask;
            backup[idx_boundary] &= end_mask;
        }
        
        // Volumetric water body: Fill everything below the wavy surface with solid blue water
        for (int y = ripple_y + 1; y < win_h; y++) {
            active[y * win_w + col] = 0xFF051224;
            backup[y * win_w + col] = 0xFF051224;
        }
    }
}

// Pre-baked physics collision timeline frame structure mapping collision_20250902081037.inc
typedef struct {
    uint8_t delta_frame;
    uint8_t water_change;
    struct {
        uint8_t char_id;
        int16_t x;
        int16_t y;
    } updates[6];
    uint8_t update_count;
} PrebakedFrame;

static const PrebakedFrame prebaked_script[5] = {
    {0, 0, {{0, 100, 120}, {1, 160, 130}, {2, 220, 140}, {3, 280, 150}, {4, 340, 160}, {5, 400, 170}}, 6},
    {72, 0, {{1, 162, 148}, {2, 222, 148}, {3, 282, 148}}, 3},
    {13, 1, {{4, 342, 157}, {5, 402, 157}}, 2},
    {1, 1, {{0, 102, 145}, {1, 162, 145}}, 2},
    {1, 2, {{2, 222, 160}, {3, 282, 160}, {4, 342, 160}, {5, 402, 160}}, 4}
};

#include "auncient_wayland_scene.c"


#include "auncient_wayland_screen.c"

// XDG Shell Base listeners
static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_handle_ping,
};

// XDG Surface listeners
static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    (void)data; (void)xdg_toplevel; (void)states;
    if (width > 0 && height > 0) {
        win_width = width;
        win_height = height;
        redraw_screen();
    }
}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    (void)data; (void)xdg_toplevel;
    running = false;
}

static void xdg_toplevel_handle_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
    (void)data; (void)xdg_toplevel; (void)width; (void)height;
}

static void xdg_toplevel_handle_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
    (void)data; (void)xdg_toplevel; (void)capabilities;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close,
    .configure_bounds = xdg_toplevel_handle_configure_bounds,
    .wm_capabilities = xdg_toplevel_handle_wm_capabilities,
};

#include "auncient_wayland_input.c"

// Seat capabilities listener
static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
    (void)data;
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    }
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
    (void)data; (void)wl_seat; (void)name;
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = seat_handle_name,
};

// Registry handler
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void)data;
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
        wl_seat_add_listener(seat, &seat_listener, NULL);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

int main(void) {
    srand(time(NULL));

    printf("[DECRUNCH] Initiating in-place decompressed payload relocation...\n");
    char decrunch_temp[128];
    int de_idx = 0;
    for (int i = 0; i < 4; i++) {
        for (int k = 0; k < 30; k++) {
            if (compressed_scroller[k] == '\0') break;
            decrunch_temp[de_idx++] = compressed_scroller[k];
        }
    }
    decrunch_temp[de_idx] = '\0';
    memcpy(decompressed_scroller, decrunch_temp, de_idx + 1);
    printf("[DECRUNCH] Relocated %d bytes from 0x2000 to 0x0800\n", de_idx);

    for (int i = 0; i < 256; i++) {
        sine_lut[i] = sinf((float)i * (2.0f * M_PI / 256.0f));
    }

    precompute_morph_cache();

    // USD Composition Resolver: LIVRPS (Local > Inherits > VariantSets > References > Payloads > Specializes) Precedence Stack
    // 1. Specializes (S - Weakest composition arc)
    FILE *f_usd_spec = fopen("assets/usd_special.dat.bin", "rb");
    if (f_usd_spec) {
        USDStageRecord spec_layer;
        if (fread(&spec_layer, sizeof(spec_layer), 1, f_usd_spec) == 1) {
            raymarch_mode = (int)spec_layer.active_model;
            material_variant = (int)spec_layer.material_variant;
            printf("[USD] LIVRPS: Resolved Specializes Layer from assets/usd_special.dat.bin\n");
        }
        fclose(f_usd_spec);
    }

    // 2. Payloads (P)
    FILE *f_usd_pay = fopen("assets/usd_payload.dat.bin", "rb");
    if (f_usd_pay) {
        USDStageRecord pay_layer;
        if (fread(&pay_layer, sizeof(pay_layer), 1, f_usd_pay) == 1) {
            raymarch_mode = (int)pay_layer.active_model;
            material_variant = (int)pay_layer.material_variant;
            printf("[USD] LIVRPS: Resolved Payload Layer from assets/usd_payload.dat.bin\n");
        }
        fclose(f_usd_pay);
    }

    // 3. References (R)
    FILE *f_usd_ref = fopen("assets/usd_reference.dat.bin", "rb");
    if (f_usd_ref) {
        USDStageRecord ref_layer;
        if (fread(&ref_layer, sizeof(ref_layer), 1, f_usd_ref) == 1) {
            raymarch_mode = (int)ref_layer.active_model;
            material_variant = (int)ref_layer.material_variant;
            printf("[USD] LIVRPS: Resolved Reference Layer from assets/usd_reference.dat.bin\n");
        }
        fclose(f_usd_ref);
    }

    // 4. VariantSets & Base Stage (V)
    FILE *f_usd_load = fopen("assets/usd_stage.dat.bin", "rb");
    if (f_usd_load) {
        USDStageRecord stage;
        if (fread(&stage, sizeof(stage), 1, f_usd_load) == 1) {
            raymarch_mode = (int)stage.active_model;
            material_variant = (int)stage.material_variant;
            printf("[USD] LIVRPS: Resolved Base Stage Layer from assets/usd_stage.dat.bin\n");
        }
        fclose(f_usd_load);
    } else {
        USDStageRecord default_stage = {
            .active_model = 0,
            .material_variant = 0,
            .rotation_angle = 0.0f,
            .camera_y = 0.0f,
            .starfield_count = 15
        };
        FILE *f_usd_write = fopen("assets/usd_stage.dat.bin", "wb");
        if (f_usd_write) {
            fwrite(&default_stage, sizeof(default_stage), 1, f_usd_write);
            fclose(f_usd_write);
            printf("[USD] Created default scene stage containing Cactus asset at assets/usd_stage.dat.bin\n");
        }
    }

    // 5. Inherits (I)
    FILE *f_usd_inh = fopen("assets/usd_class.dat.bin", "rb");
    if (f_usd_inh) {
        USDStageRecord inh_layer;
        if (fread(&inh_layer, sizeof(inh_layer), 1, f_usd_inh) == 1) {
            raymarch_mode = (int)inh_layer.active_model;
            material_variant = (int)inh_layer.material_variant;
            printf("[USD] LIVRPS: Resolved Inherits Layer from assets/usd_class.dat.bin\n");
        }
        fclose(f_usd_inh);
    }

    // 6. Local Overrides (L - Strongest composition arc)
    // Local layer includes sub-layers evaluated from weakest to strongest (index 0 to 2)
    const char *sublayers[3] = {
        "assets/usd_sublayer_2.dat.bin",
        "assets/usd_sublayer_1.dat.bin",
        "assets/usd_sublayer_0.dat.bin"
    };
    for (int i = 0; i < 3; i++) {
        FILE *f_sub = fopen(sublayers[i], "rb");
        if (f_sub) {
            USDStageRecord sub_layer;
            if (fread(&sub_layer, sizeof(sub_layer), 1, f_sub) == 1) {
                raymarch_mode = (int)sub_layer.active_model;
                material_variant = (int)sub_layer.material_variant;
                printf("[USD] LIVRPS: Resolved Sub-layer [%s]\n", sublayers[i]);
            }
            fclose(f_sub);
        }
    }
    FILE *f_usd_local = fopen("assets/usd_local.dat.bin", "rb");
    if (f_usd_local) {
        USDStageRecord local_layer;
        if (fread(&local_layer, sizeof(local_layer), 1, f_usd_local) == 1) {
            raymarch_mode = (int)local_layer.active_model;
            material_variant = (int)local_layer.material_variant;
            printf("[USD] LIVRPS: Resolved Local Overrides Layer from assets/usd_local.dat.bin\n");
        }
        fclose(f_usd_local);
    }

    for (int i = 0; i < 15; i++) {
        starfield[i].x = (float)((rand() % 200) - 100) / 10.0f;
        starfield[i].y = (float)((rand() % 200) - 100) / 10.0f;
        starfield[i].z = (float)(rand() % 100) / 10.0f + 0.5f;
        
        char glyphs[] = ".*+o";
        starfield[i].glyph = glyphs[rand() % 4];
    }

    for (int i = 0; i < 30; i++) {
        scroll_stars[i].x = (float)(rand() % 1280);
        scroll_stars[i].y = rand() % 720;
        scroll_stars[i].speed = 15.0f + (float)(rand() % 45);
        char glyphs[] = ".*+";
        scroll_stars[i].glyph = glyphs[rand() % 3];
        scroll_stars[i].color = (scroll_stars[i].speed > 35.0f) ? 0xFFFFCC00 : 0xFF884400;
    }

    sid_chip.voices[0].adsr[0] = 0x21;
    sid_chip.voices[0].adsr[1] = 0xF5;
    sid_chip.voices[1].adsr[0] = 0x42;
    sid_chip.voices[1].adsr[1] = 0x99;
    sid_chip.volume = 0x0F;
    sid_chip.hard_sync_enabled = true;
    sid_chip.ring_mod_enabled = true;

    ted_chip.ff06 = 0x28;
    ted_chip.ff07 = 0x18;
    ted_chip.ff12 = 0x20;
    ted_chip.ff14 = 0x78;
    ted_chip.ff16 = 0x41;

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "[ERROR] Connect to Wayland compositor failed.\n");
        return 1;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    
    wl_display_roundtrip(display);

    if (compositor && xdg_wm_base && shm) {
        surface = wl_compositor_create_surface(compositor);
        xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
        xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);

        xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
        xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        redraw_screen();
    }

    struct pollfd fds[1] = {
        { .fd = wl_display_get_fd(display), .events = POLLIN }
    };

    struct timespec start_ts;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);
    float last_time = 0.0f;

    while (running) {
        if (wl_display_get_error(display)) {
            break;
        }
        
        while (wl_display_prepare_read(display) != 0) {
            wl_display_dispatch_pending(display);
        }
        wl_display_flush(display);
        
        int ret = poll(fds, 1, 16);
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                if (wl_display_read_events(display) < 0) {
                    break;
                }
                wl_display_dispatch_pending(display);
            } else {
                wl_display_cancel_read(display);
            }
        } else {
            wl_display_cancel_read(display);
        }
        
        struct timespec cur_ts;
        clock_gettime(CLOCK_MONOTONIC, &cur_ts);
        retro_time = (float)(cur_ts.tv_sec - start_ts.tv_sec) + 
                     (float)(cur_ts.tv_nsec - start_ts.tv_nsec) * 1e-9f;
        
        float dt = retro_time - last_time;
        last_time = retro_time;

        if (loader_flash_time > 0.0f) {
            loader_flash_time -= dt;
        }

        if (type_activity > 0.0f) {
            type_activity -= dt * 2.5f;
        }

        float star_speed = active_tune == 3 ? 6.0f : 2.5f;
        for (int i = 0; i < 15; i++) {
            starfield[i].z -= dt * star_speed;
            if (starfield[i].z <= 0.1f) {
                starfield[i].x = (float)((rand() % 200) - 100) / 10.0f;
                starfield[i].y = (float)((rand() % 200) - 100) / 10.0f;
                starfield[i].z = 10.0f; // Reset to far boundary
            }
        }
        // Animate 2D scrolling stars
        for (int i = 0; i < 30; i++) {
            scroll_stars[i].x -= dt * scroll_stars[i].speed;
            if (scroll_stars[i].x < 0.0f) {
                scroll_stars[i].x = (float)win_width;
                scroll_stars[i].y = rand() % win_height;
            }
        }

        // Run VIC-II screen shake displacement glitches only during typing activity to keep layout smooth
        if (type_activity > 1.0f && ((int)(retro_time * 12.0f) % 2 == 0)) {
            glitch_x = (rand() % 3) - 1; // Subtle horizontal bump
            glitch_y = (rand() % 3) - 1; // Subtle vertical bump
        } else {
            glitch_x = 0;
            glitch_y = 0;
        }

        // Run simulated 50Hz SID playroutine updates
        static float last_sid_tick = 0.0f;
        if (retro_time - last_sid_tick >= 0.020f) { // 50Hz Interval
            last_sid_tick = retro_time;
            
            // 3-note arpeggiator step loop based on active tune selection (Skip Voice 1 updates if muted)
            if (voice_active[0]) {
                sid_arp_step = (sid_arp_step + 1) % 3;
                sid_chip.voices[0].freq = sid_tunes[active_tune][sid_arp_step] + (int)(sinf(retro_time * 5.0f) * 200.0f);
            } else {
                sid_chip.voices[0].freq = 0;
            }
            
            // Hard Sync and Ring Modulation sweep simulation on Voice 2
            if (voice_active[1]) {
                uint16_t base_pw = 0x0400 + (uint16_t)(sinf(retro_time * 3.0f) * 512.0f + 512.0f);
                if (sid_chip.ring_mod_enabled && voice_active[0]) {
                    // Multiply/modulate Voice 2 frequency with Voice 1 frequency (Ring modulation)
                    sid_chip.voices[1].pw = base_pw + (sid_chip.voices[0].freq >> 2);
                } else {
                    sid_chip.voices[1].pw = base_pw;
                }
                
                // Hard sync: sweep frequency higher and reset phase
                if (sid_chip.hard_sync_enabled) {
                    sid_chip.voices[1].freq = sid_chip.voices[0].freq * 2;
                } else {
                    sid_chip.voices[1].freq = 0x1100;
                }
            } else {
                sid_chip.voices[1].pw = 0;
                sid_chip.voices[1].freq = 0;
            }
            
            // Logarithmic SID Volume Fader simulation
            int fade_index = ((int)(retro_time * 4.0f)) & 0x0F;
            sid_chip.volume = voice_active[2] ? log_volume_lut[fade_index] : 0; // Map volume logarithmic fade curve LUT
        }
        
        redraw_screen();
    }

    // Cleanup Wayland resources
    for (int i = 0; i < 2; i++) {
        if (wl_buffers[i]) wl_buffer_destroy(wl_buffers[i]);
    }
    if (xdg_toplevel) xdg_toplevel_destroy(xdg_toplevel);
    if (xdg_surface) xdg_surface_destroy(xdg_surface);
    if (surface) wl_surface_destroy(surface);
    if (keyboard) wl_keyboard_destroy(keyboard);
    if (seat) wl_seat_destroy(seat);
    if (compositor) wl_compositor_destroy(compositor);
    if (xdg_wm_base) xdg_wm_base_destroy(xdg_wm_base);
    if (shm) wl_shm_destroy(shm);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    return 0;
}
