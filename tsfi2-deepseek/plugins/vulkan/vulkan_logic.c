#include "vulkan_logic.h"
#include "vulkan_input.h"
#include "tsfi_broadcaster.h"
#include "tsfi_staging.h"
#include "window_inc/vulkan_config.h"
#include "vulkan_main.h" 
#include "vulkan_render.h"
#include "lau_memory.h"
#include "tsfi_vision.h"
#include "tsfi_wire_firmware.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <time.h>

// --- Globals ---
static VulkanSystem *logic_sys = NULL;

void set_vulkan_system(VulkanSystem *s) {
    logic_sys = s;
}

VulkanSystem* get_vulkan_system(void) {
    return logic_sys;
}

// --- Logic Implementation ---
void vulkan_logic_epoch(int *ver) {
    if (!logic_sys || !logic_sys->running) { 
        if (ver) *ver += 1; 
        return; 
    }
    
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (fw) {
        if (logic_sys->counter) tsfi_wire_firmware_strobe(fw, 0x0D, (uint64_t)*logic_sys->counter);
        else if (ver) tsfi_wire_firmware_strobe(fw, 0x0D, (uint64_t)*ver);
        fw->rtl.session_id = 0;
        tsfi_wire_firmware_step_peripheral(fw, NULL);
    }
    
    if (logic_sys->display) {
        wl_display_dispatch_pending(logic_sys->display);
        wl_display_flush(logic_sys->display);
    }
    
    if (logic_sys->vk && (logic_sys->vk->swapchain || logic_sys->dma_image)) {
        draw_frame(logic_sys);
    } else if (logic_sys->vk && logic_sys->backend_type == VULKAN_BACKEND_HEADLESS) {
        // Headless Zhong Sync: Manual signal to progress timeline
        VulkanContext *vk = logic_sys->vk;
        vk->timelineValue++;
        VkTimelineSemaphoreSubmitInfo timelineInfo = {
            .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
            .signalSemaphoreValueCount = 1,
            .pSignalSemaphoreValues = &vk->timelineValue
        };
        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = &timelineInfo,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &vk->timelineSemaphore
        };
        vk->vkQueueSubmit(vk->queue, 1, &submitInfo, VK_NULL_HANDLE);
    }
    
    if (ver) *ver += 1;
}

bool vulkan_logic_state(void *obj) { 
    (void)obj;
    return (logic_sys && logic_sys->running); 
}

void vulkan_logic_directive(int *cnt, char *dir) { 
    (*cnt)++; 

    if (dir && strcmp(dir, "TIME") == 0) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        unsigned long long ns = (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
        printf("[TSFI_VULKAN] TIME: %llu\n", ns);
    }
    else if (dir && strcmp(dir, "TOGGLE_EDIT") == 0) {
        if (logic_sys) {
            keyboard_handle_key(logic_sys, NULL, 0, 0, KEY_PAUSE, 1);
        }
    }
    else if (dir && strcmp(dir, "TOGGLE_TEXT_DIRECTION") == 0) {
        if (logic_sys) {
            logic_sys->text_forward = !logic_sys->text_forward;
            printf("[TSFI_VULKAN] Text Direction: %s\n", logic_sys->text_forward ? "FORWARDS" : "BACKWARDS");
        }
    }
    else if (dir && strcmp(dir, "TOGGLE_TEXT_ALIGN") == 0) {
        if (logic_sys) {
            logic_sys->text_right_align = !logic_sys->text_right_align;
            printf("[TSFI_VULKAN] Text Align: %s\n", logic_sys->text_right_align ? "RIGHT" : "LEFT");
        }
    }
    else if (dir && strcmp(dir, "SET_ALIGN_LEFT") == 0) {
        if (logic_sys) { logic_sys->text_right_align = false; printf("[TSFI_VULKAN] Align: LEFT\n"); }
    }
    else if (dir && strcmp(dir, "SET_ALIGN_RIGHT") == 0) {
        if (logic_sys) { logic_sys->text_right_align = true; printf("[TSFI_VULKAN] Align: RIGHT\n"); }
    }
    else if (dir && strcmp(dir, "SET_FLOW_LTR") == 0) {
        if (logic_sys) { logic_sys->text_forward = true; printf("[TSFI_VULKAN] Flow: LTR\n"); }
    }
    else if (dir && strcmp(dir, "SET_FLOW_RTL") == 0) {
        if (logic_sys) { logic_sys->text_forward = false; printf("[TSFI_VULKAN] Flow: RTL\n"); }
    }
    else if (dir && strcmp(dir, "SCREENSHOT") == 0) {
        if (logic_sys && logic_sys->paint_buffer) {
            static int screenshot_count = 0;
            char filename[64];
            snprintf(filename, sizeof(filename), "manual_%d.ppm", screenshot_count++);
            draw_ui_elements(logic_sys);
            printf("[TSFI_VULKAN] Saving screenshot to %s...\n", filename);
            save_staging_buffer_to_ppm(logic_sys->paint_buffer, filename);
        }
    }
    else if (dir && strcmp(dir, "ANALYZE_VISUALS") == 0) {
        if (logic_sys && logic_sys->paint_buffer) {
            draw_ui_elements(logic_sys); // Ensure buffer is fresh
            
            uint32_t *data = (uint32_t*)logic_sys->paint_buffer->data;
            size_t size = logic_sys->paint_buffer->size;
            int width = (int)logic_sys->paint_buffer->width;
            
            // 1. Max Value (AVX-512)
            uint8_t max = tsfi_vision_max_value_raw((uint8_t*)data, size);
            
            // 2. OCR Segment & Identify Loop
            // BASE Area starts at x=15, y=105. 
            // We use the character cell sizes from vulkan_render.c: char_w=8, char_h=10
            int cell_w = 8, cell_h = 10;
            int BASE_x = 15, BASE_y = 105;
            
            printf("[VISION] Starting Character Identification (BASE: 160x60)...\n");
            
            FILE *f_met = fopen("tsfi_metrics.log", "a");
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            unsigned long long ns = (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
            if (f_met) fprintf(f_met, "%llu | OCR_START | MaxIntensity=%d\n", ns, max);

            // Probe a small subset of the BASE for performance (e.g. first 2 rows, first 20 columns)
            TSFiGlyphPhenotype p_template = { .target_coverage = 0.2f }; 
            int recognized_count = 0;

            for (int gy = 0; gy < 2; gy++) {
                for (int gx = 0; gx < 20; gx++) {
                    int cx = BASE_x + gx * cell_w;
                    int cy = BASE_y + gy * cell_h;
                    
                    // Extract Cell Pixels (Sub-buffer analysis)
                    // We can't easily pass sub-rects to analyze_glyph without a stride-aware version,
                    // so we copy to a temporary local buffer.
                    uint32_t cell_pixels[80]; // 8x10
                    bool has_content = false;
                    for (int j = 0; j < cell_h; j++) {
                        for (int i = 0; i < cell_w; i++) {
                            uint32_t px = data[(cy + j) * width + (cx + i)];
                            cell_pixels[j * cell_w + i] = px;
                            if (((px >> 16) & 0xFF) > 50) has_content = true;
                        }
                    }

                    if (has_content) {
                        TSFiResonanceAnalysis res;
                        // Use a larger virtual box for analysis to let the algorithm "see" the glyph context
                        // but analyze_glyph expects minimum 64x32. We must pad or use a different probe.
                        // For now, we use the classification logic on the cell directly.
                        
                        // Hack: Pad to 64x32 for analyze_glyph compatibility
                        uint32_t padded[64 * 32];
                        memset(padded, 0, sizeof(padded));
                        for(int j=0; j<cell_h; j++) memcpy(&padded[(j+11)*64 + 28], &cell_pixels[j*cell_w], cell_w*4);

                        tsfi_vision_analyze_glyph(padded, 64, 32, &p_template, &res);
                        TSFiClassification cls = tsfi_vision_classify(&res);
                        
                        if (cls.confidence > 0.6f) {
                            if (f_met) fprintf(f_met, "%llu | OCR_HIT | Pos=%d,%d Class=%d Conf=%.2f\n", ns, gx, gy, cls.class_id, cls.confidence);
                            recognized_count++;
                        }
                    }
                }
            }

            printf("[VISION] OCR Complete. Identified %d glyph candidates. Results in tsfi_metrics.log\n", recognized_count);
            if (f_met) {
                fprintf(f_met, "%llu | OCR_END | Recognized=%d\n", ns, recognized_count);
                fclose(f_met);
            }
        }
    }
    else if (dir && strcmp(dir, "DEBUG_PROBE_BROADCASTER") == 0) {
        extern LauBroadcaster* get_active_broadcaster(void);
        LauBroadcaster *lb = get_active_broadcaster();
        if (lb) {
            printf("[DEBUG] Broadcaster Found: Key=%s, Cursor=%d,%d, Active=%d\n", 
                   lb->stream_key, lb->cursor_x, lb->cursor_y, lb->stream_active);
        } else {
            printf("[DEBUG] Broadcaster NOT FOUND in global registry.\n");
        }
    }
    else if (dir && strncmp(dir, "SET_TITLE ", 10) == 0) {
        if (logic_sys && logic_sys->xdg_toplevel) {
            const char *title = dir + 10;
            xdg_toplevel_set_title(logic_sys->xdg_toplevel, title);
            printf("[TSFI_VULKAN] Setting Title: %s\n", title); fflush(stdout);
        }
    }
    else if (dir && strcmp(dir, "TOGGLE_FULLSCREEN") == 0) {
        if (logic_sys && logic_sys->xdg_toplevel) {
            if (logic_sys->fullscreen) {
                xdg_toplevel_unset_fullscreen(logic_sys->xdg_toplevel);
                logic_sys->fullscreen = false;
                printf("[TSFI_VULKAN] Toggling Fullscreen: OFF\n"); fflush(stdout);
            } else {
                xdg_toplevel_set_fullscreen(logic_sys->xdg_toplevel, NULL);
                logic_sys->fullscreen = true;
                printf("[TSFI_VULKAN] Toggling Fullscreen: ON\n"); fflush(stdout);
            }
        }
    }
    else if (dir && strcmp(dir, "TOGGLE_MAXIMIZED") == 0) {
        if (logic_sys && logic_sys->xdg_toplevel) {
            if (logic_sys->maximized) {
                xdg_toplevel_unset_maximized(logic_sys->xdg_toplevel);
                logic_sys->maximized = false;
                printf("[TSFI_VULKAN] Toggling Maximized: OFF\n"); fflush(stdout);
            } else {
                xdg_toplevel_set_maximized(logic_sys->xdg_toplevel);
                logic_sys->maximized = true;
                printf("[TSFI_VULKAN] Toggling Maximized: ON\n"); fflush(stdout);
            }
        }
    }
    else if (dir && strcmp(dir, "RESIZE") == 0) {
        if (logic_sys) {
            printf("[TSFI_VULKAN] Simulating Resize to 1024x768...\n");
            logic_sys->width = 1024;
            logic_sys->height = 768;
            recreate_swapchain(logic_sys);
        }
    }
    else if (dir && strncmp(dir, "TEST_INPUT ", 11) == 0) {
        if (logic_sys) {
            int key_code = atoi(dir + 11);
            printf("[TSFI_VULKAN] Injecting Test Input: Key %d\n", key_code);
            keyboard_handle_key(logic_sys, NULL, 0, 0, (uint32_t)key_code, 1);
            keyboard_handle_key(logic_sys, NULL, 0, 0, (uint32_t)key_code, 0);
        }
    }
    else if (dir && strncmp(dir, "TEST_MOUSE_MOVE ", 16) == 0) {
        if (logic_sys) {
            int x, y;
            if (sscanf(dir + 16, "%d %d", &x, &y) == 2) {
                logic_sys->mouse_x = x;
                logic_sys->mouse_y = y;
                printf("[TSFI_VULKAN] Injecting Mouse Move: %d, %d\n", x, y);
            }
        }
    }
    else if (dir && strcmp(dir, "TEST_MOUSE_CLICK") == 0) {
        if (logic_sys) {
            logic_sys->mouse_down = true;
            printf("[TSFI_VULKAN] Injecting Mouse Click (Down)\n");
        }
    }
    else if (dir && strcmp(dir, "TEST_MOUSE_RELEASE") == 0) {
        if (logic_sys) {
            logic_sys->mouse_down = false;
            printf("[TSFI_VULKAN] Injecting Mouse Click (Up)\n");
        }
    }
    else {
        printf("[TSFI_VULKAN] Unknown Directive: '%s' (Len: %zu)\n", dir ? dir : "NULL", dir ? strlen(dir) : 0);
        if (dir) {
            printf("Hex:");
            for (size_t i = 0; i < strlen(dir); i++) printf(" %02X", (unsigned char)dir[i]);
            printf("\n");
        }
    }
}

void vulkan_logic_scramble(void *ws_ptr) { 
    (void)ws_ptr; 
    if(logic_sys && logic_sys->paint_buffer) {
        uint32_t *px = (uint32_t*)logic_sys->paint_buffer->data;
        size_t count = logic_sys->paint_buffer->width * logic_sys->paint_buffer->height;
        for(size_t i=0; i<count; i+=100) { 
            px[i] ^= 0xFFFFFFFF;
        }
        printf("[TSFI_VULKAN] Scrambled Paint Buffer.\n");
    }
}

void vulkan_logic_provenance(void *ws_ptr) { 
    (void)ws_ptr; 
}
