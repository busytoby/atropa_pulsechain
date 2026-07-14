#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../src/tsfi_computel_secure_tty.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Secure TTY Console test suite...\n");

    SecureTtyConsole client;
    SecureTtyConsole server;

    init_secure_tty(&client, 98765ULL);
    init_secure_tty(&server, 12345ULL);

    // Share generator base parameter
    server.crypto.base = client.crypto.base;

    // Derive and exchange public parameters to establish shared session key
    uint64_t client_pub = derive_public_parameter(&client.crypto);
    uint64_t server_pub = derive_public_parameter(&server.crypto);

    compute_shared_session_key(&client.crypto, server_pub);
    compute_shared_session_key(&server.crypto, client_pub);

    assert(client.crypto.shared_key == server.crypto.shared_key);

    // Client sends an administrative shell command
    const char *cmd = "sudo systemctl restart zmachine";
    uint8_t tx_buf[64];
    uint8_t len = 0;

    client_send_command(&client, cmd, tx_buf, &len);
    
    // Bytes must be encrypted/obfuscated
    assert(memcmp(tx_buf, cmd, len) != 0);

    // Server VM receives and decrypts command
    char decoded_cmd[64];
    bool ok = vm_process_tty_input(&server, tx_buf, len, decoded_cmd);
    assert(ok == true);
    printf("[TEST] Decoded TTY Command: \"%s\"\n", decoded_cmd);
    assert(strcmp(decoded_cmd, cmd) == 0);

    // Test Case 2: Helmholtz Command execution simulation
    char helmholtz_cmd[] = "helmholtz tune 100";
    char response[128];
    ok = execute_helmholtz_command(&server, helmholtz_cmd, response);
    assert(ok == true);
    printf("[TEST] Helmholtz Execution Result: \"%s\"\n", response);
    
    // Verify math matches Base^100 mod MotzkinPrime
    uint64_t expected_channel = secure_mod_pow(server.crypto.base, 100, MOTZKIN_PRIME);
    char expected_resp[128];
    sprintf(expected_resp, "Channel = %lu", expected_channel);
    assert(strcmp(response, expected_resp) == 0);

    printf("[SUCCESS] Secure TTY Console shell command encryption tests passed successfully.\n");
    return 0;
}
