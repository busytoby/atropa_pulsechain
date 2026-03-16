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
#include <termios.h>
#include <poll.h>

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
        uint32_t pong[] = {p[0]};
        send_msg(fd, global_xdg_id, 3, pong, 4, -1); 
    }
    if (p) free(p);
    return conf;
}

// Geometric rendering loop simulating PM4 structures for the Teddy Bear
void render_sick_teddy_bear(uint32_t *px, int w, int h, int frame) {
    memset(px, 0, w * h * 4); // Clear screen to void (black)
    
    // Base Colors
    uint32_t brown = 0xFF5C4033;
    uint32_t dark_brown = 0xFF3E2723;
    uint32_t sick_green = 0xFF556B2F;
    uint32_t eye_color = (frame % 60 < 30) ? 0xFF00FF00 : 0xFF005500; // Kr0wZ Green pulse
    
    // Head (16 inch canvas = massive geometry)
    for(int y=200; y<600; y++) {
        for(int x=300; x<700; x++) {
            // Rough circle equation
            int dx = x - 500; int dy = y - 400;
            if (dx*dx + dy*dy < 40000) px[y*w + x] = brown;
        }
    }
    
    // Ears
    for(int y=100; y<300; y++) {
        for(int x=250; x<400; x++) {
            int dx = x - 325; int dy = y - 200;
            if (dx*dx + dy*dy < 8000) px[y*w + x] = dark_brown;
        }
        for(int x=600; x<750; x++) {
            int dx = x - 675; int dy = y - 200;
            if (dx*dx + dy*dy < 8000) px[y*w + x] = dark_brown;
        }
    }
    
    // Eyes (Pulsing Telemetry)
    for(int y=300; y<380; y++) {
        for(int x=400; x<480; x++) {
            int dx = x - 440; int dy = y - 340;
            if (dx*dx + dy*dy < 1000) px[y*w + x] = eye_color; // Left Eye
        }
        for(int x=520; x<600; x++) {
            int dx = x - 560; int dy = y - 340;
            if (dx*dx + dy*dy < 1000) px[y*w + x] = 0xFFFF0000; // Right Eye (Fractured Red)
        }
    }
    
    // The Sick Body (Corrupted PM4 Geometry)
    for(int y=550; y<1200; y++) {
        for(int x=250; x<750; x++) {
            int dx = x - 500; int dy = y - 875;
            if (dx*dx*0.6 + dy*dy < 100000) {
                // Sickness noise overlay
                if ((x * y * frame) % 100 < 5) px[y*w + x] = sick_green;
                else px[y*w + x] = brown;
            }
        }
    }
}

int main() {
    printf("=== TSFi Atropa/999 Sovereign Choreography ===\\n");
    printf("[INFO] Synthesizing 16-Inch (1536px) Wayland Canvas...\\n");
    printf("[INFO] Press ESC in the terminal to securely terminate the window.\\n"); fflush(stdout);
    
    // 16 Inch Canvas (At 96 DPI, ~1536 pixels high)
    int w = 1024, h = 1536, str = w * 4; size_t sz = str * h;
    int mfd = memfd_create("tsfi_teddy", MFD_CLOEXEC);
    if (mfd < 0) return 1;
    if (ftruncate(mfd, sz) < 0) return 1;
    uint32_t *px = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);

    // Wayland Setup
    const char *run = getenv("XDG_RUNTIME_DIR"), *disp = getenv("WAYLAND_DISPLAY");
    if (!run) return 1;
    if (!disp) disp = "wayland-0";
    char path[108]; snprintf(path, 108, "%s/%s", run, disp);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0}; addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) return 1;

    uint32_t reg_args[] = {WL_REGISTRY_ID}; send_msg(fd, WL_DISPLAY_ID, WL_DISPLAY_GET_REGISTRY, reg_args, 4, -1);
    for (int i = 0; i < 100; i++) { process_events(fd, 0); if (global_xdg_id) break; usleep(10000); }
    if (!global_xdg_id) return 1;

    uint32_t cid = next_id++, sid = next_id++, xid = next_id++;
    uint32_t *b = (uint32_t*)malloc(256); int off;
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

    uint32_t pid = next_id++, bid = next_id++;
    uint32_t p_args[] = {pid, (uint32_t)sz}; send_msg(fd, sid, WL_SHM_CREATE_POOL, p_args, 8, mfd);
    uint32_t bf_args[] = {bid, 0, w, h, str, 1}; send_msg(fd, pid, WL_SHM_POOL_CREATE_BUFFER, bf_args, 24, -1);

    uint32_t a_args[] = {bid, 0, 0}; send_msg(fd, surf, WL_SURFACE_ATTACH, a_args, 12, -1);
    send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);

    // Setup non-blocking terminal read for ESC key
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    int frame = 0;
    while(1) {
        // Detect ESC Key (ASCII 27)
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1 && c == 27) {
            printf("\\n[TERMINATE] ESC key registered. Shattering window geometry.\\n");
            break;
        }

        if (process_events(fd, xsurf)) {
            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
        }
        
        // Render the 16-Inch DeepSeek RAG Entity
        render_sick_teddy_bear(px, w, h, frame);
        
        send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
        
        frame++;
        usleep(16000); // 60fps
    }

    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
