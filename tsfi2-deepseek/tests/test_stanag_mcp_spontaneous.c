#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>

#define SERVER_PORT 10043
#define BUF_SIZE 1024
#define MAX_SPONTANEOUS_CLIENTS 16

typedef struct {
    int client_id;
    char ip_addr[32];
    int port;
} RegisteredClient;

// Thread-safe spontaneous client registry state
static RegisteredClient registry[MAX_SPONTANEOUS_CLIENTS];
static int registered_count = 0;
static pthread_mutex_t registry_mutex = PTHREAD_MUTEX_INITIALIZER;

// Background thread representing the Spontaneous MCP Server
static void* spontaneous_mcp_server(void *arg) {
    (void)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    int bind_ok = bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    if (bind_ok < 0) {
        perror("Bind failed");
        close(server_fd);
        return NULL;
    }

    int listen_ok = listen(server_fd, 10);
    assert(listen_ok >= 0);
    printf("[MCP_SERVER] Spontaneous Registry Server listening on Port %d...\n", SERVER_PORT);
    fflush(stdout);

    // Serve clients loop
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) break;

        char buffer[BUF_SIZE];
        memset(buffer, 0, BUF_SIZE);
        ssize_t bytes_read = read(client_fd, buffer, BUF_SIZE - 1);
        if (bytes_read > 0) {
            // Simple JSON parsing for client registration
            // Example request: {"jsonrpc":"2.0","method":"mcp.register_client","params":{"client_id":123,"port":5001},"id":1}
            int parsed_id = 0;
            int parsed_port = 0;
            if (strstr(buffer, "mcp.register_client") != NULL) {
                char *id_ptr = strstr(buffer, "\"client_id\":");
                char *port_ptr = strstr(buffer, "\"port\":");
                if (id_ptr && port_ptr) {
                    sscanf(id_ptr, "\"client_id\":%d", &parsed_id);
                    sscanf(port_ptr, "\"port\":%d", &parsed_port);
                }

                pthread_mutex_lock(&registry_mutex);
                int assigned_slot = -1;
                if (registered_count < MAX_SPONTANEOUS_CLIENTS) {
                    assigned_slot = registered_count;
                    registry[registered_count].client_id = parsed_id;
                    strcpy(registry[registered_count].ip_addr, inet_ntoa(client_addr.sin_addr));
                    registry[registered_count].port = parsed_port;
                    registered_count++;
                }
                pthread_mutex_unlock(&registry_mutex);

                char response[BUF_SIZE];
                snprintf(response, sizeof(response), 
                         "{\"jsonrpc\":\"2.0\",\"result\":{\"status\":\"registered\",\"assigned_slot\":%d},\"id\":1}\n", 
                         assigned_slot);
                ssize_t bytes_written = write(client_fd, response, strlen(response));
                assert(bytes_written > 0);
            }
        }
        close(client_fd);
        
        // Stop server once target count is reached to allow test termination
        pthread_mutex_lock(&registry_mutex);
        int count = registered_count;
        pthread_mutex_unlock(&registry_mutex);
        if (count >= 5) {
            break;
        }
    }

    close(server_fd);
    return NULL;
}

// Client simulator thread
typedef struct {
    int client_id;
    int port;
} ClientArg;

static void* client_sim_thread(void *arg) {
    ClientArg *c_arg = (ClientArg*)arg;
    
    // Allow a small delay before connecting
    usleep(50000); 

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) return NULL;

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    int connect_ok = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (connect_ok < 0) {
        close(client_fd);
        return NULL;
    }

    char query[BUF_SIZE];
    snprintf(query, sizeof(query), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"mcp.register_client\",\"params\":{\"client_id\":%d,\"port\":%d},\"id\":1}", 
             c_arg->client_id, c_arg->port);
    ssize_t bytes_written = write(client_fd, query, strlen(query));
    assert(bytes_written > 0);

    char response[BUF_SIZE];
    memset(response, 0, BUF_SIZE);
    ssize_t bytes_read = read(client_fd, response, BUF_SIZE - 1);
    if (bytes_read > 0) {
        printf("[MCP_CLIENT_%d] Received Registration Response: %s", c_arg->client_id, response);
        fflush(stdout);
        assert(strstr(response, "\"status\":\"registered\"") != NULL);
    }

    close(client_fd);
    free(c_arg);
    return NULL;
}

int main(void) {
    printf("=== TSFi Spontaneous Multi-Client MCP Registry Test ===\n");

    // 1. Start spontaneous server
    pthread_t server_thread;
    int thread_ok = pthread_create(&server_thread, NULL, spontaneous_mcp_server, NULL);
    assert(thread_ok == 0);

    usleep(100000); // Allow server to bind

    // 2. Spawn 5 dynamic spontaneous clients concurrently
    pthread_t clients[5];
    for (int i = 0; i < 5; i++) {
        ClientArg *arg = malloc(sizeof(ClientArg));
        arg->client_id = 100 + i;
        arg->port = 6000 + i;
        pthread_create(&clients[i], NULL, client_sim_thread, arg);
    }

    // Join clients
    for (int i = 0; i < 5; i++) {
        pthread_join(clients[i], NULL);
    }

    // Join server
    pthread_join(server_thread, NULL);

    // Verify registration lists
    pthread_mutex_lock(&registry_mutex);
    printf("\n[REGISTRY] Total Registered Clients: %d\n", registered_count);
    for (int i = 0; i < registered_count; i++) {
        printf("  Slot %d -> ClientID: %d, Address: %s:%d\n", 
               i, registry[i].client_id, registry[i].ip_addr, registry[i].port);
    }
    assert(registered_count == 5);
    pthread_mutex_unlock(&registry_mutex);

    printf("\n=== SPONTANEOUS REGISTRY TESTS PASSED ===\n");
    return 0;
}
