#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "tsfi_controlnet_shm.h"
#include "tsfi_liang_barsky.h"
#include "tsfi_puppetry.h"
#include "tsfi_fourier.h"

#define W 256
#define H 256

typedef struct { float x, y, z; } Vec3;

static TSFiFourierBasis g_basis;
static bool g_basis_initialized = false;

static TsfiClippingVolume g_bear_arena = {
    .x_min = 5.0f,   .x_max = 251.0f,
    .y_min = 5.0f,   .y_max = 251.0f,
    .z_min = 0.0f,   .z_max = 200.0f,
    .w_min = -1.0f,  .w_max = 1.0f
};

static inline bool is_pixel_culled(float x, float y, float z) {
    if (x < g_bear_arena.x_min || x > g_bear_arena.x_max) return true;
    if (y < g_bear_arena.y_min || y > g_bear_arena.y_max) return true;
    if (z < g_bear_arena.z_min || z > g_bear_arena.z_max) return true;
    return false;
}

void render_sphere_depth(uint8_t *depth, Vec3 center, float radius) {
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float dx = x - center.x;
            float dy = y - center.y;
            float dist2 = dx*dx + dy*dy;
            if (dist2 <= radius*radius) {
                float dz = sqrt(radius*radius - dist2);
                float d = center.z - dz; 
                if (is_pixel_culled((float)x, (float)y, d)) continue;
                int val = (int)(255.0f - (d / 2.0f));
                if (val < 0) val = 0;
                if (val > 255) val = 255;
                int idx = (y * W + x) * 3;
                if (val > depth[idx]) {
                    depth[idx] = val;
                    depth[idx+1] = val;
                    depth[idx+2] = val;
                }
            }
        }
    }
}

void render_bone_pose(uint8_t *pose, Vec3 p1, Vec3 p2, uint8_t r, uint8_t g, uint8_t b, float thickness) {
    float dx = p2.x - p1.x; float dy = p2.y - p1.y;
    float len = sqrt(dx*dx + dy*dy);
    if (len == 0) return;
    dx /= len; dy /= len;
    for (float t = 0; t < len; t += 0.5f) {
        float cx = p1.x + dx * t; float cy = p1.y + dy * t;
        if (is_pixel_culled(cx, cy, (p1.z + p2.z)*0.5f)) continue;
        for (int y = (int)(cy - thickness); y <= (int)(cy + thickness); y++) {
            for (int x = (int)(cx - thickness); x <= (int)(cx + thickness); x++) {
                if (x >= 0 && x < W && y >= 0 && y < H) {
                    float px = x - cx; float py = y - cy;
                    if (px*px + py*py <= thickness*thickness) {
                        int idx = (y * W + x) * 3;
                        pose[idx] = r;
                        pose[idx+1] = g;
                        pose[idx+2] = b;
                    }
                }
            }
        }
    }
}

// Draws a mathematically continuous line based on the Fourier reconstructed points
void render_fourier_contour(uint8_t *pose, float *points, int count, uint8_t r, uint8_t g, uint8_t b, float thickness) {
    for (int i = 0; i < count - 1; i++) {
        Vec3 p1 = { points[i*2], points[i*2+1], 100.0f };
        Vec3 p2 = { points[(i+1)*2], points[(i+1)*2+1], 100.0f };
        render_bone_pose(pose, p1, p2, r, g, b, thickness);
    }
}

// Erase previous frame state using the Void brush
void brush_void_sphere(uint8_t *depth, Vec3 center, float radius) {
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float dx = x - center.x;
            float dy = y - center.y;
            if (dx*dx + dy*dy <= radius*radius) {
                int idx = (y * W + x) * 3;
                depth[idx] = 0;
                depth[idx+1] = 0;
                depth[idx+2] = 0;
            }
        }
    }
}

void brush_void_bone(uint8_t *pose, Vec3 p1, Vec3 p2, float thickness) {
    float dx = p2.x - p1.x; float dy = p2.y - p1.y;
    float len = sqrt(dx*dx + dy*dy);
    if (len == 0) return;
    dx /= len; dy /= len;
    for (float t = 0; t < len; t += 0.5f) {
        float cx = p1.x + dx * t; float cy = p1.y + dy * t;
        for (int y = (int)(cy - thickness); y <= (int)(cy + thickness); y++) {
            for (int x = (int)(cx - thickness); x <= (int)(cx + thickness); x++) {
                if (x >= 0 && x < W && y >= 0 && y < H) {
                    float px = x - cx; float py = y - cy;
                    if (px*px + py*py <= thickness*thickness) {
                        int idx = (y * W + x) * 3;
                        pose[idx] = 0;
                        pose[idx+1] = 0;
                        pose[idx+2] = 0;
                    }
                }
            }
        }
    }
}

void render_puppet_at(uint8_t *depth, uint8_t *pose, float cx, float cy, float time_t, int id, TsfiPuppetState *pstate) {
    TsfiPuppet *p = (pstate && pstate->puppets[id].active) ? &pstate->puppets[id] : NULL;
    
    // We maintain the previous state in static memory for the Void Brush
    static TsfiPuppet prev_state[4] = {0};
    
    // Apparition Anchor: If we have a Puppet State, we use its unique location
    // otherwise we use the provided cx/cy.
    float final_cx = cx;
    float final_cy = cy;
    
    float wave = p ? 0.0f : sin(time_t * 5.0f + (float)id) * 15.0f;
    float s = p ? p->scale : 0.5f; 
    uint32_t type = p ? p->animal_type : PUPPET_TYPE_TEDDY;
    float base_head_size = p ? (p->head_size > 0.0f ? p->head_size : 1.0f) : 1.0f;
    float base_arm_len   = p ? (p->arm_length > 0.0f ? p->arm_length : 1.0f) : 1.0f;
    float base_leg_len   = p ? (p->leg_length > 0.0f ? p->leg_length : 1.0f) : 1.0f;
    float base_ear_size  = p ? (p->ear_size > 0.0f ? p->ear_size : 1.0f) : 1.0f;
    float limb_stuffing  = (type == PUPPET_TYPE_TEDDY) ? (p ? (p->stuffing > 0.0f ? p->stuffing : 1.5f) : 1.5f) : 1.0f;

    Vec3 head, body, l_shoulder, l_hand, r_shoulder, r_hand;
    Vec3 l_hip, l_foot, r_hip, r_foot;
    // Beak vars removed, handled by Fourier

    if (p && p->head.x != 0.0f) {
        final_cx = p->body.x;
        final_cy = p->body.y;
        head = (Vec3){ p->head.x, p->head.y, p->head.z };
        body = (Vec3){ p->body.x, p->body.y, p->body.z };
        l_shoulder = (Vec3){ p->l_shoulder.x, p->l_shoulder.y, p->l_shoulder.z };
        l_hand = (Vec3){ p->l_hand.x, p->l_hand.y, p->l_hand.z };
        r_shoulder = (Vec3){ p->r_shoulder.x, p->r_shoulder.y, p->r_shoulder.z };
        r_hand = (Vec3){ p->r_hand.x, p->r_hand.y, p->r_hand.z };
        l_hip = (Vec3){ p->l_hip.x, p->l_hip.y, p->l_hip.z };
        l_foot = (Vec3){ p->l_foot.x, p->l_foot.y, p->l_foot.z };
        r_hip = (Vec3){ p->r_hip.x, p->r_hip.y, p->r_hip.z };
        r_foot = (Vec3){ p->r_foot.x, p->r_foot.y, p->r_foot.z };
        
        // 1. VOID BRUSH: Erase the previous state
        if (prev_state[id].active) {
            Vec3 p_head = { prev_state[id].head.x, prev_state[id].head.y, prev_state[id].head.z };
            Vec3 p_body = { prev_state[id].body.x, prev_state[id].body.y, prev_state[id].body.z };
            Vec3 p_l_shoulder = { prev_state[id].l_shoulder.x, prev_state[id].l_shoulder.y, prev_state[id].l_shoulder.z };
            Vec3 p_l_hand = { prev_state[id].l_hand.x, prev_state[id].l_hand.y, prev_state[id].l_hand.z };
            Vec3 p_r_shoulder = { prev_state[id].r_shoulder.x, prev_state[id].r_shoulder.y, prev_state[id].r_shoulder.z };
            Vec3 p_r_hand = { prev_state[id].r_hand.x, prev_state[id].r_hand.y, prev_state[id].r_hand.z };
            float ps = prev_state[id].scale;

            if (type == PUPPET_TYPE_CROW) {
                // Brutal void brush to erase the fourier contour
                brush_void_sphere(pose, p_body, 50 * ps * 2.0f);
            } else {
                brush_void_sphere(depth, p_head, 12 * ps * 2.0f);
                brush_void_sphere(depth, p_body, 25 * ps * 2.0f);
                brush_void_bone(pose, p_l_shoulder, p_l_hand, 8.0f);
                brush_void_bone(pose, p_r_shoulder, p_r_hand, 8.0f);
            }
            // Add other types to void brush as needed...
            
            // Clear general limbs
            if (type != PUPPET_TYPE_CROW) {
                brush_void_sphere(depth, p_l_hand, 10 * ps * 2.0f);
                brush_void_sphere(depth, p_r_hand, 10 * ps * 2.0f);
                brush_void_bone(pose, p_head, p_body, 4.0f);
            }
        }
        
        // Save current state for next frame's void brush
        prev_state[id] = *p;
    } else {
        if (p) {
            final_cx = p->body.x;
            final_cy = p->body.y;
        }
        head = (Vec3){ final_cx, final_cy - 30*s, 100 };
        body = (Vec3){ final_cx, final_cy + 15*s, 120 };
        l_shoulder = (Vec3){ final_cx - 30*s, final_cy + 0*s, 110 };
        l_hand = (Vec3){ final_cx - 30*s - 15*s*base_arm_len, final_cy - (15 + wave)*s*base_arm_len, 90 };
        r_shoulder = (Vec3){ final_cx + 30*s, final_cy + 0*s, 110 };
        r_hand = (Vec3){ final_cx + 30*s + 15*s*base_arm_len, final_cy + 10*s*base_arm_len, 90 };
        l_hip = (Vec3){ final_cx - 20*s, final_cy + 40*s, 110 };
        l_foot = (Vec3){ final_cx - 20*s - 10*s*base_leg_len, final_cy + 40*s + 20*s*base_leg_len, 110 };
        r_hip = (Vec3){ final_cx + 20*s, final_cy + 40*s, 110 };
        r_foot = (Vec3){ final_cx + 20*s + 10*s*base_leg_len, final_cy + 40*s + 20*s*base_leg_len, 110 };
        
        // Also update p->head, p->l_hand etc. so the void brush works!
        if (p) {
            p->head.x = head.x; p->head.y = head.y; p->head.z = head.z;
            p->body.x = body.x; p->body.y = body.y; p->body.z = body.z;
            p->l_shoulder.x = l_shoulder.x; p->l_shoulder.y = l_shoulder.y; p->l_shoulder.z = l_shoulder.z;
            p->l_hand.x = l_hand.x; p->l_hand.y = l_hand.y; p->l_hand.z = l_hand.z;
            p->r_shoulder.x = r_shoulder.x; p->r_shoulder.y = r_shoulder.y; p->r_shoulder.z = r_shoulder.z;
            p->r_hand.x = r_hand.x; p->r_hand.y = r_hand.y; p->r_hand.z = r_hand.z;
            p->l_hip.x = l_hip.x; p->l_hip.y = l_hip.y; p->l_hip.z = l_hip.z;
            p->l_foot.x = l_foot.x; p->l_foot.y = l_foot.y; p->l_foot.z = l_foot.z;
            p->r_hip.x = r_hip.x; p->r_hip.y = r_hip.y; p->r_hip.z = r_hip.z;
            p->r_foot.x = r_foot.x; p->r_foot.y = r_foot.y; p->r_foot.z = r_foot.z;
            prev_state[id] = *p;
        }
    }

    // 2. MATTER BRUSH: Assert the present state

    if (type == PUPPET_TYPE_ALLIGATOR) {
        render_sphere_depth(depth, head, 15 * s * 2.0f); 
        Vec3 head_rear = { head.x, head.y + 10*s, head.z };
        render_sphere_depth(depth, head_rear, 22 * s * 2.0f);
        render_sphere_depth(depth, body, 40 * s * 2.0f);
    } else if (type == PUPPET_TYPE_CROW) {
        // --- FOURIER-G KINEMATIC PUPPETRY PIPELINE ---
        
        if (!g_basis_initialized) {
            tsfi_fourier_init_basis(&g_basis);
            g_basis_initialized = true;
        }

        // Base geometric points for a crude bird silhouette (pre-transform)
        float crow_points[32] = {
             0, -20,   // Head Top
             10, -15,  // Beak Upper Base
             20, -10,  // Beak Upper Tip
             20, -5,   // Beak Lower Tip
             10, -5,   // Beak Lower Base
             5, 0,     // Neck
             20, 10,   // Tail
             0, 20,    // Feet Base
            -10, 10,   // Belly
            -20, 0,    // Back
            -5, -15,   // Nape
             0, -20,   // Close Loop
             // Padding for 16 pairs
             0,-20, 0,-20, 0,-20, 0,-20
        };

        // 1. Analyze: Convert discrete points to continuous spectral DNA
        TSFiFourierGlyph crow_dna;
        tsfi_fourier_from_points(&crow_dna, crow_points, 12);

        // 2. Manipulate: Spider's Puppetry Phase
        // Center the wave at the Apparition's requested coordinate
        tsfi_fourier_translate(&crow_dna, final_cx, final_cy);

        // Extract kinematics from shared memory state
        float sickness = p ? p->sickness : 0.0f;
        float jaw_drop = p ? (p->beak_lower.y - p->beak_upper.y) : 0.0f;
        
        if (jaw_drop > 2.0f) {
            // High-frequency phase shift to mathematically 'open' the beak
            // Harmonics 5-15 generally dictate the sharp extremities in a 12-point transform
            tsfi_fourier_articulate(&crow_dna, jaw_drop * 0.1f, 5, 15);
        }

        if (sickness > 0.1f) {
            // Apply Sympathy: Attenuate the high frequencies so the bird visually "melts"
            tsfi_fourier_melt(&crow_dna, sickness);
        }

        // 3. Synthesize: Reconstruct the physical wave via AVX-512 Matrix Math
        float out_voxels[TSFI_FOURIER_SAMPLES * 2] = {0};
        tsfi_fourier_reconstruct_avx512(out_voxels, &g_basis, &crow_dna);

        // 4. Render: Draw the mathematically pure contour to the ControlNet pose map
        uint8_t r = 128, g = 0, b = 128;
        if (sickness > 0.5f) { r = 80; g = 100; b = 80; } // Sickness coloration
        
        // Ensure scale is applied
        for (int i = 0; i < TSFI_FOURIER_SAMPLES; i++) {
            out_voxels[i*2] = final_cx + (out_voxels[i*2] - final_cx) * s;
            out_voxels[i*2+1] = final_cy + (out_voxels[i*2+1] - final_cy) * s;
        }
        
        render_fourier_contour(pose, out_voxels, TSFI_FOURIER_SAMPLES, r, g, b, 2.0f);
    } else if (type == PUPPET_TYPE_POPPY) {
        // Poppy: Central pod (Head) and 4 Petals (Limbs)
        render_sphere_depth(depth, head, 14 * s * 2.0f); // Central Pod
        render_bone_pose(pose, body, head, 0, 255, 0, 4.0f); // Stem
        
        // Petals as spheres at limb locations
        render_sphere_depth(depth, l_hand, 18 * s * 2.0f);
        render_sphere_depth(depth, r_hand, 18 * s * 2.0f);
        render_sphere_depth(depth, l_foot, 18 * s * 2.0f);
        render_sphere_depth(depth, r_foot, 18 * s * 2.0f);
        
        render_bone_pose(pose, head, l_hand, 255, 0, 255, 2.0f);
        render_bone_pose(pose, head, r_hand, 255, 0, 255, 2.0f);
        render_bone_pose(pose, head, l_foot, 255, 0, 255, 2.0f);
        render_bone_pose(pose, head, r_foot, 255, 0, 255, 2.0f);
    } else {
        render_sphere_depth(depth, head, 22 * s * 2.0f * limb_stuffing * base_head_size);
        render_sphere_depth(depth, body, 32 * s * 2.0f * limb_stuffing);
        
        // Add Teddy Ears
        Vec3 l_ear = { head.x - 12*s*base_head_size, head.y - 15*s*base_head_size, head.z - 10 };
        Vec3 r_ear = { head.x + 12*s*base_head_size, head.y - 15*s*base_head_size, head.z - 10 };
        render_sphere_depth(depth, l_ear, 8 * s * 2.0f * base_ear_size);
        render_sphere_depth(depth, r_ear, 8 * s * 2.0f * base_ear_size);
    }

    if (type != PUPPET_TYPE_POPPY) {
        render_sphere_depth(depth, l_hand, 10 * s * 2.0f * limb_stuffing);
        render_sphere_depth(depth, r_hand, 10 * s * 2.0f * limb_stuffing);
        render_sphere_depth(depth, l_foot, 12 * s * 2.0f * limb_stuffing);
        render_sphere_depth(depth, r_foot, 12 * s * 2.0f * limb_stuffing);

        if (type != PUPPET_TYPE_CROW) {
            render_bone_pose(pose, l_shoulder, l_hand, 255, 0, 0, 3.0f);
            render_bone_pose(pose, r_shoulder, r_hand, 0, 255, 0, 3.0f);
        }
        render_bone_pose(pose, head, body, 0, 0, 255, 4.0f);
        render_bone_pose(pose, l_hip, l_foot, 255, 255, 0, 3.0f);
        render_bone_pose(pose, r_hip, r_foot, 0, 255, 255, 3.0f);
    }
}

int main(int argc, char **argv) {
    float time_t = 0.0f;
    if (argc > 1) time_t = atof(argv[1]);

    TsfiControlNetMap *shm_depth = tsfi_cn_shm_create(TSFI_CN_SHM_DEPTH);
    TsfiControlNetMap *shm_pose  = tsfi_cn_shm_create(TSFI_CN_SHM_POSE);
    if (!shm_depth || !shm_pose) return 1;

    TsfiPuppetState *pstate = tsfi_puppetry_shm_attach();

    memset(shm_depth->data, 0, TSFI_CN_MAP_SIZE);
    memset(shm_pose->data, 0, TSFI_CN_MAP_SIZE);

    render_puppet_at(shm_depth->data, shm_pose->data, 64, 64,   time_t, 0, pstate); 
    render_puppet_at(shm_depth->data, shm_pose->data, 192, 64,  time_t, 1, pstate); 
    render_puppet_at(shm_depth->data, shm_pose->data, 64, 192,  time_t, 2, pstate); 
    render_puppet_at(shm_depth->data, shm_pose->data, 192, 192, time_t, 3, pstate); 

    printf("[SKELETON] Puppet Channel: 4 Quadrants Active (Time: %.2f)\n", time_t);

    if (pstate) tsfi_puppetry_shm_close(pstate);
    tsfi_cn_shm_close(shm_depth);
    tsfi_cn_shm_close(shm_pose);
    return 0;
}
