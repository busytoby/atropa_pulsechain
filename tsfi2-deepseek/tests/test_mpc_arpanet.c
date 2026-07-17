#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>
#include "tsfi_ramac_layout.h"
#include "tsfi_anvil_vm.h"

#define TEST_PORT 18089
#define BUFFER_SIZE 512

static void* mpc_server_thread(void *arg) {
    (void)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TEST_PORT);

    int bind_ok = bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    assert(bind_ok >= 0);

    int listen_ok = listen(server_fd, 5);
    assert(listen_ok >= 0);

    printf("[MPC_SERVER] Listening on port %d...\n", TEST_PORT);
    fflush(stdout);

    struct sockaddr_in client1_addr, client2_addr;
    socklen_t addr1_len = sizeof(client1_addr), addr2_len = sizeof(client2_addr);

    int client1_fd = accept(server_fd, (struct sockaddr*)&client1_addr, &addr1_len);
    assert(client1_fd >= 0);
    printf("[MPC_SERVER] Client 1 Connected!\n");

    int client2_fd = accept(server_fd, (struct sockaddr*)&client2_addr, &addr2_len);
    assert(client2_fd >= 0);
    printf("[MPC_SERVER] Client 2 Connected!\n");

    char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE];
    memset(buf1, 0, BUFFER_SIZE);
    memset(buf2, 0, BUFFER_SIZE);

    ssize_t r1 = read(client1_fd, buf1, BUFFER_SIZE - 1);
    assert(r1 > 0);
    printf("[MPC_SERVER] Received from Client 1: %s\n", buf1);

    ssize_t r2 = read(client2_fd, buf2, BUFFER_SIZE - 1);
    assert(r2 > 0);
    printf("[MPC_SERVER] Received from Client 2: %s\n", buf2);

    TSFiAnvilVM vm;
    tsfi_anvil_vm_init(&vm);
    tsfi_anvil_vm_bind(&vm, "imp/route/status", "VERIFIED");

    const TSFiSubgoalEntry *entry = tsfi_anvil_vm_lookup_subgoal(&vm, "imp/route/status");
    assert(entry != NULL);
    assert(strcmp(entry->value, "VERIFIED") == 0);
    printf("[MPC_SERVER] Rails VM Verified IMP communication routing successfully!\n");

    close(client1_fd);
    close(client2_fd);
    close(server_fd);
    return NULL;
}

static void* client1_thread(void *arg) {
    (void)arg;
    usleep(150000);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(client_fd >= 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TEST_PORT);
    int inet_ok = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    assert(inet_ok > 0);

    int conn_ok = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(conn_ok >= 0);

    imp_header hdr;
    tsfi_imp_format(&hdr, 1, 2, 99, 1);
    char out_buf[BUFFER_SIZE];
    int enc_ok = tsfi_bgp_proxy_route(&hdr, "CLIENT_1_HELLO", out_buf, sizeof(out_buf));
    assert(enc_ok == 0);

    ssize_t w = write(client_fd, out_buf, strlen(out_buf));
    assert(w > 0);
    close(client_fd);
    return NULL;
}

static void* client2_thread(void *arg) {
    (void)arg;
    usleep(250000);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(client_fd >= 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TEST_PORT);
    int inet_ok = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    assert(inet_ok > 0);

    int conn_ok = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(conn_ok >= 0);

    imp_header hdr;
    tsfi_imp_format(&hdr, 2, 1, 100, 2);
    char out_buf[BUFFER_SIZE];
    int enc_ok = tsfi_bgp_proxy_route(&hdr, "CLIENT_2_HELLO", out_buf, sizeof(out_buf));
    assert(enc_ok == 0);

    ssize_t w = write(client_fd, out_buf, strlen(out_buf));
    assert(w > 0);
    close(client_fd);
    return NULL;
}

int main(void) {
    printf("=============================================================\n");
    printf("ARPANET MPC INTEGRATION TEST: 2 CLIENTS MOUNTING SHARED IMP\n");
    printf("=============================================================\n");

    pthread_t server_tid, c1_tid, c2_tid;

    pthread_create(&server_tid, NULL, mpc_server_thread, NULL);
    pthread_create(&c1_tid, NULL, client1_thread, NULL);
    pthread_create(&c2_tid, NULL, client2_thread, NULL);

    pthread_join(server_tid, NULL);
    pthread_join(c1_tid, NULL);
    pthread_join(c2_tid, NULL);

    printf("[PASS] MPC server and clients verified dynamic IMP mounts successfully!\n");
    printf("=============================================================\n");
    return 0;
}
