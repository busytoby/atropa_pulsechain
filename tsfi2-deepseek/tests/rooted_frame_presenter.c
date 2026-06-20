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

#include "lau_memory.h"
#include "tsfi_raw.h"
#include "window_inc/vulkan_struct.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

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

static uint32_t seat_id = 0;
static uint32_t pointer_id = 0;
static uint32_t keyboard_id = 0;
static uint32_t xdg_wm_base_id = 0;

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

bool decode_jpeg(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
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
    int row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    while (cinfo.output_scanline < cinfo.output_height) {
        int y = cinfo.output_scanline;
        if (y >= height) break;
        jpeg_read_scanlines(&cinfo, buffer, 1);
        uint8_t *src = buffer[0];
        uint32_t *dst = scanout_px + y * width;
        for (int x = 0; x < width && x < (int)cinfo.output_width; x++) {
            uint8_t r = src[x * 3];
            uint8_t g = src[x * 3 + 1];
            uint8_t b = src[x * 3 + 2];
            dst[x] = (r << 16) | (g << 8) | b;
        }
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return true;
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
    } else if (pointer_id && obj == pointer_id) {
        if (op == 2) { // motion
            double x = ((int32_t)p[1]) / 256.0;
            double y = ((int32_t)p[2]) / 256.0;
            printf("MOUSE_MOVE %d %d\n", (int)x, (int)y);
            fflush(stdout);
        } else if (op == 3) { // button
            uint32_t btn = p[2];
            uint32_t state = p[3];
            if (state == 1) {
                printf("MOUSE_DOWN %u\n", btn);
            } else {
                printf("MOUSE_UP %u\n", btn);
            }
            fflush(stdout);
        }
    } else if (keyboard_id && obj == keyboard_id) {
        if (op == 3) { // key
            uint32_t key = p[2];
            uint32_t state = p[3];
            if (state == 1) {
                printf("KEY_DOWN %u\n", key);
            } else {
                printf("KEY_UP %u\n", key);
            }
            fflush(stdout);
        }
    }
    if (p) free(p);
    return 1;
}

int main() {
    printf("[Auncient Presenter] Initializing Vulkan and Wayland loops...\n");

    // --- 1. Load Firmware ZMM ---
    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkGetDeviceQueue pvkGetDeviceQueue = (PFN_vkGetDeviceQueue)tsfi_vkGetInstanceProcAddr(NULL, "vkGetDeviceQueue");
    PFN_vkQueuePresentKHR pvkQueuePresentKHR = (PFN_vkQueuePresentKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkQueuePresentKHR");
    
    PFN_tsfi_drmModeAddPlane ptsfi_drmModeAddPlane = (PFN_tsfi_drmModeAddPlane)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_drmModeAddPlane");
    PFN_tsfi_drmModeGetVirtualPlaneBuffer ptsfi_drmModeGetVirtualPlaneBuffer = (PFN_tsfi_drmModeGetVirtualPlaneBuffer)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_drmModeGetVirtualPlaneBuffer");
    PFN_tsfi_zmm_set_scanout_buffer ptsfi_zmm_set_scanout_buffer = (PFN_tsfi_zmm_set_scanout_buffer)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_zmm_set_scanout_buffer");
    PFN_tsfi_drmModeFreeVirtualPlanes ptsfi_drmModeFreeVirtualPlanes = (PFN_tsfi_drmModeFreeVirtualPlanes)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_drmModeFreeVirtualPlanes");

    VkInstance instance; VkInstanceCreateInfo inst_info = {0}; pvkCreateInstance(&inst_info, NULL, &instance);
    VkDevice device; VkDeviceCreateInfo dev_info = {0}; pvkCreateDevice((VkPhysicalDevice)0x2000, &dev_info, NULL, &device);
    VkQueue queue; pvkGetDeviceQueue(device, 0, 0, &queue);

    // --- 2. Setup Wayland Connection ---
    const char *run = getenv("XDG_RUNTIME_DIR"), *disp = getenv("WAYLAND_DISPLAY");
    if (!run) return 1; 
    if (!disp) disp = "wayland-0";
    char path[108]; snprintf(path, 108, "%s/%s", run, disp);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0}; addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 1;

    uint32_t reg_args[] = {WL_REGISTRY_ID}; send_msg(fd, WL_DISPLAY_ID, WL_DISPLAY_GET_REGISTRY, reg_args, 4, -1);
    for (int i = 0; i < 100; i++) { process_events(fd, 0, NULL); tsfi_raw_usleep(2000); }
    if (!global_xdg_id) return 1;

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
    }

    uint32_t surf = next_id++, xsurf = next_id++, top = next_id++;
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

    int w = 800, h = 600, str = w * 4; size_t sz = str * h;
    int mfd = memfd_create("tsfi_scanout", MFD_CLOEXEC);
    if (mfd < 0 || ftruncate(mfd, sz) < 0) return 1;
    uint32_t *scanout_px = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);
    
    uint32_t pid = next_id++, bid = next_id++;
    uint32_t p_args[] = {pid, (uint32_t)sz}; send_msg(fd, sid, WL_SHM_CREATE_POOL, p_args, 8, mfd);
    uint32_t bf_args[] = {bid, 0, w, h, str, 1}; send_msg(fd, pid, WL_SHM_POOL_CREATE_BUFFER, bf_args, 24, -1); // 1 = XRGB8888
    
    // Initial commit must NOT have a buffer attached per XDG shell spec
    send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);

    // --- 3. Link Firmware ZMM to Wayland Scanout ---
    ptsfi_zmm_set_scanout_buffer(scanout_px, w, h);
    ptsfi_drmModeAddPlane(72, sz);
    uint32_t* p72 = (uint32_t*)ptsfi_drmModeGetVirtualPlaneBuffer(72);
    (void)p72;

    VkSwapchainKHR mock_swapchain = (VkSwapchainKHR)0xB000;
    uint32_t imageIndex = 0;
    VkPresentInfoKHR presentInfo = { .swapchainCount = 1, .pSwapchains = &mock_swapchain, .pImageIndices = &imageIndex };

    printf("[Auncient Presenter] Waiting for initial Wayland configure event...\n");
    bool initial_configured = false;
    for (int i = 0; i < 100; i++) {
        process_events(fd, xsurf, &initial_configured);
        if (initial_configured) {
            // Attach buffer and commit now that configure has been acknowledged
            uint32_t a_args[] = {bid, 0, 0};
            send_msg(fd, surf, WL_SURFACE_ATTACH, a_args, 12, -1);
            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
            break;
        }
        tsfi_raw_usleep(10000);
    }

    printf("[Auncient Presenter] Frame presenter ready. Streaming starting...\n");
    fflush(stdout);

    // Make stdin non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    struct pollfd fds[2] = {
        { .fd = fd, .events = POLLIN },
        { .fd = STDIN_FILENO, .events = POLLIN }
    };

    enum {
        STATE_READ_LEN,
        STATE_READ_DATA
    } stream_state = STATE_READ_LEN;

    size_t target_len = 4;
    size_t total_read = 0;
    static uint8_t input_buf[2 * 1024 * 1024]; // 2 MB static buffer
    uint32_t jpeg_len = 0;

    while (1) {
        // Poll both file descriptors with a 10ms timeout
        int ret = poll(fds, 2, 10);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        // 1. Process Wayland connection events immediately (keeps window responsive to ping/inputs)
        if (fds[0].revents & POLLIN) {
            bool conf = false;
            while (process_events(fd, xsurf, &conf) > 0);
            if (conf) {
                send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
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
                        if (jpeg_len > sizeof(input_buf)) {
                            fprintf(stderr, "[Auncient Presenter ERR] Frame length %u exceeds buffer size!\n", jpeg_len);
                            goto out;
                        }
                        stream_state = STATE_READ_DATA;
                        target_len = jpeg_len;
                        total_read = 0;
                    } else {
                        if (decode_jpeg(input_buf, jpeg_len, scanout_px, w, h)) {
                            pvkQueuePresentKHR(queue, &presentInfo);

                            uint32_t attach_args[] = {bid, 0, 0};
                            send_msg(fd, surf, WL_SURFACE_ATTACH, attach_args, 12, -1);

                            uint32_t damage[] = {0, 0, w, h};
                            send_msg(fd, surf, WL_SURFACE_DAMAGE, damage, 16, -1);
                            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
                        }
                        stream_state = STATE_READ_LEN;
                        target_len = 4;
                        total_read = 0;
                    }
                }
            }
        }
    }

out:
    printf("[Auncient Presenter] Cleaning up virtual planes...\n");
    ptsfi_drmModeFreeVirtualPlanes();
    free(b);
    
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    pvkDestroyDevice(device, NULL);
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    pvkDestroyInstance(instance, NULL);

    extern void tsfi_dl_thunks_teardown(void); tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void); lau_registry_teardown();
    extern void lau_free_all_active(void);
    lau_free_all_active();
    return 0;
}
