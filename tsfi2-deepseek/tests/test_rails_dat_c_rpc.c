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
             "\"gas\":\"0xF4240\""
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

static void set_storage(const char *contract, const char *slot, const char *value) {
    char payload[1024];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"anvil_setStorageAt\",\"params\":[\"%s\",\"%s\",\"%s\"],\"id\":1}",
             contract, slot, value);

    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    assert(strstr(response, "\"result\":true") != NULL || strstr(response, "result") != NULL);
}

static void get_storage(const char *contract, const char *slot, char *out_val_hex, size_t out_max) {
    char payload[1024];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getStorageAt\",\"params\":[\"%s\",\"%s\",\"latest\"],\"id\":1}",
             contract, slot);

    char response[8192];
    send_rpc_request(payload, response, sizeof(response));

    char *res_ptr = strstr(response, "\"result\":\"");
    if (res_ptr) {
        res_ptr += 10;
        char *end_quote = strchr(res_ptr, '"');
        if (end_quote) {
            size_t len = end_quote - res_ptr;
            assert(len < out_max);
            memcpy(out_val_hex, res_ptr, len);
            out_val_hex[len] = '\0';
            return;
        }
    }
    fprintf(stderr, "[ERROR] Failed to parse eth_getStorageAt response: %s\n", response);
    exit(1);
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

    for (int i = 0; i < 20; i++) {
        send_rpc_request(payload, response, sizeof(response));
        if (strstr(response, "\"result\":null") == NULL && strstr(response, "\"result\"") != NULL) {
            return;
        }
        nanosleep(&ts, NULL);
    }
    fprintf(stderr, "[ERROR] Transaction %s was not mined within timeout\n", tx_hash);
    exit(1);
}

static void trigger_execution(const char *contract) {
    char payload[1024];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x\","
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             contract);

    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    
    char *res_ptr = strstr(response, "\"result\":\"");
    if (res_ptr) {
        res_ptr += 10;
        char *end_quote = strchr(res_ptr, '"');
        if (end_quote) {
            char tx_hash[128];
            size_t len = end_quote - res_ptr;
            assert(len < sizeof(tx_hash));
            memcpy(tx_hash, res_ptr, len);
            tx_hash[len] = '\0';
            wait_for_tx(tx_hash);
            return;
        }
    }
    fprintf(stderr, "[ERROR] eth_sendTransaction failed: %s\n", response);
    exit(1);
}

static void test_ncr304_merge_sorter(const char *ncr_addr) {
    printf("[C-Test] Initializing NCR 304 Merge Sorter registers...\n");
    set_storage(ncr_addr, "0x2a", "0x0000000000000000000000000000000000000000000000000000000000000001");

    // Unsorted array: 15, 5, 10
    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0x000000000000000000000000000000000000000000000000000000000000000f0000000000000000000000000000000000000000000000000000000000000005000000000000000000000000000000000000000000000000000000000000000a\""
             "},\"latest\"],\"id\":1}",
             ncr_addr);

    char response[8192];
    send_rpc_request(payload, response, sizeof(response));

    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    
    printf("  [RESULT] Merge Sorter Sorted output: %.200s\n", res_ptr);
    assert(strstr(res_ptr, "0000000000000000000000000000000000000000000000000000000000000005000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000f") != NULL);
}

int main() {
    printf("=============================================================\n");
    printf("TSFi AUNCIENT: C-LEVEL RPC YUL HARDWARE VERIFIER (DYNAMIC DEPLOY)\n");
    printf("=============================================================\n");

    // Dynamic Compilation of Yul objects to bytecode
    printf("[C-Test] Compiling G-15D Rail Yul object...\n");
    char *g15d_hex = malloc(131072);
    assert(g15d_hex != NULL);
    compile_yul_to_bytecode("src/firmware/g15d_rail.yul", g15d_hex, 131072);

    printf("[C-Test] Compiling NCR 304 Rail Yul object...\n");
    char *ncr_hex = malloc(131072);
    assert(ncr_hex != NULL);
    compile_yul_to_bytecode("src/firmware/ncr304_rail.yul", ncr_hex, 131072);

    // Dynamic Deployment of contracts to Anvil
    char g15d_addr[128];
    char ncr_addr[128];
    printf("[C-Test] Deploying G-15D contract dynamically from C...\n");
    deploy_contract_from_c(g15d_hex, g15d_addr, sizeof(g15d_addr));
    printf("  [DEPLOYED] G-15D Contract Address: %s\n", g15d_addr);

    printf("[C-Test] Deploying NCR 304 contract dynamically from C...\n");
    deploy_contract_from_c(ncr_hex, ncr_addr, sizeof(ncr_addr));
    printf("  [DEPLOYED] NCR 304 Contract Address: %s\n", ncr_addr);

    free(g15d_hex);
    free(ncr_hex);

    // Test 1: G-15D DA-1 DDA Scaling check
    printf("[C-Test] Initializing G-15D registers...\n");
    set_storage(g15d_addr, "0x01", "0x0000000000000000000000000000000000000000000000000000000000000000");
    set_storage(g15d_addr, "0x10", "0x0000000000000000000000000000000000000000000000000000000000000000");
    set_storage(g15d_addr, "0x50", "0x000000000000000000000000000000000000000000000000000000000000000a"); // Base = 10
    set_storage(g15d_addr, "0x15", "0x0000000000000000000000000000000000000000000000000000000000000005"); // DDA Scale = 5

    printf("[C-Test] Triggering G-15D cycle transitions...\n");
    trigger_execution(g15d_addr);

    char drum_sector[128] = {0};
    char base_reg[128] = {0};
    get_storage(g15d_addr, "0x10", drum_sector, sizeof(drum_sector));
    get_storage(g15d_addr, "0x50", base_reg, sizeof(base_reg));

    printf("  [RESULT] G-15D Drum Sector: %s\n", drum_sector);
    printf("  [RESULT] G-15D DDA Scaled Base Register: %s\n", base_reg);

    // Verify 10 + 5 = 15 (0x0f)
    assert(strstr(base_reg, "f") != NULL || strstr(base_reg, "F") != NULL);

    // Test 2: NCR 304 Card-to-Tape Transcoder check
    printf("[C-Test] Initializing NCR 304 registers...\n");
    set_storage(ncr_addr, "0x28", "0x0000000000000000000000000000000000000000000000000000000000000003");
    set_storage(ncr_addr, "0x21", "0x0000000000000000000000000000000000000000000000000000000000000000");
    set_storage(ncr_addr, "0x2a", "0x0000000000000000000000000000000000000000000000000000000000000001"); // Merge sorter active
    set_storage(ncr_addr, "0x2b", "0x0000000000000000000000000000000000000000000000000000000000000000"); // Transcoder status = 0

    printf("[C-Test] Triggering NCR 304 cycle transitions...\n");
    trigger_execution(ncr_addr);

    char transcoder_status[128] = {0};
    get_storage(ncr_addr, "0x2b", transcoder_status, sizeof(transcoder_status));
    printf("  [RESULT] NCR 304 Transcoding Status: %s\n", transcoder_status);

    // Verify status progressed to 1
    assert(strstr(transcoder_status, "1") != NULL);

    // Test 3: NCR 304 Real Tape Merge Insertion Sort
    test_ncr304_merge_sorter(ncr_addr);

    printf("[PASS] All C-level RPC Yul hardware invariants verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
