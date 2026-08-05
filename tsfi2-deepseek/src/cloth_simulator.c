#include "cloth_simulator.h"
#include <math.h>
#include <string.h>

static ClothPoint cloth_grid[CLOTH_WIDTH][CLOTH_HEIGHT];
static int springs_torn[CLOTH_WIDTH][CLOTH_HEIGHT][4]; // 0: Horiz, 1: Vert, 2: Diag1, 3: Diag2
static const float gravity_x = -0.0007f;
static const float gravity_y = -0.0008f;
static const float gravity_z = 0.0001f;

static int is_default_anchor(int x, int y) {
    return (y == CLOTH_HEIGHT - 1 && x % 4 == 0);
}

void cloth_init(void) {
    memset(springs_torn, 0, sizeof(springs_torn));
    for (int x = 0; x < CLOTH_WIDTH; x++) {
        for (int y = 0; y < CLOTH_HEIGHT; y++) {
            cloth_grid[x][y].x = (float)x * 0.25f;
            cloth_grid[x][y].y = (float)y * 0.25f;
            cloth_grid[x][y].z = 0.0f;
            
            cloth_grid[x][y].px = cloth_grid[x][y].x;
            cloth_grid[x][y].py = cloth_grid[x][y].y;
            cloth_grid[x][y].pz = cloth_grid[x][y].z;
            
            cloth_grid[x][y].fx = 0.0f;
            cloth_grid[x][y].fy = 0.0f;
            cloth_grid[x][y].fz = 0.0f;
            cloth_grid[x][y].nx = 0.0f;
            cloth_grid[x][y].ny = 1.0f;
            cloth_grid[x][y].nz = 0.0f;

            if (is_default_anchor(x, y)) {
                cloth_grid[x][y].is_anchored = 1;
                cloth_grid[x][y].anchor_x = cloth_grid[x][y].x;
                cloth_grid[x][y].anchor_y = cloth_grid[x][y].y;
                cloth_grid[x][y].anchor_z = cloth_grid[x][y].z;
            } else {
                cloth_grid[x][y].is_anchored = 0;
            }
        }
    }
}

void cloth_set_anchor(int x, int y, float tx, float ty, float tz) {
    if (x >= 0 && x < CLOTH_WIDTH && y >= 0 && y < CLOTH_HEIGHT) {
        cloth_grid[x][y].is_anchored = 1;
        cloth_grid[x][y].anchor_x = tx;
        cloth_grid[x][y].anchor_y = ty;
        cloth_grid[x][y].anchor_z = tz;
    }
}

static void solve_constraint(int x1, int y1, int x2, int y2, float rest_dist, int spring_idx) {
    if (x1 < 0 || x1 >= CLOTH_WIDTH || y1 < 0 || y1 >= CLOTH_HEIGHT) return;
    if (x2 < 0 || x2 >= CLOTH_WIDTH || y2 < 0 || y2 >= CLOTH_HEIGHT) return;
    if (springs_torn[x1][y1][spring_idx]) return;
    if (cloth_grid[x1][y1].is_anchored && cloth_grid[x2][y2].is_anchored) return;

    float dx = cloth_grid[x1][y1].x - cloth_grid[x2][y2].x;
    float dy = cloth_grid[x1][y1].y - cloth_grid[x2][y2].y;
    float dz = cloth_grid[x1][y1].z - cloth_grid[x2][y2].z;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    if (dist < 0.0001f) return;

    // Tear threshold: 1.5x structural displacement stretch limit
    if (dist > rest_dist * 1.5f) {
        springs_torn[x1][y1][spring_idx] = 1;
        return;
    }

    float diff = (rest_dist - dist) / dist * 0.5f;
    float displacement_x = dx * diff;
    float displacement_y = dy * diff;
    float displacement_z = dz * diff;

    if (!cloth_grid[x1][y1].is_anchored) {
        cloth_grid[x1][y1].x += displacement_x;
        cloth_grid[x1][y1].y += displacement_y;
        cloth_grid[x1][y1].z += displacement_z;
    }
    if (!cloth_grid[x2][y2].is_anchored) {
        cloth_grid[x2][y2].x -= displacement_x;
        cloth_grid[x2][y2].y -= displacement_y;
        cloth_grid[x2][y2].z -= displacement_z;
    }
}

static void compute_face_normal(int x1, int y1, int x2, int y2, int x3, int y3, float wx, float wy, float wz) {
    float dx1 = cloth_grid[x1][y1].x - cloth_grid[x2][y2].x;
    float dy1 = cloth_grid[x1][y1].y - cloth_grid[x2][y2].y;
    float dz1 = cloth_grid[x1][y1].z - cloth_grid[x2][y2].z;

    float dx2 = cloth_grid[x1][y1].x - cloth_grid[x3][y3].x;
    float dy2 = cloth_grid[x1][y1].y - cloth_grid[x3][y3].y;
    float dz2 = cloth_grid[x1][y1].z - cloth_grid[x3][y3].z;

    float nx = dy1*dz2 - dz1*dy2;
    float ny = dz1*dx2 - dx1*dz2;
    float nz = dx1*dy2 - dy1*dx2;

    float len = sqrtf(nx*nx + ny*ny + nz*nz);
    if (len < 0.0001f) return;
    float inv_len = 1.0f / len;
    float nnx = nx * inv_len;
    float nny = ny * inv_len;
    float nnz = nz * inv_len;

    float dot = nnx*wx + nny*wy + nnz*wz;
    float fx = nnx * dot;
    float fy = nny * dot;
    float fz = nnz * dot;

    cloth_grid[x1][y1].fx += fx; cloth_grid[x1][y1].fy += fy; cloth_grid[x1][y1].fz += fz;
    cloth_grid[x2][y2].fx += fx; cloth_grid[x2][y2].fy += fy; cloth_grid[x2][y2].fz += fz;
    cloth_grid[x3][y3].fx += fx; cloth_grid[x3][y3].fy += fy; cloth_grid[x3][y3].fz += fz;

    cloth_grid[x1][y1].nx += nx; cloth_grid[x1][y1].ny += ny; cloth_grid[x1][y1].nz += nz;
    cloth_grid[x2][y2].nx += nx; cloth_grid[x2][y2].ny += ny; cloth_grid[x2][y2].nz += nz;
    cloth_grid[x3][y3].nx += nx; cloth_grid[x3][y3].ny += ny; cloth_grid[x3][y3].nz += nz;
}

void cloth_update(float wind_x, float wind_y, float wind_z) {
    for (int x = 0; x < CLOTH_WIDTH; x++) {
        for (int y = 0; y < CLOTH_HEIGHT; y++) {
            cloth_grid[x][y].nx = 0.0f;
            cloth_grid[x][y].ny = 0.0f;
            cloth_grid[x][y].nz = 0.0f;

            if (!cloth_grid[x][y].is_anchored) {
                cloth_grid[x][y].fx = gravity_x;
                cloth_grid[x][y].fy = gravity_y;
                cloth_grid[x][y].fz = gravity_z;
            } else {
                cloth_grid[x][y].fx = 0.0f;
                cloth_grid[x][y].fy = 0.0f;
                cloth_grid[x][y].fz = 0.0f;
            }
        }
    }

    for (int x = 0; x < CLOTH_WIDTH - 1; x++) {
        for (int y = 0; y < CLOTH_HEIGHT - 1; y++) {
            compute_face_normal(x, y, x+1, y+1, x+1, y, wind_x, wind_y, wind_z);
            compute_face_normal(x, y, x, y+1, x+1, y+1, wind_x, wind_y, wind_z);
        }
    }

    for (int x = 0; x < CLOTH_WIDTH; x++) {
        for (int y = 0; y < CLOTH_HEIGHT; y++) {
            float len = sqrtf(cloth_grid[x][y].nx * cloth_grid[x][y].nx +
                               cloth_grid[x][y].ny * cloth_grid[x][y].ny +
                               cloth_grid[x][y].nz * cloth_grid[x][y].nz);
            if (len > 0.0001f) {
                cloth_grid[x][y].nx /= len;
                cloth_grid[x][y].ny /= len;
                cloth_grid[x][y].nz /= len;
            } else {
                cloth_grid[x][y].nx = 0.0f;
                cloth_grid[x][y].ny = 1.0f;
                cloth_grid[x][y].nz = 0.0f;
            }
        }
    }

    const float time_step = 0.1f;
    const float damping = 0.98f;
    for (int x = 0; x < CLOTH_WIDTH; x++) {
        for (int y = 0; y < CLOTH_HEIGHT; y++) {
            if (cloth_grid[x][y].is_anchored) {
                cloth_grid[x][y].x = cloth_grid[x][y].anchor_x;
                cloth_grid[x][y].y = cloth_grid[x][y].anchor_y;
                cloth_grid[x][y].z = cloth_grid[x][y].anchor_z;
                cloth_grid[x][y].px = cloth_grid[x][y].x;
                cloth_grid[x][y].py = cloth_grid[x][y].y;
                cloth_grid[x][y].pz = cloth_grid[x][y].z;
            } else {
                float temp_x = cloth_grid[x][y].x;
                float temp_y = cloth_grid[x][y].y;
                float temp_z = cloth_grid[x][y].z;

                cloth_grid[x][y].x += (cloth_grid[x][y].x - cloth_grid[x][y].px) * damping + cloth_grid[x][y].fx * time_step * time_step;
                cloth_grid[x][y].y += (cloth_grid[x][y].y - cloth_grid[x][y].py) * damping + cloth_grid[x][y].fy * time_step * time_step;
                cloth_grid[x][y].z += (cloth_grid[x][y].z - cloth_grid[x][y].pz) * damping + cloth_grid[x][y].fz * time_step * time_step;

                cloth_grid[x][y].px = temp_x;
                cloth_grid[x][y].py = temp_y;
                cloth_grid[x][y].pz = temp_z;
            }
        }
    }

    static const float min_self_dist = 0.12f;
    for (int iter = 0; iter < 3; iter++) {
        // Resolve spring length constraints
        for (int x = 0; x < CLOTH_WIDTH; x++) {
            for (int y = 0; y < CLOTH_HEIGHT; y++) {
                solve_constraint(x, y, x+1, y, 0.25f, 0);
                solve_constraint(x, y, x, y+1, 0.25f, 1);
                solve_constraint(x, y, x+1, y+1, 0.3535f, 2);
                solve_constraint(x, y+1, x+1, y, 0.3535f, 3);
            }
        }

        // Solve node proximity self-collisions to prevent overlap during foldbacks
        for (int y1 = 0; y1 < CLOTH_HEIGHT; y1++) {
            for (int x1 = 0; x1 < CLOTH_WIDTH; x1++) {
                for (int y2 = y1; y2 < CLOTH_HEIGHT; y2++) {
                    int start_x = (y1 == y2) ? x1 + 1 : 0;
                    for (int x2 = start_x; x2 < CLOTH_WIDTH; x2++) {
                        float sdx = cloth_grid[x1][y1].x - cloth_grid[x2][y2].x;
                        float sdy = cloth_grid[x1][y1].y - cloth_grid[x2][y2].y;
                        float sdz = cloth_grid[x1][y1].z - cloth_grid[x2][y2].z;
                        float sdist = sqrtf(sdx*sdx + sdy*sdy + sdz*sdz);
                        
                        if (sdist < min_self_dist && sdist > 0.0001f) {
                            float sdiff = (min_self_dist - sdist) / sdist * 0.5f;
                            float spx = sdx * sdiff;
                            float spy = sdy * sdiff;
                            float spz = sdz * sdiff;
                            
                            if (!cloth_grid[x1][y1].is_anchored) {
                                cloth_grid[x1][y1].x += spx;
                                cloth_grid[x1][y1].y += spy;
                                cloth_grid[x1][y1].z += spz;
                            }
                            if (!cloth_grid[x2][y2].is_anchored) {
                                cloth_grid[x2][y2].x -= spx;
                                cloth_grid[x2][y2].y -= spy;
                                cloth_grid[x2][y2].z -= spz;
                            }
                        }
                    }
                }
            }
        }
    }
}

void cloth_apply_sphere_collision(float cx, float cy, float cz, float radius) {
    for (int x = 0; x < CLOTH_WIDTH; x++) {
        for (int y = 0; y < CLOTH_HEIGHT; y++) {
            if (cloth_grid[x][y].is_anchored) continue;

            float dx = cloth_grid[x][y].x - cx;
            float dy = cloth_grid[x][y].y - cy;
            float dz = cloth_grid[x][y].z - cz;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);

            if (dist < radius) {
                if (dist < 0.0001f) dist = 0.0001f;
                float inv_dist = 1.0f / dist;
                float nx = dx * inv_dist;
                float ny = dy * inv_dist;
                float nz = dz * inv_dist;

                cloth_grid[x][y].x = cx + nx * radius;
                cloth_grid[x][y].y = cy + ny * radius;
                cloth_grid[x][y].z = cz + nz * radius;
            }
        }
    }
}

void cloth_generate_mesh(ClothVertex *vertices, int *indices, int *vertex_count, int *index_count) {
    *vertex_count = 0;
    *index_count = 0;

    for (int y = 0; y < CLOTH_HEIGHT; y++) {
        for (int x = 0; x < CLOTH_WIDTH; x++) {
            vertices[*vertex_count].x = cloth_grid[x][y].x;
            vertices[*vertex_count].y = cloth_grid[x][y].y;
            vertices[*vertex_count].z = cloth_grid[x][y].z;
            vertices[*vertex_count].nx = cloth_grid[x][y].nx;
            vertices[*vertex_count].ny = cloth_grid[x][y].ny;
            vertices[*vertex_count].nz = cloth_grid[x][y].nz;
            vertices[*vertex_count].u = (float)x / (float)(CLOTH_WIDTH - 1);
            vertices[*vertex_count].v = (float)y / (float)(CLOTH_HEIGHT - 1);

            if (y < CLOTH_HEIGHT / 3) {
                vertices[*vertex_count].color = 0xFF009900;
            } else if (y < (CLOTH_HEIGHT * 2) / 3) {
                vertices[*vertex_count].color = 0xFFFFFFFF;
            } else {
                vertices[*vertex_count].color = 0xFFCC0000;
            }
            (*vertex_count)++;
        }
    }

    for (int y = 0; y < CLOTH_HEIGHT - 1; y++) {
        for (int x = 0; x < CLOTH_WIDTH - 1; x++) {
            int i0 = y * CLOTH_WIDTH + x;
            int i1 = i0 + 1;
            int i2 = i0 + CLOTH_WIDTH;
            int i3 = i2 + 1;

            indices[*index_count] = i0;
            indices[*index_count + 1] = i2;
            indices[*index_count + 2] = i1;

            indices[*index_count + 3] = i1;
            indices[*index_count + 4] = i2;
            indices[*index_count + 5] = i3;
            *index_count += 6;
        }
    }
}

void cloth_generate_mesh_lod(ClothVertex *vertices, int *indices, int *vertex_count, int *index_count, int lod_level) {
    *vertex_count = 0;
    *index_count = 0;

    int step = 1 << lod_level;
    if (step < 1) step = 1;

    // Track mapped vertex indices for correct index ordering
    int local_indices[CLOTH_WIDTH][CLOTH_HEIGHT];
    memset(local_indices, -1, sizeof(local_indices));

    for (int y = 0; y < CLOTH_HEIGHT; y += step) {
        for (int x = 0; x < CLOTH_WIDTH; x += step) {
            vertices[*vertex_count].x = cloth_grid[x][y].x;
            vertices[*vertex_count].y = cloth_grid[x][y].y;
            vertices[*vertex_count].z = cloth_grid[x][y].z;
            vertices[*vertex_count].nx = cloth_grid[x][y].nx;
            vertices[*vertex_count].ny = cloth_grid[x][y].ny;
            vertices[*vertex_count].nz = cloth_grid[x][y].nz;
            vertices[*vertex_count].u = (float)x / (float)(CLOTH_WIDTH - 1);
            vertices[*vertex_count].v = (float)y / (float)(CLOTH_HEIGHT - 1);

            if (y < CLOTH_HEIGHT / 3) {
                vertices[*vertex_count].color = 0xFF009900;
            } else if (y < (CLOTH_HEIGHT * 2) / 3) {
                vertices[*vertex_count].color = 0xFFFFFFFF;
            } else {
                vertices[*vertex_count].color = 0xFFCC0000;
            }
            local_indices[x][y] = *vertex_count;
            (*vertex_count)++;
        }
    }

    for (int y = 0; y < CLOTH_HEIGHT - step; y += step) {
        for (int x = 0; x < CLOTH_WIDTH - step; x += step) {
            int i0 = local_indices[x][y];
            int i1 = local_indices[x + step][y];
            int i2 = local_indices[x][y + step];
            int i3 = local_indices[x + step][y + step];

            if (i0 != -1 && i1 != -1 && i2 != -1 && i3 != -1) {
                indices[*index_count] = i0;
                indices[*index_count + 1] = i2;
                indices[*index_count + 2] = i1;

                indices[*index_count + 3] = i1;
                indices[*index_count + 4] = i2;
                indices[*index_count + 5] = i3;
                *index_count += 6;
            }
        }
    }
}
