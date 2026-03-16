#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "lau_memory.h"

uint32_t g_socket_mem_sync = 0;

void* socket_server_thread(void* arg) {
    int port = *(int*)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    int opt = 1; setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return NULL; }
    listen(server_fd, 1);

    printf("[SERVER] Waiting for connection...\n");
    int client_fd = accept(server_fd, NULL, NULL);
    
    usleep(500000);
    printf("[SERVER] Sending data...\n");
    if (write(client_fd, "RTMP_DATA", 9) < 0) perror("write server");
    
    usleep(500000);
    printf("[SERVER] Triggering memory sync...\n");
    g_socket_mem_sync = 0xBC;

    close(client_fd);
    close(server_fd);
    return NULL;
}

int main() {
    printf("=== TSFi Socket Wait Wavefront Test ===\n");

    int port = 12346; // Use different port
    pthread_t thread;
    pthread_create(&thread, NULL, socket_server_thread, &port);

    usleep(200000); 
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); return 1;
    }
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    int fds[1] = { client_fd };
    void* mem_addrs[1] = { &g_socket_mem_sync };
    uint32_t mem_masks[1] = { 0 };

    int events_found = 0;
    while (events_found < 2) {
        int res = tsfi_io_wait_universal(fds, 1, mem_addrs, mem_masks, 1, 3000);
        
        if (res == 0) { 
            char buf[64];
            int n = read(client_fd, buf, sizeof(buf));
            if (n > 0) {
                buf[n] = 0;
                printf("[PASS] Detected Socket Event: %s\n", buf);
                events_found++;
            }
        } else if (res == 1) { 
            printf("[PASS] Detected Memory Sync Event: 0x%x\n", g_socket_mem_sync);
            mem_masks[0] = g_socket_mem_sync;
            events_found++;
        } else if (res == -1) {
            printf("[FAIL] Timeout waiting for socket/memory events.\n");
            break;
        }
    }

    pthread_join(thread, NULL);
    close(client_fd);

    if (events_found == 2) {
        printf("=== Socket Wait Wavefront Passed ===\n");

        extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    return 0;
    }
    return 1;
}
