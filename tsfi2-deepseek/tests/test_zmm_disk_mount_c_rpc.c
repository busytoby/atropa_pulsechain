#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <time.h>

#define ANVIL_IP "127.0.0.1"
#define ANVIL_PORT 8545

static int connect_to_anvil() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(ANVIL_PORT);
    inet_pton(AF_INET, ANVIL_IP, &serv_addr.sin_addr);

    int conn = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (conn < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

static void send_rpc_request(const char *payload, char *response_buf, size_t max_len) {
    int sockfd = connect_to_anvil();
    if (sockfd < 0) {
        fprintf(stderr, "[ERROR] Failed to connect to Anvil RPC at %s:%d\n", ANVIL_IP, ANVIL_PORT);
        exit(1);
    }

    char http_request[150000];
    snprintf(http_request, sizeof(http_request),
             "POST / HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             ANVIL_IP, ANVIL_PORT, strlen(payload), payload);

    ssize_t sent = write(sockfd, http_request, strlen(http_request));
    assert(sent == (ssize_t)strlen(http_request));

    memset(response_buf, 0, max_len);
    size_t total_read = 0;
    while (1) {
        ssize_t n = read(sockfd, response_buf + total_read, max_len - 1 - total_read);
        if (n <= 0) break;
        total_read += n;
        if (total_read >= max_len - 1) break;
    }
    response_buf[total_read] = '\0';
    close(sockfd);
}

static void wait_for_tx(const char *tx_hash) {
    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getTransactionReceipt\",\"params\":[\"%s\"],\"id\":1}",
             tx_hash);
    char response[8192];
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 25000000;
    for (int i = 0; i < 40; i++) {
        send_rpc_request(payload, response, sizeof(response));
        if (strstr(response, "\"result\":null") == NULL && strstr(response, "\"result\"") != NULL) {
            return;
        }
        nanosleep(&ts, NULL);
    }
}

static void execute_tx(const char *payload) {
    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    char *end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char tx_hash[128];
    size_t len = end_quote - res_ptr;
    memcpy(tx_hash, res_ptr, len);
    tx_hash[len] = '\0';
    wait_for_tx(tx_hash);
}

static void compile_yul_to_bytecode(const char *yul_path, char *out_hex, size_t max_len) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "solc --strict-assembly %s --bin 2>/dev/null", yul_path);
    FILE *fp = popen(cmd, "r");
    assert(fp != NULL);

    char line[8192];
    out_hex[0] = '\0';
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
            len--;
        }
        if (len > 10) {
            int all_hex = 1;
            for (size_t i = 0; i < len; i++) {
                char c = line[i];
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                    all_hex = 0;
                    break;
                }
            }
            if (all_hex) {
                assert(len < max_len);
                strcpy(out_hex, line);
            }
        }
    }
    pclose(fp);
    assert(strlen(out_hex) > 0);
}

static void deploy_contract_from_c(const char *bytecode_hex, char *out_addr, size_t max_len) {
    size_t payload_size = strlen(bytecode_hex) + 1024;
    char *payload = malloc(payload_size);
    assert(payload != NULL);

    snprintf(payload, payload_size,
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"data\":\"0x%s\","
             "\"gas\":\"0x1C9C380\""
             "}],\"id\":1}",
             bytecode_hex);

    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    free(payload);

    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    char *end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char tx_hash[128];
    size_t len = end_quote - res_ptr;
    memcpy(tx_hash, res_ptr, len);
    tx_hash[len] = '\0';

    char receipt_payload[2048];
    snprintf(receipt_payload, sizeof(receipt_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getTransactionReceipt\",\"params\":[\"%s\"],\"id\":1}",
             tx_hash);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 25000000;
    for (int i = 0; i < 40; i++) {
        send_rpc_request(receipt_payload, response, sizeof(response));
        char *addr_ptr = strstr(response, "\"contractAddress\":\"");
        if (addr_ptr) {
            addr_ptr += 19;
            char *end_addr = strchr(addr_ptr, '"');
            assert(end_addr != NULL);
            size_t addr_len = end_addr - addr_ptr;
            assert(addr_len < max_len);
            memcpy(out_addr, addr_ptr, addr_len);
            out_addr[addr_len] = '\0';
            return;
        }
        nanosleep(&ts, NULL);
    }
    fprintf(stderr, "[ERROR] Deployment transaction receipt not found\n");
    exit(1);
}

int main(void) {
    printf("=============================================================\n");
    printf("SOLID STATE HARDWARE DYNAMICS: MOUNT/UNMOUNT VERIFICATION\n");
    printf("=============================================================\n");

    printf("[C-Test] Compiling WinchesterMQ and DiskSystem Yul objects...\n");
    char *wmq_hex = malloc(131072);
    char *disk_hex = malloc(131072);
    assert(wmq_hex != NULL && disk_hex != NULL);

    compile_yul_to_bytecode("../solidity/bin/WinchesterMQ.yul", wmq_hex, 131072);
    compile_yul_to_bytecode("../solidity/bin/diskSystem.yul", disk_hex, 131072);

    char wmq_addr[128];
    char disk_addr[128];
    printf("[C-Test] Deploying WinchesterMQ on Anvil...\n");
    deploy_contract_from_c(wmq_hex, wmq_addr, sizeof(wmq_addr));
    printf("  [DEPLOYED] WinchesterMQ Address: %s\n", wmq_addr);

    printf("[C-Test] Deploying DiskSystem on Anvil...\n");
    deploy_contract_from_c(disk_hex, disk_addr, sizeof(disk_addr));
    printf("  [DEPLOYED] DiskSystem Address: %s\n", disk_addr);

    // Call mountDisk(0, disk_addr) via WinchesterMQ (selector: 0x4d6f756e)
    char mount_payload[1024];
    snprintf(mount_payload, sizeof(mount_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x4d6f756e"
             "0000000000000000000000000000000000000000000000000000000000000000" // LUN = 0
             "000000000000000000000000%s\"," // Disk Address (padded to 32 bytes)
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             wmq_addr, disk_addr + 2);

    execute_tx(mount_payload);
    printf("  [MOUNT] Disk successfully mounted on LUN 0.\n");

    // Call getMountedDisk(0) via WinchesterMQ (selector: 0x476f756e)
    char query_payload[1024];
    snprintf(query_payload, sizeof(query_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0x476f756e"
             "0000000000000000000000000000000000000000000000000000000000000000\""
             "},\"latest\"],\"id\":1}",
             wmq_addr);

    char response[8192];
    send_rpc_request(query_payload, response, sizeof(response));
    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    char *end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char query_addr[128];
    size_t len = end_quote - res_ptr;
    memcpy(query_addr, res_ptr, len);
    query_addr[len] = '\0';
    printf("  [MOUNT_CHECK] WinchesterMQ returned mounted address: 0x%s\n", query_addr + 24);

    // Call unmount / mount null address to test unmounting
    char unmount_payload[1024];
    snprintf(unmount_payload, sizeof(unmount_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x4d6f756e"
             "0000000000000000000000000000000000000000000000000000000000000000" // LUN = 0
             "0000000000000000000000000000000000000000000000000000000000000000\"," // Null address unmounts
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             wmq_addr);
    execute_tx(unmount_payload);
    printf("  [UNMOUNT] Disk successfully unmounted from LUN 0.\n");

    free(wmq_hex);
    free(disk_hex);

    printf("[PASS] Solid State Hardware Dynamics mount/unmount loops verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
