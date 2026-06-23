#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <poll.h>
#include <jpeglib.h>
#include <setjmp.h>

#include "node_interop.h"
#include <pthread.h>
#include "tsfi_dom.h"
#include "tsfi_layout.h"
#include "tsfi_paint.h"
#include "tsfi_vision.h"
#include "tsfi_jpeg_encoder.h"

#include <arpa/inet.h>
#include <netinet/in.h>



#include "lau_memory.h"
#include "tsfi_raw.h"
#include "window_inc/vulkan_struct.h"
#include "tsfi_zmm_vm.h"
#include "../src/lau_yul_thunk.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);
extern void tsfi_wire_firmware_init(void);
void tsfi_dl_thunks_teardown(void);
void lau_registry_teardown(void);
void lau_free_all_active(void);

extern bool tsfi_mozilla_wmq_bridge_init(const char *so_path);
extern void tsfi_mozilla_wmq_bridge_tick(TsfiZmmVmState *vm_state);
extern void tsfi_mozilla_wmq_bridge_destroy(void);

typedef int (VKAPI_CALL *PFN_tsfi_drmModeAddPlane)(uint32_t plane_id, size_t buffer_size);
typedef void* (VKAPI_CALL *PFN_tsfi_drmModeGetVirtualPlaneBuffer)(uint32_t plane_id);
typedef void (VKAPI_CALL *PFN_tsfi_zmm_set_scanout_buffer)(void* ptr, int w, int h);
typedef void (VKAPI_CALL *PFN_tsfi_drmModeFreeVirtualPlanes)(void);

#include "rooted_wayland_events.h"

extern const uint8_t *g_active_video_frame;
extern int g_active_video_w;
extern int g_active_video_h;

#include "tsfi_media_decode.h"
TsfiMediaDecoder *g_video_decoder = NULL;
uint8_t *g_video_rgba_buf = NULL;
int g_video_w = 0;
int g_video_h = 0;

TsfiZmmVmState g_zmm_vm;
pthread_mutex_t g_zmm_vm_mutex = PTHREAD_MUTEX_INITIALIZER;


static void fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

#define MAX_W 3840
#define MAX_H 2160


static int g_main_img_w = 1024;
static int g_main_img_h = 768;

struct presenter_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

static void presenter_error_exit(j_common_ptr cinfo) {
    struct presenter_error_mgr *myerr = (struct presenter_error_mgr *)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

bool decode_jpeg_full(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
    struct jpeg_decompress_struct cinfo;
    struct presenter_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = presenter_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, jpeg_buf, jpeg_sz);
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    cinfo.out_color_space = JCS_RGB;
    if (!jpeg_start_decompress(&cinfo)) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    int row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    while (cinfo.output_scanline < cinfo.output_height) {
        int y = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, buffer, 1);
        if (y >= height) continue;
        uint8_t *src = buffer[0];
        uint32_t *dst = scanout_px + y * width;
        for (int x = 0; x < width && x < (int)cinfo.output_width; x++) {
            uint8_t r = src[x * 3];
            uint8_t g = src[x * 3 + 1];
            uint8_t b = src[x * 3 + 2];
            dst[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return true;
}

static int g_dest_x = 410;
static int g_dest_y = 80;
static int g_dest_w = 0;
static int g_dest_h = 0;

bool decode_jpeg_subrect(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
    (void)jpeg_buf; (void)jpeg_sz; (void)scanout_px; (void)width; (void)height;
    return false;
}

#include <sys/stat.h>

void draw_youtube_frame(uint32_t *scanout_px, int width, int height, bool new_youtube) {
    (void)new_youtube;
    (void)scanout_px; (void)width; (void)height;
    (void)g_main_img_w; (void)g_main_img_h;
}

bool decode_jpeg_zero_copy(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
    (void)jpeg_buf; (void)jpeg_sz; (void)scanout_px; (void)width; (void)height;
    return false;
}

void commit_wayland_surface(int fd_wl, uint32_t surf, uint32_t bid) {
    uint32_t attach_args[] = {bid, 0, 0};
    send_msg(fd_wl, surf, WL_SURFACE_ATTACH, attach_args, 12, -1);
    uint32_t damage[] = {0, 0, g_w, g_h};
    send_msg(fd_wl, surf, WL_SURFACE_DAMAGE, damage, 16, -1);
    send_msg(fd_wl, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
}

void process_logs(uint32_t *scanout_px, int w, int h) {
    int ui_logs = lau_yul_thunk_get_log_count();
    for (int k = 0; k < ui_logs; k++) {
        uint64_t log_addr = 0;
        int num_topics = 0;
        u256_t topics[4] = {0};
        uint8_t log_data[2048] = {0};
        size_t log_data_size = sizeof(log_data);
        if (lau_yul_thunk_get_log(k, &log_addr, &num_topics, topics, log_data, &log_data_size)) {
            if (num_topics > 0 && topics[0].d[3] == 0x9e7a02c89e7a02c8ULL && topics[0].d[2] == 0x9e7a02c89e7a02c8ULL) {
                uint16_t rx = (log_data[30] << 8) | log_data[31];
                uint16_t ry = (log_data[62] << 8) | log_data[63];
                uint16_t rw = (log_data[94] << 8) | log_data[95];
                uint16_t rh = (log_data[126] << 8) | log_data[127];
                uint32_t rcol = (log_data[156] << 24) | (log_data[157] << 16) | (log_data[158] << 8) | log_data[159];
                if ((rcol & 0xFF000000) == 0) rcol |= 0xFF000000;

                // Scale VM coordinate bounds to headed window dimensions
                float scale_x = (float)w / 320.0f;
                float scale_y = (float)h / 300.0f;
                int start_x = (int)(rx * scale_x);
                int start_y = (int)(ry * scale_y);
                int rect_w = (int)(rw * scale_x);
                int rect_h = (int)(rh * scale_y);

                for (int y = start_y; y < start_y + rect_h && y < h; y++) {
                    for (int x = start_x; x < start_x + rect_w && x < w; x++) {
                        scanout_px[y * w + x] = rcol;
                    }
                }
            }
        }
    }
}
void run_rooted_zmm_tick() {
    if (!g_scanout_px) return;
    
    char cmd[1024];
    char hex_mx[65], hex_my[65], hex_click[65];
    
    // Scale mouse coordinates from screen coordinates back to VM space
    uint16_t vm_mx = (uint16_t)((float)g_mouse_x * 320.0f / (float)g_w);
    uint16_t vm_my = (uint16_t)((float)g_mouse_y * 300.0f / (float)g_h);
    
    fill_hex_uint256(hex_mx, vm_mx);
    fill_hex_uint256(hex_my, vm_my);
    fill_hex_uint256(hex_click, g_click_state);

    snprintf(cmd, sizeof(cmd), "YULEXEC \"MicroUI\", \"8b5c90d2%s%s%s\"", hex_mx, hex_my, hex_click);
    tsfi_zmm_vm_exec(&g_zmm_vm, cmd);
}

TsfiNode *g_dashboard_dom = NULL;
static TsfiLayoutBox *g_dashboard_layout = NULL;

static void render_native_dashboard(uint32_t *pixels, int w, int h) {
    static int g_demo_tick = 0;
    g_demo_tick = (g_demo_tick + 1) % 2000;
    
    char *html_buf = NULL;
    int unused_ret;
    (void)unused_ret;
    
    if (g_demo_tick < 500) {
        // STATE_TYPING (0 to 499 ticks)
        const char *query = "";
        if (g_demo_tick >= 360) query = "atropa";
        else if (g_demo_tick >= 300) query = "atrop";
        else if (g_demo_tick >= 240) query = "atro";
        else if (g_demo_tick >= 180) query = "atr";
        else if (g_demo_tick >= 120) query = "at";
        else if (g_demo_tick >= 60) query = "a";
        
        unused_ret = asprintf(&html_buf, 
            "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
            "  <div style=\"background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f;\">\n"
            "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold;\">YouTube</span>\n"
            "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">%s</span>\n"
            "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px;\">Search</span>\n"
            "  </div>\n"
            "  <div style=\"width: 980px; height: 640px;\">\n"
            "    <video src=\"segment_0.mp4\" style=\"width: 640px; height: 360px; margin-bottom: 10px;\"></video>\n"
            "    <div style=\"font-size: 22px; font-weight: bold; margin-bottom: 10px; color: #ffffff;\">Atropa Pulsechain: The Helmholtz Web Launch</div>\n"
            "    <div style=\"color: #aaaaaa; font-size: 14px;\">12K views &bull; Oct 15, 2026</div>\n"
            "  </div>\n"
            "</div>", query);
    } else if (g_demo_tick < 1000) {
        // STATE_RESULTS (500 to 999 ticks)
        unused_ret = asprintf(&html_buf,
            "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
            "  <div style=\"background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f;\">\n"
            "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold;\">YouTube</span>\n"
            "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">atropa</span>\n"
            "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px;\">Search</span>\n"
            "  </div>\n"
            "  <div style=\"width: 980px; height: 40px; margin-bottom: 15px;\">\n"
            "    <span style=\"background-color: #303030; width: 100px; height: 30px; color: #ffffff; padding: 5px; font-size: 14px;\">Filters</span>\n"
            "    <span style=\"color: #aaaaaa; font-size: 14px; margin-left: 15px;\">About 4,210 results for \"atropa\"</span>\n"
            "  </div>\n"
            "  <div style=\"width: 980px; height: 580px;\">\n"
            "    <div style=\"width: 950px; height: 120px; margin-bottom: 20px; background-color: #111111;\">\n"
            "      <div style=\"font-size: 20px; font-weight: bold; color: #ffffff;\">Atropa Pulsechain: Complete Auncient Guide (2026)</div>\n"
            "      <div style=\"color: #aaaaaa; font-size: 13px; margin-top: 5px;\">Auncient Tech &bull; 85K views &bull; 3 months ago</div>\n"
            "    </div>\n"
            "    <div style=\"width: 950px; height: 120px; margin-bottom: 20px; background-color: #111111;\">\n"
            "      <div style=\"font-size: 20px; font-weight: bold; color: #ffffff;\">Atropa DNA Tracker Tutorial</div>\n"
            "      <div style=\"color: #aaaaaa; font-size: 13px; margin-top: 5px;\">Auncient Tech &bull; 12K views &bull; 2 weeks ago</div>\n"
            "    </div>\n"
            "  </div>\n"
            "</div>");
    } else if (g_demo_tick < 1500) {
        // STATE_FILTER_SHOWN (1000 to 1499 ticks)
        unused_ret = asprintf(&html_buf,
            "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
            "  <div style=\"background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f;\">\n"
            "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold;\">YouTube</span>\n"
            "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">atropa</span>\n"
            "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px;\">Search</span>\n"
            "  </div>\n"
            "  <div style=\"width: 980px; height: 140px; background-color: #161616; padding: 15px; margin-bottom: 15px; border: 1px solid #3f3f3f;\">\n"
            "    <div style=\"font-weight: bold; font-size: 14px; margin-bottom: 10px; color: #aaaaaa;\">UPLOAD DATE</div>\n"
            "    <div style=\"font-size: 14px; margin-bottom: 8px; color: #ffffff;\">Today</div>\n"
            "    <div style=\"font-size: 14px; margin-bottom: 8px; color: #ff0000; font-weight: bold;\">This week &larr;</div>\n"
            "  </div>\n"
            "</div>");
    } else {
        // STATE_FILTERED_RESULTS (1500 to 1999 ticks)
        unused_ret = asprintf(&html_buf,
            "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
            "  <div style=\"background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f;\">\n"
            "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold;\">YouTube</span>\n"
            "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">atropa</span>\n"
            "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px;\">Search</span>\n"
            "  </div>\n"
            "  <div style=\"width: 980px; height: 40px; margin-bottom: 15px;\">\n"
            "    <span style=\"background-color: #ff0000; width: 150px; height: 30px; color: #ffffff; padding: 5px; font-size: 14px; font-weight: bold;\">Filter: This Week</span>\n"
            "    <span style=\"color: #aaaaaa; font-size: 14px; margin-left: 15px;\">124 results found this week for \"atropa\"</span>\n"
            "  </div>\n"
            "  <div style=\"width: 980px; height: 580px;\">\n"
            "    <div style=\"display: flex; width: 950px; height: 120px; margin-bottom: 20px; background-color: #111111;\">\n"
            "      <video src=\"segment_0.mp4\" style=\"width: 200px; height: 110px;\"></video>\n"
            "      <div style=\"display: block; margin-left: 20px; width: 700px;\">\n"
            "        <div style=\"font-size: 20px; font-weight: bold; color: #ff0000;\">Atropa Pulsechain: New Epoch Launches This Week!</div>\n"
            "        <div style=\"color: #aaaaaa; font-size: 13px; margin-top: 5px;\">Auncient Tech &bull; 3.2K views &bull; 2 days ago &bull; NEW</div>\n"
            "      </div>\n"
            "    </div>\n"
            "    <div style=\"display: flex; width: 950px; height: 120px; margin-bottom: 20px; background-color: #111111;\">\n"
            "      <div style=\"background-color: #333333; width: 200px; height: 110px;\"></div>\n"
            "      <div style=\"display: block; margin-left: 20px; width: 700px;\">\n"
            "        <div style=\"font-size: 20px; font-weight: bold; color: #ffffff;\">Pulsechain: Atropa's Helmholtz Resonance this week</div>\n"
            "        <div style=\"color: #aaaaaa; font-size: 13px; margin-top: 5px;\">Helmholtz Spec &bull; 850 views &bull; 5 days ago &bull; NEW</div>\n"
            "      </div>\n"
            "    </div>\n"
            "  </div>\n"
            "</div>");
    }

    if (g_dashboard_layout) tsfi_layout_box_free(g_dashboard_layout);
    if (g_dashboard_dom) tsfi_node_free(g_dashboard_dom);
    g_dashboard_dom = tsfi_dom_parse_html(html_buf);
    g_dashboard_layout = tsfi_layout_tree_build(g_dashboard_dom);
    free(html_buf);

    if (g_dashboard_layout) {
        tsfi_layout_solve(g_dashboard_layout, 0.0f, 0.0f, (float)w, (float)h);
        tsfi_paint_tree(g_dashboard_layout, pixels, w, h);
    } else {
        for (int i = 0; i < w * h; i++) {
            pixels[i] = 0xFF0B0214;
        }
    }
}

bool update_and_present(int fd_wl, uint32_t surf, uint32_t bid, bool force_redraw) {
    (void)force_redraw;
    if (g_scanout_px) {
        if (is_projector) {
            if (g_dest_w > 0 && g_dest_h > 0) {
                draw_youtube_frame(g_scanout_px, g_w, g_h, false);
            } else {
                for (int i = 0; i < g_w * g_h; i++) {
                    g_scanout_px[i] = 0xFF000000;
                }
            }
        } else {
            render_native_dashboard(g_scanout_px, g_w, g_h);
        }
        
        pthread_mutex_lock(&g_zmm_vm_mutex);
        run_rooted_zmm_tick();
        // process_logs(g_scanout_px, g_w, g_h);
        tsfi_mozilla_wmq_bridge_tick(&g_zmm_vm);
        pthread_mutex_unlock(&g_zmm_vm_mutex);

        // Runtime tsfi_vision validation against the expected YouTube frame
        if (g_active_video_frame && g_active_video_w > 0 && g_active_video_h > 0) {
            static int log_ticks = 0;
            if (log_ticks++ % 30 == 0) {
                uint32_t *ref_pixels = (uint32_t *)malloc(g_w * g_h * sizeof(uint32_t));
                if (ref_pixels) {
                    for (int y = 0; y < g_h; y++) {
                        int sy = y * g_active_video_h / g_h;
                        const uint8_t *src_row = g_active_video_frame + sy * g_active_video_w * 4;
                        uint32_t *dst_row = ref_pixels + y * g_w;
                        for (int x = 0; x < g_w; x++) {
                            int sx = x * g_active_video_w / g_w;
                            const uint8_t *pixel = src_row + sx * 4;
                            dst_row[x] = (0xFF000000) | (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
                        }
                    }
                    float score = tsfi_vision_calculate_similarity(g_scanout_px, ref_pixels, g_w, g_h);
                    long scanout_non_zero = 0;
                    for (int i = 0; i < g_w * g_h; i++) {
                        if ((g_scanout_px[i] & 0x00FFFFFF) > 0x00050505) scanout_non_zero++;
                    }
                    printf("[Auncient Vision] YouTube Window similarity score: %f, scanout non-zero: %ld\n", score, scanout_non_zero);
                    free(ref_pixels);
                }

                // Write current scanout buffer as a JPEG to latest_frame.jpg for verify_browser_opencv.py
                uint8_t *rgb24 = (uint8_t *)malloc(g_w * g_h * 3);
                if (rgb24) {
                    for (int i = 0; i < g_w * g_h; i++) {
                        uint32_t pixel = g_scanout_px[i];
                        rgb24[i * 3 + 0] = (pixel >> 16) & 0xFF; // R
                        rgb24[i * 3 + 1] = (pixel >> 8) & 0xFF;  // G
                        rgb24[i * 3 + 2] = pixel & 0xFF;         // B
                    }
                    unsigned char *jpeg_buf = NULL;
                    unsigned long jpeg_size = 0;
                    int enc_res = tsfi_jpeg_encode(&jpeg_buf, &jpeg_size, rgb24, g_w, g_h, 90);
                    if (enc_res == 0) {
                        const char *abs_path = "/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/latest_frame.jpg";
                        FILE *f = fopen(abs_path, "wb");
                        if (f) {
                            fwrite(jpeg_buf, 1, jpeg_size, f);
                            fclose(f);
                            printf("[Auncient Presenter] Saved screenshot to %s (%lu bytes)\n", abs_path, jpeg_size);
                        } else {
                            printf("[Auncient Presenter ERR] Failed to open %s for writing: %m\n", abs_path);
                        }
                        tsfi_free_buffer(jpeg_buf);
                    } else {
                        printf("[Auncient Presenter ERR] JPEG encode failed with code: %d\n", enc_res);
                    }
                    free(rgb24);
                }
            }
        }
        
        commit_wayland_surface(fd_wl, surf, bid);
        return true;
    }
    return false;
}




#ifndef VALIDATE_LAYOUT_MAIN
static void* run_presenter_thread(void* arg) {
    (void)arg;
    // Initialize dependencies and global hardware structures (wired allocation table)
    tsfi_wire_firmware_init();

    // Initialize the global ZMM VM
    printf("[Auncient Presenter] Initializing global ZMM VM...\n");
    fflush(stdout);
    tsfi_zmm_vm_init(&g_zmm_vm);

    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"MicroUI\", \"../solidity/bin/microui.yul\", 256");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"MicroUI\", \"solidity/bin/microui.yul\", 256");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"WinchesterMQ\", \"../solidity/bin/WinchesterMQ.yul\", 0xccb077a0");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"WinchesterMQ\", \"solidity/bin/WinchesterMQ.yul\", 0xccb077a0");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"laufactory\", \"../solidity/bin/laufactory.yul\", 0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"laufactory\", \"solidity/bin/laufactory.yul\", 0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3");
    
    // Invoke LAUFactory.New("ROOTED Browser", "ROOTED") on the VM to deploy and bind the legal username token context
    // Selector for New(string,string) is 0xc55c7075. Calldata requires string variables padded in EVM format.
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULEXEC \"laufactory\", \"c55c7075"
        "0000000000000000000000000000000000000000000000000000000000000040" // Offset of name
        "0000000000000000000000000000000000000000000000000000000000000080" // Offset of symbol
        "000000000000000000000000000000000000000000000000000000000000000e" // Length of name "ROOTED Browser" (14 bytes)
        "524f4f5445442042726f77736572000000000000000000000000000000000000" // Data "ROOTED Browser"
        "0000000000000000000000000000000000000000000000000000000000000006" // Length of symbol "ROOTED" (6 bytes)
        "524f4f5445440000000000000000000000000000000000000000000000000000\""); // Data "ROOTED"
    
    printf("[Auncient Presenter] ZMM VM diagnostics, WinchesterMQ, and legal immutable LAU ROOTED Browser username setup completed.\n");
    fflush(stdout);

    // Initialize Mozilla Bridge
    printf("[Auncient Presenter] Initializing Mozilla WMQ bridge...\n");
    tsfi_mozilla_wmq_bridge_init("./libtsfi2.so");

    // --- 1. Load Firmware ZMM ---
    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkGetDeviceQueue pvkGetDeviceQueue = (PFN_vkGetDeviceQueue)tsfi_vkGetInstanceProcAddr(NULL, "vkGetDeviceQueue");
    PFN_vkQueuePresentKHR pvkQueuePresentKHR = (PFN_vkQueuePresentKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkQueuePresentKHR");
    (void)pvkQueuePresentKHR;
    
    PFN_tsfi_zmm_set_scanout_buffer ptsfi_zmm_set_scanout_buffer = (PFN_tsfi_zmm_set_scanout_buffer)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_zmm_set_scanout_buffer");

    if (!pvkCreateInstance || !pvkCreateDevice || !pvkGetDeviceQueue || !ptsfi_zmm_set_scanout_buffer) {
        fprintf(stderr, "[Auncient Presenter ERR] Failed to resolve critical Vulkan/ZMM function pointers!\n");
        return NULL;
    }

    VkInstance instance; VkInstanceCreateInfo inst_info = {0}; pvkCreateInstance(&inst_info, NULL, &instance);
    VkDevice device; VkDeviceCreateInfo dev_info = {0}; pvkCreateDevice((VkPhysicalDevice)0x2000, &dev_info, NULL, &device);
    VkQueue queue; pvkGetDeviceQueue(device, 0, 0, &queue);

    // --- 2. Setup Wayland Connection ---
    const char *run = getenv("XDG_RUNTIME_DIR"), *disp = getenv("WAYLAND_DISPLAY");
    if (!run) return NULL; 
    if (!disp) disp = "wayland-0";
    char path[108]; snprintf(path, 108, "%s/%s", run, disp);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0}; addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return NULL;

    uint32_t reg_args[] = {WL_REGISTRY_ID}; send_msg(fd, WL_DISPLAY_ID, WL_DISPLAY_GET_REGISTRY, reg_args, 4, -1);
    for (int i = 0; i < 100; i++) { process_events(fd, 0, NULL); tsfi_raw_usleep(2000); }
    if (!global_xdg_id) return NULL;

    uint32_t cid = next_id++, sid = next_id++, xid = next_id++;
    xdg_wm_base_id = xid;
    uint32_t *b = (uint32_t*)malloc(256); int off;
    off = 0; b[off++] = global_compositor_id; off += pack_string(&b[off], "wl_compositor"); b[off++] = 4; b[off++] = cid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
    off = 0; b[off++] = global_shm_id; off += pack_string(&b[off], "wl_shm"); b[off++] = 1; b[off++] = sid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
    off = 0; b[off++] = global_xdg_id; off += pack_string(&b[off], "xdg_wm_base"); b[off++] = 1; b[off++] = xid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);

    if (global_seat_name) {
        seat_id = next_id++;
        off = 0;
        b[off++] = global_seat_name;
        off += pack_string(&b[off], "wl_seat");
        b[off++] = 1; // version
        b[off++] = seat_id;
        send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);

        // Bind pointer and keyboard
        pointer_id = next_id++;
        uint32_t p_args[] = {pointer_id};
        send_msg(fd, seat_id, 0, p_args, 4, -1); // get_pointer

        keyboard_id = next_id++;
        uint32_t k_args[] = {keyboard_id};
        send_msg(fd, seat_id, 1, k_args, 4, -1); // get_keyboard

        // Dispatch events to flush seat and device binds
        for (int i = 0; i < 50; i++) {
            process_events(fd, 0, NULL);
            tsfi_raw_usleep(2000);
        }
    }

    surf_id = next_id++;
    uint32_t surf = surf_id, xsurf = next_id++, top = next_id++;
    uint32_t s_args[] = {surf}; send_msg(fd, cid, WL_COMPOSITOR_CREATE_SURFACE, s_args, 4, -1);
    uint32_t xs_args[] = {xsurf, surf}; send_msg(fd, xid, XDG_WM_BASE_GET_XDG_SURFACE, xs_args, 8, -1);
    uint32_t t_args[] = {top}; send_msg(fd, xsurf, XDG_SURFACE_GET_TOPLEVEL, t_args, 4, -1);

    // Set window title and app_id so compositor maps the window with decorations/class
    uint32_t *title_buf = (uint32_t*)malloc(256);
    int title_off = 0;
    title_off += pack_string(title_buf, "Auncient Web");
    send_msg(fd, top, 2, title_buf, title_off * 4, -1); // 2 = set_title
    
    int app_off = 0;
    app_off += pack_string(title_buf, "auncient-web");
    send_msg(fd, top, 3, title_buf, app_off * 4, -1); // 3 = set_app_id
    free(title_buf);

    int str = g_w * 4; size_t sz = MAX_W * MAX_H * 4;
    int mfd = memfd_create("tsfi_scanout", MFD_CLOEXEC);
    if (mfd < 0 || ftruncate(mfd, sz) < 0) return NULL;
    uint32_t *scanout_px = (uint32_t*)mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);
    for (int i = 0; i < MAX_W * MAX_H; i++) {
        scanout_px[i] = 0xFF0B0214; // Brand purple
    }
    
    pid_val = next_id++; bid_val = next_id++;
    uint32_t p_args[] = {pid_val, (uint32_t)sz}; send_msg(fd, sid, WL_SHM_CREATE_POOL, p_args, 8, mfd);
    uint32_t bf_args[] = {bid_val, 0, (uint32_t)g_w, (uint32_t)g_h, (uint32_t)str, 1}; send_msg(fd, pid_val, WL_SHM_POOL_CREATE_BUFFER, bf_args, 24, -1); // 1 = XRGB8888
    
    // Initial commit must NOT have a buffer attached per XDG shell spec
    send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);

    // --- 3. Link Firmware ZMM to Wayland Scanout ---
    g_scanout_px = scanout_px;
    g_ptsfi_zmm_set_scanout_buffer = ptsfi_zmm_set_scanout_buffer;
    ptsfi_zmm_set_scanout_buffer(scanout_px, g_w, g_h);
    // ptsfi_drmModeAddPlane(72, sz);
    // uint32_t* p72 = (uint32_t*)ptsfi_drmModeGetVirtualPlaneBuffer(72);

    VkSwapchainKHR mock_swapchain = (VkSwapchainKHR)0xB000;
    uint32_t imageIndex = 0;
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = (VkStructureType)8; // VK_STRUCTURE_TYPE_PRESENT_INFO_KHR is 1000001003 or mock val, let's cast or use empty struct
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mock_swapchain;
    presentInfo.pImageIndices = &imageIndex;
    (void)presentInfo;
    printf("[Auncient Presenter] Waiting for initial Wayland configure event...\n");
    bool initial_configured = false;
    for (int i = 0; i < 100; i++) {
        process_events(fd, xsurf, &initial_configured);
        if (initial_configured) {
            // Attach buffer and commit now that configure has been acknowledged
            uint32_t a_args[] = {bid_val, 0, 0};
            send_msg(fd, surf, WL_SURFACE_ATTACH, a_args, 12, -1);
            // Render native dashboard layout directly into the scanout buffer before commit
            render_native_dashboard(scanout_px, g_w, g_h);
            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
            break;
        }
        tsfi_raw_usleep(10000);
    }

    printf("[Auncient Presenter] Frame presenter ready. Streaming starting...\n");
    fflush(stdout);

    // Make stdin non-blocking if running as projector
    if (is_projector) {
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    struct pollfd fds[2] = {0};
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    fds[1].fd = is_projector ? STDIN_FILENO : -1;
    fds[1].events = POLLIN;

    enum {
        STATE_READ_LEN,
        STATE_READ_DATA,
        STATE_READ_CMD
    } stream_state = STATE_READ_LEN;

    size_t target_len = 20;
    size_t total_read = 0;
    static uint8_t input_buf[2 * 1024 * 1024]; // 2 MB static buffer
    uint32_t jpeg_len = 0;

    g_video_decoder = tsfi_media_decoder_create("segment_0.mp4");
    if (!g_video_decoder) {
        g_video_decoder = tsfi_media_decoder_create("tsfi2-deepseek/segment_0.mp4");
    }
    if (g_video_decoder) {
        g_video_rgba_buf = (uint8_t *)malloc(1920 * 1080 * 4);
        if (g_video_rgba_buf) {
            int vw = 0, vh = 0;
            // Decode first 300 frames to fast-forward past initial dark browser load phase
            for (int i = 0; i < 300; i++) {
                tsfi_media_decoder_next_frame(g_video_decoder, &vw, &vh, g_video_rgba_buf);
            }
            g_video_w = vw;
            g_video_h = vh;
            g_active_video_frame = g_video_rgba_buf;
            g_active_video_w = vw;
            g_active_video_h = vh;

            long non_zero = 0;
            for (int i = 0; i < vw * vh * 4; i++) {
                if (g_video_rgba_buf[i] > 10) non_zero++;
            }
            printf("[Auncient Presenter] Fast-forwarded frame: %dx%d, non-zero bytes (val > 10): %ld\n", vw, vh, non_zero);
        }
    }

    while (1) {
        extern bool g_video_playing;
        extern double g_video_current_time;
        extern double g_video_duration;

        if (g_video_playing && g_video_decoder && g_video_rgba_buf) {
            int vw = 0, vh = 0;
            if (tsfi_media_decoder_next_frame(g_video_decoder, &vw, &vh, g_video_rgba_buf)) {
                g_video_w = vw;
                g_video_h = vh;
                g_active_video_frame = g_video_rgba_buf;
                g_active_video_w = vw;
                g_active_video_h = vh;
                g_input_dirty = true;
                g_video_current_time += 0.033; // 30 fps approximation
                if (g_video_current_time > g_video_duration) {
                    g_video_current_time = 0.0;
                }
            } else {
                tsfi_media_decoder_destroy(g_video_decoder);
                g_video_decoder = tsfi_media_decoder_create("segment_0.mp4");
                if (!g_video_decoder) {
                    g_video_decoder = tsfi_media_decoder_create("tsfi2-deepseek/segment_0.mp4");
                }
                g_video_current_time = 0.0;
            }
        }

        // Poll two file descriptors with a 10ms timeout
        int ret = poll(fds, 2, 10);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        // 1. Process Wayland connection events immediately (keeps window responsive to ping/inputs)
        if (fds[0].revents & POLLIN) {
            bool conf = false;
            int pev;
            while ((pev = process_events(fd, xsurf, &conf)) > 0) {
                // drain events
            }
            if (conf) {
                update_and_present(fd, surf, bid_val, true);
            }
        }

        // 2. Read incoming command payload from stdin
        if (fds[1].revents & POLLIN) {
            ssize_t bytes = read(STDIN_FILENO, input_buf + total_read, target_len - total_read);
            if (bytes < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    fprintf(stderr, "[Auncient Presenter] STDIN read error: %s\n", strerror(errno));
                    goto out;
                }
            } else if (bytes == 0) {
                printf("[Auncient Presenter] STDIN EOF reached. Exiting loop.\n");
                goto out;
            } else {
                total_read += bytes;
                if (total_read >= target_len) {
                    if (stream_state == STATE_READ_LEN) {
                        memcpy(&jpeg_len, input_buf, 4);
                        memcpy(&g_dest_x, input_buf + 4, 4);
                        memcpy(&g_dest_y, input_buf + 8, 4);
                        memcpy(&g_dest_w, input_buf + 12, 4);
                        memcpy(&g_dest_h, input_buf + 16, 4);

                        if (jpeg_len & 0x80000000) {
                            uint32_t payload_len = jpeg_len & 0x7FFFFFFF;
                            if (payload_len > sizeof(input_buf)) {
                                fprintf(stderr, "[Auncient Presenter ERR] Command payload length %u exceeds buffer size!\n", payload_len);
                                goto out;
                            }
                            stream_state = STATE_READ_CMD;
                            target_len = payload_len;
                            total_read = 0;
                        } else {
                            // Ignore raw video frame chunks on stdin
                            stream_state = STATE_READ_LEN;
                            target_len = 20;
                            total_read = 0;
                        }
                    } else if (stream_state == STATE_READ_DATA) {
                        stream_state = STATE_READ_LEN;
                        target_len = 20;
                        total_read = 0;
                    } else if (stream_state == STATE_READ_CMD) {
                        if (target_len < sizeof(input_buf)) {
                            input_buf[target_len] = '\0';
                        } else {
                            input_buf[sizeof(input_buf) - 1] = '\0';
                        }
                        // Execute the clipboard command directly on the Wayland side!
                        // Since this is spawned from the active headed Wayland window process,
                        // it can set the clipboard natively.
                        FILE *py_proc = popen("python3 scripts/set_clipboard.py", "w");
                        if (py_proc) {
                            fwrite(input_buf, 1, target_len, py_proc);
                            pclose(py_proc);
                        }
                        stream_state = STATE_READ_LEN;
                        target_len = 20;
                        total_read = 0;
                    }
                }
            }
        }
        // 3. Asynchronously present new YouTube frames immediately when available in RAM
        if (g_input_dirty) {
            g_input_dirty = false;
            update_and_present(fd, surf, bid_val, true);
        } else {
            update_and_present(fd, surf, bid_val, false);
        }
    }
out:
    printf("[Auncient Presenter] Cleaning up virtual planes...\n");
    free(b);
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    pvkDestroyDevice(device, NULL);
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    pvkDestroyInstance(instance, NULL);

    tsfi_zmm_vm_destroy(&g_zmm_vm);
    tsfi_dl_thunks_teardown();
    lau_registry_teardown();
    lau_free_all_active();
    return NULL;
}
#endif
#ifndef VALIDATE_LAYOUT_MAIN
int main(int argc, char **argv) {
    is_projector = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--projector") == 0) {
            is_projector = true;
        }
    }
    g_dest_x = 0;
    g_dest_y = 0;
    g_dest_w = g_w;
    g_dest_h = g_h;
    printf("[Auncient Presenter] Initializing Vulkan and Wayland loops... (projector=%d)\n", is_projector);

    pthread_t presenter_thread_handle;
    if (pthread_create(&presenter_thread_handle, NULL, run_presenter_thread, NULL) != 0) {
        fprintf(stderr, "[Auncient Presenter ERR] Failed to create presenter loop thread!\n");
        return 1;
    }
    pthread_detach(presenter_thread_handle);

    // Run Node.js on the main process thread to satisfy V8 engine threading constraints
    printf("[Auncient Presenter] Spawning embedded Node.js engine on main thread...\n");
    const char *script_path = "scripts/embedded_browser_controller.js";
    if (access(script_path, F_OK) != 0) {
        script_path = "../scripts/embedded_browser_controller.js";
    }
    char* args[] = { (char*)"node", (char*)script_path, NULL };
    start_embedded_node(2, args);
    printf("[Auncient Presenter] Embedded Node.js engine exited.\n");
    return 0;
}
#endif
