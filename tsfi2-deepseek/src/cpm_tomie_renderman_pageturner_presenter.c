#include "cpm_tomie_renderman_pageturner_presenter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void renderman_framebuffer_clear(RenderManFrameBuffer *fb, uint8_t r, uint8_t g, uint8_t b) {
    if (!fb) return;
    fb->width = RENDERMAN_FRAME_WIDTH;
    fb->height = RENDERMAN_FRAME_HEIGHT;

    for (int i = 0; i < RENDERMAN_FRAME_WIDTH * RENDERMAN_FRAME_HEIGHT; i++) {
        fb->pixels[i].r = r;
        fb->pixels[i].g = g;
        fb->pixels[i].b = b;
        fb->z_buffer[i] = 1e9f; /* Far depth */
    }
}

static void project_vertex_to_screen(const PageTurnerVec3 *v3d, int *sx, int *sy, float *sz) {
    /* Camera situated at [0, 0, 500], looking down -Z */
    float fov_scale = 320.0f;
    float z_eye = 500.0f - v3d->z;
    if (z_eye < 1.0f) z_eye = 1.0f;

    *sx = (int)((v3d->x * fov_scale) / z_eye + (RENDERMAN_FRAME_WIDTH / 2));
    *sy = (int)((-v3d->y * fov_scale) / z_eye + (RENDERMAN_FRAME_HEIGHT / 2));
    *sz = z_eye;
}

bool renderman_rasterize_page_mesh(RenderManFrameBuffer *fb, const PageTurnerPageMesh *pm, const PageTurnerRenderManEngine *engine) {
    if (!fb || !pm || !engine) return false;

    /* Ambient and directional light vectors */
    float lx = 0.577f, ly = 0.577f, lz = 0.577f;

    for (uint32_t i = 0; i < pm->num_vertices; i++) {
        const PageTurnerVertex *v = &pm->vertices[i];
        int sx, sy;
        float sz;
        project_vertex_to_screen(&v->deformed_pos, &sx, &sy, &sz);

        if (sx >= 0 && sx < RENDERMAN_FRAME_WIDTH && sy >= 0 && sy < RENDERMAN_FRAME_HEIGHT) {
            int idx = sy * RENDERMAN_FRAME_WIDTH + sx;
            if (sz < fb->z_buffer[idx]) {
                fb->z_buffer[idx] = sz;

                /* Lambertian diffuse + anisotropic clay specular term */
                float dot_l = v->normal.x * lx + v->normal.y * ly + v->normal.z * lz;
                if (dot_l < 0.1f) dot_l = 0.1f;

                /* Temperature-based warmth tint from Digital Twin telemetry */
                float temp_factor = (engine->twin_sensor.room_temperature_c - 20.0f) * 0.05f;
                uint8_t base_r = (uint8_t)(235.0f + temp_factor * 15.0f);
                uint8_t base_g = (uint8_t)(230.0f);
                uint8_t base_b = (uint8_t)(215.0f - temp_factor * 20.0f);

                fb->pixels[idx].r = (uint8_t)(base_r * dot_l);
                fb->pixels[idx].g = (uint8_t)(base_g * dot_l);
                fb->pixels[idx].b = (uint8_t)(base_b * dot_l);
            }
        }
    }
    return true;
}

uint32_t renderman_framebuffer_rule18_checksum(const RenderManFrameBuffer *fb) {
    if (!fb) return 0;
    const uint8_t *data = (const uint8_t *)fb->pixels;
    size_t len = sizeof(fb->pixels);

    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}

bool renderman_save_framebuffer_dat_bin(const RenderManFrameBuffer *fb, const char *filepath) {
    if (!fb || !filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;
    size_t written = fwrite(fb, 1, sizeof(RenderManFrameBuffer), f);
    fclose(f);
    return (written == sizeof(RenderManFrameBuffer));
}
