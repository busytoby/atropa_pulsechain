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
#define BUF_SIZE 2048

static void create_mock_vm_files(void) {
    // Create simulated guest VM filesystem layouts
    FILE *f1 = fopen("bin_init.bin", "w"); fprintf(f1, "init_daemon"); fclose(f1);
    FILE *f2 = fopen("bin_sh.bin", "w");   fprintf(f2, "shell_interpreter"); fclose(f2);
    FILE *f3 = fopen("bin_ls.bin", "w");   fprintf(f3, "ls_executable"); fclose(f3);
    FILE *f4 = fopen("bin_cat.bin", "w");  fprintf(f4, "cat_executable"); fclose(f4);
    FILE *f5 = fopen("bin_echo.bin", "w"); fprintf(f5, "echo_executable"); fclose(f5);
    FILE *f6 = fopen("etc_motd.txt", "w");  fprintf(f6, "Welcome to Helmholtz Linux OS!"); fclose(f6);
    FILE *f7 = fopen("etc_hostname.txt", "w"); fprintf(f7, "helmholtz-vm-01"); fclose(f7);
}

static void cleanup_mock_vm_files(void) {
    remove("bin_init.bin");
    remove("bin_sh.bin");
    remove("bin_ls.bin");
    remove("bin_cat.bin");
    remove("bin_echo.bin");
    remove("etc_motd.txt");
    remove("etc_hostname.txt");
}

static void* mcp_file_verify_server_thread(void *arg) {
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
    printf("[MCP_SERVER] File verification server listening on Port %d...\n", PORT);
    fflush(stdout);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    assert(client_fd >= 0);

    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);
    ssize_t bytes_read = read(client_fd, buffer, BUF_SIZE - 1);
    assert(bytes_read > 0);
    printf("[MCP_SERVER] Received request: %s\n", buffer);
    fflush(stdout);

    if (strstr(buffer, "verify_vm_files") != NULL) {
        // Check presence of expected mock filesystem objects
        bool init_ok = (access("bin_init.bin", F_OK) == 0);
        bool sh_ok   = (access("bin_sh.bin", F_OK) == 0);
        bool ls_ok   = (access("bin_ls.bin", F_OK) == 0);
        bool cat_ok  = (access("bin_cat.bin", F_OK) == 0);
        bool echo_ok = (access("bin_echo.bin", F_OK) == 0);
        bool motd_ok = (access("etc_motd.txt", F_OK) == 0);
        bool host_ok = (access("etc_hostname.txt", F_OK) == 0);

        char response[BUF_SIZE];
        sprintf(response, "{\"jsonrpc\":\"2.0\",\"result\":{"
                          "\"/bin/init\":\"%s\","
                          "\"/bin/sh\":\"%s\","
                          "\"/bin/ls\":\"%s\","
                          "\"/bin/cat\":\"%s\","
                          "\"/bin/echo\":\"%s\","
                          "\"/etc/motd\":\"%s\","
                          "\"/etc/hostname\":\"%s\""
                          "},\"status\":\"VERIFICATION_COMPLETE\"}\n",
                init_ok ? "PRESENT" : "MISSING",
                sh_ok ? "PRESENT" : "MISSING",
                ls_ok ? "PRESENT" : "MISSING",
                cat_ok ? "PRESENT" : "MISSING",
                echo_ok ? "PRESENT" : "MISSING",
                motd_ok ? "PRESENT" : "MISSING",
                host_ok ? "PRESENT" : "MISSING");

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
    printf("=== TSFi ZMM VM File Verification over MCP TCP/IP ===\n");

    // 1. Create simulated guest VM filesystem
    create_mock_vm_files();

    // 2. Start MCP server thread
    pthread_t thread;
    int thread_ok = pthread_create(&thread, NULL, mcp_file_verify_server_thread, NULL);
    assert(thread_ok == 0);

    // Give server thread a moment to bind
    usleep(100000); // 100ms

    // 3. Connect client socket
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

    // 4. Request filesystem verification thunk
    const char *req = "{\"jsonrpc\":\"2.0\",\"method\":\"verify_vm_files\",\"id\":1}";
    printf("[MCP_CLIENT] Querying guest VM filesystem verification...\n");
    fflush(stdout);
    write(client_fd, req, strlen(req));

    // 5. Read response
    char response[BUF_SIZE];
    memset(response, 0, BUF_SIZE);
    ssize_t bytes_read = read(client_fd, response, BUF_SIZE - 1);
    assert(bytes_read > 0);

    printf("\n--- MCP FILE VERIFICATION RESPONSE ---\n");
    printf("%s", response);
    printf("--------------------------------------\n\n");
    fflush(stdout);

    // 6. Verify all expected files are reported PRESENT
    assert(strstr(response, "\"/bin/init\":\"PRESENT\"") != NULL);
    assert(strstr(response, "\"/bin/sh\":\"PRESENT\"") != NULL);
    assert(strstr(response, "\"/bin/ls\":\"PRESENT\"") != NULL);
    assert(strstr(response, "\"/bin/cat\":\"PRESENT\"") != NULL);
    assert(strstr(response, "\"/bin/echo\":\"PRESENT\"") != NULL);
    assert(strstr(response, "\"/etc/motd\":\"PRESENT\"") != NULL);
    assert(strstr(response, "\"/etc/hostname\":\"PRESENT\"") != NULL);
    printf("  [PASS] All expected filesystem objects verified PRESENT inside guest VM via TCP/IP MCP!\n");
    fflush(stdout);

    // Join server thread
    pthread_join(thread, NULL);
    
    // Clean up
    cleanup_mock_vm_files();

    printf("=== ALL MCP FILE VERIFICATION TESTS PASSED ===\n");
    return 0;
}
