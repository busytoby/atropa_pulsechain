#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PROXY_PORT     8888
#define SERVICE_PORT   9999

// Simulated MCP pipe channel (bidirectional)
int g_mcp_pipe_b_to_a[2]; // Peer B writes, Peer A reads
int g_mcp_pipe_a_to_b[2]; // Peer A writes, Peer B reads

typedef struct {
    pthread_t thread;
    bool is_running;
} WorkerContext;

// Destination Echo Service (Run by Peer B)
void *echo_service_thread(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(SERVICE_PORT);
    
    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 5);
    
    while (ctx->is_running) {
        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        
        int r = select(listen_fd + 1, &rfds, NULL, NULL, &tv);
        if (r > 0) {
            int client_fd = accept(listen_fd, NULL, NULL);
            char buf[128];
            ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = '\0';
                // Append " [SERVICE RESPONSE]" to verify it reached the destination
                char resp[256];
                snprintf(resp, sizeof(resp), "%s [SERVICE RESPONSE]", buf);
                send(client_fd, resp, strlen(resp), 0);
            }
            close(client_fd);
        }
    }
    close(listen_fd);
    return NULL;
}

// Peer B MCP Proxy Handler (Forwards data from MCP channel to local service on 9999)
void *peer_b_mcp_handler(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    char mcp_buf[512];
    
    while (ctx->is_running) {
        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_mcp_pipe_a_to_b[0], &rfds);
        
        int r = select(g_mcp_pipe_a_to_b[0] + 1, &rfds, NULL, NULL, &tv);
        if (r > 0) {
            // Read from Peer A over MCP channel
            ssize_t n = read(g_mcp_pipe_a_to_b[0], mcp_buf, sizeof(mcp_buf));
            if (n <= 0) break;
            
            // Connect to local service (port 9999)
            int svc_fd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(SERVICE_PORT);
            
            if (connect(svc_fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                // Forward request to service
                send(svc_fd, mcp_buf, n, 0);
                
                // Read response from service
                char svc_resp[512];
                ssize_t svc_n = recv(svc_fd, svc_resp, sizeof(svc_resp), 0);
                if (svc_n > 0) {
                    // Send response back to Peer A over MCP channel
                    write(g_mcp_pipe_b_to_a[1], svc_resp, svc_n);
                }
            }
            close(svc_fd);
        }
    }
    return NULL;
}

// Peer A Proxy Listener (Listens on port 8888, forwards connections over MCP to Peer B)
void *peer_a_proxy_listener(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PROXY_PORT);
    
    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 5);
    
    while (ctx->is_running) {
        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        
        int r = select(listen_fd + 1, &rfds, NULL, NULL, &tv);
        if (r > 0) {
            int client_fd = accept(listen_fd, NULL, NULL);
            char buf[512];
            ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
            if (n > 0) {
                // 1. Forward client request to Peer B over MCP channel
                write(g_mcp_pipe_a_to_b[1], buf, n);
                
                // 2. Wait for response from Peer B over MCP channel
                char mcp_resp[512];
                ssize_t mcp_n = read(g_mcp_pipe_b_to_a[0], mcp_resp, sizeof(mcp_resp));
                if (mcp_n > 0) {
                    // 3. Write response back to local TCP client
                    send(client_fd, mcp_resp, mcp_n, 0);
                }
            }
            close(client_fd);
        }
    }
    close(listen_fd);
    return NULL;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: MCP-TUNNELED TCP CLIENT PROXY TEST\n");
    printf("=============================================================\n");

    // Initialize MCP Pipe Channels
    if (pipe(g_mcp_pipe_b_to_a) != 0 || pipe(g_mcp_pipe_a_to_b) != 0) {
        perror("Pipe creation failed");
        return 1;
    }

    WorkerContext svc_ctx = { .is_running = true };
    WorkerContext b_mcp_ctx = { .is_running = true };
    WorkerContext a_proxy_ctx = { .is_running = true };

    // 1. Spin up Peer B's internal service (port 9999)
    pthread_create(&svc_ctx.thread, NULL, echo_service_thread, &svc_ctx);
    
    // 2. Spin up Peer B's MCP listener
    pthread_create(&b_mcp_ctx.thread, NULL, peer_b_mcp_handler, &b_mcp_ctx);

    // 3. Spin up Peer A's TCP proxy listener (port 8888) representing Peer B's mounted service
    pthread_create(&a_proxy_ctx.thread, NULL, peer_a_proxy_listener, &a_proxy_ctx);

    usleep(50000); // Wait for listeners to start

    // 4. Connect a standard TCP Client to Peer A's proxy port (8888)
    printf("1. TCP client connecting to Peer A local proxy port %d...\n", PROXY_PORT);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PROXY_PORT);
    
    int conn_res = connect(client_fd, (struct sockaddr *)&addr, sizeof(addr));
    assert(conn_res == 0);
    printf("   ✓ Connected successfully.\n");

    // 5. Send message from client to Peer A proxy
    const char *msg = "HELLO VIA TUNNEL";
    printf("2. Sending request: '%s'\n", msg);
    send(client_fd, msg, strlen(msg), 0);

    // 6. Read back response from proxy
    char resp_buf[512] = {0};
    ssize_t n = recv(client_fd, resp_buf, sizeof(resp_buf) - 1, 0);
    assert(n > 0);
    resp_buf[n] = '\0';
    printf("3. Received response: '%s'\n", resp_buf);

    // Verify proxy forwarded data over MCP to Peer B's service and back
    assert(strcmp(resp_buf, "HELLO VIA TUNNEL [SERVICE RESPONSE]") == 0);
    printf("   ✓ Proxy forwarding and routing verified!\n\n");

    // Clean up
    close(client_fd);
    svc_ctx.is_running = false;
    b_mcp_ctx.is_running = false;
    a_proxy_ctx.is_running = false;

    // Close pipes to unblock any reading threads
    close(g_mcp_pipe_b_to_a[0]);
    close(g_mcp_pipe_b_to_a[1]);
    close(g_mcp_pipe_a_to_b[0]);
    close(g_mcp_pipe_a_to_b[1]);

    pthread_join(svc_ctx.thread, NULL);
    pthread_join(b_mcp_ctx.thread, NULL);
    pthread_join(a_proxy_ctx.thread, NULL);

    printf("=============================================================\n");
    printf("AUNCIENT MCP-TUNNELED TCP CLIENT PROXY TEST PASSED\n");
    printf("=============================================================\n");
    return 0;
}
