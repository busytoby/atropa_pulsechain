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
#include <math.h>

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

// Particle definition for 4D Verlet Integration
typedef struct {
    float x, y, z, w;
    float px, py, pz, pw;
    float radius;
    uint32_t color;
} Particle;

// Distance constraints between body parts
typedef struct {
    int p1, p2;
    float target_dist;
} DistanceConstraint;

// Slugcat-style Teddy Bear Skeleton Structure (Slender & Floppy):
// 0: Torso (Chest)
// 1: Hips (Pelvis)
// 2: Head (Cranium)
// 3: Left Arm
// 4: Right Arm
// 5: Left Leg
// 6: Right Leg
// 7: Tail 1 (Base)
// 8: Tail 2 (Mid)
// 9: Tail 3 (Thin)
// 10: Tail 4 (Tip)
// 11: Left Ear
// 12: Right Ear
#define NUM_PARTICLES 11
#define NUM_CONSTRAINTS 15

static Particle particles[NUM_PARTICLES];
static DistanceConstraint constraints[NUM_CONSTRAINTS];
static bool physics_initialized = false;

// Continuous mathematical floor function driving the terrain mesh
float floor_y(float x, int frame) {
    return 1000.0f + 60.0f * sinf(x * 0.006f + frame * 0.02f);
}

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

// Low-level render helpers
void draw_circle(uint32_t *px, int w, int h, float cx, float cy, float r, uint32_t color) {
    int start_x = (int)(cx - r); if (start_x < 0) start_x = 0;
    int end_x = (int)(cx + r); if (end_x >= w) end_x = w - 1;
    int start_y = (int)(cy - r); if (start_y < 0) start_y = 0;
    int end_y = (int)(cy + r); if (end_y >= h) end_y = h - 1;
    
    float r2 = r * r;
    for (int y = start_y; y <= end_y; y++) {
        for (int x = start_x; x <= end_x; x++) {
            float dx = x - cx;
            float dy = y - cy;
            if (dx*dx + dy*dy <= r2) {
                px[y * w + x] = color;
            }
        }
    }
}

void draw_capsule(uint32_t *px, int w, int h, float x1, float y1, float x2, float y2, float r, uint32_t color) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx*dx + dy*dy);
    if (len == 0.0f) {
        draw_circle(px, w, h, x1, y1, r, color);
        return;
    }
    
    float min_x = fminf(x1, x2) - r; if (min_x < 0) min_x = 0;
    float max_x = fmaxf(x1, x2) + r; if (max_x >= w) max_x = w - 1;
    float min_y = fminf(y1, y2) - r; if (min_y < 0) min_y = 0;
    float max_y = fmaxf(y1, y2) + r; if (max_y >= h) max_y = h - 1;
    
    for (int y = (int)min_y; y <= (int)max_y; y++) {
        for (int x = (int)min_x; x <= (int)max_x; x++) {
            float t = ((x - x1) * dx + (y - y1) * dy) / (len * len);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            
            float proj_x = x1 + t * dx;
            float proj_y = y1 + t * dy;
            
            float dist_x = x - proj_x;
            float dist_y = y - proj_y;
            if (dist_x*dist_x + dist_y*dist_y <= r * r) {
                px[y * w + x] = color;
            }
        }
    }
}

// Initializing the Spinning Rigid Bone with hanging Cloth Grid
void init_physics() {
    uint32_t bone_color = 0xFFFF4500;  // Orange-red
    uint32_t cloth_color = 0xFF1E90FF; // Dodger-blue
    
    // 1. Rigid Bone (Particles 0..1)
    particles[0] = (Particle){ -120.0f, 0.0f, 0.0f, 0.0f, -120.0f, 0.0f, 0.0f, 0.0f, 35.0f, bone_color };
    particles[1] = (Particle){ 120.0f, 0.0f, 0.0f, 0.0f, 120.0f, 0.0f, 0.0f, 0.0f, 35.0f, bone_color };
    constraints[0] = (DistanceConstraint){ 0, 1, 240.0f }; // Rigid bone link

    // 2. Cloth grid (Particles 2..10) arranged in 3x3 hanging rows
    float spacing = 60.0f;
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            int idx = 2 + y * 3 + x;
            float px = x * spacing - 60.0f;
            float py = (y + 1) * spacing + 100.0f;
            particles[idx] = (Particle){ px, py, 0.0f, 0.0f, px, py, 0.0f, 0.0f, 18.0f, cloth_color };
        }
    }

    // Attach cloth corners to the bone endpoints dynamically:
    constraints[1] = (DistanceConstraint){ 0, 2, spacing }; // Left bone to cloth top-left
    constraints[2] = (DistanceConstraint){ 1, 4, spacing }; // Right bone to cloth top-right

    // Horizontal & vertical cloth constraints
    int c_idx = 3;
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            int idx = 2 + y * 3 + x;
            if (x < 2) {
                constraints[c_idx++] = (DistanceConstraint){ idx, idx + 1, spacing };
            }
            if (y < 2) {
                constraints[c_idx++] = (DistanceConstraint){ idx, idx + 3, spacing };
            }
        }
    }
    physics_initialized = true;
}

// Global simulated storage registers for Auncient WinchesterMQ Virtual Hardware (Storage[100..105])
static uint32_t wmq_storage[106] = {
    [100] = 0,    // SCSI Status/Handshake Phase (0=IDLE, 1=REQ, 2=ACK, 3=DATA_IN)
    [101] = 0,    // SCSI Data Latch register
    [102] = 0,    // Keycode register (30=a/A, 32=d/D)
    [103] = 1000, // Clock Divisor
    [104] = 2300, // Hyper-perspective projection focal distance
    [105] = 0     // Local loopback transaction counter
};

// 1. Velocity-Less Impulse Injector (Thomas Jakobsen 2001 GDC framework)
void apply_impulse(int particle_index, float fx, float fy, float fz, float fw) {
    // Pushing the previous position backwards simulates a sudden velocity jump forward
    particles[particle_index].px -= fx;
    particles[particle_index].py -= fy;
    particles[particle_index].pz -= fz;
    particles[particle_index].pw -= fw;
}

// 2. Auncient SCSI WinchesterMQ loopback handshake controller
void run_winchester_mq_loopback(int frame) {
    // Generate simulated SCSI keycode events periodically on specific frame intervals
    if (frame % 40 == 10) {
        wmq_storage[102] = 30; // keycode 30 (Left)
        wmq_storage[100] = 1;  // Request (REQ=1)
    } else if (frame % 40 == 30) {
        wmq_storage[102] = 32; // keycode 32 (Right)
        wmq_storage[100] = 1;  // Request (REQ=1)
    }

    // SCSI Handshake Phase processing loop
    if (wmq_storage[100] == 1) { // Host target loopback transaction active
        wmq_storage[100] = 2;    // Acknowledge (ACK=2)
        uint32_t keycode = wmq_storage[102];
        wmq_storage[105]++;      // Increment loopback counter
        
        printf("[WinchesterMQ] SCSI loopback ACK: Keycode %u mapped. Storage[105]=%u\n", 
               keycode, wmq_storage[105]);

        // Route Keycodes via low-level state verification
        if (keycode == 30) {
            apply_impulse(0, -14.0f, -4.0f, 0.0f, 0.0f); // Left impulse to chest
        } else if (keycode == 32) {
            apply_impulse(0, 14.0f, -4.0f, 0.0f, 0.0f);  // Right impulse to chest
        }

        // Deassert & return to idle
        wmq_storage[100] = 0; 
        wmq_storage[102] = 0;
    }
}

// Obstacle definition for environment colliders
typedef struct {
    float cx, cy;
    float radius;
    uint32_t color;
} Obstacle;

#define NUM_OBSTACLES 3
static Obstacle obstacles[NUM_OBSTACLES] = {
    { 120.0f, 100.0f, 80.0f, 0xFF444444 },
    { -150.0f, 0.0f, 60.0f, 0xFF553333 },
    { 0.0f, 250.0f, 70.0f, 0xFF333355 }
};

// 3. Iterative Stick & Capsule Collision Constraints (Thomas Jakobsen 2001 GDC framework)
void handle_stick_obstacle_collision(int p1, int p2, Obstacle *obs) {
    float cx = obs->cx;
    float cy = obs->cy;
    float obstacle_r = obs->radius;

    float x1 = particles[p1].x;
    float y1 = particles[p1].y;
    float x2 = particles[p2].x;
    float y2 = particles[p2].y;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float len2 = dx*dx + dy*dy;
    if (len2 == 0.0f) return;

    // Find closest projection point on segment
    float t = ((cx - x1) * dx + (cy - y1) * dy) / len2;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float proj_x = x1 + t * dx;
    float proj_y = y1 + t * dy;

    float ox = proj_x - cx;
    float oy = proj_y - cy;
    float dist2 = ox*ox + oy*oy;
    float min_dist = obstacle_r + (particles[p1].radius + particles[p2].radius) * 0.25f;

    if (dist2 < min_dist * min_dist && dist2 > 0.0f) {
        float dist = sqrtf(dist2);
        float overlap = min_dist - dist;
        float push_x = (ox / dist) * overlap;
        float push_y = (oy / dist) * overlap;

        // Push particles proportionally outwards based on barycentric position
        if (p1 != 0) {
            particles[p1].x += push_x * (1.0f - t);
            particles[p1].y += push_y * (1.0f - t);
        }
        if (p2 != 5 && p2 != 6) {
            particles[p2].x += push_x * t;
            particles[p2].y += push_y * t;
        }
    }
}

// Procedural 4D Verlet rendering loop
void render_sick_teddy_bear(uint32_t *px, int w, int h, int frame) {
    memset(px, 0, w * h * 4); // Clear screen to void (black)
    
    if (!physics_initialized) {
        init_physics();
    }
    
    uint32_t sick_green = 0xFF556B2F;
    uint32_t active_orange = 0xFFFF4500;

    // Run WinchesterMQ SCSI handler to apply impulses via simulated key events
    run_winchester_mq_loopback(frame);

    // Apply continuous rotational force (spinning torque) to bone endpoints (0 and 1)
    float spin_speed = 0.05f;
    float dx_bone = particles[1].x - particles[0].x;
    float dy_bone = particles[1].y - particles[0].y;
    // Apply perpendicular tangent force to endpoints
    particles[0].px += dy_bone * spin_speed * 0.05f;
    particles[0].py -= dx_bone * spin_speed * 0.05f;
    particles[1].px -= dy_bone * spin_speed * 0.05f;
    particles[1].py += dx_bone * spin_speed * 0.05f;

    // --- Verlet Physics Tick (Applying Forces) ---
    float decay_rate = 0.98f; 
    
    for (int i = 0; i < NUM_PARTICLES; i++) {
        float temp_x = particles[i].x;
        float temp_y = particles[i].y;
        float temp_z = particles[i].z;
        float temp_w = particles[i].w;
        
        // Cloth particles (2..10) react to gravity and wind, bone particles (0..1) spin in orbit
        float wind_x = (i >= 2) ? 0.25f * sinf(frame * 0.08f + i * 0.3f) : 0.0f;
        float gravity_y = (i >= 2) ? 0.35f : 0.0f;
        
        particles[i].x += (particles[i].x - particles[i].px) * decay_rate + wind_x;
        particles[i].y += (particles[i].y - particles[i].py) * decay_rate + gravity_y;
        particles[i].z += (particles[i].z - particles[i].pz) * decay_rate;
        particles[i].w += (particles[i].w - particles[i].pw) * decay_rate;
        
        particles[i].px = temp_x;
        particles[i].py = temp_y;
        particles[i].pz = temp_z;
        particles[i].pw = temp_w;
    }
    
    // --- Constraint & Collision Resolution (8 iterations) ---
    for (int iter = 0; iter < 8; iter++) {
        // Resolve Stick-to-Obstacle Collisions across all loaded obstacles
        for (int i = 0; i < NUM_CONSTRAINTS; i++) {
            for (int j = 0; j < NUM_OBSTACLES; j++) {
                handle_stick_obstacle_collision(constraints[i].p1, constraints[i].p2, &obstacles[j]);
            }
        }

        // Distance constraint projections
        for (int i = 0; i < NUM_CONSTRAINTS; i++) {
            DistanceConstraint c = constraints[i];
            float dx = particles[c.p2].x - particles[c.p1].x;
            float dy = particles[c.p2].y - particles[c.p1].y;
            float dz = particles[c.p2].z - particles[c.p1].z;
            float dw = particles[c.p2].w - particles[c.p1].w;
            
            float dist = sqrtf(dx*dx + dy*dy + dz*dz + dw*dw);
            if (dist == 0.0f) continue;
            
            float diff = c.target_dist - dist;
            
            // Mass ratios: Bone (0, 1) has infinite mass relative to cloth (>= 2)
            float factor_p1 = 0.5f;
            float factor_p2 = 0.5f;

            if (c.p1 < 2 && c.p2 >= 2) {
                factor_p1 = 0.0f; // Bone does not move
                factor_p2 = 1.0f; // Cloth takes 100% correction
            }
            
            float ox = (dx / dist) * diff;
            float oy = (dy / dist) * diff;
            float oz = (dz / dist) * diff;
            float ow = (dw / dist) * diff;
            
            particles[c.p1].x -= ox * factor_p1;
            particles[c.p1].y -= oy * factor_p1;
            particles[c.p1].z -= oz * factor_p1;
            particles[c.p1].w -= ow * factor_p1;

            particles[c.p2].x += ox * factor_p2;
            particles[c.p2].y += oy * factor_p2;
            particles[c.p2].z += oz * factor_p2;
            particles[c.p2].w += ow * factor_p2;
        }

        // Center of mass constraint: Anchor bone rotation around (x=0, y=0)
        float cx = (particles[0].x + particles[1].x) * 0.5f;
        float cy = (particles[0].y + particles[1].y) * 0.5f;
        particles[0].x -= cx; particles[0].y -= cy;
        particles[1].x -= cx; particles[1].y -= cy;
    }
    
    // --- 5. Draw Morphing Terrain Line ---
    for (int x = 0; x < w; x++) {
        int ty = (int)floor_y((float)x, frame);
        if (ty >= 0 && ty < h) {
            for (int dy = -4; dy <= 4; dy++) {
                if (ty + dy >= 0 && ty + dy < h) {
                    px[(ty + dy) * w + x] = 0xFF555555;
                }
            }
        }
    }
    
    // --- 6. Tessarant 4D-to-3D projection (XW Plane Rotation Matrix) ---
    float rot_angle = frame * 0.012f;
    float cos_r = cosf(rot_angle);
    float sin_r = sinf(rot_angle);
    
    float proj_x[NUM_PARTICLES];
    float proj_y[NUM_PARTICLES];
    float proj_r[NUM_PARTICLES];
    
    for (int i = 0; i < NUM_PARTICLES; i++) {
        float rx = particles[i].x * cos_r - particles[i].w * sin_r;
        float rw = particles[i].x * sin_r + particles[i].w * cos_r;
        
        float depth_scale = 1.0f + (particles[i].z + rw * 0.2f) / 600.0f;
        
        proj_x[i] = 500.0f + rx * depth_scale;
        proj_y[i] = 500.0f + particles[i].y * depth_scale;
        proj_r[i] = particles[i].radius * depth_scale;
    }
    
    // --- 6.5 Draw Environmental Obstacles ---
    for (int j = 0; j < NUM_OBSTACLES; j++) {
        draw_circle(px, w, h, 500.0f + obstacles[j].cx, 500.0f + obstacles[j].cy, obstacles[j].radius, obstacles[j].color);
    }
    
    // --- 7. Draw Segment Capsules ---
    for (int i = 0; i < NUM_CONSTRAINTS; i++) {
        DistanceConstraint c = constraints[i];
        uint32_t color = (c.p1 < 2 && c.p2 < 2) ? active_orange : particles[c.p2].color;
        
        float avg_radius = (proj_r[c.p1] + proj_r[c.p2]) * 0.5f;
        draw_capsule(px, w, h, proj_x[c.p1], proj_y[c.p1], proj_x[c.p2], proj_y[c.p2], avg_radius, color);
    }
    
    // --- 8. Draw Joint Circles ---
    for (int i = 0; i < NUM_PARTICLES; i++) {
        draw_circle(px, w, h, proj_x[i], proj_y[i], proj_r[i], particles[i].color);
    }
}

int main() {
    printf("=== TSFi Atropa/999 Sovereign Choreography ===\n");
    printf("[INFO] Synthesizing Soft Slugcat-Style Teddy Bear Wayland Canvas...\n");
    printf("[INFO] Press ESC in the terminal to securely terminate the window.\n"); fflush(stdout);
    
    int w = 1024, h = 1536, str = w * 4; size_t sz = str * h;
    int mfd = memfd_create("tsfi_teddy", MFD_CLOEXEC);
    if (mfd < 0) return 1;
    if (ftruncate(mfd, sz) < 0) return 1;
    uint32_t *px = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);

    // Wayland Setup
    const char *run = getenv("XDG_RUNTIME_DIR"), *disp = getenv("WAYLAND_DISPLAY");
    if (!run) {
        printf("[WARN] XDG_RUNTIME_DIR not set. Cannot start Wayland window. Render-only mode.\n");
    } else {
        if (!disp) disp = "wayland-0";
        char path[108]; snprintf(path, 108, "%s/%s", run, disp);
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr = {0}; addr.sun_family = AF_UNIX;
        snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
        if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) != 0) {
            printf("[WARN] Could not connect to Wayland. Render-only mode.\n");
        }
    }
    
    int frame = 0;
    for(int i=0; i<100; i++) {
        render_sick_teddy_bear(px, w, h, frame);
        frame++;
        if (i % 10 == 0) printf("Rendered frame %d\n", i);
    }

    printf("[SUCCESS] Rendered 100 frames locally.\n");
    return 0;
}
