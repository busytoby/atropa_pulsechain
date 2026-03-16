#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <poll.h>
#include "tsfi_raw.h"

#define RTMP_PORT_TEST 19350

void* mock_rtmp_server(void *arg) {
    (void)arg;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(RTMP_PORT_TEST), .sin_addr.s_addr = INADDR_ANY };
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 1);

    int client_fd = accept(listen_fd, NULL, NULL);
    
    uint8_t c0;
    ssize_t nr = read(client_fd, &c0, 1); (void)nr;
    uint8_t c1[1536];
    nr = read(client_fd, c1, 1536); (void)nr;

    uint8_t s0 = 0x03;
    ssize_t nw = write(client_fd, &s0, 1); (void)nw;
    uint8_t s1[1536]; memset(s1, 0xAA, 1536);
    nw = write(client_fd, s1, 1536); (void)nw;
    uint8_t s2[1536]; memset(s2, 0xBB, 1536);
    nw = write(client_fd, s2, 1536); (void)nw;

    uint8_t c2[1536];
    nr = read(client_fd, c2, 1536); (void)nr;

    close(client_fd);
    close(listen_fd);
    return NULL;
}

typedef struct {
    int sockfd;
    char *stream_key;
} TSFiRtmpContext;

extern int rtmp_handshake(TSFiRtmpContext *ctx);

#include <signal.h>
#include "tsfi_raw.h"

void test_rtmp_handshake_resilience() {
    signal(SIGPIPE, SIG_IGN);
    printf("[TEST] Verifying RTMP Handshake Resilience...\n");

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, mock_rtmp_server, NULL);
    tsfi_raw_usleep(100000); 

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(RTMP_PORT_TEST) };
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("connect failed");
        return;
    }

    TSFiRtmpContext ctx = { .sockfd = sockfd, .stream_key = "TEST" };
    
    int res = rtmp_handshake(&ctx);
    (void)res;
    assert(res == 0);

    printf("[PASS] RTMP Handshake Sequence (C0-C2 / S0-S2) Verified.\n");

    pthread_join(server_thread, NULL);
    close(sockfd);
}

void* mock_rtmp_server_fail_once(void *arg) {
    (void)arg;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(RTMP_PORT_TEST), .sin_addr.s_addr = INADDR_ANY };
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[MOCK] bind failed");
        return NULL;
    }
    listen(listen_fd, 1);

    // 1. First client: accept and close immediately (fail)
    printf("[MOCK] Waiting for first attempt...\n");
    int client_fd1 = accept(listen_fd, NULL, NULL);
    if (client_fd1 >= 0) {
        printf("[MOCK] Closing first client immediately.\n");
        close(client_fd1);
    }
    tsfi_raw_usleep(100000); // Wait for client to detect close

    // 2. Second client: process normally
    printf("[MOCK] Waiting for second attempt...\n");
    int client_fd2 = accept(listen_fd, NULL, NULL);
    if (client_fd2 >= 0) {
        printf("[MOCK] Processing second client...\n");
        uint8_t c0; ssize_t nr = read(client_fd2, &c0, 1); (void)nr;
        uint8_t c1[1536]; nr = read(client_fd2, c1, 1536); (void)nr;
        uint8_t s0 = 0x03; ssize_t nw = write(client_fd2, &s0, 1); (void)nw;
        uint8_t s1[1536]; memset(s1, 0xAA, 1536); nw = write(client_fd2, s1, 1536); (void)nw;
        uint8_t s2[1536]; memset(s2, 0xBB, 1536); nw = write(client_fd2, s2, 1536); (void)nw;
        uint8_t c2[1536]; nr = read(client_fd2, c2, 1536); (void)nr;
        close(client_fd2);
        printf("[MOCK] Second client finished.\n");
    }

    close(listen_fd);
    return NULL;
}

void test_rtmp_handshake_retry() {
    printf("[TEST] Verifying RTMP Handshake Retry (Fail Once)...\n");

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, mock_rtmp_server_fail_once, NULL);
    tsfi_raw_usleep(100000);

    int sockfd;
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(RTMP_PORT_TEST) };
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    // First attempt: Connect, but server closes client_fd1 immediately
    printf("[TEST] Attempt 1...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        TSFiRtmpContext ctx = { .sockfd = sockfd, .stream_key = "TEST" };
        int res = rtmp_handshake(&ctx);
        (void)res;
        printf("[TEST] Attempt 1 Handshake Result: %d (Expected Non-Zero)\n", res);
        assert(res != 0); 
        close(sockfd);
    }

    // Wait for mock server to reach second accept
    tsfi_raw_usleep(200000);

    // Second attempt: Re-connect, server should process normally
    printf("[TEST] Attempt 2...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        TSFiRtmpContext ctx2 = { .sockfd = sockfd, .stream_key = "TEST" };
        int res2 = rtmp_handshake(&ctx2);
        (void)res2;
        printf("[TEST] Attempt 2 Handshake Result: %d (Expected 0)\n", res2);
        assert(res2 == 0); 
        close(sockfd);
    }

    printf("[PASS] Handshake Retry verified manually via sequence.\n");
    pthread_join(server_thread, NULL);
}

int main() {
    printf("=== TSFi RTMP Handshake Unit Test ===\n");
    test_rtmp_handshake_resilience();
    test_rtmp_handshake_retry();
    printf("ALL HANDSHAKE TESTS PASSED\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
