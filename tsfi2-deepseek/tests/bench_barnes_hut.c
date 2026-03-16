#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>

#include "lau_memory.h"
#include "tsfi_wavelet_arena.h"
#include "tsfi_opt_zmm.h"

// Define external execution for DeepSeek MLA Kernel
extern void tsfi_kernel_deepseek_mla(void *regs, ZmmSynapse *syn);

// --- CONFIGURATION ---
#define NUM_BODIES 15000
#define WIDTH 800
#define HEIGHT 800
#define FPS 60
#define SECONDS 4
#define TOTAL_FRAMES (FPS * SECONDS)
#define G 0.5f // Gravitational constant
#define DT 0.016f // Time step
#define THETA 0.5f // Barnes-Hut threshold

// --- GLOBALS ---
TsfiWaveletArena arena;
uint8_t image[HEIGHT][WIDTH][3];
void *deepseek_regs;
ZmmSynapse deepseek_syn;
TsfiWavelet *quantum_core;

TsfiWavelet* alloc_tsfi_wavelet(uint64_t Prime) {
    TsfiWavelet *W = tsfi_STAT(&arena, Prime);
    W->telemetry.unique_id = arena.wavelet_uid_counter++;
    W->telemetry.Fa.base = (((uint64_t)rand() << 32) | rand()) % Prime;
    W->telemetry.Fa.secret = (((uint64_t)rand() << 32) | rand()) % Prime;
    W->telemetry.Fa.signal = (((uint64_t)rand() << 32) | rand()) % Prime;
    tsfi_Tune(W, Prime);
    W->Prime = Prime;
    // for (int i = 0; i < 4; i++) { W->rods[i] = NULL; W->cones[i] = NULL; }
    W->state = WAVELET_STATE_STAT;
    return W;
}

TsfiWavelet* alloc_seal11_wavelet(uint64_t Prime) {
    TsfiWavelet *W = alloc_tsfi_wavelet(Prime);
    TsfiWavelet *Shadow = alloc_tsfi_wavelet(Prime);
    uint64_t Xi = (W->telemetry.unique_id ^ 0xDEADBEEF) % Prime;
    if (Xi == 0) Xi = 1;
    uint64_t Alpha = (Shadow->telemetry.unique_id ^ 0xCAFEBABE) % Prime;
    if (Alpha == 0) Alpha = 1;
    uint64_t Beta = (Xi * Alpha) % Prime;
    tsfi_CREATE_YI(W, Shadow, Xi, Alpha, Beta, Prime);
    tsfi_SealHeader(W, Prime);
    tsfi_ActivateYang(W, Beta ^ 0x777, Prime);
    // W->cones[3] = Shadow;
    return W;
}

typedef struct {
    float x, y;
    float vx, vy;
    float mass;
    TsfiWavelet *yi;
} Body;

Body bodies[NUM_BODIES];

typedef struct QuadNode {
    float x, y; // Center of mass
    float mass;
    float cx, cy, size; // Bounding box
    struct QuadNode *children[4];
    int body_idx; // -1 if internal, >=0 if leaf
} QuadNode;

QuadNode *node_pool = NULL;
int node_count = 0;
int node_capacity = 0;

void init_pool() {
    node_capacity = NUM_BODIES * 4;
    node_pool = malloc(node_capacity * sizeof(QuadNode));
}

void reset_pool() {
    node_count = 0;
}

QuadNode* alloc_node() {
    if (node_count >= node_capacity) {
        node_capacity *= 2;
        node_pool = realloc(node_pool, node_capacity * sizeof(QuadNode));
    }
    QuadNode *n = &node_pool[node_count++];
    n->mass = 0;
    n->x = 0; n->y = 0;
    n->body_idx = -1;
    for(int i=0; i<4; i++) n->children[i] = NULL;
    return n;
}

void insert_body(QuadNode *node, int b_idx) {
    if (node->body_idx == -1 && node->mass == 0) {
        // Empty leaf
        node->body_idx = b_idx;
        node->mass = bodies[b_idx].mass;
        node->x = bodies[b_idx].x;
        node->y = bodies[b_idx].y;
        return;
    }

    if (node->body_idx != -1) {
        // Convert leaf to internal node, push existing body down
        int ex_idx = node->body_idx;
        node->body_idx = -1;
        
        for (int i=0; i<4; i++) {
            node->children[i] = alloc_node();
            node->children[i]->size = node->size / 2.0f;
            node->children[i]->cx = node->cx + (i % 2 == 1 ? node->children[i]->size : 0);
            node->children[i]->cy = node->cy + (i >= 2 ? node->children[i]->size : 0);
        }
        
        int q_ex = (bodies[ex_idx].y >= node->cy + node->size/2.0f ? 2 : 0) + (bodies[ex_idx].x >= node->cx + node->size/2.0f ? 1 : 0);
        insert_body(node->children[q_ex], ex_idx);
    }
    
    // Update center of mass
    float total_mass = node->mass + bodies[b_idx].mass;
    node->x = (node->x * node->mass + bodies[b_idx].x * bodies[b_idx].mass) / total_mass;
    node->y = (node->y * node->mass + bodies[b_idx].y * bodies[b_idx].mass) / total_mass;
    node->mass = total_mass;
    
    // Insert new body
    int q_new = (bodies[b_idx].y >= node->cy + node->size/2.0f ? 2 : 0) + (bodies[b_idx].x >= node->cx + node->size/2.0f ? 1 : 0);
    insert_body(node->children[q_new], b_idx);
}

void apply_force(int b_idx, QuadNode *node, float current_gravity) {
    if (!node || node->mass == 0) return;
    
    float dx = node->x - bodies[b_idx].x;
    float dy = node->y - bodies[b_idx].y;
    float dist_sq = dx*dx + dy*dy + 0.1f; // Softening
    float dist = sqrtf(dist_sq);
    
    if (node->body_idx != -1 || (node->size / dist) < THETA) {
        // Leaf or far enough away to approximate
        if (node->body_idx != b_idx) {
            float f = (current_gravity * node->mass) / dist_sq;
            bodies[b_idx].vx += f * (dx / dist) * DT;
            bodies[b_idx].vy += f * (dy / dist) * DT;
        }
    } else {
        // Recurse
        for (int i=0; i<4; i++) apply_force(b_idx, node->children[i], current_gravity);
    }
}

float halton(uint32_t index, uint32_t base) {
    float result = 0.0f;
    float f = 1.0f / (float)base;
    uint32_t i = index;
    while (i > 0) {
        result += f * (float)(i % base);
        i = i / base;
        f = f / (float)base;
    }
    return result;
}

float clampf(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}

void render_frame(int frame) {
    // 1. Render Photorealistic White Ipomoea Background
    for (int py = 0; py < HEIGHT; py++) {
        for (int px = 0; px < WIDTH; px++) {
            float ux = ((float)px / WIDTH) * 2.0f - 1.0f;
            float uy = ((float)py / HEIGHT) * 2.0f - 1.0f;
            float r_coord = sqrtf(ux*ux + uy*uy) * 16384.0f; 
            float theta = atan2f(uy, ux);
            
            // Fourier Realization (5 Lobes + DeepSeek Breathing)
            float r_limit = 8000.0f + 4000.0f * cosf(5.0f * theta);
            r_limit *= (1.0f + deepseek_syn.mass_density * 0.05f); // DeepSeek Breathing
            
            float field = 0.0f;
            if (r_coord < r_limit) {
                field = 1.0f;
                field *= clampf(r_coord / 8000.0f, 0.4f, 1.0f);
            }
            
            // Shimmer / Observability
            uint32_t resonance_id = (py * WIDTH + px) ^ (0x999 + frame);
            
            // React() Entropic Modulation on Halton Bases
            uint32_t base_a = 3 + (quantum_core->ichidai % 5);
            uint32_t base_b = 5 + (quantum_core->daiichi % 7);
            
            float bA = halton(resonance_id, base_a);
            float bB = halton(resonance_id + 1, base_b);
            float observability = bA * bB;
            
            // Apply Quantum React Color Tints
            float r_tint = 1.0f - ((quantum_core->ichidai % 100) / 1000.0f);
            float g_tint = 1.0f;
            float b_tint = 1.0f + ((quantum_core->daiichi % 100) / 500.0f);
            
            float intensity = field * observability * 255.0f;
            
            image[py][px][0] = (uint8_t)clampf(intensity * r_tint, 0.0f, 255.0f);
            image[py][px][1] = (uint8_t)clampf(intensity * g_tint, 0.0f, 255.0f);
            image[py][px][2] = (uint8_t)clampf(intensity * b_tint, 0.0f, 255.0f);
        }
    }
    
    // 2. Render Gravity Particles (DeepSeek Swarm) Over Top
    for (int i=0; i<NUM_BODIES; i++) {
        int px = (int)bodies[i].x;
        int py = (int)bodies[i].y;
        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
            // React() Colors for individual particle
            int swarm_r = 50 + (bodies[i].yi->ichidai % 50);
            int swarm_g = 150 - (bodies[i].yi->daiichi % 50);
            int swarm_b = 255;
            
            // Additive blending for glow effect (Cyan/Blue)
            int r = image[py][px][0] + swarm_r;
            int g = image[py][px][1] + swarm_g;
            int b = image[py][px][2] + swarm_b; 
            image[py][px][0] = r > 255 ? 255 : r;
            image[py][px][1] = g > 255 ? 255 : g;
            image[py][px][2] = b > 255 ? 255 : b;
        }
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "frames/frame_%04d.ppm", frame);
    FILE *f = fopen(filename, "wb");
    if (f) {
        fprintf(f, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
        fwrite(image, 1, WIDTH * HEIGHT * 3, f);
        fclose(f);
    }
}

int main() {
    printf("=== TSFi Barnes-Hut Galaxy Simulation (DeepSeek Enhanced) ===\n");
    if (system("mkdir -p frames") != 0) {
        perror("Failed to create frames directory");
        return 1;
    }
    
    // Initialize the Arena for the Quantum Core Wavelet
    uint64_t arena_size = 1024 * 1024 * 64; // 64 MB is plenty for one core
    uint8_t *mem = mmap(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(mem != MAP_FAILED);
    tsfi_wavelet_arena_init(&arena, mem, arena_size);

    init_pool();
    
    // Allocate registers for the DeepSeek kernel
    deepseek_regs = lau_memalign_wired(512, 2048);
    memset(&deepseek_syn, 0, sizeof(ZmmSynapse));
    
    // Allocate the Quantum Core for React() color modulation
    quantum_core = alloc_seal11_wavelet(TSFI_WAVELET_PRIME);
    
    // Initialize a spinning Ipomoea flower
    for (int i=0; i<NUM_BODIES; i++) {
        float angle = (float)rand() / RAND_MAX * 2.0f * M_PI;
        
        // Ipomoea Harmonic Base (5 Lobes)
        float base_radius = WIDTH / 5.0f;
        float lobe_strength = WIDTH / 6.0f;
        float r_max = base_radius + lobe_strength * cosf(5.0f * angle);
        
        // Randomize within the petal boundary
        float radius = ((float)rand() / RAND_MAX) * r_max;
        
        bodies[i].x = WIDTH/2.0f + cosf(angle) * radius;
        bodies[i].y = HEIGHT/2.0f + sinf(angle) * radius;
        bodies[i].mass = 1.0f + ((float)rand() / RAND_MAX) * 2.0f;
        
        // Give it a delicate orbital velocity to swirl the petals
        float vel = sqrtf(G * 5000.0f / (radius + 10.0f)) * 0.5f; 
        bodies[i].vx = -sinf(angle) * vel;
        bodies[i].vy = cosf(angle) * vel;
        
        // Bind an independent Quantum Core (YI) to every single particle
        bodies[i].yi = alloc_seal11_wavelet(TSFI_WAVELET_PRIME);
    }
    
    printf("[INFO] Simulating %d frames (%d seconds at %d FPS)...\n", TOTAL_FRAMES, SECONDS, FPS);
    
    for (int frame=0; frame < TOTAL_FRAMES; frame++) {
        reset_pool();
        QuadNode *root = alloc_node();
        root->cx = 0; root->cy = 0; root->size = (WIDTH > HEIGHT ? WIDTH : HEIGHT) * 2.0f; // Ensure large bounds
        
        // --- DEEPSEEK ENTROPY INJECTION ---
        // Scramble the registers to simulate dynamic LLM output context
        tsfi_scramble_wave512(deepseek_regs, 2048);
        tsfi_kernel_deepseek_mla(deepseek_regs, &deepseek_syn);
        
        // --- REACT() QUANTUM COLOR EVOLUTION ---
        tsfi_EvolveDai(quantum_core, frame % 2, TSFI_WAVELET_PRIME);
        
        // DeepSeek outputs an ontological Synaptic Density. We map this to the global gravity constant.
        float current_gravity = G + (deepseek_syn.mass_density * 0.0001f);
        if (current_gravity > G * 3.0f) {
             current_gravity = G * 3.0f; // Cap explosive gravitational bursts
             deepseek_syn.mass_density = 0.0f; // Reset pressure
        }
        
        // 1. Build Quadtree
        for (int i=0; i<NUM_BODIES; i++) {
            insert_body(root, i);
        }
        
        // 2. Compute Forces
        for (int i=0; i<NUM_BODIES; i++) {
            apply_force(i, root, current_gravity);
        }
        
        // 3. Integrate & Render
        for (int i=0; i<NUM_BODIES; i++) {
            // Evolve the specific particle's quantum core
            tsfi_EvolveDai(bodies[i].yi, frame % 2, TSFI_WAVELET_PRIME);
            
            bodies[i].x += bodies[i].vx * DT;
            bodies[i].y += bodies[i].vy * DT;
        }
        render_frame(frame);
        
        if (frame % 30 == 0) {
            printf("  -> Processed frame %d / %d\n", frame, TOTAL_FRAMES);
        }
    }
    
    printf("\n[INFO] Simulation complete. Compiling MP4 using FFmpeg...\n");
    if (system("ffmpeg -y -framerate 60 -i frames/frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p -preset fast tsfi_galaxy_demo.mp4") != 0) {
        perror("Failed to compile mp4");
        return 1;
    }
    
    printf("[SUCCESS] Demo compiled to 'tsfi_galaxy_demo.mp4'!\n");
    return 0;
}