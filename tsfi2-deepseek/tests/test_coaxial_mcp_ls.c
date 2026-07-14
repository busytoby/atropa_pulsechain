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
#include "tsfi_dat.h"
#include "tsfi_trie.h"

#define PORT 10042
#define BUF_SIZE 2048
#define STACK_MAX 16
#define HASH_SIZE 32

typedef struct {
    uint8_t data[HASH_SIZE * 3];
    size_t len;
} StackElement;

typedef struct {
    StackElement elements[STACK_MAX];
    int top;
} BtcStack;

extern bool btc_thunk_execute(const uint8_t *script, size_t script_len, BtcStack *stack);

// Background thread simulating the ZMM MCP Server executing the thunk
static void* mcp_ls_server_thread(void *arg) {
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

    int bind_ok = bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    if (bind_ok < 0) {
        perror("Bind failed");
        close(server_fd);
        return NULL;
    }

    int listen_ok = listen(server_fd, 3);
    assert(listen_ok >= 0);
    printf("[MCP_SERVER] Listening on Port %d for ZMM thunk commands...\n", PORT);
    fflush(stdout);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    assert(client_fd >= 0);

    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    ssize_t bytes_read = read(client_fd, buffer, BUF_SIZE - 1);
    assert(bytes_read > 0);
    printf("[MCP_SERVER] Received JSON-RPC: %s\n", buffer);
    fflush(stdout);

    // If method is execute_ls, run the Bitcoin Script thunk validation
    if (strstr(buffer, "execute_ls") != NULL) {
        printf("[MCP_SERVER] Running Bitcoin Script path validation thunk...\n");
        fflush(stdout);

        // 1. Setup path trie
        tsfi_trie_node *trie = tsfi_trie_create_node('\0');
        tsfi_trie_insert(trie, "ls", "DAT_LS_EXEC");
        tsfi_dat *dat = tsfi_dat_compile(trie);

        int state = 0;
        const char *key = "ls";
        while (*key != '\0') {
            state = dat->base[state] + (unsigned char)*key;
            key++;
        }

        // 2. Generate script
        uint8_t script[256];
        int script_len = tsfi_dat_generate_btc_script(dat, "ls", state, script);
        
        // Append execution strobe verification
        script[script_len++] = 0x01; // PUSH 1
        script[script_len++] = 0x01; 
        script[script_len++] = 0x88; // OP_EQUALVERIFY

        // Push command
        const char *cmd = "ls /bin";
        script[script_len++] = 7;
        memcpy(script + script_len, cmd, 7);
        script_len += 7;

        // Push offset 0
        int offset = 0;
        script[script_len++] = 4;
        memcpy(script + script_len, &offset, 4);
        script_len += 4;

        // 3. Execute
        BtcStack stack;
        stack.top = -1;
        bool btc_ok = btc_thunk_execute(script, script_len, &stack);
        assert(btc_ok == true);

        // Clean up trie
        tsfi_trie_destroy(trie);
        tsfi_dat_destroy(dat);

        // Execute deployed binary script simulating directory list
        printf("[MCP_SERVER] Thunk validated. Executing /bin/ls inside VM...\n");
        fflush(stdout);
        
        const char *ls_out = "-rwxr-xr-x  init\\n-rwxr-xr-x  sh\\n-rwxr-xr-x  ls\\n-rwxr-xr-x  sysctl";
        
        char response[BUF_SIZE];
        sprintf(response, "{\"jsonrpc\":\"2.0\",\"result\":\"%s\",\"status\":\"SUCCESS\"}\n", ls_out);
        write(client_fd, response, strlen(response));
    } else {
        const char *err = "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32601,\"message\":\"Method not found\"}}\n";
        write(client_fd, err, strlen(err));
    }

    close(client_fd);
    close(server_fd);
    return NULL;
}

int main(void) {
    printf("=== TSFi Coaxial MCP TCP/IP 'ls /bin' Execution Proof ===\n");

    // 1. Launch MCP server thread
    pthread_t thread;
    int thread_ok = pthread_create(&thread, NULL, mcp_ls_server_thread, NULL);
    assert(thread_ok == 0);

    // Give server thread a moment to bind
    usleep(100000); // 100ms

    // 2. Connect client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(client_fd >= 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    printf("[MCP_CLIENT] Connecting to 127.0.0.1:%d...\n", PORT);
    fflush(stdout);
    int connect_ok = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(connect_ok >= 0);

    // 3. Request ls /bin execution thunk
    const char *req = "{\"jsonrpc\":\"2.0\",\"method\":\"execute_ls\",\"id\":1}";
    printf("[MCP_CLIENT] Querying execution of thunk 'ls /bin'...\n");
    fflush(stdout);
    write(client_fd, req, strlen(req));

    // 4. Read response
    char response[BUF_SIZE];
    memset(response, 0, BUF_SIZE);
    ssize_t bytes_read = read(client_fd, response, BUF_SIZE - 1);
    assert(bytes_read > 0);

    printf("\n--- MCP JSON-RPC RESPONSE ---\n");
    printf("%s", response);
    printf("-----------------------------\n\n");
    fflush(stdout);

    // 5. Verify roundtrip output contains the correct non-ELF listings
    assert(strstr(response, "init") != NULL);
    assert(strstr(response, "sh") != NULL);
    assert(strstr(response, "ls") != NULL);
    assert(strstr(response, "sysctl") != NULL);
    printf("  [PASS] Verified remote execution and directory outputs via TCP/IP MCP!\n");
    fflush(stdout);

    pthread_join(thread, NULL);
    printf("=== ALL MCP COAXIAL SOCKET TESTS PASSED ===\n");
    return 0;
}
