#include <node_api.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Structure definitions matching tsfi headers
typedef struct {
    uint64_t d[4];
} u256_t;

// Forward declarations of C functions from presenter/thunk library
extern "C" {
    void handle_bridge_command(const char *cmd);
    int lau_yul_thunk_get_log_count(void);
    int lau_yul_thunk_get_log(int index, uint64_t *log_addr, int *num_topics, u256_t *topics, uint8_t *log_data, size_t *log_data_size);
    char* pop_input_event(void);
}

static napi_value GetNextInputEvent(napi_env env, napi_callback_info info) {
    (void)info;
    char* cmd = pop_input_event();
    if (cmd) {
        napi_value js_str;
        napi_create_string_utf8(env, cmd, strlen(cmd), &js_str);
        free(cmd);
        return js_str;
    }
    napi_value js_null;
    napi_get_null(env, &js_null);
    return js_null;
}

static napi_value PushEvent(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    char buf[256];
    size_t copied;
    napi_get_value_string_utf8(env, args[0], buf, sizeof(buf), &copied);

    // Call presenter command handler directly in-memory
    handle_bridge_command(buf);

    return NULL;
}

static napi_value GetNextLogEvent(napi_env env, napi_callback_info info) {
    (void)info;
    static int processed_logs_count = 0;
    
    int total_logs = lau_yul_thunk_get_log_count();
    while (processed_logs_count < total_logs) {
        int k = processed_logs_count++;
        uint64_t log_addr = 0;
        int num_topics = 0;
        u256_t topics[4];
        memset(topics, 0, sizeof(topics));
        uint8_t log_data[2048] = {0};
        size_t log_data_size = sizeof(log_data);
        if (lau_yul_thunk_get_log(k, &log_addr, &num_topics, topics, log_data, &log_data_size)) {
            // Check if topic matching topic0 = 0xe1dae1da...
            if (num_topics > 0 && topics[0].d[3] == 0xe1dae1dae1dae1daULL && topics[0].d[2] == 0xe1dae1dae1dae1daULL) {
                char cmd_str[256] = {0};
                size_t csize = log_data_size < 255 ? log_data_size : 255;
                memcpy(cmd_str, log_data, csize);
                
                // Clean any null-padding or non-ascii
                for (size_t i = 0; i < csize; i++) {
                    if (cmd_str[i] < 32 || cmd_str[i] > 126) {
                        cmd_str[i] = '\0';
                        break;
                    }
                }
                
                napi_value js_str;
                napi_create_string_utf8(env, cmd_str, strlen(cmd_str), &js_str);
                return js_str;
            }
        }
    }
    
    napi_value js_null;
    napi_get_null(env, &js_null);
    return js_null;
}

static napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_push, fn_get, fn_get_input;
    napi_create_function(env, NULL, 0, PushEvent, NULL, &fn_push);
    napi_set_named_property(env, exports, "pushEvent", fn_push);
    
    napi_create_function(env, NULL, 0, GetNextLogEvent, NULL, &fn_get);
    napi_set_named_property(env, exports, "getNextLogEvent", fn_get);

    napi_create_function(env, NULL, 0, GetNextInputEvent, NULL, &fn_get_input);
    napi_set_named_property(env, exports, "getNextInputEvent", fn_get_input);
    
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
