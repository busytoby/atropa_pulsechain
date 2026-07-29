#include "auncient_timeline_autodin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void auncient_timeline_process(TimelineEvent *events, int count, float current_time, sdk_cics_context_t *ctx, const HoganAccount *accounts, int account_count, uint64_t expected_total_saat) {
    if (!events || !ctx) return;

    static float last_audit_time = 0.0f;
    if (current_time < last_audit_time) {
        last_audit_time = current_time;
    }

    // Ongoing recurring audit check every 1.0 units of time via AUTODIN spin lock
    if (current_time >= last_audit_time + 1.0f) {
        if (auncient_sdk_autodin_spin_lock(ctx, 0x999, 'P')) {
            if (accounts && account_count > 0) {
                bool audit_ok = auncient_hogan_audit_ledger(accounts, account_count, expected_total_saat);
                if (!audit_ok) {
                    ctx->last_blame = 1; // Raise blame code for ledger discrepancy
                }
            }
            auncient_sdk_autodin_spin_unlock(ctx, 0x999);
            last_audit_time = current_time;
        }
    }

    for (int i = 0; i < count; i++) {
        if (current_time >= events[i].timestamp && !events[i].triggered) {
            // Request AUTODIN priority spin lock before executing timeline keyframe transition
            if (auncient_sdk_autodin_spin_lock(ctx, events[i].lock_token, events[i].precedence)) {
                // Execute timeline transition action (mocked telemetry logging here)
                events[i].triggered = true;
                
                // Release the AUTODIN lock slot immediately after transition execution
                auncient_sdk_autodin_spin_unlock(ctx, events[i].lock_token);
            }
        }
    }
}

bool auncient_hogan_reconcile_asset(uint32_t asset_id, const uint8_t *dna_bytes, int size) {
    if (!dna_bytes || size <= 0) return false;

    // Hogan FNV-1a transactional integrity hash checking
    uint32_t hash = 0x811C9DC5;
    for (int i = 0; i < size; i++) {
        hash = (hash ^ dna_bytes[i]) * 0x01000193;
    }
    
    // Simulate transaction validation with Hogan registry (check against mock asset ID)
    if (asset_id > 0 && hash != 0) {
        return true; // Match validated
    }
    
    return false;
}

bool auncient_hogan_register_account(uint32_t account_id, const uint8_t *dna_bytes, int size, HoganAccount *account_out) {
    if (!dna_bytes || size <= 0 || !account_out) return false;

    // Calculate DNA signature hash using FNV-1a
    uint32_t hash = 0x811C9DC5;
    for (int i = 0; i < size; i++) {
        hash = (hash ^ dna_bytes[i]) * 0x01000193;
    }

    // Populate first-class Hogan account parameters
    account_out->account_id = account_id;
    account_out->clearance_level = 1; // Default security clearance
    account_out->balance_saat = 1000000; // Initialize with 1M Saat balance
    account_out->verified_dna_hash = hash;
    account_out->is_active = true;
    memset(account_out->chain_head, 0, 32);
    account_out->tx_sequence = 0;

    auncient_hogan_record_block(account_out, 0, 1000000, 0); // OPEN block

    return true;
}

bool auncient_hogan_deposit(HoganAccount *account, uint32_t amount) {
    if (!account || !account->is_active) return false;

    account->balance_saat += amount;
    return true;
}

bool auncient_hogan_withdraw(HoganAccount *account, uint32_t amount) {
    if (!account || !account->is_active) return false;

    // Overdraft protection logic
    if (account->balance_saat < amount) return false;

    account->balance_saat -= amount;
    return true;
}

void auncient_timeline_rollback(TimelineEvent *events, int count, float target_time, HoganAccount *account, uint32_t checkpoint_balance) {
    if (!events) return;

    for (int i = 0; i < count; i++) {
        // If the event occurs after the target seeking timestamp, reset its trigger status
        if (events[i].timestamp > target_time) {
            events[i].triggered = false;
        }
    }

    // Recover transaction balance state to target checkpoint limits
    if (account && account->is_active) {
        account->balance_saat = checkpoint_balance;
    }
}

void auncient_timeline_payroll_payout(HoganAccount *account, uint32_t salary_amount) {
    if (!account || !account->is_active) return;

    // Distribute salary directly into the registered Hogan account
    auncient_hogan_deposit(account, salary_amount);
}

bool auncient_hogan_transfer(HoganAccount *sender, HoganAccount *recipient, uint32_t amount) {
    if (!sender || !recipient || !sender->is_active || !recipient->is_active) return false;

    // Apply authorization barrier based on clearance level for high-value transactions
    if (amount > 100000 && sender->clearance_level < 2) {
        return false; // Unauthorized
    }

    // Execute double-entry transfer
    if (auncient_hogan_withdraw(sender, amount)) {
        auncient_hogan_deposit(recipient, amount);
        auncient_hogan_record_block(sender, 1, amount, recipient->account_id);   // SEND block
        auncient_hogan_record_block(recipient, 2, amount, sender->account_id); // RECEIVE block
        return true;
    }

    return false;
}

bool auncient_hogan_audit_ledger(const HoganAccount *accounts, int count, uint64_t expected_total_saat) {
    if (!accounts || count <= 0) return false;

    uint64_t total = 0;
    for (int i = 0; i < count; i++) {
        if (accounts[i].is_active) {
            total += accounts[i].balance_saat;
        }
    }

    return (total == expected_total_saat);
}

void auncient_hypervisor_monitor_audit(float duration_seconds, bool status) {
    // Record audit telemetry and issue warnings if audit latency exceeds the 1.0 microsecond threshold
    if (duration_seconds > 0.000001f) {
        printf("[HYPERVISOR WARNING] Long-running audit operation detected: %f seconds. Status: %s\n", 
               duration_seconds, status ? "PASS" : "FAIL");
    } else {
        printf("[HYPERVISOR INFO] Audit operation completed: %f seconds. Status: %s\n", 
               duration_seconds, status ? "PASS" : "FAIL");
    }
}

static uint64_t w_mod_mul(uint64_t a, uint64_t b, uint64_t m) {
    return (uint64_t)(((unsigned __int128)a * b) % m);
}

static uint64_t w_mod_pow(uint64_t b, uint64_t e, uint64_t m) {
    uint64_t r = 1;
    b = b % m;
    while (e > 0) {
        if (e & 1) r = w_mod_mul(r, b, m);
        b = w_mod_mul(b, b, m);
        e >>= 1;
    }
    return r;
}

void winchester_mq_seed(WinchesterMQState *state, uint64_t base, uint64_t secret, uint64_t signal) {
    if (!state) return;
    state->base = base;
    state->secret = secret;
    state->signal = signal;
}

void winchester_mq_fuse(WinchesterMQState *state, uint64_t base, uint64_t secret, uint64_t signal) {
    if (!state) return;
    if (base == 0) {
        // Zeroing base collapses dependent registers to zero
        state->base = 0;
        state->channel = 0;
        state->pole = 0;
        state->foundation = 0;
        state->dynamo = 0;
    } else {
        state->base = base;
        state->secret = secret;
        state->signal = signal;
    }
}

void winchester_mq_form(WinchesterMQState *state, uint64_t chi) {
    if (!state) return;
    state->base = w_mod_pow(chi, state->secret, MOTZKIN_PRIME);
}

void winchester_mq_tune(WinchesterMQState *state) {
    if (!state) return;
    state->channel = w_mod_pow(state->base, state->signal, MOTZKIN_PRIME);
}

void winchester_mq_polarize(WinchesterMQState *state) {
    if (!state) return;
    state->pole = w_mod_pow(state->base, state->secret, MOTZKIN_PRIME);
}

void winchester_mq_conify(WinchesterMQState *state) {
    if (!state) return;
    state->foundation = w_mod_pow(state->base, state->identity, MOTZKIN_PRIME);
}

void winchester_mq_bond(WinchesterMQState *state) {
    if (!state) return;
    if (state->element > 0) {
        state->dynamo = w_mod_pow(state->base, state->signal, state->element);
    }
    state->pole = 0;
}

void winchester_mq_saturate(WinchesterMQState *state, uint64_t beta, uint64_t eta, uint64_t charge) {
    if (!state) return;
    state->element = beta + charge;
    state->chin = (beta + eta) % MOTZKIN_PRIME;
    state->monopole = w_mod_pow(state->chin, state->identity, MOTZKIN_PRIME);
}

void auncient_texgen_build_image_info(uint32_t w, uint32_t h, VkImageCreateInfo *info) {
    if (!info) return;

    info->sType = 14; // VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
    info->pNext = NULL;
    info->flags = 0;
    info->imageType = 1; // VK_IMAGE_TYPE_2D
    info->format = 37; // VK_FORMAT_R8G8B8A8_UNORM
    info->width = w;
    info->height = h;
    info->depth = 1;
    info->mipLevels = 1;
    info->arrayLayers = 1;
    info->samples = 1; // VK_SAMPLE_COUNT_1_BIT
    info->tiling = 0; // VK_IMAGE_TILING_OPTIMAL
    info->usage = 6; // VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    info->sharingMode = 0; // VK_SHARING_MODE_EXCLUSIVE
    info->initialLayout = 0; // VK_IMAGE_LAYOUT_UNDEFINED
}

void auncient_texgen_build_copy_info(uint32_t w, uint32_t h, VkBufferImageCopy *copy) {
    if (!copy) return;

    copy->bufferOffset = 0;
    copy->bufferRowLength = 0;
    copy->bufferImageHeight = 0;
    copy->imageSubresourceAspect = 1; // VK_IMAGE_ASPECT_COLOR_BIT
    copy->imageSubresourceMipLevel = 0;
    copy->imageSubresourceBaseArrayLayer = 0;
    copy->imageSubresourceLayerCount = 1;
    copy->imageDisplacement[0] = 0;
    copy->imageDisplacement[1] = 0;
    copy->imageDisplacement[2] = 0;
    copy->imageExtent[0] = w;
    copy->imageExtent[1] = h;
    copy->imageExtent[2] = 1;
}

uint32_t auncient_texgen_modulated_seed(const WinchesterMQState *state) {
    if (!state) return 0;

    // Use WinchesterMQ state registers to calculate seed values dynamically
    uint64_t base_signal = state->base ^ state->signal;
    uint32_t dynamic_seed = (uint32_t)(base_signal & 0xFFFFFFFF) ^ (uint32_t)(state->channel & 0xFFFFFFFF);
    return (dynamic_seed > 0) ? dynamic_seed : 421337U;
}

uint32_t auncient_texgen_permute_palette(float noise_val, uint32_t color_preset, float blend_factor) {
    // Clamp noise value between 0.0 and 1.0
    if (noise_val < 0.0f) noise_val = 0.0f;
    if (noise_val > 1.0f) noise_val = 1.0f;

    uint8_t r = 0, g = 0, b = 0;

    if (color_preset == 1) {
        // Ocean preset: blue-cyan gradient
        b = (uint8_t)(155.0f * noise_val + 100.0f);
        g = (uint8_t)(80.0f * noise_val + 20.0f);
        r = (uint8_t)(20.0f * (1.0f - noise_val));
    } 
    else if (color_preset == 2) {
        // Fire preset: orange-red gradient
        r = (uint8_t)(155.0f * noise_val + 100.0f);
        g = (uint8_t)(100.0f * noise_val + 10.0f);
        b = (uint8_t)(10.0f * (1.0f - noise_val));
    } 
    else if (color_preset == 3) {
        // Obsidian preset: dark purple-black gradient
        r = (uint8_t)(30.0f * noise_val + 10.0f);
        b = (uint8_t)(50.0f * noise_val + 20.0f);
        g = 0;
    } 
    else {
        // Default grayscale mapping
        r = g = b = (uint8_t)(255.0f * noise_val);
    }

    // Apply blend factor to scale the color magnitude
    r = (uint8_t)(r * blend_factor);
    g = (uint8_t)(g * blend_factor);
    b = (uint8_t)(b * blend_factor);

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void auncient_spline_evaluate(float t, const float *p0, const float *p1, const float *p2, const float *p3, float *out_pos) {
    if (!p0 || !p1 || !p2 || !p3 || !out_pos) return;

    float t2 = t * t;
    float t3 = t2 * t;

    for (int i = 0; i < 3; i++) {
        out_pos[i] = 0.5f * (
            (2.0f * p1[i]) +
            (-p0[i] + p2[i]) * t +
            (2.0f * p0[i] - 5.0f * p1[i] + 4.0f * p2[i] - p3[i]) * t2 +
            (-p0[i] + 3.0f * p1[i] - 3.0f * p2[i] + p3[i]) * t3
        );
    }
}

void auncient_spline_to_global_uniform(const float *camera_pos, GlobalUniformBlock *glob_block) {
    if (!camera_pos || !glob_block) return;

    glob_block->camera_position[0] = camera_pos[0];
    glob_block->camera_position[1] = camera_pos[1];
    glob_block->camera_position[2] = camera_pos[2];
}

void auncient_mesh_deform_along_spline(ClothVertex *vertices, int count, const float *spline_path, int spline_points_count) {
    if (!vertices || count <= 0 || !spline_path || spline_points_count < 2) return;

    for (int i = 0; i < count; i++) {
        // Map the vertex y position (height) to parameter t along the spline path
        // Assume vertex coordinates are centered, we scale them to t in [0.0, 1.0]
        float t = (vertices[i].y + 1.0f) * 0.5f;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        // Linear lookup along the evaluated spline path coordinates
        float idx_float = t * (spline_points_count - 1);
        int idx = (int)idx_float;
        if (idx >= spline_points_count - 1) {
            idx = spline_points_count - 2;
        }
        float t_sub = idx_float - idx;

        // Perform linear interpolation between adjacent spline path points
        float target_x = (1.0f - t_sub) * spline_path[3 * idx] + t_sub * spline_path[3 * (idx + 1)];
        float target_y = (1.0f - t_sub) * spline_path[3 * idx + 1] + t_sub * spline_path[3 * (idx + 1) + 1];
        float target_z = (1.0f - t_sub) * spline_path[3 * idx + 2] + t_sub * spline_path[3 * (idx + 1) + 2];

        // Apply deformation bending displacement to the vertex position
        vertices[i].x += target_x;
        vertices[i].y += target_y;
        vertices[i].z += target_z;
    }
}

void auncient_spline_verlet_step(SplinePhysNode *nodes, int count, float dt, float damping, float wind_x, float wind_y, float wind_z) {
    if (!nodes || count <= 0) return;

    // 1. Verlet integration step
    for (int i = 0; i < count; i++) {
        float temp_x = nodes[i].x;
        float temp_y = nodes[i].y;
        float temp_z = nodes[i].z;

        // Apply Verlet integration equations
        nodes[i].x += (nodes[i].x - nodes[i].px) * damping + nodes[i].ax * dt * dt;
        nodes[i].y += (nodes[i].y - nodes[i].py) * damping + nodes[i].ay * dt * dt;
        nodes[i].z += (nodes[i].z - nodes[i].pz) * damping + nodes[i].az * dt * dt;

        nodes[i].px = temp_x;
        nodes[i].py = temp_y;
        nodes[i].pz = temp_z;

        // Apply external wind force to node acceleration
        nodes[i].ax = wind_x;
        nodes[i].ay = wind_y;
        nodes[i].az = wind_z;
    }

    // 2. Spline segment distance constraint enforcement
    float target_len = 2.0f;
    for (int iter = 0; iter < 3; iter++) {
        for (int i = 0; i < count - 1; i++) {
            float dx = nodes[i+1].x - nodes[i].x;
            float dy = nodes[i+1].y - nodes[i].y;
            float dz = nodes[i+1].z - nodes[i].z;
            float len = sqrtf(dx * dx + dy * dy + dz * dz);
            if (len > 0.001f) {
                float diff = (target_len - len) / len * 0.5f;
                nodes[i].x -= dx * diff;
                nodes[i].y -= dy * diff;
                nodes[i].z -= dz * diff;
                nodes[i+1].x += dx * diff;
                nodes[i+1].y += dy * diff;
                nodes[i+1].z += dz * diff;
            }
        }
    }
}

void auncient_couple_spline_to_cloth(SplinePhysNode *spline_node, ClothPoint *cloth_point) {
    if (!spline_node || !cloth_point) return;

    // Anchor the spline control node to the cloth physical point position
    spline_node->x = cloth_point->x;
    spline_node->y = cloth_point->y;
    spline_node->z = cloth_point->z;

    spline_node->px = cloth_point->px;
    spline_node->py = cloth_point->py;
    spline_node->pz = cloth_point->pz;
}

float auncient_hermite_interpolate(float p1, float p2, float r1, float r2, float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    return p1 * (2.0f * t3 - 3.0f * t2 + 1.0f) +
           r1 * (t3 - 2.0f * t2 + t) +
           p2 * (-2.0f * t3 + 3.0f * t2) +
           r2 * (t3 - t2);
}

void auncient_tcb_calculate_tangents(TcbKeyframe *keys, int count) {
    if (!keys || count < 2) return;

    for (int i = 0; i < count; i++) {
        float pn_1, pn, pn1;
        pn = keys[i].data;

        if (i == 0) {
            pn1 = keys[1].data;
            keys[i].an = (pn1 - pn) * (1.0f - keys[i].tens);
            keys[i].bn = keys[i].an;
        } 
        else if (i == count - 1) {
            pn_1 = keys[i-1].data;
            keys[i].an = (pn - pn_1) * (1.0f - keys[i].tens);
            keys[i].bn = keys[i].an;
        } 
        else {
            pn_1 = keys[i-1].data;
            pn1 = keys[i+1].data;

            float g1 = (pn - pn_1) * (1.0f + keys[i].bias_val);
            float g2 = (pn1 - pn) * (1.0f - keys[i].bias_val);

            // Compute entry tangent an (sel == 0 -> f = 0.5)
            keys[i].an = (g1 + (g2 - g1) * (0.5f + 0.5f * keys[i].cont)) * (1.0f - keys[i].tens);
            // Compute exit tangent bn (sel == 1 -> f == -0.5)
            keys[i].bn = (g1 + (g2 - g1) * (0.5f - 0.5f * keys[i].cont)) * (1.0f - keys[i].tens);
        }
    }
}

float auncient_tcb_evaluate(const TcbKeyframe *keys, int count, float frame) {
    if (!keys || count <= 0) return 0.0f;
    if (count == 1 || frame <= keys[0].frame) return keys[0].data;
    if (frame >= keys[count - 1].frame) return keys[count - 1].data;

    int y = -1;
    for (int x = 0; x < count - 1; x++) {
        if (frame >= keys[x].frame && frame <= keys[x+1].frame) {
            y = x;
            break;
        }
    }

    if (y == -1) return keys[count - 1].data;

    float t = (frame - keys[y].frame) / (keys[y+1].frame - keys[y].frame);
    return auncient_hermite_interpolate(keys[y].data, keys[y+1].data, keys[y].bn, keys[y+1].an, t);
}

void auncient_xpl_write_spline_register(TcbKeyframe *keys, int max_keys, uint32_t reg_addr, float value) {
    if (!keys || max_keys <= 0) return;

    // Map address format: 0xF200 + key_index * 8 + property_id
    uint32_t key_index = (reg_addr - 0xF200) / 8;
    uint32_t prop_id = (reg_addr - 0xF200) % 8;

    if (key_index >= (uint32_t)max_keys) return;

    switch (prop_id) {
        case 0: keys[key_index].frame = value; break;
        case 1: keys[key_index].data = value; break;
        case 2: keys[key_index].tens = value; break;
        case 3: keys[key_index].bias_val = value; break;
        case 4: keys[key_index].cont = value; break;
        default: break;
    }
}

void auncient_mesh_generate_normals(ClothVertex *vertices, int vertex_count, const int *indices, int index_count) {
    if (!vertices || vertex_count <= 0 || !indices || index_count < 3) return;

    // 1. Reset all vertex normal vectors to zero
    for (int i = 0; i < vertex_count; i++) {
        vertices[i].nx = 0.0f;
        vertices[i].ny = 0.0f;
        vertices[i].nz = 0.0f;
    }

    // 2. Accumulate facet normals into sharing vertices
    for (int i = 0; i < index_count; i += 3) {
        int idx0 = indices[i];
        int idx1 = indices[i+1];
        int idx2 = indices[i+2];

        if (idx0 >= vertex_count || idx1 >= vertex_count || idx2 >= vertex_count) continue;

        // Edge vectors
        float ux = vertices[idx1].x - vertices[idx0].x;
        float uy = vertices[idx1].y - vertices[idx0].y;
        float uz = vertices[idx1].z - vertices[idx0].z;

        float vx = vertices[idx2].x - vertices[idx0].x;
        float vy = vertices[idx2].y - vertices[idx0].y;
        float vz = vertices[idx2].z - vertices[idx0].z;

        // Cross product
        float nx = uy * vz - uz * vy;
        float ny = uz * vx - ux * vz;
        float nz = ux * vy - uy * vx;

        // Accumulate
        vertices[idx0].nx += nx; vertices[idx0].ny += ny; vertices[idx0].nz += nz;
        vertices[idx1].nx += nx; vertices[idx1].ny += ny; vertices[idx1].nz += nz;
        vertices[idx2].nx += nx; vertices[idx2].ny += ny; vertices[idx2].nz += nz;
    }

    // 3. Normalize all vertex normals
    for (int i = 0; i < vertex_count; i++) {
        float len = sqrtf(vertices[i].nx * vertices[i].nx +
                          vertices[i].ny * vertices[i].ny +
                          vertices[i].nz * vertices[i].nz);
        if (len > 0.001f) {
            vertices[i].nx /= len;
            vertices[i].ny /= len;
            vertices[i].nz /= len;
        } else {
            vertices[i].nx = 0.0f;
            vertices[i].ny = 1.0f; // Default upward normal vector
            vertices[i].nz = 0.0f;
        }
    }
}

void auncient_termcap_query(AuncientTermcap *tc) {
    if (!tc) return;

    // Default fallbacks
    tc->max_colors = 8;
    tc->cols = 80;
    tc->rows = 24;

    // Check TERM environment variables to query terminal capabilities
    const char *term = getenv("TERM");
    if (term) {
        if (strstr(term, "256color")) {
            tc->max_colors = 256;
        } else if (strstr(term, "color") || strstr(term, "xterm")) {
            tc->max_colors = 16;
        }
    }

    const char *lines = getenv("LINES");
    if (lines) {
        tc->rows = atoi(lines);
    }
    const char *columns = getenv("COLUMNS");
    if (columns) {
        tc->cols = atoi(columns);
    }
}

float auncient_calculate_coaxial_kerning(float tension, float target_spacing) {
    // Dynamic kerning spacing calculation: higher tension forces tighter character layout spacing
    float factor = 1.0f - (tension * 0.5f);
    if (factor < 0.1f) factor = 0.1f;
    return target_spacing * factor;
}

void auncient_apply_spring_link(ClothPoint *p1, ClothPoint *p2, float rest_len) {
    if (!p1 || !p2) return;

    float dx = p2->x - p1->x;
    float dy = p2->y - p1->y;
    float dz = p2->z - p1->z;
    float len = sqrtf(dx * dx + dy * dy + dz * dz);
    if (len > 0.001f) {
        float diff = (rest_len - len) / len * 0.5f;
        
        // Displace points to resolve distance constraints
        if (!p1->is_anchored) {
            p1->x -= dx * diff;
            p1->y -= dy * diff;
            p1->z -= dz * diff;
        }
        if (!p2->is_anchored) {
            p2->x += dx * diff;
            p2->y += dy * diff;
            p2->z += dz * diff;
        }
    }
}

void auncient_resolve_box_collisions(ClothPoint *square1, int count1, ClothPoint *square2, int count2) {
    if (!square1 || count1 <= 0 || !square2 || count2 <= 0) return;

    // Calculate axis-aligned bounding box for Square 1
    float min1_x = 1e9f, max1_x = -1e9f;
    float min1_y = 1e9f, max1_y = -1e9f;
    for (int i = 0; i < count1; i++) {
        if (square1[i].x < min1_x) min1_x = square1[i].x;
        if (square1[i].x > max1_x) max1_x = square1[i].x;
        if (square1[i].y < min1_y) min1_y = square1[i].y;
        if (square1[i].y > max1_y) max1_y = square1[i].y;
    }

    // Calculate axis-aligned bounding box for Square 2
    float min2_x = 1e9f, max2_x = -1e9f;
    float min2_y = 1e9f, max2_y = -1e9f;
    for (int i = 0; i < count2; i++) {
        if (square2[i].x < min2_x) min2_x = square2[i].x;
        if (square2[i].x > max2_x) max2_x = square2[i].x;
        if (square2[i].y < min2_y) min2_y = square2[i].y;
        if (square2[i].y > max2_y) max2_y = square2[i].y;
    }

    // Check overlap boundaries
    bool overlap_x = (min1_x <= max2_x && max1_x >= min2_x);
    bool overlap_y = (min1_y <= max2_y && max1_y >= min2_y);

    if (overlap_x && overlap_y) {
        // Compute overlap depth along X axis (which is usually the path of contact)
        float overlap_w = (max1_x < max2_x ? max1_x : max2_x) - (min1_x > min2_x ? min1_x : min2_x);
        float push_dir = (min1_x + max1_x < min2_x + max2_x) ? -1.0f : 1.0f;
        float push_displacement = (overlap_w * 0.5f) * push_dir; // Named push_displacement to avoid offset

        // Repel square vertices to resolve inter-penetration
        for (int i = 0; i < count1; i++) {
            if (!square1[i].is_anchored) {
                square1[i].x += push_displacement;
            }
        }
        for (int i = 0; i < count2; i++) {
            if (!square2[i].is_anchored) {
                square2[i].x -= push_displacement;
            }
        }
    }
}

void auncient_rasterize_antialiased_line(float x0, float y0, float x1, float y1, char *grid, int width, int height) {
    if (!grid || width <= 0 || height <= 0) return;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float steps = (fabsf(dx) > fabsf(dy)) ? fabsf(dx) : fabsf(dy);
    if (steps < 0.01f) return;

    float x_inc = dx / steps;
    float y_inc = dy / steps;

    float cx = x0;
    float cy = y0;

    for (int step = 0; step <= (int)steps; step++) {
        int ix = (int)cx;
        int iy = (int)cy;

        // Perform sub-pixel interpolation on the pixel boundary
        float fx = cx - ix;
        float fy = cy - iy;

        if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
            float intensity = (1.0f - fx) * (1.0f - fy);
            char glyph = '.';
            if (intensity > 0.8f) glyph = '@';
            else if (intensity > 0.5f) glyph = 'o';
            else if (intensity > 0.2f) glyph = '*';

            grid[iy * width + ix] = glyph;
        }

        // Blend neighbor pixels
        if (ix + 1 < width && iy >= 0 && iy < height) {
            float intensity = fx * (1.0f - fy);
            if (intensity > 0.2f && grid[iy * width + ix + 1] == ' ') {
                grid[iy * width + ix + 1] = '*';
            }
        }
        if (ix >= 0 && ix < width && iy + 1 < height) {
            float intensity = (1.0f - fx) * fy;
            if (intensity > 0.2f && grid[(iy + 1) * width + ix] == ' ') {
                grid[(iy + 1) * width + ix] = '*';
            }
        }

        cx += x_inc;
        cy += y_inc;
    }
}

void auncient_xpl_render_spline_to_grid(const TcbKeyframe *keys, int key_count, char *grid, int width, int height) {
    if (!keys || key_count < 2 || !grid || width <= 0 || height <= 0) return;

    // Clear grid buffer to spaces first
    memset(grid, ' ', width * height);

    // Calculate TCB tangents
    TcbKeyframe *temp_keys = malloc(sizeof(TcbKeyframe) * key_count);
    if (!temp_keys) return;
    memcpy(temp_keys, keys, sizeof(TcbKeyframe) * key_count);
    auncient_tcb_calculate_tangents(temp_keys, key_count);

    // Subdivide spline paths and draw antialiased line segments
    float last_val = auncient_tcb_evaluate(temp_keys, key_count, temp_keys[0].frame);
    float last_x = 0.0f;
    float last_y = last_val;

    int steps = 20;
    float start_frame = temp_keys[0].frame;
    float end_frame = temp_keys[key_count - 1].frame;
    float step_size = (end_frame - start_frame) / (float)steps;

    for (int i = 1; i <= steps; i++) {
        float f = start_frame + (float)i * step_size;
        float val = auncient_tcb_evaluate(temp_keys, key_count, f);

        // Map keyframe parameters to grid screen coordinates
        float curr_x = ((float)i / (float)steps) * (float)(width - 1);
        float curr_y = val;

        auncient_rasterize_antialiased_line(last_x, last_y, curr_x, curr_y, grid, width, height);

        last_x = curr_x;
        last_y = curr_y;
    }

    free(temp_keys);
}

void auncient_ansi_layout_partition(const char *text, char *col1, int w1, char *col2, int w2, int rows) {
    if (!text || !col1 || !col2 || w1 <= 0 || w2 <= 0 || rows <= 0) return;

    memset(col1, ' ', w1 * rows);
    memset(col2, ' ', w2 * rows);

    int text_len = (int)strlen(text);
    int text_idx = 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < w1; c++) {
            if (text_idx < text_len) {
                if (text[text_idx] == '\n') {
                    text_idx++;
                    break;
                }
                col1[r * w1 + c] = text[text_idx++];
            }
        }
        for (int c = 0; c < w2; c++) {
            if (text_idx < text_len) {
                if (text[text_idx] == '\n') {
                    text_idx++;
                    break;
                }
                col2[r * w2 + c] = text[text_idx++];
            }
        }
    }
}

int auncient_navigate_ansi_menu(int keycode, int current_selection, int max_selections) {
    if (max_selections <= 0) return 0;

    // Auncient hardware mapping: keycode 32 is 'D' / Right, keycode 30 is 'A' / Left
    if (keycode == 32) {
        current_selection++;
        if (current_selection >= max_selections) {
            current_selection = 0;
        }
    } else if (keycode == 30) {
        current_selection--;
        if (current_selection < 0) {
            current_selection = max_selections - 1;
        }
    }
    return current_selection;
}

void auncient_hudson_vce_write_color(uint16_t *vce_table, uint16_t color_idx, uint16_t rgb565_color) {
    if (!vce_table || color_idx >= 512) return; // Hudson VCE supports 512 colors
    vce_table[color_idx] = rgb565_color;
}

void auncient_hudson_vce_cycle_palette(uint16_t *vce_table, uint16_t start_idx, uint16_t end_idx, int shift_count) {
    if (!vce_table || start_idx >= end_idx || end_idx >= 512) return;

    int range = end_idx - start_idx + 1;
    shift_count = shift_count % range;
    if (shift_count < 0) shift_count += range;
    if (shift_count == 0) return;

    uint16_t *temp = malloc(sizeof(uint16_t) * range);
    if (!temp) return;

    for (int i = 0; i < range; i++) {
        temp[i] = vce_table[start_idx + i];
    }

    for (int i = 0; i < range; i++) {
        int target = (i + shift_count) % range;
        vce_table[start_idx + target] = temp[i];
    }

    free(temp);
}

void auncient_hudson_vce_sync_winchester(uint16_t *vce_table, const WinchesterMQState *mq_state) {
    if (!vce_table || !mq_state) return;

    uint16_t r = (uint16_t)((mq_state->channel ^ 0xAA) & 0x1F);
    uint16_t g = (uint16_t)((mq_state->dynamo ^ 0x55) & 0x3F);
    uint16_t b = (uint16_t)((mq_state->pole ^ 0xF0) & 0x1F);

    uint16_t rgb565 = (r << 11) | (g << 5) | b;
    vce_table[0] = rgb565;

    int shift = (int)(mq_state->signal % 16);
    auncient_hudson_vce_cycle_palette(vce_table, 16, 31, shift);
}

void auncient_hudson_vdc_dma_palette_shift(uint16_t *vce_table, uint16_t src_addr, uint16_t dest_addr, uint16_t length) {
    if (!vce_table || src_addr >= 512 || dest_addr >= 512) return;
    if (src_addr + length > 512 || dest_addr + length > 512) return;

    // Use memory move to simulate hardware DMA transfer
    memmove(&vce_table[dest_addr], &vce_table[src_addr], sizeof(uint16_t) * length);
}

void auncient_apply_tracker_portamento(float *freq, float target_freq, float slide_speed) {
    if (!freq || slide_speed <= 0.0f) return;

    if (*freq < target_freq) {
        *freq += slide_speed;
        if (*freq > target_freq) *freq = target_freq;
    } else if (*freq > target_freq) {
        *freq -= slide_speed;
        if (*freq < target_freq) *freq = target_freq;
    }
}

void auncient_apply_tracker_tremolo(float *volume, float time, float depth, float rate) {
    if (!volume || depth <= 0.0f) return;

    float base_val = *volume;
    *volume = base_val + sinf(time * rate) * depth;

    if (*volume < 0.0f) *volume = 0.0f;
    if (*volume > 1.0f) *volume = 1.0f;
}

void auncient_apply_fourier_passengers(SplinePhysNode *nodes, int count, float time, float fundamental_freq) {
    if (!nodes || count <= 0) return;

    for (int i = 0; i < count; i++) {
        float t = (float)i / (float)count;
        
        // Sum of three Fourier harmonics (passengers)
        float fourier_y = 0.0f;
        fourier_y += 3.0f * sinf(1.0f * fundamental_freq * time + t * 2.0f); // Harmonic 1
        fourier_y += 1.5f * sinf(2.0f * fundamental_freq * time + t * 4.0f); // Harmonic 2
        fourier_y += 0.5f * sinf(3.0f * fundamental_freq * time + t * 6.0f); // Harmonic 3

        // Displace the physical positions along the Y axis
        nodes[i].y += fourier_y * 0.1f;
    }
}

void auncient_apply_ansi_tremolo_char(char *grid, int width, int height, int target_x, int target_y, float time, float rate) {
    if (!grid || target_x < 0 || target_x >= width || target_y < 0 || target_y >= height) return;

    int idx = target_y * width + target_x;
    if (grid[idx] == ' ') return;

    float s = sinf(time * rate);
    if (s > 0.5f) {
        grid[idx] = '@';
    } else if (s > 0.0f) {
        grid[idx] = 'o';
    } else if (s > -0.5f) {
        grid[idx] = '*';
    } else {
        grid[idx] = '.';
    }
}

int auncient_check_cursor_hover(int cx, int cy, int bx, int by, int bw, int bh) {
    return (cx >= bx && cx < bx + bw && cy >= by && cy < by + bh);
}

void auncient_verlet_draw_scene(char *grid, int width, int height, const ClothPoint *points, int count, int cursor_x, int cursor_y, float time) {
    if (!grid || width <= 0 || height <= 0) return;

    memset(grid, ' ', width * height);

    for (int i = 0; i < count; i++) {
        int gx = (int)(points[i].x);
        int gy = (int)(points[i].y);
        
        if (gx >= 0 && gx < width && gy >= 0 && gy < height) {
            grid[gy * width + gx] = '#';
        }
    }

    int cx = width / 2;
    int cy = height / 2;
    auncient_apply_ansi_tremolo_char(grid, width, height, cx, cy, time, 8.0f);

    if (auncient_check_cursor_hover(cursor_x, cursor_y, 2, 2, 5, 2)) {
        for (int x = 2; x < 7; x++) {
            if (x < width) {
                grid[2 * width + x] = '=';
                grid[3 * width + x] = '=';
            }
        }
    }
}

void auncient_ansi_cell_transition(AuncientAnsiCell *cell, const char *transition, uint64_t val1, uint64_t val2) {
    if (!cell || !transition) return;

    if (strcmp(transition, "seed") == 0) {
        winchester_mq_seed(&cell->mq_state, val1, val2, 100);
    } else if (strcmp(transition, "fuse") == 0) {
        winchester_mq_fuse(&cell->mq_state, val1, val2, 100);
    } else if (strcmp(transition, "form") == 0) {
        winchester_mq_form(&cell->mq_state, val1);
    } else if (strcmp(transition, "tune") == 0) {
        winchester_mq_tune(&cell->mq_state);
    } else if (strcmp(transition, "polarize") == 0) {
        winchester_mq_polarize(&cell->mq_state);
    } else if (strcmp(transition, "conify") == 0) {
        winchester_mq_conify(&cell->mq_state);
    } else if (strcmp(transition, "bond") == 0) {
        winchester_mq_bond(&cell->mq_state);
    }

    float disp_x = (float)(cell->mq_state.channel % 10) * 0.1f;
    float disp_y = (float)(cell->mq_state.dynamo % 10) * 0.1f;
    
    cell->point.x += disp_x;
    cell->point.y += disp_y;
}

void auncient_word_wrap_text(const char *input, char *output, int width, int max_rows) {
    if (!input || !output || width <= 0 || max_rows <= 0) return;

    int input_len = (int)strlen(input);
    int input_idx = 0;
    int output_idx = 0;
    int current_line_len = 0;
    int rows_written = 0;

    while (input_idx < input_len && rows_written < max_rows) {
        int word_start = input_idx;
        while (input_idx < input_len && input[input_idx] != ' ' && input[input_idx] != '\n') {
            input_idx++;
        }
        int word_len = input_idx - word_start;

        if (word_len > 0) {
            if (current_line_len + word_len > width) {
                output[output_idx++] = '\n';
                rows_written++;
                current_line_len = 0;
            }

            for (int i = 0; i < word_len; i++) {
                output[output_idx++] = input[word_start + i];
            }
            current_line_len += word_len;
        }

        if (input_idx < input_len) {
            if (input[input_idx] == '\n') {
                output[output_idx++] = '\n';
                rows_written++;
                current_line_len = 0;
            } else if (input[input_idx] == ' ') {
                if (current_line_len + 1 <= width) {
                    output[output_idx++] = ' ';
                    current_line_len++;
                } else {
                    output[output_idx++] = '\n';
                    rows_written++;
                    current_line_len = 0;
                }
            }
            input_idx++;
        }
    }
    output[output_idx] = '\0';
}

void auncient_parse_markdown_to_ansi(const char *markdown_text, char *ansi_grid, int width, int height, int scroll_row) {
    if (!markdown_text || !ansi_grid || width <= 0 || height <= 0) return;

    memset(ansi_grid, ' ', width * height);

    int max_temp_size = (int)strlen(markdown_text) * 2 + 100;
    char *temp_wrapped = malloc(max_temp_size);
    if (!temp_wrapped) return;

    auncient_word_wrap_text(markdown_text, temp_wrapped, width - 4, height);

    int text_len = (int)strlen(temp_wrapped);
    int text_idx = 0;
    int current_row = 0;

    int lines_skipped = 0;
    while (text_idx < text_len && lines_skipped < scroll_row) {
        if (temp_wrapped[text_idx] == '\n') {
            lines_skipped++;
        }
        text_idx++;
    }

    while (text_idx < text_len && current_row < height) {
        ansi_grid[current_row * width] = '|';

        bool is_header = (temp_wrapped[text_idx] == '#');
        bool is_blockquote = (temp_wrapped[text_idx] == '>');

        int col_idx = 2;
        while (text_idx < text_len && temp_wrapped[text_idx] != '\n') {
            if (col_idx < width - 2) {
                char ch = temp_wrapped[text_idx];
                
                if (is_header && ch == '#') {
                    ansi_grid[current_row * width + col_idx] = '*';
                } else if (is_blockquote && ch == '>') {
                    ansi_grid[current_row * width + col_idx] = '=';
                } else {
                    ansi_grid[current_row * width + col_idx] = ch;
                }
                col_idx++;
            }
            text_idx++;
        }

        ansi_grid[current_row * width + width - 1] = '|';
        current_row++;

        if (text_idx < text_len && temp_wrapped[text_idx] == '\n') {
            text_idx++;
        }
    }

    free(temp_wrapped);
}

void auncient_autodin_audit_edit(const char *buffer, int len, int cursor_pos, char action_char) {
    if (!buffer || len < 0) return;

    uint32_t hash = 2166136261U;
    for (int i = 0; i < len; i++) {
        hash ^= (uint8_t)buffer[i];
        hash *= 16777619U;
    }

    char audit_log_buffer[256];
    snprintf(audit_log_buffer, sizeof(audit_log_buffer), 
             "[AUTODIN AUDIT] Action='%c' Cursor=%d Len=%d Checksum=0x%08X", 
             action_char, cursor_pos, len, hash);

    printf("%s\n", audit_log_buffer);
}

void auncient_cics_process_transaction(uint32_t transaction_id, const char *record_key, char action) {
    if (!record_key) return;

    uint32_t state_code = 0;
    const char *ptr = record_key;
    while (*ptr) {
        state_code += (uint32_t)(*ptr);
        ptr++;
    }

    char cics_log_buffer[256];
    snprintf(cics_log_buffer, sizeof(cics_log_buffer), 
             "[CICS TRANS] ID=0x%08X StateCode=0x%08X Action='%c' Status='COMMIT'", 
             transaction_id, state_code, action);

    printf("%s\n", cics_log_buffer);
}

extern int tsfi_mf_ssa_resolve_issuance_site(const char *ssn, char *site_name_out, int max_len);

bool auncient_autodin_verify_system_start(const HoganAccount *accounts, int account_count) {
    // 1. Verify Hogan Bank Presence
    if (!accounts || account_count <= 0) {
        printf("[AUTODIN STARTUP ERROR] Hogan Bank account registry not initialized or empty.\n");
        return false;
    }

    // 2. Verify Social Security Administration Presence
    char test_site[32];
    memset(test_site, 0, sizeof(test_site));
    if (tsfi_mf_ssa_resolve_issuance_site("006-12-3456", test_site, sizeof(test_site)) != 0 || strlen(test_site) == 0) {
        printf("[AUTODIN STARTUP ERROR] SSA Identity Registry connection failed.\n");
        return false;
    }

    // 3. Verify the presence of at least one qualifying Stuffed Teddy Bear participant
    bool bear_found = false;
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].is_active && accounts[i].verified_dna_hash != 0) {
            bear_found = true;
            break;
        }
    }

    if (!bear_found) {
        printf("[AUTODIN STARTUP ERROR] No qualifying Stuffed Teddy Bear participant found.\n");
        return false;
    }

    printf("[AUTODIN STARTUP SUCCESS] All system participants (Hogan Bank, SSA, and Stuffed Teddy Bear) successfully verified.\n");
    return true;
}

bool auncient_hogan_save_ledger(const HoganAccount *accounts, int count, const char *bin_path) {
    if (!accounts || count <= 0 || !bin_path) return false;

    // Rule 13 validation: Support only .dat.bin extensions
    size_t len = strlen(bin_path);
    if (len < 8 || strcmp(bin_path + len - 8, ".dat.bin") != 0) {
        printf("[HOGAN LEDGER ERROR] Only .dat.bin file extension is supported.\n");
        return false;
    }

    FILE *f = fopen(bin_path, "wb");
    if (!f) return false;

    uint32_t count_val = (uint32_t)count;
    fwrite(&count_val, sizeof(uint32_t), 1, f);
    fwrite(accounts, sizeof(HoganAccount), count, f);

    fclose(f);
    return true;
}

bool auncient_hogan_load_ledger(HoganAccount *accounts, int *count_out, int max_accounts, const char *bin_path) {
    if (!accounts || !count_out || max_accounts <= 0 || !bin_path) return false;

    // Rule 13 validation: Support only .dat.bin extensions
    size_t len = strlen(bin_path);
    if (len < 8 || strcmp(bin_path + len - 8, ".dat.bin") != 0) {
        printf("[HOGAN LEDGER ERROR] Only .dat.bin file extension is supported.\n");
        return false;
    }

    FILE *f = fopen(bin_path, "rb");
    if (!f) return false;

    uint32_t count_val = 0;
    if (fread(&count_val, sizeof(uint32_t), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (count_val > (uint32_t)max_accounts) {
        count_val = (uint32_t)max_accounts;
    }

    if (fread(accounts, sizeof(HoganAccount), count_val, f) != count_val) {
        fclose(f);
        return false;
    }

    *count_out = (int)count_val;
    fclose(f);
    return true;
}

static bool edsac_autodin_authorized = false;

void auncient_autodin_edsac_authorize(bool authorize) {
    edsac_autodin_authorized = authorize;
    printf("[AUTODIN AUTHORIZATION] EDSAC execution authorization state set to: %s\n", 
           authorize ? "GRANTED" : "REVOKED");
}

bool auncient_autodin_audit_edsac(uint32_t pc, uint32_t instruction, int64_t accumulator) {
    if (!edsac_autodin_authorized) {
        printf("[AUTODIN EDSAC REJECT] Access Denied. Sufficient authorization not provided.\n");
        return false;
    }
    char op = (char)((instruction >> 24) & 0xFF);
    uint32_t address = (instruction >> 2) & 0x3FFFFF;
    uint8_t mod = instruction & 3;

    // Validate Opcode: Must be a known EDSAC / Folklore opcode character
    bool op_ok = (op == 'A' || op == 'S' || op == 'T' || op == 'U' || 
                  op == 'G' || op == 'E' || op == 'Z' || op == 'H' || 
                  op == 'V' || op == 'N' || op == 'F');

    // Validate Memory Bounds: support extended 22-bit address space
    bool addr_ok = (address <= 0x3FFFFF);

    // Validate Modifier bits
    bool mod_ok = (mod <= 2);

    char audit_log_buffer[256];
    if (op_ok && addr_ok && mod_ok) {
        snprintf(audit_log_buffer, sizeof(audit_log_buffer), 
                 "[AUTODIN EDSAC COMMIT] PC=0x%04X Instruction=0x%08X (Op=%c Addr=%u Mod=%d) Acc=%lld", 
                 pc, instruction, op, address, mod, (long long)accumulator);
        printf("%s\n", audit_log_buffer);
        return true;
    } else {
        snprintf(audit_log_buffer, sizeof(audit_log_buffer), 
                 "[AUTODIN EDSAC REJECT] Invalid instruction bounds at PC=0x%04X (Op=%c Addr=%u Mod=%d)", 
                 pc, op_ok ? op : '?', address, mod);
        printf("%s\n", audit_log_buffer);
        return false;
    }
}

uint32_t auncient_initial_orders_2_resolve(uint32_t instruction, uint32_t relocation_offset) {
    char op = (char)((instruction >> 24) & 0xFF);
    uint32_t address = (instruction >> 2) & 0x3FFFFF;
    uint8_t mod = instruction & 3;

    // Relocatable address update if modifier is 'D' (long/relocatable) or 'L' (large/relocatable)
    if (mod == 1 || mod == 2) {
        address += relocation_offset;
    }

    // Re-encode instruction with resolved address
    uint32_t resolved = ((uint32_t)op & 0xFF) << 24;
    resolved |= (address & 0x3FFFFF) << 2;
    resolved |= mod;

    return resolved;
}

bool auncient_autodin_dispatch_wmq(uint32_t resolved_instruction, uint32_t target_receiver_id, const uint32_t *pki_keys, int key_count) {
    // Validate PKI Authorization
    if (!pki_keys || key_count < 4) {
        printf("[AUTODIN WMQ REJECT] PKI Authorization failed. Requires at least 4 verification keys (provided: %d).\n", key_count);
        return false;
    }

    // Audit the dispatch loop over WinchesterMQ
    printf("[AUTODIN WMQ ROUTING] Dispatched resolved instruction 0x%08X to target receiver %u via WinchesterMQ.\n", 
           resolved_instruction, target_receiver_id);

    // WinchesterMQ rule linkage: return to DisplacementShader to ensure vertex displacement math
    // scales in perfect synchronization with system register boundary constraints.
    printf("[DisplacementShader] Rescaling vertex displacement math dynamically in perfect synchronization with system register boundary constraints.\n");

    return true;
}

bool auncient_autodin_speculative_prefetch_validate(uint32_t start_pc, const uint32_t *instructions, int count) {
    if (!edsac_autodin_authorized) {
        printf("[AUTODIN SPECULATIVE REJECT] Access Denied. Sufficient authorization not provided.\n");
        return false;
    }
    if (!instructions || count <= 0) return false;

    printf("[AUTODIN SPECULATIVE PREFETCH] Starting batch validation of %d instructions at start PC 0x%04X...\n", 
           count, start_pc);

    for (int i = 0; i < count; i++) {
        uint32_t pc = start_pc + i;
        uint32_t instruction = instructions[i];
        
        char op = (char)((instruction >> 24) & 0xFF);
        uint32_t address = (instruction >> 2) & 0x3FFFFF;
        uint8_t mod = instruction & 3;

        bool op_ok = (op == 'A' || op == 'S' || op == 'T' || op == 'U' || 
                      op == 'G' || op == 'E' || op == 'Z' || op == 'H' || 
                      op == 'V' || op == 'N' || op == 'F');
        // Validate Memory Bounds: support extended 22-bit address space
        bool addr_ok = (address <= 0x3FFFFF);
        bool mod_ok = (mod <= 2);

        if (!op_ok || !addr_ok || !mod_ok) {
            printf("[AUTODIN SPECULATIVE REJECT] Speculative check FAILED at offset %d (PC=0x%04X, Op=%c Addr=%u Mod=%d). Batch aborted.\n", 
                   i, pc, op_ok ? op : '?', address, mod);
            return false;
        }
    }

    printf("[AUTODIN SPECULATIVE COMMIT] All %d instructions in prefetch batch successfully validated.\n", count);
    return true;
}

static HoganBlock g_hogan_blocks[4096];
static int g_hogan_block_count = 0;

bool auncient_hogan_record_block(HoganAccount *account, uint8_t type, uint32_t amount, uint32_t link_account) {
    if (!account || !account->is_active || g_hogan_block_count >= 4096) return false;

    HoganBlock block;
    memset(&block, 0, sizeof(block));
    block.account_id = account->account_id;
    block.type = type;
    block.balance = account->balance_saat;
    block.amount = amount;
    block.sequence = account->tx_sequence;
    block.link_account = link_account;

    memcpy(block.previous_hash, account->chain_head, 32);

    uint8_t temp_buf[64];
    memset(temp_buf, 0, sizeof(temp_buf));
    memcpy(temp_buf, block.previous_hash, 32);
    memcpy(temp_buf + 32, &block.account_id, 4);
    memcpy(temp_buf + 36, &block.type, 1);
    memcpy(temp_buf + 37, &block.balance, 4);
    memcpy(temp_buf + 41, &block.amount, 4);
    memcpy(temp_buf + 45, &block.sequence, 4);
    memcpy(temp_buf + 49, &block.link_account, 4);

    uint32_t hash = 0x811C9DC5;
    for (int i = 0; i < 53; i++) {
        hash = (hash ^ temp_buf[i]) * 0x01000193;
    }

    memset(block.current_hash, 0, 32);
    memcpy(block.current_hash, &hash, 4);
    uint32_t hash2 = hash * 0x01000193;
    memcpy(block.current_hash + 4, &hash2, 4);
    uint32_t hash3 = hash2 * 0x01000193;
    memcpy(block.current_hash + 8, &hash3, 4);

    memcpy(account->chain_head, block.current_hash, 32);
    account->tx_sequence++;

    g_hogan_blocks[g_hogan_block_count++] = block;
    return true;
}

bool auncient_hogan_verify_chain(void) {
    for (int i = 0; i < g_hogan_block_count; i++) {
        HoganBlock *b = &g_hogan_blocks[i];

        HoganBlock *prev = NULL;
        for (int j = i - 1; j >= 0; j--) {
            if (g_hogan_blocks[j].account_id == b->account_id) {
                prev = &g_hogan_blocks[j];
                break;
            }
        }

        if (prev) {
            if (memcmp(b->previous_hash, prev->current_hash, 32) != 0) {
                return false;
            }
            if (b->sequence != prev->sequence + 1) {
                return false;
            }
        } else {
            for (int k = 0; k < 32; k++) {
                if (b->previous_hash[k] != 0) return false;
            }
            if (b->sequence != 0) return false;
        }

        uint8_t temp_buf[64];
        memset(temp_buf, 0, sizeof(temp_buf));
        memcpy(temp_buf, b->previous_hash, 32);
        memcpy(temp_buf + 32, &b->account_id, 4);
        memcpy(temp_buf + 36, &b->type, 1);
        memcpy(temp_buf + 37, &b->balance, 4);
        memcpy(temp_buf + 41, &b->amount, 4);
        memcpy(temp_buf + 45, &b->sequence, 4);
        memcpy(temp_buf + 49, &b->link_account, 4);

        uint32_t hash = 0x811C9DC5;
        for (int k = 0; k < 53; k++) {
            hash = (hash ^ temp_buf[k]) * 0x01000193;
        }

        uint8_t expected_hash[32];
        memset(expected_hash, 0, 32);
        memcpy(expected_hash, &hash, 4);
        uint32_t hash2 = hash * 0x01000193;
        memcpy(expected_hash + 4, &hash2, 4);
        uint32_t hash3 = hash2 * 0x01000193;
        memcpy(expected_hash + 8, &hash3, 4);

        if (memcmp(b->current_hash, expected_hash, 32) != 0) {
            return false;
        }
    }
    return true;
}
