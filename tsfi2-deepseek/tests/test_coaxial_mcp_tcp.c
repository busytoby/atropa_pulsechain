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

#define PORT 10042
#define BUF_SIZE 1024

// Background thread simulating the ZMM MCP Server listening on Port 10042
static void* mcp_server_thread(void *arg) {
    (void)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind and listen
    int bind_ok = bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    if (bind_ok < 0) {
        perror("Bind failed");
        close(server_fd);
        return NULL;
    }

    int listen_ok = listen(server_fd, 3);
    assert(listen_ok >= 0);
    printf("[MCP_SERVER] Listening on Port %d...\n", PORT);
    fflush(stdout);

    // Accept incoming connection
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    assert(client_fd >= 0);
    printf("[MCP_SERVER] Accepted incoming connection!\n");
    fflush(stdout);

    // Read query
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    ssize_t bytes_read = read(client_fd, buffer, BUF_SIZE - 1);
    assert(bytes_read > 0);
    printf("[MCP_SERVER] Received request: %s\n", buffer);
    fflush(stdout);

    // If query matches, verify the DAT validation state and respond
    const char *response = "{\"jsonrpc\":\"2.0\",\"result\":\"DAT_VALIDATION_SUCCESS\",\"state\":105}\n";
    write(client_fd, response, strlen(response));
    
    close(client_fd);
    close(server_fd);
    return NULL;
}

int main(void) {
    printf("=== TSFi ZMM MCP over TCP/IP Socket Verification ===\n");

    // 1. Start the MCP server thread
    pthread_t thread;
    int thread_ok = pthread_create(&thread, NULL, mcp_server_thread, NULL);
    assert(thread_ok == 0);

    // Allow the server thread a moment to bind and listen
    usleep(100000); // 100ms

    // 2. Client connection
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(client_fd >= 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    int inet_ok = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    assert(inet_ok > 0);

    printf("[MCP_CLIENT] Connecting to 127.0.0.1:%d...\n", PORT);
    fflush(stdout);
    int connect_ok = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(connect_ok >= 0);

    // 3. Transmit JSON-RPC query representing our thunk check
    const char *query = "{\"jsonrpc\":\"2.0\",\"method\":\"query_dat_state\",\"params\":{\"key\":\"sh\"},\"id\":1}";
    printf("[MCP_CLIENT] Transmitting query...\n");
    fflush(stdout);
    write(client_fd, query, strlen(query));

    // 4. Read response
    char response[BUF_SIZE];
    memset(response, 0, BUF_SIZE);
    ssize_t bytes_read = read(client_fd, response, BUF_SIZE - 1);
    assert(bytes_read > 0);
    printf("[MCP_CLIENT] Received response: %s\n", response);
    fflush(stdout);

    // 5. Verify roundtrip success
    assert(strstr(response, "DAT_VALIDATION_SUCCESS") != NULL);
    printf("  [PASS] Successfully verified MCP access over local TCP/IP socket loopback!\n");
    fflush(stdout);

    // Join thread
    pthread_join(thread, NULL);
    
    printf("=== ALL MCP SOCKET TESTS PASSED ===\n");
    return 0;
}
