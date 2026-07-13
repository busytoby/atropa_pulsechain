#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <pthread.h>

#define PROXY_PORT 8999
#define SERVER_PORT 9000
#define BUFFER_SIZE 512

typedef struct {
    int client_fd;
    int dest_fd;
} ProxyBridge;

// Server thread logic
void *server_thread_func(void *arg) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Server bind failed");
        return NULL;
    }
    listen(server_fd, 3);

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);

    char buffer[BUFFER_SIZE] = {0};
    int valread = read(client_socket, buffer, BUFFER_SIZE);
    if (valread > 0) {
        printf("   [Server] Received payload: '%s'\n", buffer);
        // Respond with acknowledgment
        const char *ack = "Auncient-ACK";
        send(client_socket, ack, strlen(ack), 0);
    }

    close(client_socket);
    close(server_fd);
    return NULL;
}

// Proxy forwarding bridge worker
void *bridge_worker(void *arg) {
    ProxyBridge *bridge = (ProxyBridge *)arg;
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes = recv(bridge->client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        printf("   [Proxy Bridge] Intercepted %d bytes: '%s'\n", bytes, buffer);
        
        // Forward the packet to target destination
        send(bridge->dest_fd, buffer, bytes, 0);
    }
    close(bridge->client_fd);
    close(bridge->dest_fd);
    free(bridge);
    return NULL;
}

// Proxy server thread logic
void *proxy_thread_func(void *arg) {
    int proxy_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PROXY_PORT);

    if (bind(proxy_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Proxy bind failed");
        return NULL;
    }
    listen(proxy_fd, 3);

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_socket = accept(proxy_fd, (struct sockaddr *)&client_addr, &addrlen);

    // Connect proxy to destination server
    int dest_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);

    if (connect(dest_fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Proxy failed to connect to destination server");
        close(client_socket);
        close(proxy_fd);
        return NULL;
    }

    // Spawn bidirectional bridges
    pthread_t thread1, thread2;
    ProxyBridge *b1 = malloc(sizeof(ProxyBridge));
    b1->client_fd = client_socket;
    b1->dest_fd = dest_fd;
    pthread_create(&thread1, NULL, bridge_worker, b1);

    ProxyBridge *b2 = malloc(sizeof(ProxyBridge));
    b2->client_fd = dup(dest_fd);
    b2->dest_fd = dup(client_socket);
    pthread_create(&thread2, NULL, bridge_worker, b2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    close(proxy_fd);
    return NULL;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT MCP PEER TCP/IP CONNECTIVITY & PROXY TESTS\n");
    printf("=============================================================\n");

    pthread_t server_tid, proxy_tid;
    
    // 1. Spawn Server and Proxy instances
    printf("1. Launching local target server and proxy...\n");
    pthread_create(&server_tid, NULL, server_thread_func, NULL);
    usleep(100000); // Wait for server to bind
    pthread_create(&proxy_tid, NULL, proxy_thread_func, NULL);
    usleep(100000); // Wait for proxy to bind

    // 2. Client connects to the Proxy port instead of the Server port directly
    printf("2. Client connecting to proxy on port %d...\n", PROXY_PORT);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(PROXY_PORT);
    inet_pton(AF_INET, "127.0.0.1", &proxy_addr.sin_addr);

    int connection_status = connect(client_fd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr));
    assert(connection_status >= 0);
    printf("   ✓ Client connected successfully.\n\n");

    // 3. Client transmits telemetry payload via Proxy
    printf("3. Transmitting Auncient payload...\n");
    const char *payload = "Auncient-Peer-Wavelet-1.0";
    send(client_fd, payload, strlen(payload), 0);

    // 4. Client reads acknowledgment routed back through Proxy
    char response[BUFFER_SIZE] = {0};
    int bytes_received = recv(client_fd, response, sizeof(response) - 1, 0);
    assert(bytes_received > 0);
    printf("   [Client] Received response: '%s'\n", response);
    assert(strcmp(response, "Auncient-ACK") == 0);
    printf("   ✓ Bidirectional Proxy routing verified.\n\n");

    close(client_fd);
    pthread_join(proxy_tid, NULL);
    pthread_join(server_tid, NULL);

    printf("=============================================================\n");
    printf("TCP/IP MCP PROXY TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
