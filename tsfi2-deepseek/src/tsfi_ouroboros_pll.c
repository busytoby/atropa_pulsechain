#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MAX_HEAP_SIZE 64
#define MOTZKIN_PRIME 953467954114363ULL

// Dynamic PI loop filter state variables for second-order Ouroboros PLL
static double s_integral_error = 0.0;

// System Event Types
typedef enum {
    EVENT_PLL_DRIFT = 1,
    EVENT_PMG_COLLISION = 2,
    EVENT_STACK_STORAGE_SYNC = 3,
    EVENT_GUEST_CONTRACT_CALL = 4
} EventType;

// Event Payload structure coordinating PLL, PMG, and Stack Storage
typedef struct {
    uint32_t priority;      // Lower priority value = higher precedence (Min-Heap)
    EventType type;         // PLL, PMG, or Storage
    uint64_t timestamp;     // PLL calibrated clock tick
    uint8_t data[32];       // Associated data (e.g. collision mask or keycode)
} CoordinatedEvent;

// Min-Heap (Priority Queue) structure aligned to 64-byte cache boundaries
typedef struct __attribute__((aligned(64))) {
    CoordinatedEvent data[MAX_HEAP_SIZE] __attribute__((aligned(64)));
    uint32_t size;
} PriorityQueue;

extern uint64_t lau_yul_thunk_sload(uint64_t key);
extern void lau_yul_thunk_sstore(uint64_t key, uint64_t val);
extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
extern void blue_box_accumulate_state(uint64_t val);

// Modulo exponentiation helper
static uint64_t pmg_mod_pow(uint64_t base, uint64_t exp, uint64_t modulus) {
    uint64_t result = 1;
    base = base % modulus;
    while (exp > 0) {
        if (exp % 2 == 1) {
            __uint128_t temp = (__uint128_t)result * base;
            result = (uint64_t)(temp % modulus);
        }
        __uint128_t temp_base = (__uint128_t)base * base;
        base = (uint64_t)(temp_base % modulus);
        exp /= 2;
    }
    return result;
}

// Push event into Min-Heap (optimized with vectorization hints and aggressive inlining)
__attribute__((hot, always_inline))
static inline bool pq_push(PriorityQueue *restrict pq, CoordinatedEvent event) {
    if (pq->size >= MAX_HEAP_SIZE) return false;
    uint32_t i = pq->size;
    pq->data[i] = event;
    pq->size++;
    while (i > 0) {
        uint32_t parent = (i - 1) / 2;
        if (pq->data[i].priority < pq->data[parent].priority) {
            CoordinatedEvent temp = pq->data[i];
            pq->data[i] = pq->data[parent];
            pq->data[parent] = temp;
            i = parent;
        } else {
            break;
        }
    }
    return true;
}

// Pop event from Min-Heap (optimized with vectorization hints and aggressive inlining)
__attribute__((hot, always_inline))
static inline bool pq_pop(PriorityQueue *restrict pq, CoordinatedEvent *restrict out_event) {
    if (pq->size == 0) return false;
    *out_event = pq->data[0];
    pq->size--;
    pq->data[0] = pq->data[pq->size];
    uint32_t i = 0;
    while (2 * i + 1 < pq->size) {
        uint32_t left = 2 * i + 1;
        uint32_t right = 2 * i + 2;
        uint32_t smallest = left;
        if (right < pq->size && pq->data[right].priority < pq->data[left].priority) {
            smallest = right;
        }
        if (pq->data[smallest].priority < pq->data[i].priority) {
            CoordinatedEvent temp = pq->data[i];
            pq->data[i] = pq->data[smallest];
            pq->data[smallest] = temp;
            i = smallest;
        } else {
            break;
        }
    }
    return true;
}

// PI-controller based Ouroboros loop filter tick
void tsfi_ouroboros_pll_tick(uint64_t base) {
    uint64_t pos = lau_yul_thunk_sload(0xF100);
    uint64_t neg = lau_yul_thunk_sload(0xF101);
    
    double raw_diff = (double)pos - (double)neg;
    double diff_abs = raw_diff >= 0.0 ? raw_diff : -raw_diff;
    double dither_scale = (diff_abs < 10.0) ? 0.1 : 1.0; // Scale dither down when locked
    
    // Simulate small thermal clock jitter dither using a fast deterministic LCG
    static uint32_t jitter_seed = 0x5EEDULL;
    jitter_seed = jitter_seed * 1103515245ULL + 12345ULL;
    double jitter = (((double)(jitter_seed % 200) - 100.0) / 100.0) * dither_scale;
    
    double phase_error = raw_diff + jitter;
    s_integral_error += phase_error;
    
    // PI integrator anti-windup guard clamp
    if (s_integral_error > 1000.0) s_integral_error = 1000.0;
    if (s_integral_error < -1000.0) s_integral_error = -1000.0;
    
    double kp = 0.15;
    double ki = 0.05;
    double freq_correction = (kp * phase_error) + (ki * s_integral_error);
    (void)freq_correction;
    
    uint64_t deviation = (uint64_t)(phase_error >= 0.0 ? phase_error : -phase_error);
    lau_yul_thunk_sstore(0xF125, deviation);
    
    uint64_t signal = deviation;
    if (signal == 0) signal = 1;
    uint64_t next_signal = (signal * base) % MOTZKIN_PRIME;
    
    uint8_t yul_cd[36] = {0xe3, 0x99, 0xf0, 0xe0};
    yul_cd[35] = (uint8_t)(next_signal & 0xFF);
    yul_cd[34] = (uint8_t)((next_signal >> 8) & 0xFF);
    yul_cd[33] = (uint8_t)((next_signal >> 16) & 0xFF);
    yul_cd[32] = (uint8_t)((next_signal >> 24) & 0xFF);
    
    uint8_t yul_ret[32];
    size_t yul_ret_len = 32;
    lau_yul_thunk_execute("WinchesterMQ", yul_cd, 36, yul_ret, &yul_ret_len);
    
    blue_box_accumulate_state(next_signal);
}

// Static persistent scheduler queue
static PriorityQueue s_scheduler_pq = { .size = 0 };

// Perform aging on all events remaining in the queue to prevent starvation
static void pq_age(PriorityQueue *pq) {
    for (uint32_t i = 0; i < pq->size; i++) {
        if (pq->data[i].priority > 1) {
            pq->data[i].priority--; // Boost precedence
        }
    }
    // Re-heapify to maintain min-heap invariant after modifying priorities
    if (pq->size > 1) {
        for (int i = (int)(pq->size / 2) - 1; i >= 0; i--) {
            uint32_t parent = (uint32_t)i;
            while (2 * parent + 1 < pq->size) {
                uint32_t left = 2 * parent + 1;
                uint32_t right = 2 * parent + 2;
                uint32_t smallest = left;
                if (right < pq->size && pq->data[right].priority < pq->data[left].priority) {
                    smallest = right;
                }
                if (pq->data[smallest].priority < pq->data[parent].priority) {
                    CoordinatedEvent temp = pq->data[parent];
                    pq->data[parent] = pq->data[smallest];
                    pq->data[smallest] = temp;
                    parent = smallest;
                } else {
                    break;
                }
            }
        }
    }
}

// Host-side wrapper that handles queue overflow spillover
bool tsfi_ouroboros_push_event(uint32_t priority, uint32_t type, uint64_t timestamp, const uint8_t *data) {
    uint64_t size = lau_yul_thunk_sload(0xF302);
    if (size >= 16) {
        // Spill over directly to persistent host queue
        CoordinatedEvent ev = { .priority = priority, .type = (EventType)type, .timestamp = timestamp };
        memcpy(ev.data, data, 32);
        return pq_push(&s_scheduler_pq, ev);
    }
    
    // Otherwise, push to EVM ring-buffer
    uint8_t push_cd[132] = {0};
    push_cd[0] = 0x0f; push_cd[1] = 0xf2; push_cd[2] = 0x20; push_cd[3] = 0x00;
    push_cd[35] = priority;
    push_cd[67] = type;
    push_cd[99] = timestamp & 0xFF; push_cd[98] = (timestamp >> 8) & 0xFF;
    memcpy(&push_cd[100], data, 32);
    
    uint8_t push_ret[32];
    size_t push_ret_len = sizeof(push_ret);
    return lau_yul_thunk_execute("WinchesterMQ", push_cd, sizeof(push_cd), push_ret, &push_ret_len);
}

// Integrated Scheduler execution driving PLL loop filter ticks and TDMA proofs
void tsfi_ouroboros_run_integrated_tick(uint32_t delta_time_ms, uint64_t base) {
    // 1. Read dynamic telemetry to construct priorities
    uint64_t collision_mask = lau_yul_thunk_sload(0xF210); // PMG collision mask
    uint64_t drift_metric = lau_yul_thunk_sload(0xF125);
    
    // Pop dynamic events from WinchesterMQ Yul ring-buffer queue and push to scheduler heap
    while (1) {
        uint8_t pop_cd[4] = {0x0f, 0xf2, 0x30, 0x00};
        uint8_t pop_ret[128];
        size_t pop_ret_len = sizeof(pop_ret);
        bool pop_ok = lau_yul_thunk_execute("WinchesterMQ", pop_cd, sizeof(pop_cd), pop_ret, &pop_ret_len);
        if (!pop_ok || pop_ret_len < 128) {
            break;
        }
        
        // Decoded popped values (Yul returns as 32-byte big-endian words)
        uint64_t p_priority = 0;
        for (int i = 0; i < 32; i++) p_priority = (p_priority << 8) | pop_ret[i];
        uint64_t p_type = 0;
        for (int i = 32; i < 64; i++) p_type = (p_type << 8) | pop_ret[i];
        uint64_t p_timestamp = 0;
        for (int i = 64; i < 96; i++) p_timestamp = (p_timestamp << 8) | pop_ret[i];
        
        // Check if a valid event was popped (priority > 0)
        if (p_priority == 0) {
            break;
        }
        
        CoordinatedEvent ev_yul;
        ev_yul.priority = p_priority;
        ev_yul.type = (uint32_t)p_type;
        ev_yul.timestamp = p_timestamp;
        memcpy(ev_yul.data, &pop_ret[96], 32);
        
        pq_push(&s_scheduler_pq, ev_yul);
    }
    
    // Enqueue PMG event if active collision is detected
    if (collision_mask > 0) {
        CoordinatedEvent ev = { .priority = 1, .type = EVENT_PMG_COLLISION, .timestamp = delta_time_ms };
        ev.data[0] = (uint8_t)(collision_mask & 0xFF);
        pq_push(&s_scheduler_pq, ev);
    }
    
    // Enqueue PLL drift updates
    CoordinatedEvent ev_pll = { .priority = 5, .type = EVENT_PLL_DRIFT, .timestamp = delta_time_ms };
    ev_pll.data[0] = (uint8_t)(drift_metric & 0xFF);
    pq_push(&s_scheduler_pq, ev_pll);
    
    // Enqueue Stack storage sync
    CoordinatedEvent ev_sync = { .priority = 10, .type = EVENT_STACK_STORAGE_SYNC, .timestamp = delta_time_ms };
    pq_push(&s_scheduler_pq, ev_sync);
    
    // 2. Dispatch events (up to a budget of 3 events per tick to trigger dynamic priority aging)
    CoordinatedEvent popped;
    uint32_t budget = 3;
    while (budget > 0 && pq_pop(&s_scheduler_pq, &popped)) {
        budget--;
        if (popped.type == EVENT_PMG_COLLISION) {
            // TDMA Proof verification: Compute coordinate distance and assert lock keys for channels 0-3
            for (int ch = 0; ch < 4; ch++) {
                uint64_t offset = ch * 0x20;
                uint64_t px = lau_yul_thunk_sload(0xF200 + offset);
                uint64_t py = lau_yul_thunk_sload(0xF201 + offset);
                uint64_t mx = lau_yul_thunk_sload(0xF202 + offset);
                uint64_t my = lau_yul_thunk_sload(0xF203 + offset);
                
                uint64_t dx = px >= mx ? px - mx : mx - px;
                uint64_t dy = py >= my ? py - my : my - py;
                uint64_t distance = dx + dy;
                
                uint64_t expected_key = lau_yul_thunk_sload(0xF205 + offset);
                uint64_t calculated = pmg_mod_pow(base, distance, MOTZKIN_PRIME);
                
                if (calculated == expected_key && expected_key > 0) {
                    lau_yul_thunk_sstore(0xF208 + offset, 1); // Enable TDMA slot lock for player channel
                } else {
                    lau_yul_thunk_sstore(0xF208 + offset, 0); // Reject/Revoke TDMA slot lock for player channel
                }
            }
        } else if (popped.type == EVENT_PLL_DRIFT) {
            // Operate the PLL filter clock adjustment
            tsfi_ouroboros_pll_tick(base);
        } else if (popped.type == EVENT_STACK_STORAGE_SYNC) {
            // Synchronize stacked registers (using F185 instead of F180 to avoid corrupting system ticks)
            uint64_t live_pll = lau_yul_thunk_sload(0xF125);
            lau_yul_thunk_sstore(0xF185, live_pll);
        } else if (popped.type == EVENT_GUEST_CONTRACT_CALL) {
            // Extract guest contract address from the first 8 bytes of data
            uint64_t target_addr = 0;
            for (int i = 0; i < 8; i++) {
                target_addr = (target_addr << 8) | popped.data[i];
            }
            
            // Build calldata: generic execution selector (0xe399f0e0) + dynamic 24-byte payload
            uint8_t guest_cd[32] = {0xe3, 0x99, 0xf0, 0xe0};
            memcpy(&guest_cd[4], &popped.data[8], 24);
            
            uint8_t guest_ret[32];
            size_t guest_ret_len = sizeof(guest_ret);
            
            // Resolve and query dynamic contract address directly using 0xX format representation (Rule 9 compliant)
            char addr_str[32];
            snprintf(addr_str, sizeof(addr_str), "0x%lx", target_addr);
            lau_yul_thunk_execute(addr_str, guest_cd, sizeof(guest_cd), guest_ret, &guest_ret_len);
        }
    }
    
    // Perform dynamic priority aging on remaining events to prevent starvation
    if (s_scheduler_pq.size > 0) {
        pq_age(&s_scheduler_pq);
    }
}

uint32_t tsfi_ouroboros_get_pq_size(void) {
    return s_scheduler_pq.size;
}

uint32_t tsfi_ouroboros_get_pq_priority(uint32_t index) {
    if (index >= s_scheduler_pq.size) return 0;
    return s_scheduler_pq.data[index].priority;
}
