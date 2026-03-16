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
#include <netinet/in.h>
#include <arpa/inet.h>

// Wayland Opcodes
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

// PM4 Packet Type 3
#define PM4_PKT3(opcode, count) ((3U << 30) | (((count) & 0x3FFF) << 16) | (((opcode) & 0xFF) << 8))
#define PKT3_DRAW_RECT 0x2A
#define PKT3_DRAW_OVAL 0x2B

// TSFi Registry
#define REGISTRY_SHM_NAME "/tsfi_global_registry"
typedef struct {
    uint64_t last_heartbeat_ns;
    uint64_t firmware_cycle_count;
    uint64_t vulkan_vram_usage;
    uint64_t deepseek_ppo_epochs;
    int system_status_flag; 
    char latest_gemini_directive[256];
    char latest_deepseek_observation[256];
} TsfiGlobalRegistry;

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
    size_t plen = sz - 8; uint32_t *p = plen ? (uint32_t*)malloc(plen) : NULL;
    if (p) {
        if (recv(fd, p, plen, 0) <= 0) { free(p); return false; }
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
    } else if (obj == global_xdg_id && op == 0) {
        // xdg_wm_base.ping -> respond with pong
        uint32_t pong[] = {p[0]};
        send_msg(fd, global_xdg_id, 3, pong, 4, -1); // xdg_wm_base.pong opcode is 3
    }
    if (p) free(p);
    return conf;
}

// Emulate a PM4 graphics engine executing a command stream into the Wayland memfd
void execute_pm4_stream(uint32_t *pm4_stream, uint32_t packet_count, uint32_t *pixels, int w, int h) {
    uint32_t pc = 0;
    while(pc < packet_count) {
        uint32_t header = pm4_stream[pc++];
        uint32_t opcode = (header >> 8) & 0xFF;
        uint32_t count = (header >> 16) & 0x3FFF;
        
        if ((opcode == PKT3_DRAW_RECT || opcode == PKT3_DRAW_OVAL) && pc + count <= packet_count) {
            uint32_t x = pm4_stream[pc++];
            uint32_t y = pm4_stream[pc++];
            uint32_t rect_w = pm4_stream[pc++];
            uint32_t rect_h = pm4_stream[pc++];
            uint32_t color = pm4_stream[pc++];
            
            if (opcode == PKT3_DRAW_RECT) {
                for (uint32_t ry = y; ry < y + rect_h && ry < (uint32_t)h; ry++) {
                    for (uint32_t rx = x; rx < x + rect_w && rx < (uint32_t)w; rx++) {
                        pixels[ry * w + rx] = color;
                    }
                }
            } else {
                // Draw Oval
                float cx = x + rect_w / 2.0f;
                float cy = y + rect_h / 2.0f;
                float rw = rect_w / 2.0f;
                float rh = rect_h / 2.0f;
                for (uint32_t ry = y; ry < y + rect_h && ry < (uint32_t)h; ry++) {
                    for (uint32_t rx = x; rx < x + rect_w && rx < (uint32_t)w; rx++) {
                        float dx = (rx - cx) / rw;
                        float dy = (ry - cy) / rh;
                        if (dx*dx + dy*dy <= 1.0f) {
                            pixels[ry * w + rx] = color;
                        }
                    }
                }
            }
        } else {
            pc += count; // Skip unknown packets
        }
    }
}

int main() {
    printf("=== TSFi Wayland PM4 Visual Matrix ===\n"); fflush(stdout);
    
    // Connect to DeepSeek / TSFi Registry
    int shm_fd = shm_open(REGISTRY_SHM_NAME, O_RDWR, 0666);
    TsfiGlobalRegistry *reg = NULL;
    if (shm_fd >= 0) {
        reg = mmap(NULL, sizeof(TsfiGlobalRegistry), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        printf("[INFO] Connected to TSFi Trilateral Registry.\n"); fflush(stdout);
    }
    (void)reg; // Explicit cast to silence unused-variable warning if AI takes total control

    // Ping DeepSeek via UDP to generate an observation for the display
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ai_addr;
    memset(&ai_addr, 0, sizeof(ai_addr));
    ai_addr.sin_family = AF_INET;
    ai_addr.sin_port = htons(10044);
    inet_pton(AF_INET, "127.0.0.1", &ai_addr.sin_addr);
    const char* msg = "PM4_WAYLAND_INIT";
    sendto(udp_sock, msg, strlen(msg), 0, (struct sockaddr*)&ai_addr, sizeof(ai_addr));

    int w = 800, h = 600, str = w * 4; size_t sz = str * h;
    int mfd = memfd_create("tsfi_pm4", MFD_CLOEXEC);
    if (mfd < 0) {
        printf("[FAIL] Cannot allocate Trilateral Framebuffer.\n"); fflush(stdout);
        return 1;
    }
    if (ftruncate(mfd, sz) < 0) return 1;
    uint32_t *px = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);

    // Create a secondary observation file strictly for the Python Vision script
    int obs_fd = open("/dev/shm/tsfi_wayland_fb", O_CREAT | O_RDWR, 0666);
    ftruncate(obs_fd, sz);
    uint32_t *obs_px = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, obs_fd, 0);

    // Wayland Setup (Graceful Headless Degradation)
    int fd = -1;
    uint32_t surf = 0, xsurf = 0;
    const char *run = getenv("XDG_RUNTIME_DIR"), *disp = getenv("WAYLAND_DISPLAY");
    if (run && disp) {
        char path[108]; snprintf(path, 108, "%s/%s", run, disp);
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr = {0}; addr.sun_family = AF_UNIX;
        snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            uint32_t reg_args[] = {WL_REGISTRY_ID}; send_msg(fd, WL_DISPLAY_ID, WL_DISPLAY_GET_REGISTRY, reg_args, 4, -1);
            for (int i = 0; i < 100; i++) { process_events(fd, 0); if (global_xdg_id) break; usleep(10000); }
            if (global_xdg_id) {
                uint32_t cid = next_id++, sid = next_id++, xid = next_id++;
                uint32_t *b = (uint32_t*)malloc(256); int off;
                off = 0; b[off++] = global_compositor_id; off += pack_string(&b[off], "wl_compositor"); b[off++] = 4; b[off++] = cid;
                send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
                off = 0; b[off++] = global_shm_id; off += pack_string(&b[off], "wl_shm"); b[off++] = 1; b[off++] = sid;
                send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
                off = 0; b[off++] = global_xdg_id; off += pack_string(&b[off], "xdg_wm_base"); b[off++] = 1; b[off++] = xid;
                send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);

                surf = next_id++; xsurf = next_id++; uint32_t top = next_id++;
                uint32_t s_args[] = {surf}; send_msg(fd, cid, WL_COMPOSITOR_CREATE_SURFACE, s_args, 4, -1);
                uint32_t xs_args[] = {xsurf, surf}; send_msg(fd, xid, XDG_WM_BASE_GET_XDG_SURFACE, xs_args, 8, -1);
                uint32_t t_args[] = {top}; send_msg(fd, xsurf, XDG_SURFACE_GET_TOPLEVEL, t_args, 4, -1);

                uint32_t pid = next_id++, bid = next_id++;
                uint32_t p_args[] = {pid, (uint32_t)sz}; send_msg(fd, sid, WL_SHM_CREATE_POOL, p_args, 8, mfd);
                uint32_t bf_args[] = {bid, 0, w, h, str, 1}; send_msg(fd, pid, WL_SHM_POOL_CREATE_BUFFER, bf_args, 24, -1);

                // Attach the buffer ONCE before the execution loop, identically to raw_wayland.c
                uint32_t a_args[] = {bid, 0, 0}; send_msg(fd, surf, WL_SURFACE_ATTACH, a_args, 12, -1);
                send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
                printf("[RAW] Wayland Compositor Bound. Initial Buffer Attached.\n"); fflush(stdout);
            }
        }
    }
    
    if (fd < 0 || !global_xdg_id) {
        printf("[RAW] Compositor Offline. Proceeding in Headless PM4 Matrix Mode.\n"); fflush(stdout);
    }

    printf("[RAW] Dispatching Autonomous PM4 Stream...\n"); fflush(stdout);
    alarm(600); 
    
    int pm4_shm_fd = shm_open("/tsfi_pm4_stream", O_RDWR, 0666);
    uint32_t *pm4_map = NULL;
    if (pm4_shm_fd >= 0) {
        pm4_map = mmap(NULL, 8192, PROT_READ | PROT_WRITE, MAP_SHARED, pm4_shm_fd, 0);
        if (pm4_map == MAP_FAILED) pm4_map = NULL;
    }
    
    int frame = 0;
    while(1) {
        // Strictly mirror raw_wayland.c event loop
        if (process_events(fd, xsurf)) {
            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
        }
        
        memset(px, 0, sz); 
        
        bool ai_rendered = false;
        if (pm4_map != NULL) {
            uint32_t packet_count = pm4_map[0];
            if (packet_count > 0 && packet_count < 2000) {
                execute_pm4_stream(&pm4_map[1], packet_count, px, w, h);
                ai_rendered = true;
            }
        }
        
        if (!ai_rendered) {
            uint32_t placeholder[12];
            int pc = 0;
            uint32_t pulse_color = (frame % 60 < 30) ? 0xAA00AAFF : 0x5500AAFF;
            placeholder[pc++] = PM4_PKT3(PKT3_DRAW_RECT, 5);
            placeholder[pc++] = (800 / 2) - 50; 
            placeholder[pc++] = (600 / 2) - 50; 
            placeholder[pc++] = 100; 
            placeholder[pc++] = 100; 
            placeholder[pc++] = pulse_color;
            
            int orbit_x = (800 / 2) - 10 + (frame % 200) - 100;
            placeholder[pc++] = PM4_PKT3(PKT3_DRAW_RECT, 5);
            placeholder[pc++] = orbit_x; 
            placeholder[pc++] = 400; 
            placeholder[pc++] = 20; 
            placeholder[pc++] = 20; 
            placeholder[pc++] = 0xFFFF0000; 
            
            execute_pm4_stream(placeholder, pc, px, w, h);
        }
        
        if (obs_px) memcpy(obs_px, px, sz);
        
        // Strictly mirror raw_wayland.c: Just commit the surface without sending explicit damage packets.
        send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
        
        frame++;
        usleep(16000); // 60fps
    }

    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
