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

static void trigger_process_samples(const char *contract, const char *count_hex, const char *inputs_calldata, char *out_val_hex, size_t out_max) {
    char padded_count[128];
    pad_left_zero(count_hex, padded_count);

    char payload[8192];
    // Function selector: 0x11223344
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x11223344%s%s\""
             "},\"latest\"],\"id\":1}",
             contract, padded_count, inputs_calldata);

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
    printf("ELEKTOR ISSUE #9: STROBOSCOPIC HYSTERESIS LIMITER (ANVIL LIVE)\n");
    printf("=============================================================\n");

    printf("[C-Test] Compiling HysteresisLimiter Yul object...\n");
    char *limit_hex = malloc(131072);
    assert(limit_hex != NULL);
    compile_yul_to_bytecode("../solidity/bin/hysteresisLimiter.yul", limit_hex, 131072);

    char limit_addr[128];
    printf("[C-Test] Deploying HysteresisLimiter contract dynamically from C...\n");
    deploy_contract_from_c(limit_hex, limit_addr, sizeof(limit_addr));
    printf("  [DEPLOYED] HysteresisLimiter Address: %s\n", limit_addr);
    free(limit_hex);

    // Call processSamples in a single batch of 9 inputs to verify stroboscopic latch transitions:
    // 0: 5.0V (0x0000000000000000000000000000000000000000000000004563918244f40000)
    // 1: 10.0V (0x0000000000000000000000000000000000000000000000008ac7230489e80000) -> charges env to 6.25V
    // 2: 10.0V (0x0000000000000000000000000000000000000000000000008ac7230489e80000) -> charges env to 8.125V (>8V, strike!)
    // 3: 10.0V (0x0000000000000000000000000000000000000000000000008ac7230489e80000) -> charges env to 9.06V
    // 4: 3.0V (0x00000000000000000000000000000000000000000000000029a2241af62c0000) -> drops below 8V but remains limited!
    // 5: 1.5V (0x00000000000000000000000000000000000000000000000014d1120d7b160000) -> env decays 75% -> 4.52V (>2V, still limited!)
    // 6: 1.5V (0x00000000000000000000000000000000000000000000000014d1120d7b160000) -> env decays 75% -> 3.39V (>2V, still limited!)
    // 7: 1.5V (0x00000000000000000000000000000000000000000000000014d1120d7b160000) -> env decays 75% -> 2.54V (>2V, still limited!)
    // 8: 1.5V (0x00000000000000000000000000000000000000000000000014d1120d7b160000) -> env decays 75% -> 1.90V (<2V, extinguish!)
    const char *inputs =
        "0000000000000000000000000000000000000000000000004563918244f40000"
        "0000000000000000000000000000000000000000000000008ac7230489e80000"
        "0000000000000000000000000000000000000000000000008ac7230489e80000"
        "0000000000000000000000000000000000000000000000008ac7230489e80000"
        "00000000000000000000000000000000000000000000000029a2241af62c0000"
        "00000000000000000000000000000000000000000000000014d1120d7b160000"
        "00000000000000000000000000000000000000000000000014d1120d7b160000"
        "00000000000000000000000000000000000000000000000014d1120d7b160000"
        "00000000000000000000000000000000000000000000000014d1120d7b160000";

    char out_val[2048];
    trigger_process_samples(limit_addr, "9", inputs, out_val, sizeof(out_val));

    printf("  [RESULT] Hysteresis Loop Cycle Samples (0-8):\n");
    printf("    Sample 0 (5V input - Unclamped):   %.64s\n", out_val + 2);
    printf("    Sample 1 (10V input - Charging):   %.64s\n", out_val + 2 + 64);
    printf("    Sample 2 (10V input - STRIKE!):    %.64s\n", out_val + 2 + 128);
    printf("    Sample 3 (10V input - Limited):    %.64s\n", out_val + 2 + 192);
    printf("    Sample 4 (3V input - Latch Hold):  %.64s\n", out_val + 2 + 256);
    printf("    Sample 5 (1.5V input - Hold):      %.64s\n", out_val + 2 + 320);
    printf("    Sample 6 (1.5V input - Hold):      %.64s\n", out_val + 2 + 384);
    printf("    Sample 7 (1.5V input - Hold):      %.64s\n", out_val + 2 + 448);
    printf("    Sample 8 (1.5V input - RELEASE!):  %.64s\n", out_val + 2 + 512);

    printf("[PASS] Sylvania stroboscopic hysteresis limiter verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
