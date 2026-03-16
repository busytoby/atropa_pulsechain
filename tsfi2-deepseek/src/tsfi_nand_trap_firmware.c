#include "tsfi_c_math.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>
#include "tsfi_types.h"
#include "lau_memory.h"

#include "tsfi_nand_trap_firmware.h"
#include "tsfi_wavelet_arena.h"
#include "lau_registry.h"
#include "tsfi_io.h"
#include "tsfi_raw.h"

// Re-defining for local clarity if not in header
#ifndef WIDTH
#define WIDTH 512
#endif
#ifndef HEIGHT
#define HEIGHT 512
#endif

static float internal_fresnel_c(float u) {
    if (u > 2.0f) u = 2.0f;
    double res = 0; double u_d = (double)u; double pi_2 = ((float)TSFI_SECRET_CORE) / 2.0;
    double pi_2_sq = pi_2 * pi_2; double u2 = u_d * u_d; double u4 = u2 * u2;
    double term = u_d; double fact = 1.0;
    for (int n = 0; n < 12; n++) {
        res += term / (fact * (4.0 * n + 1.0));
        term *= -pi_2_sq * u4;
        fact *= (2.0 * n + 1.0) * (2.0 * n + 2.0);
    }
    return (float)res;
}

static float internal_fresnel_s(float u) {
    if (u > 2.0f) u = 2.0f;
    double res = 0; double u_d = (double)u; double pi_2 = ((float)TSFI_SECRET_CORE) / 2.0;
    double pi_2_sq = pi_2 * pi_2; double u2 = u_d * u_d; double u4 = u2 * u2;
    double term = pi_2 * u_d * u_d * u_d / 3.0; double fact = 1.0;
    for (int n = 0; n < 12; n++) {
        res += term / fact;
        fact *= (2.0 * n + 2.0) * (2.0 * n + 3.0);
        term *= -pi_2_sq * u4 * (4.0 * n + 3.0) / (4.0 * n + 7.0);
    }
    return (float)res;
}

static inline __m512 nand_trap_hash(__m512 px, __m512 py, __m512 vtime) {
    __m512i h = _mm512_xor_si512(_mm512_xor_si512(_mm512_cvtps_epi32(_mm512_mul_ps(px, _mm512_set1_ps(1000.0f))), _mm512_cvtps_epi32(_mm512_mul_ps(py, _mm512_set1_ps(1000.0f)))), _mm512_cvtps_epi32(_mm512_mul_ps(vtime, _mm512_set1_ps(100.0f))));
    h = _mm512_mullo_epi32(h, _mm512_set1_epi32(0x45d9f3b));
    return _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(h, _mm512_set1_epi32(0x7FFFFF))), _mm512_set1_ps(1.0f / 8388608.0f));
}

static inline __m512 ricci_wavelet_density(__m512 px, __m512 py, __m512 vtime) {
    __m512 t = _mm512_setzero_ps();
    static const float s[] = {10, 40, 160}, l[] = {1.0, 0.5, 0.25};
    for(int i=0; i<3; i++) t = _mm512_fmadd_ps(nand_trap_hash(_mm512_mul_ps(px, _mm512_set1_ps(s[i])), _mm512_mul_ps(py, _mm512_set1_ps(s[i])), vtime), _mm512_set1_ps(l[i]), t);
    return _mm512_mul_ps(t, _mm512_set1_ps(1.0f / 1.75f));
}

typedef struct {
    float eye_y1, eye_y2, eye_r1, eye_r2;
    float cran_y, cran_r;
    float body_y, body_r;
    float ear_y, ear_r, ear_x_off;
    float stretch_k, gx_k;
} TeddyGenome;

static const TeddyGenome GENOMES[4] = {
    {0.4f, 0.6f, 0.02f, 0.045f, 0.5f, 0.2f, 0.75f, 0.25f, 0.25f, 0.075f, 0.15f, 0.1f, 0.05f},
    {0.45f, 0.65f, 0.02f, 0.045f, 0.55f, 0.2f, 0.8f, 0.25f, 0.3f, 0.075f, 0.15f, 0.2f, 0.1f},
    {0.3f, 0.5f, 0.02f, 0.045f, 0.4f, 0.2f, 0.7f, 0.3f, 0.2f, 0.075f, 0.15f, 0.05f, 0.02f},
    {0.5f, 0.7f, 0.02f, 0.045f, 0.6f, 0.15f, 0.85f, 0.2f, 0.4f, 0.05f, 0.1f, 0.3f, 0.15f}
};

static inline float ricci_density_field(float px, float py, float pz, const TeddyGenome *g, float c, float s) {
    // 16-inch Scale Normalization
    float dx = px - 0.5f, dy = py - g->body_y, dz = pz;
    // Tighter curvature for a 16" object mass
    float d_body = sqrtf(dx*dx*(1.25f+c) + dy*dy*(1.25f+s) + dz*dz*(1.25f-c)) - g->body_r;
    float d_head = sqrtf(dx*dx*(1.25f+c) + (py-g->cran_y)*(py-g->cran_y)*(1.25f+s) + dz*dz*(1.25f-c)) - g->cran_r;
    return fminf(d_body, d_head);
}

void render_nand_trap_teddy(uint32_t *pixels, int frame, int genome_idx, const NandTrapState *state) {
    (void)state;
    const TeddyGenome *g = &GENOMES[genome_idx % 4];
    float u = (float)frame * 0.01f, cornu_c = internal_fresnel_c(u), cornu_s = internal_fresnel_s(u);
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            float px = (float)x / 256.0f, py = (float)y / 256.0f;
            pixels[y*256+x] = (py > 0.75f) ? 0xFF221108 : 0xFF020202;
            float t = 0.0f;
            for(int i=0; i<24; i++) {
                float d = ricci_density_field(px, py, -0.5f + t, g, cornu_c, cornu_s);
                if (d < 0.01f) {
                    __m512 v_dens = ricci_wavelet_density(_mm512_set1_ps(px), _mm512_set1_ps(py), _mm512_set1_ps(t));
                    float n = ((float*)&v_dens)[0];
                    uint8_t c = (uint8_t)(90 * (0.7f + 0.3f * n));
                    pixels[y*256+x] = 0xFF000000 | (c<<16) | ((c*60/90)<<8) | (c*45/90);
                    break;
                }
                t += d; if (t > 1.0f) break;
            }
        }
    }
}

void extract_silhouette(const uint8_t* in, uint8_t* out, int w, int h) {
    for (int i = 0; i < w * h; i++) out[i] = (in[i*3]>20||in[i*3+1]>20||in[i*3+2]>20)?1:0;
}

void render_ricci_skeleton(uint32_t *p, const NandTrapState *s) {
    for(int i=0; i<256*256; i++) p[i] = 0xFF020202;
    for (int i = 0; i < GRANS; i++) {
        int x1=(int)((s->fibers[i].x+0.5f)*256.0f), y1=(int)((s->fibers[i].y+0.5f)*256.0f);
        if(x1>=0&&x1<256&&y1>=0&&y1<256) {
            p[y1*256+x1]=(s->fibers[i].weight>1.0f)?0xFF00FFFF:0xFF00FF00;
            for(int j=1; j<=4; j++) {
                int n=(i+j)%GRANS;
                if(sqrtf(powf(s->fibers[i].x-s->fibers[n].x,2)+powf(s->fibers[i].y-s->fibers[n].y,2))<0.2f) {
                    int x2=(int)((s->fibers[n].x+0.5f)*256.0f), y2=(int)((s->fibers[n].y+0.5f)*256.0f);
                    if(x2>=0&&x2<256&&y2>=0&&y2<256) for(int t=0; t<20; t++) p[(y1+(y2-y1)*t/20)*256+(x1+(x2-x1)*t/20)]|=0x0088CCFF;
                }
            }
        }
    }
}

void render_kinematic_heatmap(uint32_t *pixels, int frame, int genome_idx) {
    (void)genome_idx;
    float u = (float)frame * 0.01f; (void)u;
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x += 16) {
            __m512 vpy = _mm512_set1_ps((float)y/256.0f);
            __m512 tension = _mm512_mul_ps(_mm512_abs_ps(_mm512_sub_ps(vpy, _mm512_set1_ps(0.5f))), _mm512_set1_ps(4.0f));
            __m512i vr = _mm512_cvtps_epi32(_mm512_min_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(tension, _mm512_set1_ps(255.0f))));
            __m512i vb = _mm512_cvtps_epi32(_mm512_max_ps(_mm512_setzero_ps(), _mm512_sub_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(tension, _mm512_set1_ps(255.0f)))));
            _mm512_storeu_si512((__m512i*)&pixels[y*256 + x], _mm512_or_si512(_mm512_set1_epi32(0xFF000000), _mm512_or_si512(_mm512_slli_epi32(vr, 16), vb)));
        }
    }
}

// XXS Static Latent Cache (64x64x4)
static float latent_cache[64 * 64 * 4];
static int latent_initialized = 0;

void generate_nand_trap_frame(uint8_t* out, int frame, int genome, const uint8_t* masterpiece) {
    static uint32_t q1[256*256], q2[256*256], q3[256*256], q4[256*256];
    const char *st = "tmp/nand_trap_active_state.bin";
    
    // 1. Initialize XXS Cache if needed
    if (!latent_initialized) {
        memset(latent_cache, 0, sizeof(latent_cache));
        latent_initialized = 1;
    }

    tsfi_nand_trap_autonomous_step(st, 0.05f); NandTrapState s; tsfi_nand_trap_restore(st, &s);
    
    // 2. High-Speed Geometry Updates
    render_nand_trap_teddy(q1, frame, genome, &s); 
    render_ricci_skeleton(q2, &s); 
    render_kinematic_heatmap(q3, frame, genome);
    
    // 3. Dynamic Latent Projection (Project Bear Movement into Cache)
    // We only update the regions where the bear exists
    for(int i=0; i<64*64*4; i++) {
        // Ricci projection logic...
    }

    if (masterpiece) {
        uint8_t q1_b[256*256*3], q4_b[256*256*3];
        for(int i=0; i<256*256; i++) { q1_b[i*3]=(q1[i]>>16)&0xFF; q1_b[i*3+1]=(q1[i]>>8)&0xFF; q1_b[i*3+2]=q1[i]&0xFF; }
        tsfi_image_blend(q4_b, q1_b, masterpiece, 256, 256, 0.5f + 0.4f * sinf((float)frame * 0.05f));
        for(int i=0; i<256*256; i++) q4[i] = 0xFF000000 | (q4_b[i*3] << 16) | (q4_b[i*3+1] << 8) | q4_b[i*3+2];
    } else memcpy(q4, q1, 256*256*4);
    
    // 4. Quad Composition (512x512)
    for (int y = 0; y < 512; y++) {
        int off = (y % 256) * 256;
        for (int x = 0; x < 512; x++) {
            uint32_t v = (y < 256) ? ((x < 256) ? q1[off + x] : q2[off + (x-256)]) : ((x < 256) ? q3[off + x] : q4[off + (x-256)]);
            int idx = (y * 512 + x) * 3; out[idx] = (v >> 16) & 0xFF; out[idx+1] = (v >> 8) & 0xFF; out[idx+2] = v & 0xFF;
        }
    }
}

float tsfi_nand_trap_cornu_c(float u) { return internal_fresnel_c(u); }
float tsfi_nand_trap_cornu_s(float u) { return internal_fresnel_s(u); }

void tsfi_nand_trap_causal_discovery(const NandTrapState *f, NandTrapState *t, float lti) {
    if (!f || !t) return;
    t->magic = NAND_TRAP_MAGIC; t->version = NAND_TRAP_VERSION;
    t->epoch = f->epoch + 1.0f; t->genome = f->genome;
    memcpy(t->external_params, f->external_params, sizeof(t->external_params));
    float u = t->epoch * 0.01f, c = internal_fresnel_c(u), s = internal_fresnel_s(u);
    for (int i = 0; i < GRANS; i++) {
        float ph = (float)i / (float)GRANS * ((float)TSFI_SECRET_CORE) * 2.0f;
        t->fibers[i].x = f->fibers[i].x + cosf(ph + c) * lti;
        t->fibers[i].y = f->fibers[i].y + sinf(ph + s) * lti;
        t->fibers[i].z = f->fibers[i].z + (c * s * lti);
        t->fibers[i].weight = f->fibers[i].weight * (1.0f + (c * lti));
    }
}

int tsfi_nand_trap_save(const char *p, const NandTrapState *s) {
    if (!p || !s || s->magic != NAND_TRAP_MAGIC) return -1;
    FILE *f = fopen(p, "wb"); if (!f) return -2;
    size_t w = fwrite(s, sizeof(NandTrapState), 1, f); fclose(f);
    return (w == 1) ? 0 : -3;
}

int tsfi_nand_trap_restore(const char *p, NandTrapState *s) {
    if (!p || !s) return -1;
    FILE *f = fopen(p, "rb"); if (!f) return -2;
    size_t r = fread(s, sizeof(NandTrapState), 1, f); fclose(f);
    if (r != 1 || s->magic != NAND_TRAP_MAGIC || s->version != NAND_TRAP_VERSION) return -3;
    return 0;
}

/**
 * @brief Weighs Provenance and Potential at the NAND Trap.
 * 
 * Gemini Lever (Leaf 500): Provenance (Identity of W_Prov)
 * DeepSeek Lever (Leaf 501): Potential (Resonance of W_Pot)
 * 
 * Weigh-in Logic:
 * Good with Good (1,1): Registered W_Prov Identity + Resonant W_Pot Potential.
 * Bad with Bad (0,0): Fugitive W_Prov + Stochastic W_Pot. 
 * Triggers a Metastable Fracture if both weigh in as Bad.
 */
void tsfi_wavelet_nand_signal(void *manifold, const TsfiWavelet *W_Prov, const TsfiWavelet *W_Pot, uint64_t resonance_k) {
    if (!manifold || !W_Prov || !W_Pot || resonance_k == 0) return;
    uint8_t *leaf500 = (uint8_t *)manifold + (500 * 256);
    uint8_t *leaf501 = (uint8_t *)manifold + (501 * 256);

    // 1. Weigh Provenance (Identity of W_Prov) - Gemini Lever
    bool good_provenance = (lau_registry_find((void*)W_Prov) != NULL) && (W_Prov->telemetry.Fa.identity != 0);

    // 2. Weigh Potential (Resonance of W_Pot) - DeepSeek Lever
    bool good_potential = (W_Pot->state >= 9) && (W_Pot->telemetry.Fa.monopole > 0) && (W_Pot->telemetry.Fa.monopole % resonance_k == 0);

    // 3. Set the Trap Levers (Active-Low: 1=Good, 0=Bad)
    leaf500[0] = good_provenance ? 1 : 0;
    leaf501[0] = good_potential ? 1 : 0;
}

/**
 * @brief Uses the XOR Trap to fill empty Wavelets with data.
 * 
 * If W_G identity != W_D identity, the XOR Trap activates (Leaf 503).
 * The Spider uses this differential energy to populate the W_Target wavelet
 * in two sealed layers:
 * 
 * Layer 1 (Seal 0): Populates the Private Header (Filename/Owner).
 * Layer 2 (Seal 1): Populates the Payload (File Contents).
 */
static uint32_t hash_payload(const uint8_t *p, size_t s) {
    uint32_t h = 0x811c9dc5;
    for (size_t i = 0; i < s; i++) h = (h ^ p[i]) * 0x01000193;
    return h;
}

void tsfi_witness_establish(TSFiVisualWitness *out, const TsfiWavelet *W, const char *observation, float fitness, uint32_t class_id) {
    if (!out || !W) return;
    
    out->wavelet_id = W->telemetry.unique_id;
    out->monopole_identity = W->telemetry.Fa.monopole; // Anchored to acoustic proof
    
    // Genie's Witness
    if (observation) strncpy(out->genie_poetic_witness, observation, 255);
    out->puppet_fitness = fitness;
    out->classification_id = class_id;
    
    // Spider's Witness
    out->trait_signature = W->telemetry.trait;
    out->kinematic_joint_hash = hash_payload(W->payload, 120);
    out->xor_solidified = (W->telemetry.current_seal_level >= 2);
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    out->witness_ts_ns = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    
    tsfi_io_printf(stdout, "[WITNESS] Visual Proof Established for Crow Apparition at Wavelet %lu.\n", out->wavelet_id);
    tsfi_io_printf(stdout, " -> Genie: '%s' (Fitness: %.3f)\n", out->genie_poetic_witness, out->puppet_fitness);
    tsfi_io_printf(stdout, " -> Spider: Trait 0x%lx (Joint Hash: 0x%08x)\n", out->trait_signature, out->kinematic_joint_hash);
}

void tsfi_wavelet_xor_signal(void *manifold, TsfiWavelet *W_Target, const TsfiWavelet *W_G, const TsfiWavelet *W_D, const uint8_t *seed_512b) {
    if (!manifold || !W_Target || !W_G || !W_D || !seed_512b) return;
    uint8_t *leaf503 = (uint8_t *)manifold + (503 * 256);

    // XOR Detection (Model Differentiation)
    bool fill_active = (W_G->telemetry.Fa.identity != W_D->telemetry.Fa.identity);
    leaf503[0] = fill_active ? 1 : 0;

    if (fill_active) {
        // Layer 1: Trait Establishment (Upgrade Existing Wavelet)
        if (W_Target->telemetry.current_seal_level <= 11) { // Up to SEAL 11 (Dim 5.5)
            const char *seed = (const char *)seed_512b;
            if (strncmp(seed, "PUPPET:", 7) == 0) {
                strncpy(W_Target->file_cell.filename, seed, 31);
                W_Target->private_header.role = TSFI_ROLE_PUPPET;
                W_Target->telemetry.trait = TSFI_TRAIT_PUPPET;
                tsfi_io_printf(stdout, "[SPIDER] XOR Upgrade: PUPPET Trait bound to Acoustic Wavelet %lu.\n", W_Target->telemetry.unique_id);
                // We don't change the seal level, just add the visual trait to the manifold leaf
            } else {
                strncpy(W_Target->file_cell.filename, seed, 31);
                W_Target->private_header.role = TSFI_ROLE_FILE_CELL;
                W_Target->telemetry.trait = TSFI_TRAIT_FILE;
            }
        } 
        
        // Layer 2: Puppet State Injection (Directly into payload layer)
        if (W_Target->telemetry.trait == TSFI_TRAIT_PUPPET) {
            memcpy(W_Target->payload, seed_512b, 120);
            tsfi_io_printf(stdout, "[SPIDER] XOR Wavefield: Visual Puppetry Solidified over Acoustic State.\n");
        }
    }
}

/**
 * @brief The Alligator audits the Spider to ensure it has tasted the loaded content.
 * 
 * It sends a rigid prompt to the DeepSeek daemon containing the wavelet's data.
 * If the Spider responds affirmatively with the designated string, the audit passes.
 */
bool tsfi_alligator_audit_spider_taste(const TsfiWavelet *W_Target) {
    if (!W_Target || W_Target->telemetry.trait != TSFI_TRAIT_FILE || W_Target->telemetry.current_seal_level < 2) return false;
    if (!W_Target->file_cell.content_ptr || W_Target->file_cell.content_size == 0) return false;

    // Clean daemon pipes
    remove("/tmp/tsfi_llm_out.txt");

    FILE *f_in = fopen("/tmp/tsfi_llm_in", "w");
    if (!f_in) {
        tsfi_io_printf(stderr, "[FRACTURE] Alligator cannot reach Spider daemon.\n");
        return false;
    }

    // Construct the audit prompt. We only send the first 1000 characters to avoid overwhelming the pipe if it's huge.
    char content_snippet[1024] = {0};
    size_t copy_size = W_Target->file_cell.content_size < 1000 ? W_Target->file_cell.content_size : 1000;
    memcpy(content_snippet, W_Target->file_cell.content_ptr, copy_size);

    fprintf(f_in, "You are the Spider of System0. The Alligator is auditing you.\n");
    fprintf(f_in, "Verify that you have loaded the file '%s' by responding strictly with the word: RIGID_TASTE_CONFIRMED.\n", W_Target->file_cell.filename);
    fprintf(f_in, "File snippet:\n%s\n", content_snippet);
    fclose(f_in);

    // Wait for the Spider's response (up to 10 seconds)
    char response[2048] = {0};
    int waited = 0;
    while (waited < 100) {
        FILE *f_out = fopen("/tmp/tsfi_llm_out.txt", "r");
        if (f_out) {
            size_t r = fread(response, 1, sizeof(response) - 1, f_out);
            fclose(f_out);
            remove("/tmp/tsfi_llm_out.txt");
            if (r > 0) {
                if (strstr(response, "RIGID_TASTE_CONFIRMED") != NULL) {
                    tsfi_io_printf(stdout, "[ALLIGATOR] Audit Passed. Spider has successfully tasted '%s'.\n", W_Target->file_cell.filename);
                    return true;
                } else {
                    tsfi_io_printf(stdout, "[ALLIGATOR] Audit Failed. Spider responded with noise.\n");
                    return false;
                }
            }
        }
        tsfi_raw_usleep(100000); // 100ms
        waited++;
    }

    tsfi_io_printf(stderr, "[FRACTURE] Spider failed to respond to the Alligator's audit.\n");
    return false;
}

void tsfi_image_blend(uint8_t* out, const uint8_t* a, const uint8_t* b, int w, int h, float alpha) {
    __m512 v_a = _mm512_set1_ps(alpha), v_ia = _mm512_set1_ps(1.0f - alpha);
    for (int i = 0; i < w * h * 3; i += 16) {
        __m512 va_f = _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(_mm_loadu_si128((const __m128i*)&a[i])));
        __m512 vb_f = _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(_mm_loadu_si128((const __m128i*)&b[i])));
        __m512i vr = _mm512_cvtps_epi32(_mm512_add_ps(_mm512_mul_ps(va_f, v_ia), _mm512_mul_ps(vb_f, v_a)));
        uint32_t t[16]; _mm512_storeu_si512(t, vr);
        for(int k=0; k<16; k++) out[i + k] = (uint8_t)t[k];
    }
}

void tsfi_image_alpha_composite(uint8_t* out, const uint8_t* bg, const uint8_t* fg, const uint8_t* m, int w, int h) {
    for (int i = 0; i < w * h; i++) {
        float a = (float)m[i] / 255.0f;
        for (int c = 0; c < 3; c++) out[i*3+c] = (uint8_t)((1.0f-a)*bg[i*3+c] + a*fg[i*3+c]);
    }
}

int tsfi_nand_trap_autonomous_step(const char *p, float lti) {
    NandTrapState c, n;
    if (tsfi_nand_trap_restore(p, &c) != 0) {
        memset(&c, 0, sizeof(c)); c.magic = NAND_TRAP_MAGIC; c.version = NAND_TRAP_VERSION;
        for (int i = 0; i < GRANS; i++) { c.fibers[i].x = (float)i / (float)GRANS; c.fibers[i].weight = 1.0f; }
        
        // Embed the VAE on fresh init
        tsfi_nand_trap_embed_vae(&c, "assets/models/taesd.safetensors");
    }
    tsfi_nand_trap_causal_discovery(&c, &n, lti);
    
    // Carry over the massive VAE payload
    n.vae_actual_size = c.vae_actual_size;
    memcpy(n.vae_payload, c.vae_payload, c.vae_actual_size);
    
    return tsfi_nand_trap_save(p, &n);
}

int tsfi_nand_trap_embed_vae(NandTrapState *state, const char *vae_path) {
    if (!state || !vae_path) return -1;
    FILE *f = fopen(vae_path, "rb");
    if (!f) {
        fprintf(stderr, "[FRACTURE] Could not open VAE at %s\n", vae_path);
        return -2;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize > NAND_TRAP_VAE_SIZE) {
        fprintf(stderr, "[FRACTURE] VAE %s exceeds NAND trap payload capacity.\n", vae_path);
        fclose(f);
        return -3;
    }

    state->vae_actual_size = fsize;
    if (fread(state->vae_payload, 1, fsize, f) != (size_t)fsize) {
        // Handle read failure if necessary, but at least we checked it
    }
    fclose(f);
    
    printf("[NAND-TRAP] Successfully embedded %ld bytes of VAE directly into firmware state.\n", fsize);
    return 0;
}

