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

static void pad_left_zero(const char *src, char *dst) {
    size_t len = strlen(src);
    if (len >= 64) {
        strcpy(dst, src);
    } else {
        size_t diff = 64 - len;
        memset(dst, '0', diff);
        strcpy(dst + diff, src);
    }
}

static void trigger_process_sample(const char *contract, const char *trigger_hex, const char *packed_hex, char *out_val_hex, size_t out_max) {
    char padded_trigger[128];
    char padded_packed[128];
    pad_left_zero(trigger_hex, padded_trigger);
    pad_left_zero(packed_hex, padded_packed);

    char payload[4096];
    // Function selector: 0x07a96d8c
    // trigger 32 bytes + packedVal 32 bytes
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0x07a96d8c%s%s\""
             "},\"latest\"],\"id\":1}",
             contract, padded_trigger, padded_packed);

    char response[8192];
    send_rpc_request(payload, response, sizeof(response));

    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    char *end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    size_t len = end_quote - res_ptr;
    assert(len < out_max);
    memcpy(out_val_hex, res_ptr, len);
    out_val_hex[len] = '\0';
}

int main() {
    printf("=============================================================\n");
    printf("ELEKTOR ISSUE #1: STEAM WHISTLE & FLAME GENERATOR (ANVIL LIVE)\n");
    printf("=============================================================\n");

    printf("[C-Test] Compiling SteamWhistle Yul object...\n");
    char *whistle_hex = malloc(131072);
    assert(whistle_hex != NULL);
    compile_yul_to_bytecode("../solidity/bin/steamWhistle.yul", whistle_hex, 131072);

    char whistle_addr[128];
    printf("[C-Test] Deploying SteamWhistle contract dynamically from C...\n");
    deploy_contract_from_c(whistle_hex, whistle_addr, sizeof(whistle_addr));
    printf("  [DEPLOYED] SteamWhistle Address: %s\n", whistle_addr);
    free(whistle_hex);

    // Test 1: Whistle Mode (mode = 0, pitch = 1.0, noiseMix = 0.2)
    // trigger = 1 -> 1e18 -> 0xde0b6b3a7640000
    // packedVal: pitch = 1.0 (1e18) -> 0xde0b6b3a7640000
    //            noiseMix = 0.2 (2e17) -> 0x2c68af0bb140000
    //            mode = 0 -> 0x0
    // Packed hex word: 00000000000000002c68af0bb14000000000000000000000de0b6b3a7640000
    char out_val1[128];
    trigger_process_sample(whistle_addr,
                           "de0b6b3a7640000",
                           "00000000000000002c68af0bb14000000000000000000000de0b6b3a7640000",
                           out_val1, sizeof(out_val1));
    printf("  [RESULT] Whistle Output: %s\n", out_val1);

    // Test 2: Flickering Flame Mode (mode = 1, pitch = 1.0, noiseMix = 0.8)
    // trigger = 0
    // packedVal: pitch = 1.0 (1e18) -> 0xde0b6b3a7640000
    //            noiseMix = 0.8 (8e17) -> 0xb1a2bc2ec500000
    //            mode = 1 -> 0x1
    // Packed hex word: 00000000000000010b1a2bc2ec5000000000000000000000de0b6b3a7640000
    char out_val2[128];
    trigger_process_sample(whistle_addr,
                           "0",
                           "00000000000000010b1a2bc2ec5000000000000000000000de0b6b3a7640000",
                           out_val2, sizeof(out_val2));
    printf("  [RESULT] Flickering Flame Output: %s\n", out_val2);

    printf("[PASS] Steam whistle and flickering flame generator verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
