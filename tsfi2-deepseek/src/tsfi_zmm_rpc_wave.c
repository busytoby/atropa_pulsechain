#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <unistd.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <math.h>
#include "tsfi_zmm_rpc.h"
#include "tsfi_trie_dispatcher.h"
#include "tsfi_tst.h"
#include "tsfi_opt_zmm.h"
#include "tsfi_genetic.h"
#include "tsfi_svdag.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include "tsfi_block_monitor.h"
#include "tsfi_mainframe_computerworld.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#define MAX_DILEMMA_LOGS 128
#define MAX_STORE_KEYS 1024
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define SAMPLE_RATE 8000
#define MAX_ARRANGEMENT 64
#define MAX_STEPS 32
#define MAX_REGISTERED_OPERATORS 8


int tsfi_zmm_rpc_dispatch_wave(TsfiZmmVmState *state, int method_type, const char *min_ptr, char *output_buf, size_t out_max, int id) {
    if (method_type == 28) { // wave512.dilemma_log
        char event_str[64];
        if (extract_json_string(min_ptr, "\"event\"", event_str, sizeof(event_str))) {
            char src_str[64] = "External";
            char det_str[256] = "";
            extract_json_string(min_ptr, "\"source\"", src_str, sizeof(src_str));
            extract_json_string(min_ptr, "\"details\"", det_str, sizeof(det_str));
            
            if (g_dilemma_log_count < MAX_DILEMMA_LOGS) {
                snprintf(g_dilemma_logs[g_dilemma_log_count].event, 64, "%s", event_str);
                snprintf(g_dilemma_logs[g_dilemma_log_count].source, 64, "%s", src_str);
                snprintf(g_dilemma_logs[g_dilemma_log_count].details, 256, "%s", det_str);
                g_dilemma_logs[g_dilemma_log_count].timestamp = (uint64_t)time(NULL);
                g_dilemma_log_count++;
            } else {
                for (int i = 1; i < MAX_DILEMMA_LOGS; i++) {
                    g_dilemma_logs[i - 1] = g_dilemma_logs[i];
                }
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].event, 64, "%s", event_str);
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].source, 64, "%s", src_str);
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].details, 256, "%s", det_str);
                g_dilemma_logs[MAX_DILEMMA_LOGS - 1].timestamp = (uint64_t)time(NULL);
            }
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Log registered\", \"id\": %d}\n", id);
            return 1;
        } else {
            int head = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": [");
            char *optr = output_buf + head;
            size_t rem = out_max - head - 64;
            for (int i = 0; i < g_dilemma_log_count; i++) {
                int n = snprintf(optr, rem, "{\"event\": \"%s\", \"source\": \"%s\", \"details\": \"%s\", \"timestamp\": %lu}%s", 
                                 g_dilemma_logs[i].event, g_dilemma_logs[i].source, g_dilemma_logs[i].details,
                                 (unsigned long)g_dilemma_logs[i].timestamp, (i < g_dilemma_log_count - 1 ? ", " : ""));
                optr += n; rem -= n;
                if (rem < 128) break;
            }
            snprintf(optr, rem, "], \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 29) { // wave512.telemetry
        extern uint64_t g_thunk_cache_hits;
        extern uint64_t g_thunk_cache_lookups;
        extern uint64_t lau_yul_thunk_sload(uint64_t key);
        extern int tsfi_ouroboros_serialize_pq(char *buf, size_t max_len);
        extern uint64_t tsfi_ouroboros_get_adaptive_tick_rate(void);
        
        typedef struct {
            uint64_t pc;
            uint8_t op;
        } YulTraceStepLocal;
        
        extern YulTraceStepLocal s_yul_trace_history[];
        extern uint32_t s_yul_trace_count;
        extern uint32_t s_yul_trace_head;
        extern pthread_mutex_t s_yul_trace_mutex;
        
        uint64_t head = lau_yul_thunk_sload(0xF300);
        uint64_t tail = lau_yul_thunk_sload(0xF301);
        uint64_t size = lau_yul_thunk_sload(0xF302);
        uint64_t lock = lau_yul_thunk_sload(0xF303);
        
        char pq_buf[1536];
        tsfi_ouroboros_serialize_pq(pq_buf, sizeof(pq_buf));
        
        char trace_buf[1536] = "[";
        char *t_ptr = trace_buf + 1;
        size_t t_rem = sizeof(trace_buf) - 2;
        
        pthread_mutex_lock(&s_yul_trace_mutex);
        uint32_t t_count = s_yul_trace_count;
        uint32_t t_head = s_yul_trace_head;
        pthread_mutex_unlock(&s_yul_trace_mutex);
        
        for (uint32_t i = 0; i < t_count; i++) {
            uint32_t idx = (t_head + 32 - t_count + i) % 32;
            int n = snprintf(t_ptr, t_rem, "{\"pc\": %lu, \"op\": %u}%s",
                             (unsigned long)s_yul_trace_history[idx].pc, s_yul_trace_history[idx].op,
                             (i < t_count - 1 ? ", " : ""));
            t_ptr += n;
            t_rem -= n;
            if (t_rem < 64) break;
        }
        snprintf(t_ptr, t_rem, "]");
        
        uint64_t adaptive_tick = tsfi_ouroboros_get_adaptive_tick_rate();
        uint64_t scsi_tx = lau_yul_thunk_sload(0xF304);
        uint64_t scsi_err = lau_yul_thunk_sload(0xF305);
        uint64_t scsi_signals = lau_yul_thunk_sload(0xF306);
        extern int lau_yul_thunk_cache_height(void);
        extern int lau_yul_thunk_cache_balance(void);
        int avl_height = lau_yul_thunk_cache_height();
        int avl_balance = lau_yul_thunk_cache_balance();
        char stats_buf[1024] = "[]";
        extern int lau_yul_thunk_get_cache_stats(char *buf, size_t max_len);
        lau_yul_thunk_get_cache_stats(stats_buf, sizeof(stats_buf));
        
        snprintf(output_buf, out_max, 
                 "{\"jsonrpc\": \"2.0\", \"result\": {\"cache_hits\": %lu, \"cache_lookups\": %lu, "
                 "\"evm_queue\": {\"head\": %lu, \"tail\": %lu, \"size\": %lu, \"lock\": %lu}, "
                 "\"host_heap\": %s, \"yul_trace\": %s, \"adaptive_tick_rate\": %lu, "
                 "\"scsi_tx_count\": %lu, \"scsi_parity_errors\": %lu, "
                 "\"avl_height\": %d, \"avl_balance\": %d, \"scsi_signals\": %lu, "
                 "\"contract_cache_stats\": %s}, \"id\": %d}\n", 
                 (unsigned long)g_thunk_cache_hits, (unsigned long)g_thunk_cache_lookups,
                 (unsigned long)head, (unsigned long)tail, (unsigned long)size, (unsigned long)lock,
                 pq_buf, trace_buf, (unsigned long)adaptive_tick,
                 (unsigned long)scsi_tx, (unsigned long)scsi_err,
                 avl_height, avl_balance, (unsigned long)scsi_signals,
                 stats_buf, id);
        return 1;
    }
    if (method_type == 33) { // wave512.inject_event
        uint32_t priority = (uint32_t)extract_json_int(min_ptr, "\"priority\"", 10);
        uint32_t type = (uint32_t)extract_json_int(min_ptr, "\"type\"", 1);
        uint64_t timestamp = (uint64_t)extract_json_int(min_ptr, "\"timestamp\"", 0);
        
        char hex_data[128] = "";
        uint8_t data[32] = {0};
        if (extract_json_string(min_ptr, "\"data\"", hex_data, sizeof(hex_data))) {
            char *p = hex_data;
            if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
            for (int i = 0; i < 32 && *p && *(p+1); i++) {
                unsigned int byte;
                sscanf(p, "%2x", &byte);
                data[i] = (uint8_t)byte;
                p += 2;
            }
        }
        
        extern bool tsfi_ouroboros_push_event(uint32_t priority, uint32_t type, uint64_t timestamp, const uint8_t *data);
        bool push_ok = tsfi_ouroboros_push_event(priority, type, timestamp, data);
        
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"success\": %s}, \"id\": %d}\n", 
                 push_ok ? "true" : "false", id);
        return 1;
    }
    if (method_type == 34) { // wave512.clear_cache
        extern void lau_yul_thunk_cache_clear(void);
        lau_yul_thunk_cache_clear();
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"success\": true}, \"id\": %d}\n", id);
        return 1;
    }
    static tsfi_cw_unt_cics_queue g_input_audit_queue = { "INQUE", 0, "TSQ", 0 };

    if (method_type == 30) { // input.mouse_move
        int x = extract_json_int(min_ptr, "\"x\"", 0);
        int y = extract_json_int(min_ptr, "\"y\"", 0);
        
        char audit_payload[64];
        int dummy_processed;
        snprintf(audit_payload, sizeof(audit_payload), "MM X=%d Y=%d", x, y);
        tsfi_cw_unt_cics_inject_ballistic(audit_payload, (int)strlen(audit_payload), &g_input_audit_queue, &dummy_processed);

        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF004) {
            state->reu_ram[0xF000] = (uint8_t)(x & 0xFF);
            state->reu_ram[0xF001] = (uint8_t)(y & 0xFF);
            state->reu_ram[0xF003] = (uint8_t)((x >> 8) & 0xFF);
            state->reu_ram[0xF004] = (uint8_t)((y >> 8) & 0xFF);
        }
        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t command_byte = (0x00 << 6) | 0x01;
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "MM %d %d", x, y);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mouse move OK\", \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 31) { // input.mouse_button
        int button = extract_json_int(min_ptr, "\"button\"", 272);
        int btn_state = extract_json_int(min_ptr, "\"state\"", 0);
        int x = extract_json_int(min_ptr, "\"x\"", -1);
        int y = extract_json_int(min_ptr, "\"y\"", -1);

        char audit_payload[64];
        int dummy_processed;
        snprintf(audit_payload, sizeof(audit_payload), "MB BTN=%d ST=%d X=%d Y=%d", button, btn_state, x, y);
        tsfi_cw_unt_cics_inject_ballistic(audit_payload, (int)strlen(audit_payload), &g_input_audit_queue, &dummy_processed);

        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF004 && x >= 0 && y >= 0) {
            state->reu_ram[0xF000] = (uint8_t)(x & 0xFF);
            state->reu_ram[0xF001] = (uint8_t)(y & 0xFF);
            state->reu_ram[0xF003] = (uint8_t)((x >> 8) & 0xFF);
            state->reu_ram[0xF004] = (uint8_t)((y >> 8) & 0xFF);
        }
        uint8_t btn_idx = 0;
        if (button == 273) btn_idx = 1;
        if (button == 274) btn_idx = 2;
        uint8_t command_byte = (0x01 << 6) | (btn_state ? 0x04 : 0x00) | (btn_idx & 0x03);

        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "%s %d", btn_state ? "MD" : "MU", button);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mouse button OK\", \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 32) { // input.keyboard
        int keycode = extract_json_int(min_ptr, "\"keycode\"", 0);
        int key_state = extract_json_int(min_ptr, "\"state\"", 0);

        char audit_payload[64];
        int dummy_processed;
        snprintf(audit_payload, sizeof(audit_payload), "KB KC=%d ST=%d", keycode, key_state);
        tsfi_cw_unt_cics_inject_ballistic(audit_payload, (int)strlen(audit_payload), &g_input_audit_queue, &dummy_processed);

        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF002) {
            state->reu_ram[0xF002] = (uint8_t)keycode;
        }
        uint8_t command_byte = (0x02 << 6) | (key_state ? 0x20 : 0x00) | (keycode & 0x1F);

        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "%s %d", key_state ? "KD" : "KU", keycode);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Keyboard event OK\", \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 50) { // tariffs_query
        int trunk_id = extract_json_int(min_ptr, "\"trunk_id\"", 800);
        // Calculate simulated tariff rate: base rate 60 + trunk offset * 5
        uint32_t rate = 60;
        if (trunk_id >= 800 && trunk_id <= 831) {
            rate = 60 + (trunk_id - 800) * 5;
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"rate\": %u}, \"id\": %d}\n", rate, id);
        return 1;
    }
    if (method_type == 51) { // wave512.get_price_in_pls
        char address_hex[128] = {0};
        if (extract_json_string(min_ptr, "\"address\"", address_hex, sizeof(address_hex))) {
            double price = tsfi_pulse_get_price_in_pls(address_hex);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"address\": \"%s\", \"price_pls\": %.8f}, \"id\": %d}\n", address_hex, price, id);
            return 1;
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Missing address parameter\"}, \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 63) { // wave64.get_dexscreener_price
        char address_hex[128] = {0};
        if (extract_json_string(min_ptr, "\"address\"", address_hex, sizeof(address_hex))) {
            extern bool tsfi_dexscreener_get_price(const char *token_addr, double *out_price_usd);
            double price_usd = 0.0;
            bool success = tsfi_dexscreener_get_price(address_hex, &price_usd);
            if (success) {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"address\": \"%s\", \"price_usd\": %.8f}, \"id\": %d}\n", address_hex, price_usd, id);
            } else {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32000, \"message\": \"Failed to retrieve price from DexScreener\"}, \"id\": %d}\n", id);
            }
            return 1;
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Missing address parameter\"}, \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 64) { // wave64.get_dexscreener_pairs
        char address_hex[128] = {0};
        if (extract_json_string(min_ptr, "\"address\"", address_hex, sizeof(address_hex))) {
            extern bool tsfi_dexscreener_get_pairs_json(const char *token_addr, char *out_json, size_t out_max_len);
            static char *pairs_buf = NULL;
            if (!pairs_buf) {
                pairs_buf = malloc(524288);
            }
            if (pairs_buf && tsfi_dexscreener_get_pairs_json(address_hex, pairs_buf, 524288)) {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": %s, \"id\": %d}\n", pairs_buf, id);
            } else {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32000, \"message\": \"Failed to retrieve pairs from DexScreener\"}, \"id\": %d}\n", id);
            }
            return 1;
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Missing address parameter\"}, \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 52) { // wave512.get_all_prices
        static char temp_json[131072];
        tsfi_pulse_get_all_prices_json(temp_json, sizeof(temp_json));
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": %s, \"id\": %d}\n", temp_json, id);
        return 1;
    }
    if (method_type == 53) { // wave512.get_token_holders
        char address_hex[64] = {0};
        bool force_refresh = false;
        char *p_ref = strstr(min_ptr, "\"refresh\"");
        if (p_ref) {
            char *colon = strchr(p_ref, ':');
            if (colon) {
                while (*colon == ' ' || *colon == ':' || *colon == '\t') colon++;
                if (strncmp(colon, "true", 4) == 0) {
                    force_refresh = true;
                }
            }
        }
        if (extract_json_string(min_ptr, "\"token\"", address_hex, sizeof(address_hex))) {
            static char temp_json[131072];
            tsfi_pulse_get_token_holders_json(address_hex, temp_json, sizeof(temp_json), force_refresh);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": %s, \"id\": %d}\n", temp_json, id);
            return 1;
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Missing token parameter\"}, \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 54) { // wave512.add_discovered_token
        char address_hex[64] = {0};
        char symbol[32] = {0};
        char name[128] = {0};
        uint64_t decimals = (uint64_t)extract_json_int(min_ptr, "\"decimals\"", 18);
        if (extract_json_string(min_ptr, "\"address\"", address_hex, sizeof(address_hex)) &&
            extract_json_string(min_ptr, "\"symbol\"", symbol, sizeof(symbol)) &&
            extract_json_string(min_ptr, "\"name\"", name, sizeof(name))) {
            add_discovered_token(address_hex, symbol, name, decimals);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"success\": true}, \"id\": %d}\n", id);
            return 1;
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Missing address, symbol, or name parameter\"}, \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 55) { // wave512.add_swap_edge
        char token0[64] = {0};
        char token1[64] = {0};
        double price = 0.0;
        char *p_t0 = strstr(min_ptr, "\"token0\"");
        char *p_t1 = strstr(min_ptr, "\"token1\"");
        char *p_pr = strstr(min_ptr, "\"price\"");
        
        if (p_t0) {
            char *colon = strchr(p_t0, ':');
            if (colon) {
                char *start = strchr(colon, '"');
                if (start) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end && (size_t)(end - start) < sizeof(token0)) {
                        strncpy(token0, start, end - start);
                    }
                }
            }
        }
        if (p_t1) {
            char *colon = strchr(p_t1, ':');
            if (colon) {
                char *start = strchr(colon, '"');
                if (start) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end && (size_t)(end - start) < sizeof(token1)) {
                        strncpy(token1, start, end - start);
                    }
                }
            }
        }
        if (p_pr) {
            char *colon = strchr(p_pr, ':');
            if (colon) {
                price = strtod(colon + 1, NULL);
            }
        }
        
        if (strlen(token0) > 0 && strlen(token1) > 0 && price > 0.0) {
            add_swap_edge(token0, token1, price);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"success\": true}, \"id\": %d}\n", id);
            return 1;
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Missing token0, token1, or price parameter\"}, \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 56) { // wave64.retrieve
        pthread_mutex_lock(&g_seq_store_mutex);
        if (!g_seq_store_loaded) load_seq_store();
        
        char key[128] = {0};
        bool has_key = extract_json_string(min_ptr, "\"key\"", key, sizeof(key));
        
        if (has_key) {
            int found_idx = -1;
            for (int i = 0; i < g_seq_store_count; i++) {
                if (strcmp(g_seq_store[i].key, key) == 0) {
                    found_idx = i;
                    break;
                }
            }
            if (found_idx != -1) {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"key\": \"%s\", \"value\": \"%s\", \"timestamp\": %lu}, \"id\": %d}\n",
                         g_seq_store[found_idx].key, g_seq_store[found_idx].value, (unsigned long)g_seq_store[found_idx].timestamp, id);
            } else {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32002, \"message\": \"Key not found\"}, \"id\": %d}\n", id);
            }
        } else {
            // Retrieve all as JSON array
            size_t n = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": [");
            for (int i = 0; i < g_seq_store_count; i++) {
                size_t rem = out_max - n;
                int written = snprintf(output_buf + n, rem, "{\"key\":\"%s\",\"value\":\"%s\",\"timestamp\":%lu}%s",
                                       g_seq_store[i].key, g_seq_store[i].value, (unsigned long)g_seq_store[i].timestamp,
                                       (i == g_seq_store_count - 1) ? "" : ",");
                if (written > 0) n += written;
            }
            snprintf(output_buf + n, out_max - n, "], \"id\": %d}\n", id);
        }
        pthread_mutex_unlock(&g_seq_store_mutex);
        return 1;
    }
    if (method_type == 57) { // wave64.augment
        pthread_mutex_lock(&g_seq_store_mutex);
        if (!g_seq_store_loaded) load_seq_store();
        
        char key[128] = {0};
        char val[4096] = {0};
        
        if (extract_json_string(min_ptr, "\"key\"", key, sizeof(key)) &&
            extract_json_string(min_ptr, "\"value\"", val, sizeof(val))) {
            
            int found_idx = -1;
            for (int i = 0; i < g_seq_store_count; i++) {
                if (strcmp(g_seq_store[i].key, key) == 0) {
                    found_idx = i;
                    break;
                }
            }
            
            if (found_idx != -1) {
                // augment/overwrite
                snprintf(g_seq_store[found_idx].value, sizeof(g_seq_store[found_idx].value), "%s", val);
                g_seq_store[found_idx].timestamp = (uint64_t)time(NULL);
            } else if (g_seq_store_count < MAX_STORE_KEYS) {
                // insert new
                StoreEntry *e = &g_seq_store[g_seq_store_count++];
                snprintf(e->key, sizeof(e->key), "%s", key);
                snprintf(e->value, sizeof(e->value), "%s", val);
                e->timestamp = (uint64_t)time(NULL);
            }
            save_seq_store();
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"success\": true}, \"id\": %d}\n", id);
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32602, \"message\": \"Missing key or value parameter\"}, \"id\": %d}\n", id);
        }
        pthread_mutex_unlock(&g_seq_store_mutex);
        return 1;
    }
    if (method_type == 58) { // wave64.prune
        pthread_mutex_lock(&g_seq_store_mutex);
        if (!g_seq_store_loaded) load_seq_store();
        
        char key[128] = {0};
        bool has_key = extract_json_string(min_ptr, "\"key\"", key, sizeof(key));
        
        int older_than = extract_json_int(min_ptr, "\"older_than\"", -1);
        int pruned_count = 0;
        
        if (has_key) {
            for (int i = 0; i < g_seq_store_count; i++) {
                if (strcmp(g_seq_store[i].key, key) == 0) {
                    // Remove by shifting
                    for (int j = i; j < g_seq_store_count - 1; j++) {
                        g_seq_store[j] = g_seq_store[j + 1];
                    }
                    g_seq_store_count--;
                    pruned_count++;
                    break;
                }
            }
        } else if (older_than != -1) {
            for (int i = 0; i < g_seq_store_count; ) {
                if (g_seq_store[i].timestamp < (uint64_t)older_than) {
                    for (int j = i; j < g_seq_store_count - 1; j++) {
                        g_seq_store[j] = g_seq_store[j + 1];
                    }
                    g_seq_store_count--;
                    pruned_count++;
                } else {
                    i++;
                }
            }
        } else {
            // Prune all
            pruned_count = g_seq_store_count;
            g_seq_store_count = 0;
        }
        
        if (pruned_count > 0) {
            save_seq_store();
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"pruned_count\": %d}, \"id\": %d}\n", pruned_count, id);
        pthread_mutex_unlock(&g_seq_store_mutex);
        return 1;
    }
    if (method_type == 59) { // wave64.get_unpriced_tokens
        static char temp_json[131072];
        extern int tsfi_pulse_get_unpriced_tokens_json(char *out_buf, size_t max_len);
        tsfi_pulse_get_unpriced_tokens_json(temp_json, sizeof(temp_json));
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": %s, \"id\": %d}\n", temp_json, id);
        return 1;
    }
    if (method_type == 60 || method_type == 61 || method_type == 62) {
        extern int tsfi_zmm_rpc_dispatch_pulse(TsfiZmmVmState *state, int method_type, const char *min_ptr, char *output_buf, size_t out_max, int id);
        return tsfi_zmm_rpc_dispatch_pulse(state, method_type, min_ptr, output_buf, out_max, id);
    }

    
    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Method not found\", \"id\": %d}\n", id);
    return 1;
    return 0;
}

