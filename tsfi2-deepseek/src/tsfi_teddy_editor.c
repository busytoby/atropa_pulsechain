#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <linux/input-event-codes.h>


#include "lau_memory.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "vulkan/vulkan_input.h"
#include "tsfi_logic.h"
#include "tsfi_vision.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_io.h"
#include "window_inc/tsfi_input.h"

#include "tsfi_controlnet_shm.h"
#include "tsfi_puppetry.h"
#include "tsfi_wire_firmware.h"

static volatile bool g_force_quit = false;
static volatile bool g_dirty = true;

static TsfiPuppetState *g_puppet_state = NULL;

static void handle_sigint(int sig) {
    (void)sig;
    g_force_quit = true;
}


static void render_teddy(uint32_t *px, int W, int H, const TsfiControlNetMap *depth_map, const TsfiControlNetMap *pose_map) {
    // Both maps are 256x256 * 3 bytes
    if (!depth_map || !pose_map) return;
    
    for (int y=0; y<H; y++) {
        for (int x=0; x<W; x++) {
            // nearest neighbor scale up if editor window is bigger, but actually the window is dynamically sized? 
            // W, H is paint_buffer->width, paint_buffer->height.
            // map is 256x256.
            int mx = x * 256 / W;
            int my = y * 256 / H;
            if (mx >= 256) mx = 255;
            if (my >= 256) my = 255;
            
            int m_idx = (my * 256 + mx) * 3;
            
            uint8_t dr = depth_map->data[m_idx];
            // uint8_t dg = depth_map->data[m_idx+1];
            // uint8_t db = depth_map->data[m_idx+2];
            
            uint8_t pr = pose_map->data[m_idx];
            uint8_t pg = pose_map->data[m_idx+1];
            uint8_t pb = pose_map->data[m_idx+2];
            
            // Mix them for visualization: pose over depth
            if (pr > 0 || pg > 0 || pb > 0) {
                px[y * W + x] = 0xFF000000 | (pb << 16) | (pg << 8) | pr;
            } else {
                px[y * W + x] = 0xFF000000 | (dr << 16) | (dr << 8) | dr;
            }
        }
    }
}

static void application_resize_hook(void *data, int32_t width, int32_t height) {
    (void)data; (void)width; (void)height;
    g_dirty = true;
}


static void application_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time; 
    if (state == 0) return; // Release

    if (key == KEY_Q || key == KEY_ESC) {
        g_force_quit = true;
    }
    
    if (g_puppet_state) {
        if (key == KEY_EQUAL || key == KEY_KPPLUS) { // '+' key
            g_puppet_state->puppets[0].stuffing += 0.1f;
            printf("Stuffing increased: %f\n", g_puppet_state->puppets[0].stuffing);
        } else if (key == KEY_MINUS || key == KEY_KPMINUS) { // '-' key
            g_puppet_state->puppets[0].stuffing -= 0.1f;
            if (g_puppet_state->puppets[0].stuffing < 0.1f) g_puppet_state->puppets[0].stuffing = 0.1f;
            printf("Stuffing decreased: %f\n", g_puppet_state->puppets[0].stuffing);
        } else if (key == KEY_A) {
            g_puppet_state->puppets[0].arm_length += 0.1f;
            printf("Arm length: %f\n", g_puppet_state->puppets[0].arm_length);
        } else if (key == KEY_Z) {
            g_puppet_state->puppets[0].arm_length -= 0.1f;
            if (g_puppet_state->puppets[0].arm_length < 0.1f) g_puppet_state->puppets[0].arm_length = 0.1f;
            printf("Arm length: %f\n", g_puppet_state->puppets[0].arm_length);
        } else if (key == KEY_S) {
            g_puppet_state->puppets[0].leg_length += 0.1f;
            printf("Leg length: %f\n", g_puppet_state->puppets[0].leg_length);
        } else if (key == KEY_X) {
            g_puppet_state->puppets[0].leg_length -= 0.1f;
            if (g_puppet_state->puppets[0].leg_length < 0.1f) g_puppet_state->puppets[0].leg_length = 0.1f;
            printf("Leg length: %f\n", g_puppet_state->puppets[0].leg_length);
        } else if (key == KEY_D) {
            g_puppet_state->puppets[0].head_size += 0.1f;
            printf("Head size: %f\n", g_puppet_state->puppets[0].head_size);
        } else if (key == KEY_C) {
            g_puppet_state->puppets[0].head_size -= 0.1f;
            if (g_puppet_state->puppets[0].head_size < 0.1f) g_puppet_state->puppets[0].head_size = 0.1f;
            printf("Head size: %f\n", g_puppet_state->puppets[0].head_size);
        } else if (key == KEY_F) {
            g_puppet_state->puppets[0].ear_size += 0.1f;
            printf("Ear size: %f\n", g_puppet_state->puppets[0].ear_size);
        } else if (key == KEY_V) {
            g_puppet_state->puppets[0].ear_size -= 0.1f;
            if (g_puppet_state->puppets[0].ear_size < 0.1f) g_puppet_state->puppets[0].ear_size = 0.1f;
            printf("Ear size: %f\n", g_puppet_state->puppets[0].ear_size);
        }
    }
    g_dirty = true;
}



int main(int argc, char **argv) {
    (void)argc; (void)argv;
    signal(SIGINT, handle_sigint);
    
    
    
    
    
    // Vulkan UI might implicitly require the global font registry to exist.
    extern void tsfi_font_registry_init(void *reg);
    void *g_registry = lau_memalign_wired(512, 1024);
    tsfi_font_registry_init(g_registry);

    tsfi_input_init();



    
    
    
    g_puppet_state = tsfi_puppetry_shm_create();
    if (g_puppet_state) {
        memset(&g_puppet_state->puppets[0], 0, sizeof(TsfiPuppet));
        g_puppet_state->puppets[0].active = 1;
        g_puppet_state->puppets[0].animal_type = PUPPET_TYPE_TEDDY;
        g_puppet_state->puppets[0].scale = 1.0f;
        g_puppet_state->puppets[0].stuffing = 1.5f;
        g_puppet_state->puppets[0].arm_length = 1.0f;
        g_puppet_state->puppets[0].leg_length = 1.0f;
        g_puppet_state->puppets[0].head_size = 1.0f;
        g_puppet_state->puppets[0].ear_size = 1.0f;
        g_puppet_state->puppets[0].body.x = 64;
        g_puppet_state->puppets[0].body.y = 64;
    }

    
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true;
    xdg_toplevel_set_title(s->xdg_toplevel, "TSFi Teddy Bear Editor");

    tsfi_input_set_key_hook(application_key_hook);
    tsfi_input_set_resize_hook(application_resize_hook);

    
    tsfi_io_printf(stdout, "[TEDDY] Session Active.\n");
    const TsfiControlNetMap *shm_depth = tsfi_cn_shm_attach(TSFI_CN_SHM_DEPTH);
    const TsfiControlNetMap *shm_pose  = tsfi_cn_shm_attach(TSFI_CN_SHM_POSE);
    
    if (!shm_depth || !shm_pose) {
        tsfi_io_printf(stderr, "[FRACTURE] Could not attach to Teddy Skeleton SHM.\n");
    }


    uint32_t *left_buf = (uint32_t*)lau_malloc(128 * 128 * 4);
    uint32_t *right_buf = (uint32_t*)lau_malloc(128 * 128 * 4);
    
    while (s->running && !g_force_quit) {
        wl_display_dispatch_pending(s->display);
        
        
        static float time_t = 0.0f;
        char skel_cmd[256];
        snprintf(skel_cmd, sizeof(skel_cmd), "bin/tsfi_teddy_skeleton %f", time_t);
        int sys_res = system(skel_cmd);
        (void)sys_res;
        time_t += 0.1f;
        
        if (g_dirty || true) { // Always render for animation

            uint32_t *px = (uint32_t*)s->paint_buffer->data;
            int W = s->paint_buffer->width, H = s->paint_buffer->height;
            render_teddy(px, W, H, shm_depth, shm_pose);
            draw_frame(s);
            g_dirty = false;
        }
        usleep(16000);
    }

    lau_free(left_buf);
    lau_free(right_buf);
    
    if (shm_depth) tsfi_cn_shm_detach(shm_depth);
    if (shm_pose) tsfi_cn_shm_detach(shm_pose);
    destroy_vulkan_system(s);
    if (g_puppet_state) tsfi_puppetry_shm_close(g_puppet_state);

    return 0;
}