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
#include "lau_yul_thunk.h"
#include "tsfi_pulsechain_rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <time.h>
#include <math.h>

static char g_graphics_address[43] = "0xfeF51EAFacaB42889221438E87E228aABBd9E7d8"; // Default deployed graphicsSystem
static char g_music_address[43]    = "0x8F1ACB68fF904244322E5ecCa636F8Db4C635Db6";
static char g_disk_address[43]     = "0xa5Ae84ddeE04baEF9b3BB98B03Cc033F68c891a8";
static char g_cpu_address[43]      = "0x0161B2e020d8798a51f9Bce81B7d6C346414E551";
static char g_oracle_address[43]   = "0xf7C8a03878Dbf92566a0BA534e12441e0de81160";
static char g_caller_address[43]   = "0x70997970C51812dc3A010C7d01b50e0d17dc79C8";

static void load_dynamic_config_addresses(void) {
    FILE *f = fopen("../config/user_config.json", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *ptr;
        if ((ptr = strstr(line, "\"graphicsSystemAddress\":"))) {
            char *addr_start = strstr(ptr, "0x");
            if (addr_start && strlen(addr_start) >= 42) {
                strncpy(g_graphics_address, addr_start, 42);
                g_graphics_address[42] = '\0';
            }
        }
        else if ((ptr = strstr(line, "\"musicMakerAddress\":"))) {
            char *addr_start = strstr(ptr, "0x");
            if (addr_start && strlen(addr_start) >= 42) {
                strncpy(g_music_address, addr_start, 42);
                g_music_address[42] = '\0';
            }
        }
        else if ((ptr = strstr(line, "\"diskSystemAddress\":"))) {
            char *addr_start = strstr(ptr, "0x");
            if (addr_start && strlen(addr_start) >= 42) {
                strncpy(g_disk_address, addr_start, 42);
                g_disk_address[42] = '\0';
            }
        }
        else if ((ptr = strstr(line, "\"cpu6502Address\":"))) {
            char *addr_start = strstr(ptr, "0x");
            if (addr_start && strlen(addr_start) >= 42) {
                strncpy(g_cpu_address, addr_start, 42);
                g_cpu_address[42] = '\0';
            }
        }
        else if ((ptr = strstr(line, "\"acousticOracleAddress\":"))) {
            char *addr_start = strstr(ptr, "0x");
            if (addr_start && strlen(addr_start) >= 42) {
                strncpy(g_oracle_address, addr_start, 42);
                g_oracle_address[42] = '\0';
            }
        }
        else if ((ptr = strstr(line, "\"testCallerAddress\":"))) {
            char *addr_start = strstr(ptr, "0x");
            if (addr_start && strlen(addr_start) >= 42) {
                strncpy(g_caller_address, addr_start, 42);
                g_caller_address[42] = '\0';
            }
        }
    }
    fclose(f);
    fprintf(stderr, "[TSFI_VULKAN] Loaded EVM Contract Addresses:\n");
    fprintf(stderr, "  -> Graphics: %s\n", g_graphics_address);
    fprintf(stderr, "  -> Music:    %s\n", g_music_address);
    fprintf(stderr, "  -> Disk:     %s\n", g_disk_address);
    fprintf(stderr, "  -> CPU:      %s\n", g_cpu_address);
    fprintf(stderr, "  -> Oracle:   %s\n", g_oracle_address);
    fprintf(stderr, "  -> Caller:   %s\n", g_caller_address);
}

static uint64_t get_register_val(const char *contract_addr, uint64_t register_addr) {
    if (contract_addr && g_cpu_address[0] != '\0' && strcmp(contract_addr, g_cpu_address) == 0) {
        char data_hex[128];
        snprintf(data_hex, sizeof(data_hex), "0x7861d269%064lx", register_addr);
        char out_hex[512];
        const char *from_address = g_caller_address;
        if (tsfi_pulse_rpc_call_from(contract_addr, from_address, data_hex, out_hex, sizeof(out_hex))) {
            return strtoull(out_hex, NULL, 16);
        }
    } else {
        char slot_hex[64];
        snprintf(slot_hex, sizeof(slot_hex), "0x%lx", register_addr);
        char out_hex[128];
        if (tsfi_pulse_rpc_get_storage_at(contract_addr, slot_hex, out_hex, sizeof(out_hex))) {
            return strtoull(out_hex, NULL, 16);
        }
    }
    // Fallback to internal Yul thunk memory
    return lau_yul_thunk_sload(register_addr);
}

// --- Globals ---
static VulkanSystem *logic_sys = NULL;

void set_vulkan_system(VulkanSystem *s) {
    logic_sys = s;
}

VulkanSystem* get_vulkan_system(void) {
    return logic_sys;
}

// --- Logic Implementation ---
static void update_and_render_datamost_display(VulkanSystem *s) {
    if (!s || !s->paint_buffer) return;
    
    fprintf(stderr, "[DEBUG_VULKAN] update_and_render_datamost_display active! Paint buffer width: %zu\n", s->paint_buffer->width);
    fflush(stderr);

    static bool yul_thunk_initialized = false;
    if (!yul_thunk_initialized) {
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
        load_dynamic_config_addresses();
        lau_yul_thunk_init("graphicsSystem", "../solidity/bin/graphicsSystem.yul", 0x2);
        lau_yul_thunk_init("musicMaker", "../solidity/bin/musicMaker.yul", 0x3);
        lau_yul_thunk_init("diskSystem", "../solidity/bin/diskSystem.yul", 0x4);
        yul_thunk_initialized = true;

        // Initialize Sprite positions in local EVM storage
        for (int i = 0; i < 8; i++) {
            uint64_t x = 40 + i * 30;
            uint64_t y = 50 + (i % 2) * 40;
            lau_yul_thunk_sstore(53248 + i * 2, x);
            lau_yul_thunk_sstore(53249 + i * 2, y);
        }
        // Enable sprites
        lau_yul_thunk_sstore(53269, 255);
    }

    // DeepSeek Physical Filter consensus check: query the latestRoot slot 0 from the acousticOracle contract
    static int tick = 0;
    uint64_t latest_root = get_register_val(g_oracle_address, 0);
    bool deepseek_consensus = (latest_root != 0);
    
    const char *addr_src = (g_cpu_address[0] != '\0') ? g_cpu_address : g_graphics_address;

    // Fallback to progress physics tick even if consensus is metastable
    if (deepseek_consensus || true) {
        tick++;
        // Sync coordinates from the actual CPU/Graphics on-chain state to local Yul thunk for collision check
        for (int i = 0; i < 8; i++) {
            uint64_t x = get_register_val(addr_src, 53248 + i * 2);
            uint64_t y = get_register_val(addr_src, 53249 + i * 2);

            lau_yul_thunk_sstore(53248 + i * 2, x);
            lau_yul_thunk_sstore(53249 + i * 2, y);
        }
    }

    // Execute Yul Thunk: checkCollisions()
    uint8_t calldata[4] = { 0x5a, 0x18, 0xa9, 0x94 };
    uint8_t retval[32];
    size_t retval_len = 32;
    lau_yul_thunk_execute("graphicsSystem", calldata, 4, retval, &retval_len);

    uint64_t collisionMask = get_register_val(g_graphics_address, 53278);

    StagingBuffer *sb = s->paint_buffer;
    staging_clear_avx512(sb, 0xFF001A33); // Retro dark blue backdrop

    // Draw grid
    for (int y = 0; y < (int)sb->height; y += 20) {
        for (int x = 0; x < (int)sb->width; x += 20) {
            uint32_t *px = (uint32_t*)sb->data;
            px[y * sb->width + x] = 0xFF003366;
        }
    }

    // Draw active playfield header
    draw_debug_text(sb, 10, 8, "--- DATAMOST COOPERATIVE RETRO-CONSOLE ---", 0xFFFFFF00, true);

    // Draw DeepSeek Physical Filter / Alignment Status Bar
    draw_rounded_rect(sb, 10, 20, 300, 15, 2, deepseek_consensus ? 0xFF00FF00 : 0xFFFF00FF);
    if (deepseek_consensus) {
        draw_debug_text(sb, 15, 23, "DEEPSEEK ALIGNMENT: CONSENSUS STABLE (1,1)", 0xFF00FF00, true);
    } else {
        draw_debug_text(sb, 15, 23, "DEEPSEEK ALIGNMENT: METASTABLE STUTTER (0,0)", 0xFFFF00FF, true);
    }

    // Define standard retro fallback patterns (Bunny, Carrot, Space Invader)
    static const uint32_t bunny_pattern[21] = {
        0x000c00, 0x000c00, 0x001e00, 0x001e00, 0x003300, 0x003300, 0x007f80, 0x00ffff,
        0x01ffe0, 0x03fff0, 0x07ffc0, 0x07ffc0, 0x07ffc0, 0x03ffc0, 0x01ff80, 0x00ff80,
        0x007e00, 0x003c00, 0x001800, 0x001800, 0x000000
    };
    static const uint32_t carrot_pattern[21] = {
        0x001800, 0x003c00, 0x003c00, 0x003c00, 0x003c00, 0x001800, 0x001800, 0x001800,
        0x001800, 0x001800, 0x001800, 0x001800, 0x001800, 0x001800, 0x001800, 0x001800,
        0x001800, 0x001800, 0x001800, 0x000000, 0x000000
    };
    static const uint32_t invader_pattern[21] = {
        0x0003c0, 0x0007e0, 0x00ffff, 0x03dbf8, 0x03ffff, 0x00a500, 0x005a00, 0x008100,
        0x00c300, 0x018180, 0x0300c0, 0x0300c0, 0x020040, 0x000000, 0, 0, 0, 0, 0, 0, 0
    };

    // Draw Sprites
    for (int i = 0; i < 8; i++) {
        uint64_t x = get_register_val(addr_src, 53248 + i * 2);
        uint64_t y = get_register_val(addr_src, 53249 + i * 2);
        
        bool colliding = (collisionMask & (1 << i)) != 0;
        uint32_t color = colliding ? 0xFFFF0000 : 0xFF00FF00;

        // Draw Sprite using custom 24x21 bitmap if present
        bool has_pattern = false;
        uint32_t pattern[21];
        for (int r = 0; r < 21; r++) {
            pattern[r] = (uint32_t)get_register_val(g_graphics_address, 54000 + i * 32 + r);
            if (pattern[r] != 0) {
                has_pattern = true;
            }
        }

        const uint32_t *pat = NULL;
        if (has_pattern) {
            pat = pattern;
        } else {
            if (i == 0) pat = bunny_pattern;
            else if (i == 1) pat = carrot_pattern;
            else if (i == 2) pat = invader_pattern;
        }

        if (pat) {
            // Draw 24x21 custom pixel pattern
            for (int r = 0; r < 21; r++) {
                uint32_t row_val = pat[r];
                for (int c = 0; c < 24; c++) {
                    if ((row_val & (1 << (23 - c))) != 0) {
                        int py = y + r;
                        int px = x + c;
                        if (px >= 0 && px < (int)sb->width && py >= 0 && py < (int)sb->height) {
                            uint32_t *pixels = (uint32_t *)sb->data;
                            pixels[py * sb->width + px] = color;
                        }
                    }
                }
            }
        } else {
            // Fallback to default block
            draw_rounded_rect(sb, x, y, 24, 21, 4, color);
        }

        char lbl[4];
        snprintf(lbl, sizeof(lbl), "S%d", i);
        draw_debug_text(sb, x + 4, y + 5, lbl, 0xFFFFFFFF, true);
    }

    // Synthesize SID Audio Oscilloscope
    uint8_t freq_calldata[4] = { 0x11, 0x2d, 0xf4, 0x9e };
    uint8_t freq_retval[32];
    size_t freq_retval_len = 32;
    uint16_t voice_freq = 220;
    if (lau_yul_thunk_execute("musicMaker", freq_calldata, 4, freq_retval, &freq_retval_len)) {
        voice_freq = (freq_retval[30] << 8) | freq_retval[31];
    }
    if (voice_freq == 0) voice_freq = 220;

    draw_rounded_rect(sb, 10, 140, 150, 50, 2, 0xFF00FF00);
    draw_debug_text(sb, 15, 145, "SID OSCILLOSCOPE", 0xFF00FF00, true);
    for (int i = 0; i < 140; i++) {
        float t = (float)i / 140.0f;
        int y_val = 165 + (int)(15.0f * sinf(2.0f * 3.14159f * (voice_freq / 100.0f) * t + tick * 0.1f));
        if (y_val >= 142 && y_val <= 188) {
            uint32_t *px = (uint32_t*)sb->data;
            px[y_val * sb->width + (15 + i)] = 0xFF00FF00;
        }
    }

    // Fetch Jiffies from Disk System Yul Thunk
    uint8_t disk_calldata[4] = { 0xb5, 0x12, 0x3d, 0x47 };
    uint8_t disk_retval[32];
    size_t disk_retval_len = 32;
    uint64_t jiffies = 0;
    if (lau_yul_thunk_execute("diskSystem", disk_calldata, 4, disk_retval, &disk_retval_len)) {
        jiffies = ((uint64_t)disk_retval[24] << 56) |
                  ((uint64_t)disk_retval[25] << 48) |
                  ((uint64_t)disk_retval[26] << 40) |
                  ((uint64_t)disk_retval[27] << 32) |
                  ((uint64_t)disk_retval[28] << 24) |
                  ((uint64_t)disk_retval[29] << 16) |
                  ((uint64_t)disk_retval[30] << 8)  |
                  (uint64_t)disk_retval[31];
    }

    // CPU State Monitor & Disk Jiffies
    draw_rounded_rect(sb, 170, 140, 140, 50, 2, 0xFFFFFFFF);
    draw_debug_text(sb, 175, 145, "CPU 6502 STATUS", 0xFFFFFF00, true);
    
    // Fetch actual register states from EVM CPU contract storage
    uint8_t a_reg  = (uint8_t)get_register_val(g_cpu_address, 0x80);
    uint8_t x_reg  = (uint8_t)get_register_val(g_cpu_address, 0x81);
    uint8_t y_reg  = (uint8_t)get_register_val(g_cpu_address, 0x82);
    uint8_t sp_reg = (uint8_t)get_register_val(g_cpu_address, 0x83);
    uint8_t sr_reg = (uint8_t)get_register_val(g_cpu_address, 0x84);
    uint16_t pc_reg = (uint16_t)get_register_val(g_cpu_address, 0x85);

    char reg_str[64];
    snprintf(reg_str, sizeof(reg_str), "A:%02X X:%02X Y:%02X PC:%04X", a_reg, x_reg, y_reg, pc_reg);
    draw_debug_text(sb, 172, 158, reg_str, 0xFFFFFFFF, true);
    snprintf(reg_str, sizeof(reg_str), "SR:%02X SP:%02X JIFS:%lu", sr_reg, sp_reg, (unsigned long)jiffies);
    draw_debug_text(sb, 172, 170, reg_str, 0xFFFFFFFF, true);
}

void vulkan_logic_epoch(int *ver) {
    if (!logic_sys || !logic_sys->running) { 
        if (ver) *ver += 1; 
        return; 
    }

    // Run Yul Thunks and draw Datamost simulator output to framebuffer
    update_and_render_datamost_display(logic_sys);

    // Export live frame to frontend dashboard periodically
    static int frame_export_counter = 0;
    if (frame_export_counter++ % 15 == 0) {
        if (logic_sys->paint_buffer) {
            save_staging_buffer_to_ppm(logic_sys->paint_buffer, "../frontend/vulkan_frame.ppm");
        }
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
    (void)cnt; 

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
