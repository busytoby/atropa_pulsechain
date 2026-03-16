#include "tsfi_pulsechain_rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

// Bare-metal C POSIX Socket Implementation for PulseChain RPC.
// Note: To interact directly with https://rpc.pulsechain.com without OpenSSL linked,
// we will target an unencrypted RPC HTTP relay or use an internal stunnel/proxy.
// Since we require zero dependencies, we will assume a local HTTP proxy running on port 8545
// or directly support an HTTP interface if one is provided.
// For the sake of pure C execution without curl, we map directly via TCP sockets.

#define RPC_HOST "127.0.0.1" // Assumes local geth/erigon node or local HTTP relay to PulseChain
#define RPC_PORT "8545"

static bool exec_raw_socket_rpc(const char *json_payload, char *out_hex_buffer, size_t out_max_len) {
    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(RPC_HOST, RPC_PORT, &hints, &res) != 0) return false;

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
             "Host: %s:%s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             RPC_HOST, RPC_PORT, strlen(json_payload), json_payload);

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
             
    return exec_raw_socket_rpc(payload, out_hex_buffer, out_max_len);
}

bool tsfi_pulse_rpc_get_storage_at(const char *address, const char *slot_hex, char *out_hex_buffer, size_t out_max_len) {
    char payload[1024];
    snprintf(payload, sizeof(payload), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getStorageAt\",\"params\":[\"%s\",\"%s\",\"latest\"],\"id\":1}",
             address, slot_hex);
             
    return exec_raw_socket_rpc(payload, out_hex_buffer, out_max_len);
}
