#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_deepseek_cordis_composability.h"
#include "tsfi_deepseek_stanag_kermit.h"

// ----------------------------------------------------------------------------
// 1. Prefix Fingerprinting Helper (FNV-1a 64-bit)
// ----------------------------------------------------------------------------
static uint64_t fnv1a_hash64(const char *str) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    if (!str) return hash;
    while (*str) {
        hash ^= (uint64_t)(unsigned char)(*str++);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ----------------------------------------------------------------------------
// 2. Static Prefix Cache Pinning Implementation
// ----------------------------------------------------------------------------
bool tsfi_cordis_prefix_cache_init(tsfi_cordis_prefix_cache_t *cache) {
    if (!cache) return false;
    memset(cache, 0, sizeof(*cache));
    return true;
}

bool tsfi_cordis_prefix_cache_pin(
    tsfi_cordis_prefix_cache_t *cache,
    const char *static_system_prompt,
    uint32_t token_count
) {
    if (!cache || !static_system_prompt) return false;
    cache->prefix_fingerprint = fnv1a_hash64(static_system_prompt);
    cache->pinned_token_count = token_count;
    
    size_t len = strlen(static_system_prompt);
    uint32_t pages_needed = (uint32_t)((len + TSFI_PREFIX_PAGE_SIZE - 1) / TSFI_PREFIX_PAGE_SIZE);
    if (pages_needed > TSFI_MAX_PREFIX_PAGES) pages_needed = TSFI_MAX_PREFIX_PAGES;

    cache->total_pinned_pages = pages_needed;
    for (uint32_t p = 0; p < pages_needed; p++) {
        size_t offset = p * TSFI_PREFIX_PAGE_SIZE;
        size_t chunk_len = len - offset;
        if (chunk_len > TSFI_PREFIX_PAGE_SIZE) chunk_len = TSFI_PREFIX_PAGE_SIZE;
        memcpy(cache->page_buffer[p], static_system_prompt + offset, chunk_len);
    }
    return true;
}

bool tsfi_cordis_prefix_cache_lookup(
    tsfi_cordis_prefix_cache_t *cache,
    const char *incoming_prompt,
    bool *cache_hit_out
) {
    if (!cache || !incoming_prompt || !cache_hit_out) return false;
    *cache_hit_out = false;

    if (cache->total_pinned_pages == 0 || cache->prefix_fingerprint == 0) {
        cache->cache_miss_count++;
        return true;
    }

    // Direct check if incoming prompt matches the static prefix or starts with it
    size_t in_len = strlen(incoming_prompt);
    // Flatten stored prefix into temp buffer for comparison
    char temp_prefix[TSFI_MAX_PREFIX_PAGES * TSFI_PREFIX_PAGE_SIZE + 1];
    size_t cur = 0;
    for (uint32_t p = 0; p < cache->total_pinned_pages; p++) {
        size_t to_copy = TSFI_PREFIX_PAGE_SIZE;
        memcpy(temp_prefix + cur, cache->page_buffer[p], to_copy);
        cur += to_copy;
    }
    temp_prefix[cur] = '\0';

    if (fnv1a_hash64(incoming_prompt) == cache->prefix_fingerprint) {
        *cache_hit_out = true;
        cache->cache_hit_count++;
        return true;
    }

    // Prefix prefix-start check
    if (in_len > 0 && strncmp(incoming_prompt, (const char*)cache->page_buffer[0], strlen((const char*)cache->page_buffer[0])) == 0) {
        *cache_hit_out = true;
        cache->cache_hit_count++;
        return true;
    }

    cache->cache_miss_count++;
    return true;
}

// ----------------------------------------------------------------------------
// 3. Multi-Profile Layering & Composability Execution
// ----------------------------------------------------------------------------
bool tsfi_cordis_composability_init(
    tsfi_cordis_composability_engine_t *engine,
    tsfi_cordis_profile_type_t profile_type
) {
    if (!engine) return false;
    memset(engine, 0, sizeof(*engine));
    engine->current_stage = CORDIS_STAGE_IDLE;
    tsfi_cordis_prefix_cache_init(&engine->prefix_cache);

    switch (profile_type) {
        case CORDIS_PROFILE_MINIMAL:
            engine->active_profile.type = CORDIS_PROFILE_MINIMAL;
            strncpy(engine->active_profile.profile_name, "Minimal", 31);
            engine->active_profile.max_allowed_tokens = 2048;
            engine->active_profile.enable_landlock_sandbox = true;
            engine->active_profile.enable_stanag_coaxial = false;
            engine->active_profile.max_concurrency = 1;
            break;
        case CORDIS_PROFILE_CODE:
            engine->active_profile.type = CORDIS_PROFILE_CODE;
            strncpy(engine->active_profile.profile_name, "Code-Mode", 31);
            engine->active_profile.max_allowed_tokens = 8192;
            engine->active_profile.enable_landlock_sandbox = true;
            engine->active_profile.enable_stanag_coaxial = true;
            engine->active_profile.max_concurrency = 4;
            break;
        case CORDIS_PROFILE_HARDWARE_TRACE:
            engine->active_profile.type = CORDIS_PROFILE_HARDWARE_TRACE;
            strncpy(engine->active_profile.profile_name, "Hardware-Trace", 31);
            engine->active_profile.max_allowed_tokens = 16384;
            engine->active_profile.enable_landlock_sandbox = false;
            engine->active_profile.enable_stanag_coaxial = true;
            engine->active_profile.max_concurrency = 8;
            break;
        default:
            engine->active_profile.type = CORDIS_PROFILE_CUSTOM;
            strncpy(engine->active_profile.profile_name, "Custom", 31);
            engine->active_profile.max_allowed_tokens = 4096;
            engine->active_profile.enable_landlock_sandbox = true;
            engine->active_profile.enable_stanag_coaxial = true;
            engine->active_profile.max_concurrency = 2;
            break;
    }
    return true;
}

bool tsfi_cordis_composability_execute_cycle(
    tsfi_cordis_composability_engine_t *engine,
    tsfi_edsac_acp_firewall_state_t *firewall,
    const char *plan_intent,
    const char *tool_candidate,
    const char *tool_args,
    char *result_out,
    size_t max_result_len
) {
    if (!engine || !tool_candidate || !result_out || max_result_len == 0) return false;

    // Stage 1: Plan Validation
    engine->current_stage = CORDIS_STAGE_PLAN;
    uint64_t t0 = get_time_ns();
    if (!plan_intent || strlen(plan_intent) == 0) {
        snprintf(result_out, max_result_len, "[CORDIS STAGE REJECT] Empty plan intent.");
        return false;
    }
    uint64_t t1 = get_time_ns();

    // Record stage history
    tsfi_cordis_stage_metrics_t *m_plan = &engine->stage_history[0];
    strncpy(m_plan->stage_name, "Plan", sizeof(m_plan->stage_name) - 1);
    m_plan->stage_start_ns = t0;
    m_plan->stage_duration_ns = t1 - t0;
    m_plan->stage_passed = true;

    // Stage 2: Speculative Pre-Execution Sandbox Check
    engine->current_stage = CORDIS_STAGE_SPECULATIVE_PRE_EXEC;
    uint64_t t2 = get_time_ns();
    if (engine->active_profile.enable_landlock_sandbox && tool_args) {
        if (strstr(tool_args, "/etc/shadow") || strstr(tool_args, "/etc/passwd")) {
            engine->current_stage = CORDIS_STAGE_ROLLBACK;
            snprintf(result_out, max_result_len, "[CORDIS ROLLBACK] Sandbox violation prevented in speculative pre-exec.");
            return false;
        }
    }
    uint64_t t3 = get_time_ns();
    tsfi_cordis_stage_metrics_t *m_spec = &engine->stage_history[1];
    strncpy(m_spec->stage_name, "SpeculativePreExec", sizeof(m_spec->stage_name) - 1);
    m_spec->stage_start_ns = t2;
    m_spec->stage_duration_ns = t3 - t2;
    m_spec->stage_passed = true;

    // Stage 3: EDSAC ACP Firewall Validation
    engine->current_stage = CORDIS_STAGE_FIREWALL_VALIDATION;
    uint64_t t4 = get_time_ns();
    if (firewall) {
        tsfi_edsac_qualification_phase_t phase;
        if (!tsfi_edsac_initial_orders_1_qualify(firewall, engine->active_profile.profile_name, tool_candidate, &phase) ||
            phase != EDSAC_PHASE_QUALIFIED) {
            engine->current_stage = CORDIS_STAGE_ROLLBACK;
            snprintf(result_out, max_result_len, "[CORDIS ROLLBACK] Firewall rejection under EDSAC Initial Orders 1.");
            return false;
        }
    }
    uint64_t t5 = get_time_ns();
    tsfi_cordis_stage_metrics_t *m_fw = &engine->stage_history[2];
    strncpy(m_fw->stage_name, "FirewallValidation", sizeof(m_fw->stage_name) - 1);
    m_fw->stage_start_ns = t4;
    m_fw->stage_duration_ns = t5 - t4;
    m_fw->stage_passed = true;

    // Stage 4: Commit & Dispatch
    engine->current_stage = CORDIS_STAGE_COMMIT;
    uint64_t t6 = get_time_ns();
    snprintf(result_out, max_result_len, "[CORDIS COMMIT] Profile='%s' executed tool='%s' args='%s' (Latency: %.2f us)",
             engine->active_profile.profile_name, tool_candidate, tool_args ? tool_args : "", (t5 - t0) / 1000.0);
    
    // Transmit telemetry over STANAG coaxial if enabled
    if (engine->active_profile.enable_stanag_coaxial) {
        tsfi_kermit_packet_t kp;
        tsfi_kermit_packet_init(&kp, 1, KERMIT_TYPE_DATA);
        size_t rlen = strlen(result_out);
        if (rlen > sizeof(kp.data)) rlen = sizeof(kp.data);
        memcpy(kp.data, result_out, rlen);
        kp.data_len = rlen;

        tsfi_stanag_coaxial_frame_t frame;
        tsfi_stanag_coaxial_encapsulate(0x2A, 1, 0x57A1, &kp, &frame);

        tsfi_stanag_coaxial_bus_state_t bus;
        tsfi_stanag_coaxial_bus_init(&bus);
        uint8_t wire[512];
        size_t wire_len = 0;
        tsfi_stanag_coaxial_transmit(&bus, &frame, wire, &wire_len);
    }

    uint64_t t7 = get_time_ns();
    tsfi_cordis_stage_metrics_t *m_commit = &engine->stage_history[3];
    strncpy(m_commit->stage_name, "Commit", sizeof(m_commit->stage_name) - 1);
    m_commit->stage_start_ns = t6;
    m_commit->stage_duration_ns = t7 - t6;
    m_commit->stage_passed = true;

    engine->stage_history_count = 4;
    engine->total_composition_cycles++;
    engine->current_stage = CORDIS_STAGE_IDLE;
    return true;
}
