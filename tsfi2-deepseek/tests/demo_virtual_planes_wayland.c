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
#define XDG_SURFACE_ACK_CONFIGURE 3

#define ALIGN(x) (((x) + 3) & ~3)

static uint32_t global_compositor_id = 0, global_shm_id = 0, global_xdg_id = 0;
static uint32_t next_id = 3; 

int pack_string(uint32_t *buf, const char *s) {
    uint32_t len = strlen(s) + 1;
    buf[0] = len; memcpy(&buf[1], s, len);
    return 1 + (ALIGN(len) / 4);
}

void send_msg(int fd, uint32_t obj, uint16_t op, void *data, size_t len, int s_fd) {
    uint32_t head[2] = {obj, ((uint32_t)(8 + len) << 16) | op};
    struct iovec iov[2] = {{head, 8}, {data, len}};
    struct msghdr msg = {0}; msg.msg_iov = iov; msg.msg_iovlen = len ? 2 : 1;
    if (s_fd != -1) {
        char c[CMSG_SPACE(sizeof(int))]; msg.msg_control = c; msg.msg_controllen = sizeof(c);
        struct cmsghdr *cm = CMSG_FIRSTHDR(&msg);
        cm->cmsg_level = SOL_SOCKET; cm->cmsg_type = SCM_RIGHTS; cm->cmsg_len = CMSG_LEN(sizeof(int));
        *((int *)CMSG_DATA(cm)) = s_fd;
    }
    sendmsg(fd, &msg, MSG_NOSIGNAL);
}

bool process_events(int fd, uint32_t xdg_s_id) {
    uint32_t h[2]; if (recv(fd, h, 8, MSG_DONTWAIT) <= 0) return false;
    uint32_t obj = h[0], sz = h[1] >> 16, op = h[1] & 0xFFFF;
    size_t plen = sz - 8; uint32_t *p = plen ? (uint32_t*)lau_malloc(plen) : NULL;
    if (p) {
        if (recv(fd, p, plen, 0) <= 0) { lau_free(p); return false; }
    }
    bool conf = false;
    if (obj == WL_REGISTRY_ID && op == 0) {
        char *iface = (char *)&p[2];
        if (!strcmp(iface, "wl_compositor")) global_compositor_id = p[0];
        else if (!strcmp(iface, "wl_shm")) global_shm_id = p[0];
        else if (!strcmp(iface, "xdg_wm_base")) global_xdg_id = p[0];
    } else if (obj == xdg_s_id && op == 0) {
        uint32_t ack[] = {p[0]};
        send_msg(fd, xdg_s_id, XDG_SURFACE_ACK_CONFIGURE, ack, 4, -1);
        conf = true;
    }
    if (p) lau_free(p);
    return conf;
}

static void draw_circle(uint32_t* buf, int w, int h, int cx, int cy, int radius, uint32_t color) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (dx*dx + dy*dy <= radius*radius) {
                buf[y * w + x] = color;
            }
        }
    }
}

int main() {
    alarm(5);
    printf("[DEMO] TSFi Virtual Planes to Wayland Screen\n");

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
    for (int i = 0; i < 100; i++) { process_events(fd, 0); if (global_xdg_id) break; tsfi_raw_usleep(10000); }
    if (!global_xdg_id) return 1;

    uint32_t cid = next_id++, sid = next_id++, xid = next_id++;
    uint32_t *b = (uint32_t*)lau_malloc(256); int off;
    off = 0; b[off++] = global_compositor_id; off += pack_string(&b[off], "wl_compositor"); b[off++] = 4; b[off++] = cid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
    off = 0; b[off++] = global_shm_id; off += pack_string(&b[off], "wl_shm"); b[off++] = 1; b[off++] = sid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
    off = 0; b[off++] = global_xdg_id; off += pack_string(&b[off], "xdg_wm_base"); b[off++] = 1; b[off++] = xid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);

    uint32_t surf = next_id++, xsurf = next_id++, top = next_id++;
    uint32_t s_args[] = {surf}; send_msg(fd, cid, WL_COMPOSITOR_CREATE_SURFACE, s_args, 4, -1);
    uint32_t xs_args[] = {xsurf, surf}; send_msg(fd, xid, XDG_WM_BASE_GET_XDG_SURFACE, xs_args, 8, -1);
    uint32_t t_args[] = {top}; send_msg(fd, xsurf, XDG_SURFACE_GET_TOPLEVEL, t_args, 4, -1);

    int w = 800, h = 600, str = w * 4; size_t sz = str * h;
    int mfd = memfd_create("tsfi_scanout", MFD_CLOEXEC);
    if (mfd < 0 || ftruncate(mfd, sz) < 0) return 1;
    uint32_t *scanout_px = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);
    
    uint32_t pid = next_id++, bid = next_id++;
    uint32_t p_args[] = {pid, (uint32_t)sz}; send_msg(fd, sid, WL_SHM_CREATE_POOL, p_args, 8, mfd);
    uint32_t bf_args[] = {bid, 0, w, h, str, 0}; send_msg(fd, pid, WL_SHM_POOL_CREATE_BUFFER, bf_args, 24, -1); // 0 = ARGB8888
    uint32_t a_args[] = {bid, 0, 0}; send_msg(fd, surf, WL_SURFACE_ATTACH, a_args, 12, -1);
    send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);

    // --- 3. Link Firmware ZMM to Wayland Scanout ---
    ptsfi_zmm_set_scanout_buffer(scanout_px, w, h);
    ptsfi_drmModeAddPlane(72, sz);
    ptsfi_drmModeAddPlane(73, sz);
    ptsfi_drmModeAddPlane(74, sz);
    
    uint32_t* p72 = (uint32_t*)ptsfi_drmModeGetVirtualPlaneBuffer(72);
    uint32_t* p73 = (uint32_t*)ptsfi_drmModeGetVirtualPlaneBuffer(73);
    uint32_t* p74 = (uint32_t*)ptsfi_drmModeGetVirtualPlaneBuffer(74);

    VkSwapchainKHR mock_swapchain = (VkSwapchainKHR)0xB000;
    uint32_t imageIndex = 0;
    VkPresentInfoKHR presentInfo = { .swapchainCount = 1, .pSwapchains = &mock_swapchain, .pImageIndices = &imageIndex };

    
    printf("[DEMO] Waiting for initial Wayland configure event...\n");
    for (int i = 0; i < 100; i++) {
        if (process_events(fd, xsurf)) {
            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
            break;
        }
        tsfi_raw_usleep(10000);
    }

    printf("[DEMO] Rendering loop starting. Check your screen!\n"); fflush(stdout);

    
    // Animate for 60 frames (~1 second) to prove it works
    for (int f = 0; f < 100; f++) {
        if (process_events(fd, xsurf)) {
            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
        }
        
        // Clear virtual planes
        memset(p72, 0, sz); memset(p73, 0, sz); memset(p74, 0, sz);
        

        // Draw to virtual planes independently
        draw_circle(p72, w, h, 200 + (f * 4), 300, 100, 0xFFFF0000); // Red on 72
        draw_circle(p73, w, h, 400, 300 + (sin(f * 0.1) * 100), 100, 0xFF00FF00); // Green on 73
        draw_circle(p74, w, h, 600 - (f * 4), 300, 100, 0xFF0000FF); // Blue on 74


        // Tell Firmware to Blend & Present
        pvkQueuePresentKHR(queue, &presentInfo);

        // Tell Wayland to flip the screen
        uint32_t damage[] = {0, 0, w, h};
        send_msg(fd, surf, WL_SURFACE_DAMAGE, damage, 16, -1);
        send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
        
        tsfi_raw_usleep(16000); // ~60fps
    }

    printf("[DEMO] Finished.\n");
    ptsfi_drmModeFreeVirtualPlanes();
    lau_free(b);
    
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    pvkDestroyDevice(device, NULL);
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    pvkDestroyInstance(instance, NULL);

    extern void tsfi_dl_thunks_teardown(void); tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void); lau_registry_teardown();
    extern void lau_report_memory_metrics(void); extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
