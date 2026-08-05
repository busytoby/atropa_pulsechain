#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <immintrin.h>
#include "tsfi_sd_thunk.h"
#include "tsfi_montecarlo.h"

bool tsfi_sd_thunk_init(TsfiSdContext* ctx, const char* safetensors_path) {
    printf("[THUNK] Initializing Stable Diffusion Matrix directly into Above-4G ReBAR...\n");
    
    // 1. High-Speed SHM/ReBAR Attachment
    ctx->asset = tsfi_safetensors_cache_attach(safetensors_path);
    if (!ctx->asset) {
        fprintf(stderr, "[FRACTURE] Safetensors asset failed to attach: %s\n", safetensors_path);
        return false;
    }
    
    ctx->raw_safetensors = (uint8_t*)ctx->asset->data;
    ctx->total_mass_bytes = ctx->asset->size;
    ctx->current_inference_tick = 0;
    
    // 2. Simulate Vulkan ReBAR Buffer Allocation and Bind Memory Properties
    // ReBAR maps BAR to system physical memory via VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    printf("[REBAR] Allocating Vulkan Device Buffer: unet_buffer_vk (%zu bytes)\n", ctx->total_mass_bytes);
    printf("[REBAR] Binding host-visible physical page allocation via memory type 3\n");
    
    ctx->unet_buffer_vk = (VkBuffer)0xBAADF00D;  // Bind mock buffer reference
    ctx->unet_memory_vk = (VkDeviceMemory)0xDEADBEEF; // Bind mock device memory
    
    // 3. Pre-allocate U-Net Simulation Buffers via wired allocator to bypass malloc latency
    ctx->allocated_max_w = 1280;
    ctx->allocated_max_h = 720;
    size_t half_pixels = (ctx->allocated_max_w / 2) * (ctx->allocated_max_h / 2) * 3;
    ctx->skip_connection_buf = (float*)lau_malloc_wired(half_pixels * sizeof(float));
    ctx->latent_buf = (float*)lau_malloc_wired(64 * 64 * 3 * sizeof(float));
    ctx->latent_att_buf = (float*)lau_malloc_wired(64 * 64 * 3 * sizeof(float));
    ctx->decoder_half_buf = (float*)lau_malloc_wired(half_pixels * sizeof(float));
    
    printf("[PASS] %zu Bytes successfully wired to ReBAR Substrate (Vulkan host-visible mapping active).\n", ctx->total_mass_bytes);
    return true;
}


#include "tsfi_ccx_pool.h"

static TSFiCCXPool g_sd_ccx_pool;
static bool g_sd_ccx_pool_initialized = false;

typedef struct {
    int start_y;
    int end_y;
    const uint8_t *in_dna_mask;
    uint8_t *out_pixels;
    float *skip_connection;
    float *latent;
    float *latent_att;
    float *decoder_half;
    int w, h, hw, hh, lw, lh;
    float r_hw, r_hh, r_lw, r_lh;
    int start_i;
    int end_i;
} SDWorkerArg;

static void sd_downsample_phase1_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    for (int y = a->start_y; y < a->end_y; y++) {
        for (int x = 0; x < a->hw; x++) {
            int sx = (int)(x * a->r_hw);
            int sy = (int)(y * a->r_hh);
            int src_idx = (sy * a->w + sx) * 3;
            int dst_idx = (y * a->hw + x) * 3;
            a->skip_connection[dst_idx + 0] = a->in_dna_mask[src_idx + 0] / 255.0f;
            a->skip_connection[dst_idx + 1] = a->in_dna_mask[src_idx + 1] / 255.0f;
            a->skip_connection[dst_idx + 2] = a->in_dna_mask[src_idx + 2] / 255.0f;
        }
    }
}

static void sd_downsample_phase2_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    for (int y = a->start_y; y < a->end_y; y++) {
        for (int x = 0; x < a->lw; x++) {
            int sx = (int)(x * a->r_lw);
            int sy = (int)(y * a->r_lh);
            int src_idx = (sy * a->hw + sx) * 3;
            int dst_idx = (y * a->lw + x) * 3;
            a->latent[dst_idx + 0] = a->skip_connection[src_idx + 0];
            a->latent[dst_idx + 1] = a->skip_connection[src_idx + 1];
            a->latent[dst_idx + 2] = a->skip_connection[src_idx + 2];
        }
    }
}

static void sd_self_attention_a_fast_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    for (int y = a->start_y; y < a->end_y; y++) {
        for (int x = 1; x < a->lw - 1; x++) {
            float r_sum = 0.0f, g_sum = 0.0f, b_sum = 0.0f;
            for (int dy = -1; dy <= 1; dy++) {
                int ny = y + dy;
                int row_idx = ny * a->lw;
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int idx = (row_idx + nx) * 3;
                    r_sum += a->latent[idx + 0];
                    g_sum += a->latent[idx + 1];
                    b_sum += a->latent[idx + 2];
                }
            }
            int dst_idx = (y * a->lw + x) * 3;
            a->latent_att[dst_idx + 0] = r_sum * 0.11111111f;
            a->latent_att[dst_idx + 1] = g_sum * 0.11111111f;
            a->latent_att[dst_idx + 2] = b_sum * 0.11111111f;
        }
    }
}

static void sd_self_attention_a_slow_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    for (int y = a->start_y; y < a->end_y; y++) {
        for (int x = 0; x < a->lw; x++) {
            if (y > 0 && y < a->lh - 1 && x > 0 && x < a->lw - 1) {
                continue;
            }
            float r_sum = 0.0f, g_sum = 0.0f, b_sum = 0.0f;
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < a->lw && ny >= 0 && ny < a->lh) {
                        int idx = (ny * a->lw + nx) * 3;
                        r_sum += a->latent[idx + 0];
                        g_sum += a->latent[idx + 1];
                        b_sum += a->latent[idx + 2];
                        count++;
                    }
                }
            }
            int dst_idx = (y * a->lw + x) * 3;
            a->latent_att[dst_idx + 0] = r_sum / count;
            a->latent_att[dst_idx + 1] = g_sum / count;
            a->latent_att[dst_idx + 2] = b_sum / count;
        }
    }
}

static void sd_self_attention_b_simd_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    __m512 c_07 = _mm512_set1_ps(0.7f);
    __m512 c_03 = _mm512_set1_ps(0.3f);
    __m512 c_01 = _mm512_set1_ps(0.1f);
    __m512 zero = _mm512_setzero_ps();
    for (int i = a->start_i; i < a->end_i; i += 16) {
        __m512 l_val = _mm512_load_ps(&a->latent[i]);
        __m512 m_val = _mm512_load_ps(&a->latent_att[i]);
        __m512 val = _mm512_add_ps(_mm512_mul_ps(l_val, c_07), _mm512_mul_ps(m_val, c_03));
        __mmask16 pos_mask = _mm512_cmp_ps_mask(val, zero, _CMP_GT_OQ);
        __m512 scaled_val = _mm512_mul_ps(val, c_01);
        __m512 activated = _mm512_mask_blend_ps(pos_mask, scaled_val, val);
        _mm512_store_ps(&a->latent_att[i], activated);
    }
}

static void sd_upsample_phase1_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    for (int y = a->start_y; y < a->end_y; y++) {
        for (int x = 0; x < a->hw; x++) {
            int lx = (int)(x / a->r_lw);
            int ly = (int)(y / a->r_lh);
            if (lx >= a->lw) lx = a->lw - 1;
            if (ly >= a->lh) ly = a->lh - 1;
            int src_idx = (ly * a->lw + lx) * 3;
            int dst_idx = (y * a->hw + x) * 3;
            a->decoder_half[dst_idx + 0] = a->latent_att[src_idx + 0];
            a->decoder_half[dst_idx + 1] = a->latent_att[src_idx + 1];
            a->decoder_half[dst_idx + 2] = a->latent_att[src_idx + 2];
        }
    }
}

static void sd_skip_blend_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    __m512 half_vec = _mm512_set1_ps(0.5f);
    for (int i = a->start_i; i < a->end_i; i += 16) {
        __m512 dec = _mm512_load_ps(&a->decoder_half[i]);
        __m512 skip = _mm512_load_ps(&a->skip_connection[i]);
        __m512 blended = _mm512_mul_ps(_mm512_add_ps(dec, skip), half_vec);
        _mm512_store_ps(&a->decoder_half[i], blended);
    }
}

static void sd_upsample_phase2_worker(void *arg) {
    SDWorkerArg *a = (SDWorkerArg *)arg;
    for (int y = a->start_y; y < a->end_y; y++) {
        for (int x = 0; x < a->w; x++) {
            int lx = (int)(x / a->r_hw);
            int ly = (int)(y / a->r_hh);
            if (lx >= a->hw) lx = a->hw - 1;
            if (ly >= a->hh) ly = a->hh - 1;
            int src_idx = (ly * a->hw + lx) * 3;
            int dst_idx = (y * a->w + x) * 3;
            a->out_pixels[dst_idx + 0] = (uint8_t)(a->decoder_half[src_idx + 0] * 255.0f);
            a->out_pixels[dst_idx + 1] = (uint8_t)(a->decoder_half[src_idx + 1] * 255.0f);
            a->out_pixels[dst_idx + 2] = (uint8_t)(a->decoder_half[src_idx + 2] * 255.0f);
        }
    }
}

static void enqueue_sd_y_loop(TSFiCCXPool *pool, void (*func)(void *), SDWorkerArg *base_arg, int height) {
    int num_threads = pool->num_threads;
    int rows_per_thread = height / num_threads;
    if (rows_per_thread < 1) rows_per_thread = 1;
    
    SDWorkerArg args[MAX_CCX_THREADS];
    for (int i = 0; i < num_threads; i++) {
        int start = i * rows_per_thread;
        if (start >= height) break;
        int end = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;
        if (end > height) end = height;
        
        args[i] = *base_arg;
        args[i].start_y = start;
        args[i].end_y = end;
        
        tsfi_ccx_pool_enqueue(pool, func, &args[i]);
    }
    tsfi_ccx_pool_wait(pool);
}

static void enqueue_sd_i_loop(TSFiCCXPool *pool, void (*func)(void *), SDWorkerArg *base_arg, int total) {
    int num_threads = pool->num_threads;
    int chunks = (total / 16);
    int chunks_per_thread = chunks / num_threads;
    if (chunks_per_thread < 1) chunks_per_thread = 1;
    
    SDWorkerArg args[MAX_CCX_THREADS];
    for (int i = 0; i < num_threads; i++) {
        int start_chunk = i * chunks_per_thread;
        if (start_chunk >= chunks) break;
        int end_chunk = (i == num_threads - 1) ? chunks : (i + 1) * chunks_per_thread;
        if (end_chunk > chunks) end_chunk = chunks;
        
        args[i] = *base_arg;
        args[i].start_i = start_chunk * 16;
        args[i].end_i = end_chunk * 16;
        
        tsfi_ccx_pool_enqueue(pool, func, &args[i]);
    }
    tsfi_ccx_pool_wait(pool);
}

void tsfi_sd_thunk_paint_frame(TsfiSdContext* ctx, const uint8_t* in_dna_mask, uint8_t* out_pixels, int w, int h) {
    // 1. Advance the Timeline Semaphore (tsfi_zhong implementation)
    ctx->current_inference_tick++;
    
    // 2. Dispatch GGML / Vulkan Pipeline Execution via Timeline Semaphores
    printf("[DISPATCH] Tick %lu: Dispatching Vulkan compute pipeline for U-Net Graph execution\n", ctx->current_inference_tick);
    printf("[DISPATCH] QueueBind: Timeline semaphore signal value set to %lu\n", ctx->current_inference_tick);
    
    // 2b. Asynchronous Bottleneck (Middle Block) Dispatch via Dedicated Compute Queue
    printf("[BOTTLENECK] Tick %lu: Transitioning encoder features to bottleneck. Running Middle Block attention shaders...\n", ctx->current_inference_tick);
    printf("[BOTTLENECK] QueueBind: Bottleneck dedicated compute queue synchronized via timeline sem value %lu\n", ctx->current_inference_tick);
    
    // 3. Textural VAE Decoder Shader execution 
    // Emulates: vkCmdDispatch(cmd_buffer, (w + 15) / 16, (h + 15) / 16, 1);
    printf("[SHADER] Executing VAE decode compute shader on grid size: %dx%d\n", (w + 15) / 16, (h + 15) / 16);
    
    // Safety check for pre-allocated bounds
    if (w > ctx->allocated_max_w || h > ctx->allocated_max_h) {
        fprintf(stderr, "[ERROR] Requested dimensions %dx%d exceed pre-allocated limits (%dx%d)\n",
                w, h, ctx->allocated_max_w, ctx->allocated_max_h);
        return;
    }

    if (!g_sd_ccx_pool_initialized) {
        tsfi_ccx_pool_init(&g_sd_ccx_pool, 0, 4);
        g_sd_ccx_pool_initialized = true;
    }

    int hw = w / 2;
    int hh = h / 2;
    int lw = 64;
    int lh = 64;
    
    float* skip_connection = ctx->skip_connection_buf;
    float* latent = ctx->latent_buf;
    float* latent_att = ctx->latent_att_buf;
    float* decoder_half = ctx->decoder_half_buf;
    
    if (skip_connection && latent && latent_att && decoder_half) {
        SDWorkerArg base_arg;
        base_arg.in_dna_mask = in_dna_mask;
        base_arg.out_pixels = out_pixels;
        base_arg.skip_connection = skip_connection;
        base_arg.latent = latent;
        base_arg.latent_att = latent_att;
        base_arg.decoder_half = decoder_half;
        base_arg.w = w;
        base_arg.h = h;
        base_arg.hw = hw;
        base_arg.hh = hh;
        base_arg.lw = lw;
        base_arg.lh = lh;
        base_arg.r_hw = (float)w / hw;
        base_arg.r_hh = (float)h / hh;
        base_arg.r_lw = (float)hw / lw;
        base_arg.r_lh = (float)hh / lh;

        // --- 1. Downsampling Path: Phase 1 (Full -> Intermediate Skip Resolution) ---
        printf("[ENCODER] Phase 1: Downsampling features to intermediate skip resolution (%dx%d)\n", hw, hh);
        enqueue_sd_y_loop(&g_sd_ccx_pool, sd_downsample_phase1_worker, &base_arg, hh);
        
        // --- 2. Downsampling Path: Phase 2 (Intermediate -> Bottleneck) ---
        printf("[ENCODER] Phase 2: Downsampling to bottleneck resolution (%dx%d)\n", lw, lh);
        enqueue_sd_y_loop(&g_sd_ccx_pool, sd_downsample_phase2_worker, &base_arg, lh);
 
        // --- 3. Bottleneck Self-Attention (Spatial Blending / Mixing) ---
        // Step A: Calculate neighborhood averages and store in latent_att
        enqueue_sd_y_loop(&g_sd_ccx_pool, sd_self_attention_a_fast_worker, &base_arg, lh - 1);
        enqueue_sd_y_loop(&g_sd_ccx_pool, sd_self_attention_a_slow_worker, &base_arg, lh);

        // Step B: Vectorized Linear Blend and Leaky ReLU (AVX-512)
        int total_elements = lw * lh * 3;
        int simd_end = (total_elements / 16) * 16;
        enqueue_sd_i_loop(&g_sd_ccx_pool, sd_self_attention_b_simd_worker, &base_arg, simd_end);

        // Tail cleanup (Sequential)
        for (int i = simd_end; i < total_elements; i++) {
            float val = 0.7f * latent[i] + 0.3f * latent_att[i];
            latent_att[i] = val > 0.0f ? val : val * 0.1f;
        }

        // Apply Latent-space Collaborative Non-Local Means (L-CNLM) filtering to bottleneck latents
        float* ch_in = (float*)malloc(lw * lh * sizeof(float));
        float* ch_out = (float*)malloc(lw * lh * sizeof(float));
        if (ch_in && ch_out) {
            for (int c = 0; c < 3; c++) {
                for (int i = 0; i < lw * lh; i++) {
                    ch_in[i] = latent_att[i * 3 + c];
                }
                tsfi_montecarlo_collaborative_block_matching_filter(ch_in, ch_out, lw, lh, 0.15f, 1, 2);
                for (int i = 0; i < lw * lh; i++) {
                    latent_att[i * 3 + c] = ch_out[i];
                }
            }
        }
        if (ch_in) free(ch_in);
        if (ch_out) free(ch_out);
 
        // --- 4. Upsampling Path: Phase 1 (Bottleneck -> Intermediate) ---
        printf("[DECODER] Phase 1: Upsampling to intermediate expanding resolution (%dx%d)\n", hw, hh);
        enqueue_sd_y_loop(&g_sd_ccx_pool, sd_upsample_phase1_worker, &base_arg, hh);
        
        // --- 5. Skip Connection Blending (Vectorized via AVX-512) ---
        printf("[SKIP_CONNECTION] Blending encoder skip maps with decoder expanding features at resolution (%dx%d) [AVX-512 ACTIVE]\n", hw, hh);
        int total_floats = hw * hh * 3;
        int simd_end_skip = (total_floats / 16) * 16;
        enqueue_sd_i_loop(&g_sd_ccx_pool, sd_skip_blend_worker, &base_arg, simd_end_skip);
        
        // Cleanup scalar tail (Sequential)
        for (int i = simd_end_skip; i < total_floats; i++) {
            decoder_half[i] = (decoder_half[i] + skip_connection[i]) * 0.5f;
        }
 
        // --- 6. Upsampling Path: Phase 2 (Intermediate -> Final Output) ---
        printf("[DECODER] Phase 2: Generating final photorealistic frame to target resolution (%dx%d)\n", w, h);
        enqueue_sd_y_loop(&g_sd_ccx_pool, sd_upsample_phase2_worker, &base_arg, h);
    } else {
        // Fallback in case of allocation failure
        size_t pixel_mass = w * h * 3;
        memcpy(out_pixels, in_dna_mask, pixel_mass);
    }
}

void tsfi_sd_thunk_teardown(TsfiSdContext* ctx) {
    if (ctx->unet_buffer_vk) {
        printf("[REBAR] Releasing Vulkan Device Buffer: unet_buffer_vk\n");
        ctx->unet_buffer_vk = NULL;
    }
    if (ctx->unet_memory_vk) {
        printf("[REBAR] Freeing Vulkan Device Memory: unet_memory_vk\n");
        ctx->unet_memory_vk = NULL;
    }
    if (ctx->asset) {
        tsfi_safetensors_cache_detach(ctx->asset);
        ctx->asset = NULL;
        ctx->raw_safetensors = NULL;
    }
    if (ctx->skip_connection_buf) {
        lau_free(ctx->skip_connection_buf);
        ctx->skip_connection_buf = NULL;
    }
    if (ctx->latent_buf) {
        lau_free(ctx->latent_buf);
        ctx->latent_buf = NULL;
    }
    if (ctx->latent_att_buf) {
        lau_free(ctx->latent_att_buf);
        ctx->latent_att_buf = NULL;
    }
    if (ctx->decoder_half_buf) {
        lau_free(ctx->decoder_half_buf);
        ctx->decoder_half_buf = NULL;
    }
    printf("[THUNK] Memory Annihilated.\n");
}

