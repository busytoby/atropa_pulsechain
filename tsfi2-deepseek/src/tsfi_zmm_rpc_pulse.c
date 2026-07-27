#include "tsfi_zmm_vm.h"
#include "tsfi_block_monitor.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int tsfi_zmm_rpc_dispatch_pulse(TsfiZmmVmState *state, int method_type, const char *min_ptr, char *output_buf, size_t out_max, int id) {
    (void)state;
    (void)min_ptr;
    if (method_type == 60) {
        if (tsfi_block_monitor_get_pending_count() == 0) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": null, \"pending_count\": 0, \"id\": %d}\n", id);
            return 1;
        }
        uint64_t last_block = tsfi_block_monitor_get_newest_pending_block();
        char payload[256];
        snprintf(payload, sizeof(payload),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getBlockByNumber\",\"params\":[\"0x%lx\",true],\"id\":1}",
                 (unsigned long)last_block);
        
        static char rpc_body[524288];
        extern bool tsfi_pulse_rpc_exec_raw_body(const char *json_payload, char *out_buffer, size_t out_max_len);
        if (tsfi_pulse_rpc_exec_raw_body(payload, rpc_body, sizeof(rpc_body))) {
            char *result_start = strstr(rpc_body, "\"result\":");
            if (result_start) {
                result_start += 9;
                while (*result_start == ' ' || *result_start == '\t' || *result_start == '\r' || *result_start == '\n') {
                    result_start++;
                }
                size_t len = strlen(result_start);
                while (len > 0) {
                    char c = result_start[len - 1];
                    if (c == '}' || c == '\r' || c == '\n' || c == ' ' || c == '\t') {
                        if (c == '}') {
                            result_start[len - 1] = '\0';
                            break;
                        }
                        result_start[len - 1] = '\0';
                        len--;
                    } else {
                        break;
                    }
                }
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": %s, \"pending_count\": %d, \"id\": %d}\n",
                         result_start, tsfi_block_monitor_get_pending_count(), id);
                return 1;
            }
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32001, \"message\": \"Failed to retrieve block transactions\"}, \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 61) {
        uint32_t count = tsfi_block_monitor_get_discovered_tokens_count();
        size_t n = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"contracts\": [");
        bool first = true;
        for (uint32_t i = 0; i < count; i++) {
            char addr[64];
            char symbol[32];
            char name[128];
            tsfi_block_monitor_get_discovered_token(i, addr, symbol, name);
            
            size_t sym_len = strlen(symbol);
            if (sym_len >= 8 && strcmp(symbol + sym_len - 8, "Contract") == 0) continue;
            if (sym_len >= 6 && strcmp(symbol + sym_len - 6, "Router") == 0) continue;
            if (strcmp(symbol, "UNKNOWN") == 0 || strcmp(symbol, "NoContract") == 0) continue;
            
            size_t rem = out_max - n;
            int written = snprintf(output_buf + n, rem, "%s{\"address\":\"%s\",\"symbol\":\"%s\",\"name\":\"%s\"}",
                                   first ? "" : ",", addr, symbol, name);
            if (written > 0) {
                n += written;
                first = false;
            }
        }
        snprintf(output_buf + n, out_max - n, "] }, \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 62) {
        uint64_t block_to_prune = 0;
        const char *blk_ptr = strstr(min_ptr, "\"block\"");
        if (blk_ptr) {
            blk_ptr = strchr(blk_ptr, ':');
            if (blk_ptr) {
                blk_ptr++;
                while (*blk_ptr == ' ' || *blk_ptr == '\t') blk_ptr++;
                block_to_prune = strtoull(blk_ptr, NULL, 10);
            }
        }
        bool success = false;
        if (block_to_prune > 0) {
            success = tsfi_block_monitor_prune_block(block_to_prune);
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"pruned\": %lu, \"success\": %s}, \"id\": %d}\n",
                 (unsigned long)block_to_prune, success ? "true" : "false", id);
        return 1;
    }
    return 0;
}
