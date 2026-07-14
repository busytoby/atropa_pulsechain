#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../src/tsfi_computel_controller_thunk.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Controller Thunk test suite...\n");

    ControllerThunk client;
    ControllerThunk server;

    uint64_t seed = 0xDEADBEEF12345ULL;
    init_controller_thunk(&client, seed);
    init_controller_thunk(&server, seed);

    RawControllerInput input1 = {0x81, -50, 75, 1000};
    uint8_t enc_buffer[16];
    uint8_t len = 0;

    // 1. Package input at client
    thunk_package_input(&client, &input1, enc_buffer, &len);
    assert(len == 7);

    // Verify buffer is encrypted (should not match plain bytes)
    assert(enc_buffer[0] != 0x81);

    // 2. Deliver and verify at server
    RawControllerInput recovered1;
    bool ok = thunk_verify_input(&server, enc_buffer, len, &recovered1);
    assert(ok == true);
    assert(recovered1.buttons == 0x81);
    assert(recovered1.axis_x == -50);
    assert(recovered1.axis_y == 75);
    assert(recovered1.timestamp == 1000);
    printf("[TEST] Controller input thunk decrypted and verified successfully.\n");

    // 3. Deliver second packet to show key rotation sync
    RawControllerInput input2 = {0x42, 10, -20, 1050};
    uint8_t enc_buffer2[16];
    thunk_package_input(&client, &input2, enc_buffer2, &len);

    RawControllerInput recovered2;
    ok = thunk_verify_input(&server, enc_buffer2, len, &recovered2);
    assert(ok == true);
    assert(recovered2.buttons == 0x42);
    assert(recovered2.axis_x == 10);
    printf("[TEST] Multi-packet key rotation sync verified successfully.\n");

    // 4. Try replaying the first packet -> Should produce garbled results
    // because server keys have advanced/rotated
    RawControllerInput replay_recovered;
    thunk_verify_input(&server, enc_buffer, len, &replay_recovered);
    assert(replay_recovered.buttons != 0x81); // Payload is effectively garbled/rejected
    printf("[TEST] Replay attack attempt successfully mitigated via key rotation.\n");

    printf("[SUCCESS] All Controller Thunk bridge tests completed.\n");
    return 0;
}
