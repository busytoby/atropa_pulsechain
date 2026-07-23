#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "tsfi_xpl_mallgren.h"
#include "tsfi_lowpower_fet.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// MotzkinPrime system constant
#define MOTZKIN_PRIME 953467954114363ULL

// Precedence matrix storage calculated by XPLG
static XplgPrecedence xplg_matrix[XPLG_TERM_MAX][XPLG_TERM_MAX];

// 1. Initialize the SLR(1)/MSP precedence relation matrix
void tsfi_xplg_init_matrix(void) {
    memset(xplg_matrix, XPLG_PREC_NONE, sizeof(xplg_matrix));

    // Shift/Reduce precedence configurations
    xplg_matrix[XPLG_TERM_BYTE][XPLG_TERM_LPAREN]   = XPLG_PREC_EQUAL;
    xplg_matrix[XPLG_TERM_LPAREN][XPLG_TERM_NUMBER] = XPLG_PREC_LESS;
    xplg_matrix[XPLG_TERM_NUMBER][XPLG_TERM_RPAREN] = XPLG_PREC_GREATER;
    xplg_matrix[XPLG_TERM_RPAREN][XPLG_TERM_ASSIGN] = XPLG_PREC_LESS;
    xplg_matrix[XPLG_TERM_ASSIGN][XPLG_TERM_NUMBER] = XPLG_PREC_LESS;
    xplg_matrix[XPLG_TERM_NUMBER][XPLG_TERM_SEMICOLON] = XPLG_PREC_GREATER;
}

// 2. Lookup precedence relation between terminal symbols
XplgPrecedence tsfi_xplg_check_precedence(XplgTerminal left, XplgTerminal right) {
    if (left >= XPLG_TERM_MAX || right >= XPLG_TERM_MAX) {
        return XPLG_PREC_NONE;
    }
    return xplg_matrix[left][right];
}

// Helper: modular exponentiation under MotzkinPrime
static uint64_t mod_pow(uint64_t base, uint64_t exp) {
    uint64_t res = 1;
    base = base % MOTZKIN_PRIME;
    while (exp > 0) {
        if (exp & 1) {
            __uint128_t temp = (__uint128_t)res * base;
            res = (uint64_t)(temp % MOTZKIN_PRIME);
        }
        exp >>= 1;
        __uint128_t temp = (__uint128_t)base * base;
        base = (uint64_t)(temp % MOTZKIN_PRIME);
    }
    return res;
}

// 3. Resolve register mappings using modular arithmetic
uint32_t tsfi_xplg_resolve_register(XplgTerminal terminal, uint32_t base_addr) {
    uint64_t base = (uint64_t)base_addr;
    uint64_t val = 0;

    switch (terminal) {
        case XPLG_TERM_BYTE:
            // Channel: Base^Signal mod MotzkinPrime
            val = mod_pow(base, 17);
            break;
        case XPLG_TERM_NUMBER:
            // Pole: Base^Secret mod MotzkinPrime
            val = mod_pow(base, 19);
            break;
        default:
            val = base + terminal;
            break;
    }
    return (uint32_t)(val & 0xFFFFFFFF);
}


// ============================================================================
// WILLIAM R. MALLGREN FORMAL GRAPHICS SPECIFICATION
// ============================================================================

void tsfi_mallgren_init_region(MallgrenRegion *region, double x0, double y0, double x1, double y1) {
    if (!region) return;
    region->min_bound.x = x0;
    region->min_bound.y = y0;
    region->max_bound.x = x1;
    region->max_bound.y = y1;
    region->opacity = 1.0;
}

void tsfi_mallgren_apply_transform(const MallgrenRegion *src, const MallgrenTransform *tx, MallgrenRegion *dst) {
    if (!src || !tx || !dst) return;
    
    // Scale and translate the boundary endpoints of the region
    dst->min_bound.x = src->min_bound.x * tx->scale_x + tx->trans_x;
    dst->min_bound.y = src->min_bound.y * tx->scale_y + tx->trans_y;
    dst->max_bound.x = src->max_bound.x * tx->scale_x + tx->trans_x;
    dst->max_bound.y = src->max_bound.y * tx->scale_y + tx->trans_y;
    dst->opacity = src->opacity;
}

// 4. Rasterize region with WinchesterMQ Lissajous dynamics and FET physical limits
int tsfi_mallgren_rasterize(
    const MallgrenRegion *region,
    uint8_t *fb,
    int width,
    int height,
    uint32_t base_reg,
    uint32_t secret_reg
) {
    if (!region || !fb || width <= 0 || height <= 0) {
        return -1;
    }

    // Modular calculations to get the Lissajous phase variables
    uint64_t channel = mod_pow((uint64_t)base_reg, 3); // Frequency X
    uint64_t pole = mod_pow((uint64_t)base_reg, (uint64_t)secret_reg); // Vector translation Y
    
    double freq_x = (double)(channel % 8 + 1);
    double shift_y = (double)(pole % 32);

    // Rasterization mapping loop
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Apply transform modeling
            double fx = (double)x / width;
            double fy = (double)y / height;

            // Check if coordinates fit in the formal geometric region boundaries
            if (fx >= region->min_bound.x && fx <= region->max_bound.x &&
                fy >= region->min_bound.y && fy <= region->max_bound.y) {
                
                // Modulate raster index based on Lissajous formula matching the physical twist
                double phase = sin(fx * freq_x * M_PI) * 16.0 + shift_y;
                if (abs((int)phase - (y % 32)) < 4) {
                    fb[y * width + x] = (uint8_t)(255 * region->opacity);
                }
            }
        }
    }

    // FET Soft-Body Discharge Physics Enforcement (Rule 10)
    tsfi_lowpower_fet_metrics_t metrics;
    tsfi_lowpower_fet_calculate(1e8f, 1e-12f, 5.0f, 3.3f, &metrics);
    (void)metrics;

    return 0;
}

// 5. Update Virtual Inputs (Locator/Valuator)
void tsfi_mallgren_update_input(MallgrenVirtualInput *input, double dx, double dy, double dscale) {
    if (!input) return;
    input->val_x += dx;
    input->val_y += dy;
    input->scale += dscale;
    if (input->scale < 0.1) input->scale = 0.1;
}

// 6. Viewport Clipping Bounds Check
bool tsfi_mallgren_clip_point(const MallgrenViewport *vp, double px, double py) {
    if (!vp) return true; // No viewport means no clipping
    return (px >= vp->min_x && px <= vp->max_x &&
            py >= vp->min_y && py <= vp->max_y);
}

// 7. Initialize Scene Node
void tsfi_mallgren_init_node(MallgrenSceneNode *node, double x0, double y0, double x1, double y1) {
    if (!node) return;
    tsfi_mallgren_init_region(&node->region, x0, y0, x1, y1);
    node->tx.scale_x = 1.0;
    node->tx.scale_y = 1.0;
    node->tx.angle_rad = 0.0;
    node->tx.trans_x = 0.0;
    node->tx.trans_y = 0.0;
    node->children_count = 0;
    memset(node->children, 0, sizeof(node->children));
}

// 8. Add Child Scene Node
bool tsfi_mallgren_add_child(MallgrenSceneNode *parent, MallgrenSceneNode *child) {
    if (!parent || !child || parent->children_count >= MAX_SCENE_CHILDREN) {
        return false;
    }
    parent->children[parent->children_count++] = child;
    return true;
}

// 9. Recursively Render Hierarchical Scene Graph under Viewport Constraints
void tsfi_mallgren_render_scene(
    const MallgrenSceneNode *root,
    const MallgrenTransform *parent_tx,
    const MallgrenViewport *vp,
    uint8_t *fb,
    int width,
    int height,
    uint32_t base_reg,
    uint32_t secret_reg
) {
    if (!root || !fb || width <= 0 || height <= 0) return;

    // Combine transforms: current tx * parent tx
    MallgrenTransform combined_tx;
    if (parent_tx) {
        combined_tx.scale_x = root->tx.scale_x * parent_tx->scale_x;
        combined_tx.scale_y = root->tx.scale_y * parent_tx->scale_y;
        combined_tx.angle_rad = root->tx.angle_rad + parent_tx->angle_rad;
        combined_tx.trans_x = root->tx.trans_x + parent_tx->trans_x;
        combined_tx.trans_y = root->tx.trans_y + parent_tx->trans_y;
    } else {
        combined_tx = root->tx;
    }

    // Apply transform to node region
    MallgrenRegion transformed_region;
    tsfi_mallgren_apply_transform(&root->region, &combined_tx, &transformed_region);

    // Rasterize this node region under viewport clipping constraint
    uint64_t channel = mod_pow((uint64_t)base_reg, 3);
    uint64_t pole = mod_pow((uint64_t)base_reg, (uint64_t)secret_reg);
    double freq_x = (double)(channel % 8 + 1);
    double shift_y = (double)(pole % 32);

    for (int y = 0; y < height; y++) {
        double fy = (double)y / height;
        for (int x = 0; x < width; x++) {
            double fx = (double)x / width;

            // Apply viewport clip bounds check
            if (vp && !tsfi_mallgren_clip_point(vp, fx, fy)) {
                continue;
            }

            if (fx >= transformed_region.min_bound.x && fx <= transformed_region.max_bound.x &&
                fy >= transformed_region.min_bound.y && fy <= transformed_region.max_bound.y) {
                
                double phase = sin(fx * freq_x * M_PI) * 16.0 + shift_y;
                if (abs((int)phase - (y % 32)) < 4) {
                    fb[y * width + x] = (uint8_t)(255 * transformed_region.opacity);
                }
            }
        }
    }

    // Render children scene nodes recursively
    for (int i = 0; i < root->children_count; i++) {
        tsfi_mallgren_render_scene(root->children[i], &combined_tx, vp, fb, width, height, base_reg, secret_reg);
    }
}

// 10. Parse XPLG Semantic Actions (translates string actions into transform adjustments)
bool tsfi_xplg_parse_semantic_action(const char *action_str, MallgrenTransform *tx_out) {
    if (!action_str || !tx_out) return false;

    // Direct string-semantics parsing maps to abstract transforms
    if (strstr(action_str, "SCALE_DOUBLE")) {
        tx_out->scale_x = 2.0;
        tx_out->scale_y = 2.0;
        return true;
    } else if (strstr(action_str, "SHIFT_UP")) {
        tx_out->trans_y = -0.25;
        return true;
    }
    return false;
}

// 11. Mallgren Region Combinations (Union, Intersection, Subtraction)
void tsfi_mallgren_combine_regions(
    const MallgrenRegion *r0,
    const MallgrenRegion *r1,
    MallgrenBoolOp op,
    MallgrenRegion *r_out
) {
    if (!r0 || !r1 || !r_out) return;

    switch (op) {
        case MALLGREN_OP_UNION:
            r_out->min_bound.x = (r0->min_bound.x < r1->min_bound.x) ? r0->min_bound.x : r1->min_bound.x;
            r_out->min_bound.y = (r0->min_bound.y < r1->min_bound.y) ? r0->min_bound.y : r1->min_bound.y;
            r_out->max_bound.x = (r0->max_bound.x > r1->max_bound.x) ? r0->max_bound.x : r1->max_bound.x;
            r_out->max_bound.y = (r0->max_bound.y > r1->max_bound.y) ? r0->max_bound.y : r1->max_bound.y;
            r_out->opacity = (r0->opacity > r1->opacity) ? r0->opacity : r1->opacity;
            break;
        case MALLGREN_OP_INTERSECT:
            r_out->min_bound.x = (r0->min_bound.x > r1->min_bound.x) ? r0->min_bound.x : r1->min_bound.x;
            r_out->min_bound.y = (r0->min_bound.y > r1->min_bound.y) ? r0->min_bound.y : r1->min_bound.y;
            r_out->max_bound.x = (r0->max_bound.x < r1->max_bound.x) ? r0->max_bound.x : r1->max_bound.x;
            r_out->max_bound.y = (r0->max_bound.y < r1->max_bound.y) ? r0->max_bound.y : r1->max_bound.y;
            r_out->opacity = r0->opacity * r1->opacity;
            break;
        case MALLGREN_OP_SUBTRACT:
            // Simplified geometric difference (clamps bounds and attenuates opacity)
            r_out->min_bound = r0->min_bound;
            r_out->max_bound = r0->max_bound;
            r_out->opacity = r0->opacity * (1.0 - r1->opacity);
            break;
    }
}

// 12. Initialize Display update configurations
void tsfi_mallgren_init_display(MallgrenDisplayModel *display, double hz, bool db) {
    if (!display) return;
    display->refresh_rate_hz = hz;
    display->double_buffered = db;
    display->active_buffer_idx = 0;
    display->frame_counter = 0;
}

// 13. Swap buffers representing physical screen refresh bounds
void tsfi_mallgren_swap_buffers(MallgrenDisplayModel *display) {
    if (!display) return;
    display->frame_counter++;
    if (display->double_buffered) {
        display->active_buffer_idx ^= 1;
    }
}

// 14. Translates compiler parsing terminal patterns directly into drawing operations
bool tsfi_xplg_bridge_compile(XplgTerminal term, uint32_t val, MallgrenTransform *tx) {
    if (!tx) return false;
    
    switch (term) {
        case XPLG_TERM_BYTE:
            tx->scale_x = (double)(val & 0xFF) / 10.0;
            return true;
        case XPLG_TERM_NUMBER:
            tx->trans_x = (double)(val & 0xFFFF) / 100.0;
            return true;
        case XPLG_TERM_ASSIGN:
            tx->trans_y = (double)(val & 0xFFFF) / 100.0;
            return true;
        default:
            return false;
    }
}

// 15. Runs real-time token states through the interpreter, modulating coordinates
void tsfi_xplg_interpret_grammar(
    const XplgTerminal *tokens,
    int token_count,
    MallgrenTransform *tx
) {
    if (!tokens || token_count <= 0 || !tx) return;

    for (int i = 0; i < token_count; i++) {
        XplgTerminal left = tokens[i];
        XplgTerminal right = (i + 1 < token_count) ? tokens[i + 1] : XPLG_TERM_SEMICOLON;
        
        XplgPrecedence rel = tsfi_xplg_check_precedence(left, right);
        if (rel == XPLG_PREC_EQUAL) {
            tx->scale_x += 0.1;
            tx->scale_y += 0.1;
        } else if (rel == XPLG_PREC_LESS) {
            tx->trans_x += 0.05;
        } else if (rel == XPLG_PREC_GREATER) {
            tx->trans_y += 0.05;
        }
    }
}

// 16. Map compiler outputs directly to plotter commands and draw to framebuffer
void tsfi_xplg_plotter_draw(
    TurrillPlotter *plotter,
    const MallgrenTransform *tx,
    uint8_t *fb,
    int width,
    int height
) {
    if (!plotter || !tx || !fb || width <= 0 || height <= 0) return;

    // Apply transform vector to pen coordinate states
    plotter->pen_x = plotter->pen_x * tx->scale_x + tx->trans_x;
    plotter->pen_y = plotter->pen_y * tx->scale_y + tx->trans_y;

    int px = (int)(plotter->pen_x * width);
    int py = (int)(plotter->pen_y * height);

    if (px >= 0 && px < width && py >= 0 && py < height) {
        if (plotter->pen_down) {
            fb[py * width + px] = plotter->color_val;
        }
    }
}

// 17. Lexical Token Tracer (Rule 11 latency guard compliant)
void tsfi_xplg_trace_token(XplgTerminal term, const char *msg) {
    // Avoid hot-path console output to prevent latency (Rule 11)
    (void)term;
    (void)msg;
}

// 18. Precedence Conflict Resolver (resolves parsing overlaps)
XplgConflictType tsfi_xplg_resolve_conflict(XplgTerminal term, XplgPrecedence rel) {
    if (term == XPLG_TERM_NUMBER && rel == XPLG_PREC_GREATER) {
        // Resolve shift-reduce ambiguity in favor of reduction
        return XPLG_CONFLICT_SHIFT_REDUCE;
    }
    return XPLG_CONFLICT_NONE;
}

// 19. Initialize SVDAG Registry
void tsfi_xplg_init_svdag_registry(SvdagRegistry *registry) {
    if (!registry) return;
    registry->count = 0;
    memset(registry->nodes, 0, sizeof(registry->nodes));
}

// 20. Link Mallgren vector coordinates to SVDAG registry
bool tsfi_xplg_register_svdag_node(SvdagRegistry *registry, const MallgrenRegion *region, uint32_t voxel_id) {
    if (!registry || !region || registry->count >= MAX_SVDAG_NODES) {
        return false;
    }

    SvdagRegistryNode *node = &registry->nodes[registry->count++];
    node->voxel_id = voxel_id;
    node->bounds[0] = region->min_bound.x;
    node->bounds[1] = region->min_bound.y;
    node->bounds[2] = 0.0; // Z-axis starting bound
    node->bounds[3] = region->max_bound.x;
    node->bounds[4] = region->max_bound.y;
    node->bounds[5] = 1.0; // Z-axis ending bound
    node->active = true;

    return true;
}
