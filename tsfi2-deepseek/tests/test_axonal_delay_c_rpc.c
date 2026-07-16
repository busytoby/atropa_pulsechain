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

    char payload[65536];
    // Function selector: 0x11223344
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x11223344%s%s\""
             "},\"latest\"],\"id\":1}",
             contract, padded_count, inputs_calldata);

    char response[65536];
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
    printf("ELEKTOR ISSUE #17: AXONAL NERVE CONDUCTION DELAY (ANVIL LIVE)\n");
    printf("=============================================================\n");

    printf("[C-Test] Compiling AxonalDelay Yul object...\n");
    char *ax_hex = malloc(131072);
    assert(ax_hex != NULL);
    compile_yul_to_bytecode("../solidity/bin/axonalDelay.yul", ax_hex, 131072);

    char ax_addr[128];
    printf("[C-Test] Deploying AxonalDelay contract dynamically from C...\n");
    deploy_contract_from_c(ax_hex, ax_addr, sizeof(ax_addr));
    printf("  [DEPLOYED] AxonalDelay Address: %s\n", ax_addr);
    free(ax_hex);

    // Call processSamples in a single batch of 13 inputs (all 1.0V, first 12 isClipping = 1):
    // Samples 0 to 11 should be unattenuated (1.0V) because of the 12-sample delay queue.
    // Sample 12 should be attenuated to 0.7V because the spike from sample 0 has propagated!
    char *inputs = malloc(8192);
    assert(inputs != NULL);
    inputs[0] = '\0';
    for (int i = 0; i < 13; i++) {
        strcat(inputs, "0000000000000000000000000000000000000000000000000de0b6b3a7640000"); // 1.0V input
        if (i < 12) {
            strcat(inputs, "0000000000000000000000000000000100000000000000000000000000000000"); // isClipping = 1
        } else {
            strcat(inputs, "0000000000000000000000000000000000000000000000000000000000000000"); // isClipping = 0
        }
    }

    char out_val[16384];
    trigger_process_samples(ax_addr, "d", inputs, out_val, sizeof(out_val));
    free(inputs);

    printf("  [RESULT] Axonal Delay Queue Samples:\n");
    printf("    Sample 0 (1.0V, In queue):      %.64s\n", out_val + 2);
    printf("    Sample 11 (1.0V, In queue):     %.64s\n", out_val + 2 + 11 * 64);
    printf("    Sample 12 (1.0V, Triggered!):   %.64s\n", out_val + 2 + 12 * 64);

    char sample0_hex[65];
    char sample11_hex[65];
    char sample12_hex[65];
    memcpy(sample0_hex, out_val + 2, 64);
    sample0_hex[64] = '\0';
    memcpy(sample11_hex, out_val + 2 + 11 * 64, 64);
    sample11_hex[64] = '\0';
    memcpy(sample12_hex, out_val + 2 + 12 * 64, 64);
    sample12_hex[64] = '\0';

    // Verify Samples 0 and 11 are exactly 1.0V
    assert(strcmp(sample0_hex, "0000000000000000000000000000000000000000000000000de0b6b3a7640000") == 0);
    assert(strcmp(sample11_hex, "0000000000000000000000000000000000000000000000000de0b6b3a7640000") == 0);
    // Verify Sample 12 is attenuated (0.7V)
    assert(strcmp(sample12_hex, "00000000000000000000000000000000000000000000000009b6e64a8ec60000") == 0);

    printf("[PASS] Axonal nerve conduction delay line verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
