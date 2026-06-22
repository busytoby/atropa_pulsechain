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

#include <arpa/inet.h>
#include <netinet/in.h>

extern void push_input_event(const char *cmd);
static void send_input_command(const char *cmd) {
    push_input_event(cmd);
}

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

#define WL_DISPLAY_ID 1
#define WL_REGISTRY_ID 2
#define WL_DISPLAY_GET_REGISTRY 1
#define WL_REGISTRY_BIND 0
#define WL_COMPOSITOR_CREATE_SURFACE 0
#define WL_SHM_CREATE_POOL 0
#define WL_SHM_POOL_CREATE_BUFFER 0
#define WL_SURFACE_ATTACH 1
#define WL_SURFACE_DAMAGE 2
#define WL_SURFACE_COMMIT 6

#define XDG_WM_BASE_GET_XDG_SURFACE 2
#define XDG_SURFACE_GET_TOPLEVEL 1
#define XDG_SURFACE_ACK_CONFIGURE 4

#define ALIGN(x) (((x) + 3) & ~3)

static uint32_t global_compositor_id = 0, global_shm_id = 0, global_xdg_id = 0, global_seat_name = 0;
static uint32_t next_id = 3; 
static bool is_projector = false;

static uint32_t seat_id = 0;
static uint32_t pointer_id = 0;
static uint32_t keyboard_id = 0;
static uint32_t xdg_wm_base_id = 0;

static TsfiZmmVmState g_zmm_vm;
static uint16_t g_mouse_x = 0;
static uint16_t g_mouse_y = 0;
static uint8_t g_click_state = 0;

static void fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

#define MAX_W 3840
#define MAX_H 2160
static int g_w = 1024;
static int g_h = 768;
static uint32_t pid_val = 0;
static uint32_t bid_val = 0;
static uint32_t *g_scanout_px = NULL;
static PFN_tsfi_zmm_set_scanout_buffer g_ptsfi_zmm_set_scanout_buffer = NULL;
static uint32_t surf_id = 0;

static uint8_t last_jpeg_buf[2 * 1024 * 1024];
static size_t last_jpeg_sz = 0;


int pack_string(uint32_t *buf, const char *s) {
    uint32_t len = strlen(s) + 1;
    buf[0] = len; memcpy(&buf[1], s, len);
    return 1 + (ALIGN(len) / 4);
}

void send_msg(int fd, uint32_t obj, uint16_t op, void *data, size_t len, int s_fd) {
    uint32_t head[2] = {obj, ((uint32_t)(8 + len) << 16) | op};
    struct iovec iov[2] = {{head, 8}, {data, len}};
    struct msghdr msg = {0}; msg.msg_iov = iov; msg.msg_iovlen = len ? 2 : 1;
    union {
        char buf[CMSG_SPACE(sizeof(int))];
        struct cmsghdr align;
    } u;
    if (s_fd != -1) {
        memset(&u, 0, sizeof(u));
        msg.msg_control = u.buf; msg.msg_controllen = sizeof(u.buf);
        struct cmsghdr *cm = CMSG_FIRSTHDR(&msg);
        cm->cmsg_level = SOL_SOCKET; cm->cmsg_type = SCM_RIGHTS; cm->cmsg_len = CMSG_LEN(sizeof(int));
        *((int *)CMSG_DATA(cm)) = s_fd;
    }
    ssize_t ret = sendmsg(fd, &msg, MSG_NOSIGNAL);
    if (ret < 0) {
        fprintf(stderr, "[Auncient Presenter ERR] sendmsg failed: obj=%u, op=%u, err=%s\n", obj, op, strerror(errno));
    }
}

struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

static void my_error_exit(j_common_ptr cinfo) {
    struct my_error_mgr *myerr = (struct my_error_mgr *)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

static int g_main_img_w = 1024;
static int g_main_img_h = 768;

bool decode_jpeg_full(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
    if (jpeg_sz < 4 || jpeg_buf[0] != 0xFF || jpeg_buf[1] != 0xD8) {
        return false;
    }
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        fprintf(stderr, "[Auncient Presenter ERR] JPEG decompression error in decode_jpeg_full!\n");
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, jpeg_buf, jpeg_sz);
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        fprintf(stderr, "[Auncient Presenter ERR] JPEG read header failed!\n");
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    cinfo.out_color_space = JCS_RGB;
    if (!jpeg_start_decompress(&cinfo)) {
        fprintf(stderr, "[Auncient Presenter ERR] JPEG start decompress failed!\n");
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    int img_w = cinfo.output_width;
    int img_h = cinfo.output_height;
    uint8_t *src_rgb = (uint8_t *)malloc(img_w * img_h * 3);
    if (!src_rgb) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    int row_stride = img_w * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    while (cinfo.output_scanline < cinfo.output_height) {
        int y = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(src_rgb + y * img_w * 3, buffer[0], img_w * 3);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // Scale preserving aspect ratio with dark theme background letterboxing/pillarboxing if projector is enabled.
    // Otherwise, stretch to the full screen.
    int new_w = width;
    int new_h = height;
    int pad_x = 0;
    int pad_y = 0;
    if (is_projector) {
        float scale = (float)width / (float)img_w;
        if ((float)height / (float)img_h < scale) {
            scale = (float)height / (float)img_h;
        }
        new_w = (int)(img_w * scale);
        new_h = (int)(img_h * scale);
        if (new_w <= 0) new_w = 1;
        if (new_h <= 0) new_h = 1;
        pad_x = (width - new_w) / 2;
        pad_y = (height - new_h) / 2;
    }

    for (int dy = 0; dy < height; dy++) {
        uint32_t *dst_row = scanout_px + dy * width;
        if (dy < pad_y || dy >= pad_y + new_h) {
            for (int dx = 0; dx < width; dx++) {
                dst_row[dx] = 0xFF0A0A0C; // Auncient base dark background color
            }
            continue;
        }

        float sy = (float)(dy - pad_y) * (float)img_h / (float)new_h;
        int y0 = (int)sy;
        int y1 = (y0 + 1 < img_h) ? y0 + 1 : y0;
        float ty = sy - (float)y0;

        for (int dx = 0; dx < width; dx++) {
            if (dx < pad_x || dx >= pad_x + new_w) {
                dst_row[dx] = 0xFF0A0A0C;
                continue;
            }

            float sx = (float)(dx - pad_x) * (float)img_w / (float)new_w;
            int x0 = (int)sx;
            int x1 = (x0 + 1 < img_w) ? x0 + 1 : x0;
            float tx = sx - (float)x0;

            int idx00 = (y0 * img_w + x0) * 3;
            int idx10 = (y0 * img_w + x1) * 3;
            int idx01 = (y1 * img_w + x0) * 3;
            int idx11 = (y1 * img_w + x1) * 3;

            float w00 = (1.0f - tx) * (1.0f - ty);
            float w10 = tx * (1.0f - ty);
            float w01 = (1.0f - tx) * ty;
            float w11 = tx * ty;

            uint8_t r = (uint8_t)(src_rgb[idx00] * w00 + src_rgb[idx10] * w10 + src_rgb[idx01] * w01 + src_rgb[idx11] * w11);
            uint8_t g = (uint8_t)(src_rgb[idx00+1] * w00 + src_rgb[idx10+1] * w10 + src_rgb[idx01+1] * w01 + src_rgb[idx11+1] * w11);
            uint8_t b = (uint8_t)(src_rgb[idx00+2] * w00 + src_rgb[idx10+2] * w10 + src_rgb[idx01+2] * w01 + src_rgb[idx11+2] * w11);

            dst_row[dx] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }

    g_main_img_w = img_w;
    g_main_img_h = img_h;
    free(src_rgb);
    return true;
}

static int g_dest_x = 410;
static int g_dest_y = 80;
static int g_dest_w = 0;
static int g_dest_h = 0;

bool decode_jpeg_subrect(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
    if (last_jpeg_sz == 0 || g_dest_w <= 0 || g_dest_h <= 0) {
        return true;
    }
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
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

    int img_w = cinfo.output_width;
    int img_h = cinfo.output_height;
    uint8_t *src_rgb = (uint8_t *)malloc(img_w * img_h * 3);
    if (!src_rgb) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    int row_stride = img_w * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        int y = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(src_rgb + y * img_w * 3, buffer[0], img_w * 3);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // Reserved sub-rectangle for the YouTube stream in the Auncient dashboard layout
    float rx = (float)g_dest_x / (float)g_main_img_w;
    float ry = (float)g_dest_y / (float)g_main_img_h;
    float rw = (float)g_dest_w / (float)g_main_img_w;
    float rh = (float)g_dest_h / (float)g_main_img_h;

    int dest_x = (int)(rx * width);
    int dest_y = (int)(ry * height);
    int dest_w = (int)(rw * width);
    int dest_h = (int)(rh * height);

    if (dest_x < 0) dest_x = 0;
    if (dest_y < 0) dest_y = 0;
    if (dest_x >= width) dest_x = width - 1;
    if (dest_y >= height) dest_y = height - 1;
    if (dest_w <= 0) dest_w = 1;
    if (dest_h <= 0) dest_h = 1;
    if (dest_x + dest_w > width) dest_w = width - dest_x;
    if (dest_y + dest_h > height) dest_h = height - dest_y;

    // Bilinear downscaling for visual font anti-aliasing quality
    for (int dy = 0; dy < dest_h; dy++) {
        float sy = (float)dy * (float)img_h / (float)dest_h;
        int y0 = (int)sy;
        int y1 = (y0 + 1 < img_h) ? y0 + 1 : y0;
        float ty = sy - (float)y0;

        uint32_t *dst_row = scanout_px + (dest_y + dy) * width + dest_x;

        for (int dx = 0; dx < dest_w; dx++) {
            float sx = (float)dx * (float)img_w / (float)dest_w;
            int x0 = (int)sx;
            int x1 = (x0 + 1 < img_w) ? x0 + 1 : x0;
            float tx = sx - (float)x0;

            int idx00 = (y0 * img_w + x0) * 3;
            int idx10 = (y0 * img_w + x1) * 3;
            int idx01 = (y1 * img_w + x0) * 3;
            int idx11 = (y1 * img_w + x1) * 3;

            float w00 = (1.0f - tx) * (1.0f - ty);
            float w10 = tx * (1.0f - ty);
            float w01 = (1.0f - tx) * ty;
            float w11 = tx * ty;

            uint8_t r = (uint8_t)(src_rgb[idx00] * w00 + src_rgb[idx10] * w10 + src_rgb[idx01] * w01 + src_rgb[idx11] * w11);
            uint8_t g = (uint8_t)(src_rgb[idx00+1] * w00 + src_rgb[idx10+1] * w10 + src_rgb[idx01+1] * w01 + src_rgb[idx11+1] * w11);
            uint8_t b = (uint8_t)(src_rgb[idx00+2] * w00 + src_rgb[idx10+2] * w10 + src_rgb[idx01+2] * w01 + src_rgb[idx11+2] * w11);

            // Chroma key: check existing pixel to avoid overwriting HUD / DNA elements
            uint32_t cur_val = dst_row[dx];
            uint8_t cur_r = (cur_val >> 16) & 0xFF;
            uint8_t cur_g = (cur_val >> 8) & 0xFF;
            uint8_t cur_b = cur_val & 0xFF;

            if (cur_r < 25 && cur_g < 25 && cur_b < 35) {
                dst_row[dx] = (0xFF000000) | (r << 16) | (g << 8) | b;
            }
        }
    }

    free(src_rgb);
    return true;
}

#include <sys/stat.h>

static const char* get_youtube_frame_path(void) {
    struct stat st;
    if (stat("/dev/shm/atropa_latest_frame.jpg", &st) == 0 && st.st_size > 0) {
        return "/dev/shm/atropa_latest_frame.jpg";
    }
    if (stat("/dev/shm/atropa_youtube_frame.jpg", &st) == 0 && st.st_size > 0) {
        return "/dev/shm/atropa_youtube_frame.jpg";
    }
    if (stat("frontend/latest_frame.jpg", &st) == 0 && st.st_size > 0) {
        return "frontend/latest_frame.jpg";
    }
    if (stat("../frontend/latest_frame.jpg", &st) == 0 && st.st_size > 0) {
        return "../frontend/latest_frame.jpg";
    }
    if (stat("/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/latest_frame.jpg", &st) == 0 && st.st_size > 0) {
        return "/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/latest_frame.jpg";
    }
    return "/dev/shm/atropa_latest_frame.jpg"; // fallback
}

static time_t last_youtube_mtime = 0;
static long last_youtube_mtime_nsec = 0;
static uint32_t *youtube_cache_px = NULL;
static int youtube_cache_w = 0;
static int youtube_cache_h = 0;
static int youtube_cache_dest_w = 0;
static int youtube_cache_dest_h = 0;

void draw_youtube_frame(uint32_t *scanout_px, int width, int height) {
    if (g_dest_w <= 0) g_dest_w = width;
    if (g_dest_h <= 0) g_dest_h = height;

    int dest_x = 0;
    int dest_y = 0;
    int dest_w = width;
    int dest_h = height;

    if (is_projector) {
        float rx = (float)g_dest_x / (float)g_main_img_w;
        float ry = (float)g_dest_y / (float)g_main_img_h;
        float rw = (float)g_dest_w / (float)g_main_img_w;
        float rh = (float)g_dest_h / (float)g_main_img_h;

        dest_x = (int)(rx * width);
        dest_y = (int)(ry * height);
        dest_w = (int)(rw * width);
        dest_h = (int)(rh * height);
    }

    if (dest_x < 0) dest_x = 0;
    if (dest_y < 0) dest_y = 0;
    if (dest_x >= width) dest_x = width - 1;
    if (dest_y >= height) dest_y = height - 1;
    if (dest_w <= 0) dest_w = 1;
    if (dest_h <= 0) dest_h = 1;
    if (dest_x + dest_w > width) dest_w = width - dest_x;
    if (dest_y + dest_h > height) dest_h = height - dest_y;

    int required_sz = dest_w * dest_h;
    if (!youtube_cache_px || youtube_cache_w != width || youtube_cache_h != height || youtube_cache_dest_w != dest_w || youtube_cache_dest_h != dest_h) {
        youtube_cache_px = (uint32_t *)realloc(youtube_cache_px, required_sz * sizeof(uint32_t));
        youtube_cache_w = width;
        youtube_cache_h = height;
        youtube_cache_dest_w = dest_w;
        youtube_cache_dest_h = dest_h;
        if (youtube_cache_px) {
            memset(youtube_cache_px, 0, required_sz * sizeof(uint32_t));
        }
        last_youtube_mtime = 0;
        last_youtube_mtime_nsec = 0;
    }

    struct stat st;
    bool need_blit = true;
    int fd_img = open(get_youtube_frame_path(), O_RDONLY);
    if (fd_img >= 0) {
        if (fstat(fd_img, &st) >= 0 && st.st_size > 0) {
            #ifdef __APPLE__
            time_t cur_mtime = st.st_mtimespec.tv_sec;
            long cur_mtime_nsec = st.st_mtimespec.tv_nsec;
            #else
            time_t cur_mtime = st.st_mtim.tv_sec;
            long cur_mtime_nsec = st.st_mtim.tv_nsec;
            #endif
            if (cur_mtime != last_youtube_mtime || cur_mtime_nsec != last_youtube_mtime_nsec) {
                uint8_t *buf = (uint8_t *)malloc(st.st_size);
                if (buf) {
                    ssize_t r = read(fd_img, buf, st.st_size);
                    if (r == st.st_size) {
                        // Decode directly onto the scanout_px first
                        bool success = false;
                        if (!is_projector) {
                            success = decode_jpeg_full(buf, st.st_size, scanout_px, width, height);
                        } else {
                            success = decode_jpeg_subrect(buf, st.st_size, scanout_px, width, height);
                        }
                        if (success) {
                            last_youtube_mtime = cur_mtime;
                            last_youtube_mtime_nsec = cur_mtime_nsec;
                            last_jpeg_sz = st.st_size;
                            need_blit = false; // Already decoded directly to scanout
                            // Copy the decoded subrect into cache
                            if (youtube_cache_px) {
                                if (!is_projector) {
                                    memcpy(youtube_cache_px, scanout_px, width * height * sizeof(uint32_t));
                                } else {
                                    for (int dy = 0; dy < dest_h; dy++) {
                                        uint32_t *src_row = scanout_px + (dest_y + dy) * width + dest_x;
                                        uint32_t *dst_row = youtube_cache_px + dy * dest_w;
                                        memcpy(dst_row, src_row, dest_w * sizeof(uint32_t));
                                    }
                                }
                            }
                        }
                    }
                    free(buf);
                }
            }
        }
        close(fd_img);
    }

    if (need_blit && youtube_cache_px && last_youtube_mtime != 0) {
        if (!is_projector) {
            memcpy(scanout_px, youtube_cache_px, width * height * sizeof(uint32_t));
        } else {
            for (int dy = 0; dy < dest_h; dy++) {
                uint32_t *dst_row = scanout_px + (dest_y + dy) * width + dest_x;
                uint32_t *src_row = youtube_cache_px + dy * dest_w;
                for (int dx = 0; dx < dest_w; dx++) {
                    uint32_t cur_val = dst_row[dx];
                    uint8_t cur_r = (cur_val >> 16) & 0xFF;
                    uint8_t cur_g = (cur_val >> 8) & 0xFF;
                    uint8_t cur_b = cur_val & 0xFF;
                    if (cur_r < 25 && cur_g < 25 && cur_b < 35) {
                        dst_row[dx] = src_row[dx];
                    }
                }
            }
        }
    }
}

static uint32_t *dashboard_cache_px = NULL;

bool decode_jpeg_zero_copy(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
    (void)scanout_px;
    if (!dashboard_cache_px) {
        dashboard_cache_px = (uint32_t *)malloc(MAX_W * MAX_H * sizeof(uint32_t));
        for (int i = 0; i < MAX_W * MAX_H; i++) {
            dashboard_cache_px[i] = 0xFF0B0214; // default purple
        }
    }
    // Decode dashboard frame into background cache instead of active scanout directly
    return decode_jpeg_full(jpeg_buf, jpeg_sz, dashboard_cache_px, width, height);
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
bool update_and_present(int fd_wl, uint32_t surf, uint32_t bid, bool force_redraw) {
    struct stat st;
    bool new_youtube = false;
    
    bool wmq_triggered = false;

    if (g_dest_w > 0 && g_dest_h > 0) {
        int fd_img = open(get_youtube_frame_path(), O_RDONLY);
        if (fd_img >= 0) {
            if (fstat(fd_img, &st) >= 0 && st.st_size > 0) {
                #ifdef __APPLE__
                time_t cur_mtime = st.st_mtimespec.tv_sec;
                long cur_mtime_nsec = st.st_mtimespec.tv_nsec;
                #else
                time_t cur_mtime = st.st_mtim.tv_sec;
                long cur_mtime_nsec = st.st_mtim.tv_nsec;
                #endif
                if (cur_mtime != last_youtube_mtime || cur_mtime_nsec != last_youtube_mtime_nsec || wmq_triggered) {
                    new_youtube = true;
                    wmq_triggered = false;
                }
            }
            close(fd_img);
        }
    }

    if (force_redraw || new_youtube) {
        if (g_scanout_px) {
            if (g_dest_w > 0 && g_dest_h > 0) {
                if (dashboard_cache_px) {
                    memcpy(g_scanout_px, dashboard_cache_px, g_w * g_h * sizeof(uint32_t));
                } else {
                    for (int i = 0; i < g_w * g_h; i++) {
                        g_scanout_px[i] = 0xFF0B0214;
                    }
                }
                draw_youtube_frame(g_scanout_px, g_w, g_h);
            } else {
                if (dashboard_cache_px) {
                    memcpy(g_scanout_px, dashboard_cache_px, g_w * g_h * sizeof(uint32_t));
                } else {
                    for (int i = 0; i < g_w * g_h; i++) {
                        g_scanout_px[i] = 0xFF0A0A0C;
                    }
                }
            }
            
            run_rooted_zmm_tick();
            process_logs(g_scanout_px, g_w, g_h);
            tsfi_mozilla_wmq_bridge_tick(&g_zmm_vm);
            
            commit_wayland_surface(fd_wl, surf, bid);
            return true;
        }
    }
    return false;
}


static void route_event_to_yul_cpu(const char *cmd, int is_move, int vm_x, int vm_y, uint8_t command_byte, uint8_t keycode) {
    if (is_move) {
        if (g_zmm_vm.reu_ram && g_zmm_vm.reu_size > 0xF004) {
            g_zmm_vm.reu_ram[0xF000] = (uint8_t)(vm_x & 0xFF);
            g_zmm_vm.reu_ram[0xF001] = (uint8_t)(vm_y & 0xFF);
            g_zmm_vm.reu_ram[0xF003] = (uint8_t)((vm_x >> 8) & 0xFF);
            g_zmm_vm.reu_ram[0xF004] = (uint8_t)((vm_y >> 8) & 0xFF);
        }
    } else {
        if (g_zmm_vm.reu_ram && g_zmm_vm.reu_size > 0xF002) {
            g_zmm_vm.reu_ram[0xF002] = keycode;
        }
    }
    
    // Call writeDataPort(uint8) selector 0x98d400c0
    uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
    cd[35] = command_byte;
    
    uint8_t ret[32];
    size_t ret_len = 32;
    lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

    // Call postEvent(bytes32) selector 0xccb077a0
    uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
    char cmd_str[33] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "Y:%s", cmd);
    char processed[33] = {0};
    char *src = cmd_str;
    char *dst = processed;
    while (*src && (dst - processed) < 30) {
        if (strncmp(src, "MOUSE_MOVE", 10) == 0) {
            strcpy(dst, "MM"); dst += 2; src += 10;
        } else if (strncmp(src, "MOUSE_DOWN", 10) == 0) {
            strcpy(dst, "MD"); dst += 2; src += 10;
        } else if (strncmp(src, "MOUSE_UP", 8) == 0) {
            strcpy(dst, "MU"); dst += 2; src += 8;
        } else if (strncmp(src, "MOUSE_SCROLL", 12) == 0) {
            strcpy(dst, "MS"); dst += 2; src += 12;
        } else if (strncmp(src, "KEY_DOWN", 8) == 0) {
            strcpy(dst, "KD"); dst += 2; src += 8;
        } else if (strncmp(src, "KEY_UP", 6) == 0) {
            strcpy(dst, "KU"); dst += 2; src += 6;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    memcpy(cd_post + 4, processed, 32);
    
    lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);
}
void handle_bridge_command(const char *cmd) {
    if (strncmp(cmd, "Y:MM ", 5) == 0) {
        int vm_x = 0, vm_y = 0;
        if (sscanf(cmd + 5, "%d %d", &vm_x, &vm_y) == 2) {
            g_mouse_x = (uint16_t)((float)vm_x * (float)g_w / 320.0f);
            g_mouse_y = (uint16_t)((float)vm_y * (float)g_h / 300.0f);
            uint8_t cmd_byte = (0x00 << 6) | 0x01;
            route_event_to_yul_cpu(cmd + 2, 1, vm_x, vm_y, cmd_byte, 0);
        }
    } else if (strncmp(cmd, "Y:MD ", 5) == 0) {
        int btn = 0;
        if (sscanf(cmd + 5, "%d", &btn) == 1) {
            if (btn == 272) g_click_state = 1;
            uint8_t btn_idx = 0;
            if (btn == 273) btn_idx = 1;
            if (btn == 274) btn_idx = 2;
            uint8_t cmd_byte = (0x01 << 6) | 0x04 | (btn_idx & 0x03);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, 0);
        }
    } else if (strncmp(cmd, "Y:MU ", 5) == 0) {
        int btn = 0;
        if (sscanf(cmd + 5, "%d", &btn) == 1) {
            if (btn == 272) g_click_state = 0;
            uint8_t btn_idx = 0;
            if (btn == 273) btn_idx = 1;
            if (btn == 274) btn_idx = 2;
            uint8_t cmd_byte = (0x01 << 6) | (btn_idx & 0x03);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, 0);
        }
    } else if (strncmp(cmd, "Y:MS ", 5) == 0) {
        route_event_to_yul_cpu(cmd + 2, 0, 0, 0, 0, 0);
    } else if (strncmp(cmd, "Y:KD ", 5) == 0) {
        int key = 0;
        if (sscanf(cmd + 5, "%d", &key) == 1) {
            uint8_t cmd_byte = (0x02 << 6) | 0x20 | (key & 0x1F);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, (uint8_t)key);
            if (key == 1) {
                printf("[Auncient Presenter] ESC key via bridge. Exiting.\n");
                exit(0);
            }
        }
    } else if (strncmp(cmd, "Y:KU ", 5) == 0) {
        int key = 0;
        if (sscanf(cmd + 5, "%d", &key) == 1) {
            uint8_t cmd_byte = (0x02 << 6) | (key & 0x1F);
            route_event_to_yul_cpu(cmd + 2, 0, 0, 0, cmd_byte, (uint8_t)key);
        }
    }
}

int process_events(int fd, uint32_t xdg_s_id, bool *out_configure) {
    uint32_t h[2];
    ssize_t r = recv(fd, h, 8, MSG_DONTWAIT);
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        return -1;
    }
    if (r == 0) return -1;

    uint32_t obj = h[0], sz = h[1] >> 16, op = h[1] & 0xFFFF;
    size_t plen = sz - 8;
    uint32_t *p = plen ? (uint32_t*)malloc(plen) : NULL;
    if (p) {
        if (recv(fd, p, plen, 0) <= 0) {
            free(p);
            return -1;
        }
    }

    if (obj == WL_REGISTRY_ID && op == 0) {
        char *iface = (char *)&p[2];
        if (!strcmp(iface, "wl_compositor")) global_compositor_id = p[0];
        else if (!strcmp(iface, "wl_shm")) global_shm_id = p[0];
        else if (!strcmp(iface, "xdg_wm_base")) global_xdg_id = p[0];
        else if (!strcmp(iface, "wl_seat")) global_seat_name = p[0];
    } else if (obj == xdg_s_id && op == 0) {
        uint32_t ack[] = {p[0]};
        send_msg(fd, xdg_s_id, XDG_SURFACE_ACK_CONFIGURE, ack, 4, -1);
        uint32_t geom[] = {0, 0, (uint32_t)g_w, (uint32_t)g_h};
        send_msg(fd, xdg_s_id, 3, geom, 16, -1); // 3 = XDG_SURFACE_SET_WINDOW_GEOMETRY for Auncient dashboard bounds
        if (out_configure) *out_configure = true;
    } else if (xdg_wm_base_id && obj == xdg_wm_base_id && op == 0) {
        uint32_t pong[] = {p[0]};
        send_msg(fd, xdg_wm_base_id, 3, pong, 4, -1);
    } else if (obj == 1 && op == 0) {
        uint32_t err_obj = p[0];
        uint32_t code = p[1];
        uint32_t str_len = p[2];
        char *err_msg = (char *)&p[3];
        fprintf(stderr, "[Auncient Presenter ERR] Display Error: err_obj=%u, code=%u, msg=%.*s\n", err_obj, code, str_len, err_msg);
    } else if (xdg_s_id && (xdg_s_id + 1 == obj)) { // top (toplevel) close event (opcode 0 is configure, 1 is close)
        if (op == 1) {
            printf("WINDOW_CLOSE\n");
            fflush(stdout);
            usleep(500000); // Wait 500ms for node controller to mute & close chrome
            exit(0);
        } else if (op == 0) { // configure resize
            int32_t width = p[0];
            int32_t height = p[1];
            if (width > 0 && height > 0) {
                if (width <= MAX_W && height <= MAX_H) {
                    if (width != g_w || height != g_h) {
                        g_w = width;
                        g_h = height;
                        // Destroy old buffer
                        send_msg(fd, bid_val, 0, NULL, 0, -1); // wl_buffer.destroy is op 0
                        
                        // Allocate a new buffer ID to avoid compositor ID reuse race
                        bid_val = next_id++;
                        
                        // Create new buffer with new width/height from the same pool
                        uint32_t bf_args[] = {bid_val, 0, g_w, g_h, g_w * 4, 1}; // 1 = XRGB8888
                        send_msg(fd, pid_val, 0, bf_args, 24, -1); // wl_shm_pool.create_buffer is op 0
                        
                        if (g_ptsfi_zmm_set_scanout_buffer && g_scanout_px) {
                            g_ptsfi_zmm_set_scanout_buffer(g_scanout_px, g_w, g_h);
                        }
                        
                        printf("WINDOW_RESIZE %d %d\n", width, height);
                        fflush(stdout);
                    }
                }
            }
        }
    } else if (pointer_id && obj == pointer_id) {
        if (op == 2) { // motion
            double x = ((int32_t)p[1]) / 256.0;
            double y = ((int32_t)p[2]) / 256.0;
            fprintf(stderr, "[DEBUG POINTER] Motion: x=%f, y=%f\n", x, y);
            g_mouse_x = (uint16_t)x;
            g_mouse_y = (uint16_t)y;
            
            uint16_t vm_mx = (uint16_t)((float)x * 320.0f / (float)g_w);
            uint16_t vm_my = (uint16_t)((float)y * 300.0f / (float)g_h);
            char cmd_buf[128];
            snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_MOVE %d %d", vm_mx, vm_my);
            send_input_command(cmd_buf);
            // Handled via the WinchesterMQ in-memory Node.js subscriber
            // uint16_t vm_mx = (uint16_t)((float)x * 320.0f / (float)g_w);
            // ...
            
            if (p) free(p);
            return 2; // Signal redraw needed for hover state
        } else if (op == 3) { // button
            uint32_t btn = p[2];
            uint32_t state = p[3];
            fprintf(stderr, "[DEBUG POINTER] Button: btn=%u, state=%u\n", btn, state);
            if (btn == 272) { // Left mouse button
                g_click_state = (state == 1) ? 1 : 0;
            }
            
            char cmd_buf[128];
            if (state == 1) {
                snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_DOWN %u", btn);
            } else {
                snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_UP %u", btn);
            }
            send_input_command(cmd_buf);
            
            // Handled via the WinchesterMQ in-memory Node.js subscriber
            
            if (p) free(p);
            return 2; // Signal redraw needed for click state
        } else if (op == 4) { // axis
            uint32_t axis = p[1];
            double value = ((int32_t)p[2]) / 256.0;
            fprintf(stderr, "[DEBUG POINTER] Axis: axis=%u, value=%f\n", axis, value);
            
            char cmd_buf[128];
            snprintf(cmd_buf, sizeof(cmd_buf), "MOUSE_SCROLL %u %f", axis, value);
            send_input_command(cmd_buf);
            
            // Handled via the WinchesterMQ in-memory Node.js subscriber
        }
    } else if (keyboard_id && obj == keyboard_id) {
        if (op == 3) { // key
            uint32_t key = p[2];
            uint32_t state = p[3];
            fprintf(stderr, "[DEBUG KEYBOARD] key=%u, state=%u\n", key, state);
            if (state == 1) {
                char cmd_buf[128];
                snprintf(cmd_buf, sizeof(cmd_buf), "KEY_DOWN %u", key + 8);
                send_input_command(cmd_buf);
                
                // Handled via the WinchesterMQ in-memory Node.js subscriber
                
                if (key == 1) { // ESC key
                    printf("[Auncient Presenter] ESC pressed. Exiting gracefully.\n");
                    fflush(stdout);
                    exit(0);
                }
            } else {
                char cmd_buf[128];
                snprintf(cmd_buf, sizeof(cmd_buf), "KEY_UP %u", key + 8);
                send_input_command(cmd_buf);
                
                // Handled via the WinchesterMQ in-memory Node.js subscriber
            }
            if (p) free(p);
            return 2;
        }
    }
    if (p) free(p);
    return 1;
}

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

    while (1) {
        // Poll two file descriptors with a 10ms timeout
        int ret = poll(fds, 2, 10);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        // 1. Process Wayland connection events immediately (keeps window responsive to ping/inputs)
        if (fds[0].revents & POLLIN) {
            bool conf = false;
            bool event_triggered = false;
            int pev;
            while ((pev = process_events(fd, xsurf, &conf)) > 0) {
                if (pev > 0) {
                    event_triggered = true;
                }
            }
            if (conf || event_triggered) {
                if (last_jpeg_sz > 0) {
                    decode_jpeg_zero_copy(last_jpeg_buf, last_jpeg_sz, g_scanout_px, g_w, g_h);
                }
                update_and_present(fd, surf, bid_val, true);
            }
        }

        // 2. Read incoming JPEG frame buffer from stdin
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
                            if (jpeg_len > sizeof(input_buf)) {
                                fprintf(stderr, "[Auncient Presenter ERR] Frame length %u exceeds buffer size!\n", jpeg_len);
                                goto out;
                            }
                            stream_state = STATE_READ_DATA;
                            target_len = jpeg_len;
                            total_read = 0;
                        }
                    } else if (stream_state == STATE_READ_DATA) {
                        if (decode_jpeg_zero_copy(input_buf, jpeg_len, g_scanout_px, g_w, g_h)) {
                            if (jpeg_len <= sizeof(last_jpeg_buf)) {
                                memcpy(last_jpeg_buf, input_buf, jpeg_len);
                                last_jpeg_sz = jpeg_len;
                            }
                            update_and_present(fd, surf, bid_val, true);
                        }
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
        update_and_present(fd, surf, bid_val, false);
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

int main(int argc, char **argv) {
    is_projector = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--projector") == 0) {
            is_projector = true;
        }
    }
    if (!is_projector) {
        g_dest_x = 0;
        g_dest_y = 0;
        g_dest_w = g_w;
        g_dest_h = g_h;
    }
    printf("[Auncient Presenter] Initializing Vulkan and Wayland loops... (projector=%d)\n", is_projector);

    pthread_t presenter_thread_handle;
    if (pthread_create(&presenter_thread_handle, NULL, run_presenter_thread, NULL) != 0) {
        fprintf(stderr, "[Auncient Presenter ERR] Failed to create presenter loop thread!\n");
        return 1;
    }
    pthread_detach(presenter_thread_handle);

    // Run Node.js on the main process thread to satisfy V8 engine threading constraints
    printf("[Auncient Presenter] Spawning embedded Node.js engine on main thread...\n");
    char* args[] = { (char*)"node", (char*)"scripts/embedded_browser_controller.js", NULL };
    start_embedded_node(2, args);
    printf("[Auncient Presenter] Embedded Node.js engine exited.\n");
    return 0;
}
