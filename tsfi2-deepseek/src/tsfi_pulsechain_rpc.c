#include "tsfi_pulsechain_rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define RPC_HOST "127.0.0.1"
#define RPC_PORT "8545"

static bool exec_raw_http_rpc(const char *json_payload, char *out_hex_buffer, size_t out_max_len) {
    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(RPC_HOST, RPC_PORT, &hints, &res) != 0) {
        return false;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return false;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);

    char request[2048];
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             RPC_HOST, strlen(json_payload), json_payload);

    if (write(sockfd, request, strlen(request)) < 0) {
        close(sockfd);
        return false;
    }

    char response[8192];
    size_t total_read = 0;
    ssize_t n;
    while ((n = read(sockfd, response + total_read, sizeof(response) - 1 - total_read)) > 0) {
        total_read += n;
    }
    close(sockfd);
    
    response[total_read] = '\0';

    char *body = strstr(response, "\r\n\r\n");
    if (!body) return false;
    body += 4;

    char *result_ptr = strstr(body, "\"result\":\"");
    if (result_ptr) {
        result_ptr += 10;
        char *end_quote = strchr(result_ptr, '"');
        if (end_quote) {
            size_t hex_len = end_quote - result_ptr;
            if (hex_len < out_max_len) {
                memcpy(out_hex_buffer, result_ptr, hex_len);
                out_hex_buffer[hex_len] = '\0';
                return true;
            }
        }
    }

    return false;
}

bool tsfi_pulse_rpc_call(const char *to_address, const char *data_hex, char *out_hex_buffer, size_t out_max_len) {
    char payload[1024];
    snprintf(payload, sizeof(payload), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"data\":\"%s\"},\"latest\"],\"id\":1}",
             to_address, data_hex);
             
    return exec_raw_http_rpc(payload, out_hex_buffer, out_max_len);
}

bool tsfi_pulse_rpc_call_from(const char *to_address, const char *from_address, const char *data_hex, char *out_hex_buffer, size_t out_max_len) {
    char payload[1536];
    snprintf(payload, sizeof(payload), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"from\":\"%s\",\"data\":\"%s\"},\"latest\"],\"id\":1}",
             to_address, from_address, data_hex);
             
    return exec_raw_http_rpc(payload, out_hex_buffer, out_max_len);
}

bool tsfi_pulse_rpc_get_storage_at(const char *address, const char *slot_hex, char *out_hex_buffer, size_t out_max_len) {
    char payload[1024];
    snprintf(payload, sizeof(payload), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getStorageAt\",\"params\":[\"%s\",\"%s\",\"latest\"],\"id\":1}",
             address, slot_hex);
             
    return exec_raw_http_rpc(payload, out_hex_buffer, out_max_len);
}

bool tsfi_pulse_rpc_send_raw_transaction(const char *signed_tx_hex, char *out_tx_hash, size_t out_max_len) {
    // The signed_tx_hex can be quite large (e.g., contract deployments or large calldata)
    // We dynamically allocate the payload to ensure it fits.
    size_t tx_len = strlen(signed_tx_hex);
    size_t payload_size = tx_len + 256;
    char *payload = malloc(payload_size);
    if (!payload) return false;

    snprintf(payload, payload_size, 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendRawTransaction\",\"params\":[\"%s\"],\"id\":1}",
             signed_tx_hex);
             
    bool result = exec_raw_http_rpc(payload, out_tx_hash, out_max_len);
    free(payload);
    return result;
}

bool tsfi_pulse_rpc_send_wmq_transaction(const char *to_address, const char *data_hex) {
    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"%s\","
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             to_address, data_hex);
             
    char out_hex[512];
    return exec_raw_http_rpc(payload, out_hex, sizeof(out_hex));
}

static uint64_t parse_hex64(const char *hex) {
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }
    size_t len = strlen(hex);
    if (len > 16) {
        hex += (len - 16);
    }
    return strtoull(hex, NULL, 16);
}

static uint64_t get_wmq_address_u64(char *out_addr_str) {
    FILE *f = fopen("../tmp/wmq_address.txt", "r");
    if (!f) {
        f = fopen("tmp/wmq_address.txt", "r");
    }
    if (!f) {
        f = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/tmp/wmq_address.txt", "r");
    }
    char buf[128] = {0};
    if (f) {
        if (fgets(buf, sizeof(buf), f)) {
            size_t l = strlen(buf);
            while (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) {
                buf[l-1] = '\0';
                l--;
            }
        }
        fclose(f);
    }
    if (out_addr_str) {
        strcpy(out_addr_str, buf);
    }
    if (strlen(buf) > 0) {
        return parse_hex64(buf);
    }
    return 0;
}

static void forward_to_mcp_server(const char *cmd) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return;
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10042);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
        char payload[1024];
        snprintf(payload, sizeof(payload), 
                 "{\"jsonrpc\":\"2.0\",\"method\":\"wave512.dilemma_log\",\"params\":{"
                 "\"event\":\"%s\",\"source\":\"WinchesterMQ Bridge\",\"details\":\"Forwarded from C Thunk Broker\""
                 "},\"id\":1}", cmd);
        ssize_t nw = write(sockfd, payload, strlen(payload));
        (void)nw;
        char dummy[128];
        ssize_t nr = read(sockfd, dummy, sizeof(dummy));
        (void)nr;
    }
    close(sockfd);
}

void tsfi_thunk_publish_mq(const char *cmd) {
    char wmq_addr_str[128] = {0};
    uint64_t wmq_addr_u64 = get_wmq_address_u64(wmq_addr_str);
    if (wmq_addr_u64 == 0) {
        printf("[THUNK_MQ] Error: Auncient WinchesterMQ address not found in tmp/wmq_address.txt\n");
        return;
    }
    
    char cmd_buf[128];
    strncpy(cmd_buf, cmd, sizeof(cmd_buf) - 1);
    cmd_buf[sizeof(cmd_buf) - 1] = '\0';
    
    char processed[128] = {0};
    char *src = cmd_buf;
    char *dst = processed;
    while (*src && (dst - processed) < 120) {
        if (strncmp(src, "YOUTUBE:", 8) == 0) {
            strcpy(dst, "Y:");
            dst += 2;
            src += 8;
        } else if (strncmp(src, "MAIN:", 5) == 0) {
            strcpy(dst, "M:");
            dst += 2;
            src += 5;
        } else if (strncmp(src, "MOUSE_MOVE", 10) == 0) {
            strcpy(dst, "MM");
            dst += 2;
            src += 10;
        } else if (strncmp(src, "MOUSE_DOWN", 10) == 0) {
            strcpy(dst, "MD");
            dst += 2;
            src += 10;
        } else if (strncmp(src, "MOUSE_UP", 8) == 0) {
            strcpy(dst, "MU");
            dst += 2;
            src += 8;
        } else if (strncmp(src, "MOUSE_SCROLL", 12) == 0) {
            strcpy(dst, "MS");
            dst += 2;
            src += 12;
        } else if (strncmp(src, "KEY_DOWN", 8) == 0) {
            strcpy(dst, "KD");
            dst += 2;
            src += 8;
        } else if (strncmp(src, "KEY_UP", 6) == 0) {
            strcpy(dst, "KU");
            dst += 2;
            src += 6;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    if (strstr(processed, "MM") || strstr(processed, "MD") || strstr(processed, "MU") ||
        strstr(processed, "MS") || strstr(processed, "KD") || strstr(processed, "KU") ||
        strstr(processed, "MO")) {
        printf("[THUNK_MQ] I/O Event bypassed RPC publish: %s\n", processed);
        return;
    }
    
    size_t len = strlen(processed);
    uint8_t data_bytes[32] = {0};
    memcpy(data_bytes, processed, len < 32 ? len : 32);
    
    char tx_data[128];
    strcpy(tx_data, "0xccb077a0");
    for (int i = 0; i < 32; i++) {
        sprintf(tx_data + 10 + i * 2, "%02x", data_bytes[i]);
    }
    
    bool ok = tsfi_pulse_rpc_send_wmq_transaction(wmq_addr_str, tx_data);
    printf("[THUNK_MQ] Event published to Auncient WinchesterMQ: %s (status: %s)\n", processed, ok ? "SUCCESS" : "FAILED");
    forward_to_mcp_server(processed);
}

bool tsfi_pulse_rpc_exec_raw(const char *json_payload, char *out_buffer, size_t out_max_len) {
    return exec_raw_http_rpc(json_payload, out_buffer, out_max_len);
}



