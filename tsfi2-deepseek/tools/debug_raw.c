#include "tsfi_tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

int main(void) {
    printf("=== Live RPC Debug Raw ===\n");
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo("rpc.pulsechain.com", "443", &hints, &res) != 0) {
        printf("DNS Resolution failed\n");
        return 1;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        printf("Connection failed\n");
        freeaddrinfo(res);
        return 1;
    }
    freeaddrinfo(res);
    
    TsfiTlsContext tls;
    tsfi_tls_init(&tls, sockfd);
    if (!tsfi_tls_handshake(&tls, "rpc.pulsechain.com")) {
        printf("TLS handshake failed\n");
        tsfi_tls_close(&tls);
        return 1;
    }
    
    const char *payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1}";
    
    char request[2048];
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: rpc.pulsechain.com\r\n"
             "User-Agent: Mozilla/5.0\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             strlen(payload), payload);
             
    tsfi_tls_write(&tls, request, strlen(request));
    
    static char response[65536];
    size_t total = 0;
    ssize_t n;
    while ((n = tsfi_tls_read(&tls, response + total, sizeof(response) - 1 - total)) > 0) {
        total += n;
    }
    response[total] = '\0';
    tsfi_tls_close(&tls);
    
    printf("--- BlockNumber Response ---\n%s\n", response);
    
    // Now let's try eth_getLogs
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (getaddrinfo("rpc.pulsechain.com", "443", &hints, &res) == 0) {
        connect(sockfd, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
    }
    
    tsfi_tls_init(&tls, sockfd);
    tsfi_tls_handshake(&tls, "rpc.pulsechain.com");
    
    const char *logs_payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getLogs\",\"params\":[{\"fromBlock\":\"0x19c7fe0\",\"toBlock\":\"0x19c7fe0\",\"topics\":[\"0xd78ad95fa46c994b6551d0da85fc275fe613ce37657fb8d5e3d130840159d822\"]}],\"id\":1}";
    
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: rpc.pulsechain.com\r\n"
             "User-Agent: Mozilla/5.0\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             strlen(logs_payload), logs_payload);
             
    tsfi_tls_write(&tls, request, strlen(request));
    
    total = 0;
    while ((n = tsfi_tls_read(&tls, response + total, sizeof(response) - 1 - total)) > 0) {
        total += n;
    }
    response[total] = '\0';
    tsfi_tls_close(&tls);
    
    printf("--- GetLogs Response ---\n%s\n", response);
    
    return 0;
}
