#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>

int main(void) {
    printf("[INFO] Starting Google Flow Network Ingest Unit Test...\n");

    // 1. Launch MCP Server in background
    pid_t pid = fork();
    if (pid == 0) {
        // Child: Run server
        execl("./bin/tsfi_mcp_server", "tsfi_mcp_server", NULL);
        perror("execl failed");
        exit(1);
    }
    
    // Give server time to bind
    sleep(1);

    // 2. Connect to Server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    int port = 10042;
    const char *p_env = getenv("TSFI_MCP_PORT");
    if (p_env) port = atoi(p_env);
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connect failed");
        kill(pid, SIGTERM);
        return 1;
    }

    // 3. Send manifold.upload_asset (Blue Teddy Bear data)
    const char *data = "00000000000000000000000000000000"; 
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.upload_asset\", \"params\": {\"data\": \"%s\"}, \"id\": 999}", data);
    
    printf("[FLOW] Uploading Bear Asset over Network...\n");
    send(sock, cmd, strlen(cmd), 0);

    // 4. Receive Response
    char buffer[4096];
    int n = recv(sock, buffer, sizeof(buffer)-1, 0);
    if (n > 0) {
        buffer[n] = 0;
        printf("[RPC] %s\n", buffer);
        if (strstr(buffer, "address")) {
            printf("[PASS] Network Ingest Verified.\n");
        } else {
            printf("[FAIL] Unexpected response: %s\n", buffer);
        }
    } else {
        printf("[FAIL] No response from server.\n");
    }

    // Cleanup
    close(sock);
    kill(pid, SIGKILL);
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
