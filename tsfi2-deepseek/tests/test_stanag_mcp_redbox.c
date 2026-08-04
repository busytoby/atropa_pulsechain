#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include "../inc/tsfi_ccx_pool.h"

#define MAX_PARTICIPANTS 64
#define SAMPLING_RATE 96000.0
#define TOTAL_SAMPLES 10000
#define SPEED_OF_LIGHT 3e8

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    int id;
    double x;
    double y;
    bool is_compromised; // Red box adversary flag
    double clock_skew;   // Injected clock delay error (seconds)
} STANAGNode;

static STANAGNode g_nodes[MAX_PARTICIPANTS];
static int g_node_count = 0;
static pthread_mutex_t g_network_lock = PTHREAD_MUTEX_INITIALIZER;
static TSFiCCXPool g_redbox_ccx_pool;
static bool g_redbox_ccx_pool_initialized = false;
static float *g_tx_signal = NULL;

static double get_random_node() {
    static uint64_t seed = 123456789ULL;
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (double)(seed & 0xFFFFFFFFFFFFULL) / 281474976710656.0;
}

static float generate_gaussian_noise() {
    double u1 = get_random_node();
    double u2 = get_random_node();
    if (u1 < 1e-15) u1 = 1e-15;
    return (float)(sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2));
}

static void handle_mcp_negotiation(const char *json_req, char *json_resp, size_t max_len) {
    pthread_mutex_lock(&g_network_lock);
    if (strstr(json_req, "stanag.register_node") != NULL) {
        int id = 0;
        double x = 0.0, y = 0.0;
        bool is_compromised = false;
        double clock_skew = 0.0;

        char *p_id = strstr(json_req, "\"id\"");
        if (p_id) sscanf(p_id, "\"id\": %d", &id);
        char *p_x = strstr(json_req, "\"x\"");
        if (p_x) sscanf(p_x, "\"x\": %lf", &x);
        char *p_y = strstr(json_req, "\"y\"");
        if (p_y) sscanf(p_y, "\"y\": %lf", &y);
        if (strstr(json_req, "\"compromised\": true") != NULL || strstr(json_req, "\"compromised\":true") != NULL) {
            is_compromised = true;
        }
        char *p_skew = strstr(json_req, "\"clock_skew\"");
        if (p_skew) sscanf(p_skew, "\"clock_skew\": %lf", &clock_skew);

        if (g_node_count < MAX_PARTICIPANTS) {
            g_nodes[g_node_count].id = id;
            g_nodes[g_node_count].x = x;
            g_nodes[g_node_count].y = y;
            g_nodes[g_node_count].is_compromised = is_compromised;
            g_nodes[g_node_count].clock_skew = clock_skew;
            g_node_count++;
            snprintf(json_resp, max_len, "{\"jsonrpc\":\"2.0\",\"result\":{\"status\":\"registered\",\"count\":%d},\"id\":1}", g_node_count);
        } else {
            snprintf(json_resp, max_len, "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32000,\"message\":\"Network capacity exceeded\"},\"id\":1}");
        }
    } else {
        snprintf(json_resp, max_len, "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32601,\"message\":\"Method not found\"},\"id\":1}");
    }
    pthread_mutex_unlock(&g_network_lock);
}

typedef struct {
    int station_idx;
    double tx_x;
    double tx_y;
    float *signal_buffer;
} NodeSimWorkerArg;

static void node_simulation_worker(void *arg) {
    NodeSimWorkerArg *a = (NodeSimWorkerArg *)arg;
    int idx = a->station_idx;
    double dx = g_nodes[idx].x - a->tx_x;
    double dy = g_nodes[idx].y - a->tx_y;
    double dist = sqrt(dx*dx + dy*dy) * 1000.0; // meters
    double delay_sec = dist / SPEED_OF_LIGHT + g_nodes[idx].clock_skew;
    int delay_samples = (int)(delay_sec * SAMPLING_RATE);

    for (int step = 0; step < TOTAL_SAMPLES; step++) {
        int read_idx = step - delay_samples;
        a->signal_buffer[step] = (read_idx >= 0 && read_idx < TOTAL_SAMPLES) ? g_tx_signal[read_idx] : 0.0f;
    }
}

int main() {
    printf("=== STANAG Spontaneous MCP Network Red Box Isolation Test ===\n");

    // 1. Pre-generate Gaussian noise transmit signal (unambiguous PRN sequence)
    g_tx_signal = calloc(TOTAL_SAMPLES, sizeof(float));
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        g_tx_signal[i] = generate_gaussian_noise();
    }

    // 2. Spontaneously negotiate 5 network participants via MCP RPC messages
    char resp[1024];
    handle_mcp_negotiation("{\"method\":\"stanag.register_node\",\"params\":{\"id\":0,\"x\":0.0,\"y\":0.0,\"compromised\":false,\"clock_skew\":0.0}}", resp, sizeof(resp));
    handle_mcp_negotiation("{\"method\":\"stanag.register_node\",\"params\":{\"id\":1,\"x\":10.0,\"y\":0.0,\"compromised\":false,\"clock_skew\":0.0}}", resp, sizeof(resp));
    handle_mcp_negotiation("{\"method\":\"stanag.register_node\",\"params\":{\"id\":2,\"x\":0.0,\"y\":10.0,\"compromised\":false,\"clock_skew\":0.0}}", resp, sizeof(resp));
    
    // Compromised Adversary node injecting 2.0 milliseconds clock spoofing (approx 600 km equivalent range error)
    handle_mcp_negotiation("{\"method\":\"stanag.register_node\",\"params\":{\"id\":3,\"x\":10.0,\"y\":10.0,\"compromised\":true,\"clock_skew\":0.002}}", resp, sizeof(resp));
    handle_mcp_negotiation("{\"method\":\"stanag.register_node\",\"params\":{\"id\":4,\"x\":5.0,\"y\":5.0,\"compromised\":false,\"clock_skew\":0.0}}", resp, sizeof(resp));

    assert(g_node_count == 5);
    printf("[MCP_NEGOTIATION] Successfully negotiated %d participants via spontaneous RPC.\n", g_node_count);

    // 3. Target spy transmitter located at (3.0, 4.0) km
    double tx_x = 3.0;
    double tx_y = 4.0;

    // Allocate buffers
    float *buffers[MAX_PARTICIPANTS];
    for (int i = 0; i < g_node_count; i++) {
        buffers[i] = calloc(TOTAL_SAMPLES, sizeof(float));
    }

    if (!g_redbox_ccx_pool_initialized) {
        tsfi_ccx_pool_init(&g_redbox_ccx_pool, 0, 4);
        g_redbox_ccx_pool_initialized = true;
    }

    // 4. Run parallel node signal propagation simulation via CCX thread pool
    NodeSimWorkerArg sim_args[MAX_PARTICIPANTS];
    for (int i = 0; i < g_node_count; i++) {
        sim_args[i].station_idx = i;
        sim_args[i].tx_x = tx_x;
        sim_args[i].tx_y = tx_y;
        sim_args[i].signal_buffer = buffers[i];
        tsfi_ccx_pool_enqueue(&g_redbox_ccx_pool, node_simulation_worker, &sim_args[i]);
    }
    tsfi_ccx_pool_wait(&g_redbox_ccx_pool);

    // 5. Red Box Isolation Verification: Check TDOA consistency and isolate outliers
    // Station 0 is reference node.
    printf("\n[RED_BOX_TEST] Performing timing integrity validation...\n");
    int isolated_count = 0;
    for (int i = 1; i < g_node_count; i++) {
        // Compute cross-correlation peak lag
        double max_corr = -1.0;
        int best_lag = 0;
        int max_lag = 300;

        for (int lag = -max_lag; lag <= max_lag; lag++) {
            double sum_prod = 0.0;
            for (int n = max_lag; n < TOTAL_SAMPLES - max_lag; n++) {
                sum_prod += buffers[i][n] * buffers[0][n - lag];
            }
            if (sum_prod > max_corr) {
                max_corr = sum_prod;
                best_lag = lag;
            }
        }

        double measured_tdoa = (double)best_lag / SAMPLING_RATE;
        double dist0 = sqrt((g_nodes[0].x - tx_x)*(g_nodes[0].x - tx_x) + (g_nodes[0].y - tx_y)*(g_nodes[0].y - tx_y)) * 1000.0;
        double disti = sqrt((g_nodes[i].x - tx_x)*(g_nodes[i].x - tx_x) + (g_nodes[i].y - tx_y)*(g_nodes[i].y - tx_y)) * 1000.0;
        double expected_tdoa = (disti - dist0) / SPEED_OF_LIGHT;
        double tdoa_error = fabs(measured_tdoa - expected_tdoa);

        printf("  -> Node %d measured TDOA: %f ms, expected: %f ms, error: %f ms\n", 
               g_nodes[i].id, measured_tdoa * 1000.0, expected_tdoa * 1000.0, tdoa_error * 1000.0);

        // If error exceeds 0.1 milliseconds (~30 km range mismatch), isolate from network
        if (tdoa_error > 0.0001) {
            printf("  [WARN] Timing anomaly detected on Node %d! ISOLATING COMPROMISED NODE.\n", g_nodes[i].id);
            assert(g_nodes[i].is_compromised == true); // Confirmed red box caught the adversary
            isolated_count++;
        }
    }

    assert(isolated_count == 1);
    printf("[SUCCESS] Red Box analysis successfully isolated the compromised node.\n");

    // Clean up
    for (int i = 0; i < g_node_count; i++) {
        free(buffers[i]);
    }
    free(g_tx_signal);
    return 0;
}
