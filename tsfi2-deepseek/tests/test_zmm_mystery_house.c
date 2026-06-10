#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// Parse hex string of any length back to ASCII
void hex_to_ascii(const char *hex, char *ascii) {
    size_t len = strlen(hex);
    size_t ascii_idx = 0;
    for (size_t i = 0; i < len; i += 2) {
        unsigned int val;
        if (sscanf(hex + i, "%2x", &val) == 1) {
            if (val >= 32 && val < 127) {
                ascii[ascii_idx++] = (char)val;
            } else if (val == 10 || val == 13) {
                ascii[ascii_idx++] = ' ';
            }
        }
    }
    ascii[ascii_idx] = '\0';
}

void register_room(TsfiZmmVmState *vm, uint64_t roomId, const char *desc, uint64_t exits) {
    char cmd[4096];
    size_t desc_len = strlen(desc);
    size_t padded_len = ((desc_len + 31) / 32) * 32;
    
    char calldata[2048];
    // createRoom(uint256,bytes,uint256) -> selector: b4c9ac9b
    sprintf(calldata, "b4c9ac9b"
                      "%064lx"
                      "%064lx"
                      "%064lx"
                      "%064lx",
            roomId, 0x60UL, exits, desc_len);
            
    for (size_t i = 0; i < padded_len; i++) {
        char c = (i < desc_len) ? desc[i] : 0;
        sprintf(calldata + strlen(calldata), "%02x", (unsigned char)c);
    }
    
    sprintf(cmd, "YULEXEC \"zmachine\", \"%s\"", calldata);
    tsfi_zmm_vm_exec(vm, cmd);
}

void run_command(TsfiZmmVmState *vm, const char *player_addr, const char *command) {
    char cmd[4096];
    size_t cmd_len = strlen(command);
    size_t padded_len = ((cmd_len + 31) / 32) * 32;
    
    char calldata[2048];
    // parseCommand(address,bytes) -> selector: f1ba03f9
    sprintf(calldata, "f1ba03f9"
                      "000000000000000000000000%s"
                      "0000000000000000000000000000000000000000000000000000000000000040"
                      "%064lx",
            player_addr, cmd_len);
            
    for (size_t i = 0; i < padded_len; i++) {
        char c = (i < cmd_len) ? command[i] : 0;
        sprintf(calldata + strlen(calldata), "%02x", (unsigned char)c);
    }
    
    sprintf(cmd, "YULEXEC \"zmachine\", \"%s\"", calldata);
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, cmd);
}

void test_vector_scene(TsfiZmmVmState *vm, uint64_t roomIndex, uint8_t expected_length, uint8_t check_offset, uint8_t expected_val) {
    char cmd[4096];
    char calldata[2048];
    // getVectorScene(uint256) -> selector: 5d5517bf
    sprintf(calldata, "5d5517bf"
                      "%064lx",
            roomIndex);
            
    sprintf(cmd, "YULEXEC \"zmachine\", \"%s\"", calldata);
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, cmd);
    
    // The return data format of getVectorScene is:
    // Offset to bytes: 32 bytes (value = 0x20)
    // Length of bytes: 32 bytes (value = expected_length)
    // Byte payload starts at offset 64 bytes.
    // Each byte is 2 hex chars.
    char offset_str[65];
    char len_str[65];
    strncpy(offset_str, vm->output_buffer, 64);
    offset_str[64] = '\0';
    strncpy(len_str, vm->output_buffer + 64, 64);
    len_str[64] = '\0';
    
    unsigned long offset = strtoul(offset_str, NULL, 16);
    unsigned long length = strtoul(len_str, NULL, 16);
    
    printf("[VECTOR] Room %ld: offset=%ld, length=%ld\n", roomIndex, offset, length);
    assert(offset == 0x20);
    assert(length == expected_length);
    
    char byte_hex[3];
    strncpy(byte_hex, vm->output_buffer + 128 + (check_offset * 2), 2);
    byte_hex[2] = '\0';
    unsigned int val = (unsigned int)strtoul(byte_hex, NULL, 16);
    printf("[VECTOR] Checking offset %d: got %u, expected %d\n", check_offset, val, expected_val);
    assert(val == expected_val);
}

int main() {
    printf("=== TSFi ZMM VM Mystery House Verification ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize Z-Machine Yul contracts
    printf("[ZMM] Compiling and Initializing zmachine.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"zmachine\", \"../solidity/bin/zmachine.yul\", 5");

    printf("[ZMM] Compiling and Initializing zmachineParser.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"zmachineParser\", \"../solidity/bin/zmachineParser.yul\", 6");

    // 2. Link Parser address
    printf("[ZMM] Linking parser address...\n");
    // bindParserAddress(address) -> selector: 7e1ef7e9
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"zmachine\", \"7e1ef7e90000000000000000000000000000000000000000000000000000000000000006\"");

    // 3. Verify vector scenes (thunk side validation)
    printf("[ZMM] Verifying Room 0 vector scene (Victorian House Exterior)...\n");
    // Room 0, expected length 50 bytes, offset 2 (toX of Line 0) should be 240
    test_vector_scene(&vm, 0, 50, 2, 240);

    printf("[ZMM] Verifying Room 1 vector scene (Entry Hall / Foyer)...\n");
    // Room 1, expected length 50 bytes, offset 5 (fromX of Line 1) should be 20
    test_vector_scene(&vm, 1, 50, 5, 20);

    printf("[ZMM] Verifying Room 2 vector scene (Living Room / Parlor)...\n");
    // Room 2, expected length 50 bytes, offset 30 (fromX of Mantel) should be 80
    test_vector_scene(&vm, 2, 50, 30, 80);

    // 4. Register room descriptions
    printf("[ZMM] Setting up room descriptions...\n");
    register_room(&vm, 10, "You are outside a large Victorian mansion. The front door is to the north.", 0);
    register_room(&vm, 1,  "You are in the entry hall. Doors lead east and west. The exit is south.", 0);
    register_room(&vm, 2,  "You are in the library. Old books line the walls.", 0);
    register_room(&vm, 3,  "You are in the sanctuary. A stone altar stands here.", 0);

    // 5. Traversal gameplay loop simulation
    const char *player = "70997970C51812dc3A010C7d01b50e0d17dc79C8";
    char response[1024];

    // Command: look
    printf("Command: look\n");
    run_command(&vm, player, "look");
    hex_to_ascii(vm.output_buffer, response);
    printf("Output: %s\n", response);
    assert(strstr(response, "outside a large Victorian mansion") != NULL);

    // Command: north
    printf("Command: north\n");
    run_command(&vm, player, "north");
    hex_to_ascii(vm.output_buffer, response);
    printf("Output: %s\n", response);
    assert(strstr(response, "entry hall") != NULL);

    // Command: east
    printf("Command: east\n");
    run_command(&vm, player, "east");
    hex_to_ascii(vm.output_buffer, response);
    printf("Output: %s\n", response);
    assert(strstr(response, "library") != NULL);

    // Command: south (invalid exit)
    printf("Command: south\n");
    run_command(&vm, player, "south");
    hex_to_ascii(vm.output_buffer, response);
    printf("Output: %s\n", response);
    assert(strstr(response, "cannot go that way") != NULL);

    // Command: west
    printf("Command: west\n");
    run_command(&vm, player, "west");
    hex_to_ascii(vm.output_buffer, response);
    printf("Output: %s\n", response);
    assert(strstr(response, "entry hall") != NULL);

    // Command: west to sanctuary
    printf("Command: west\n");
    run_command(&vm, player, "west");
    hex_to_ascii(vm.output_buffer, response);
    printf("Output: %s\n", response);
    assert(strstr(response, "sanctuary") != NULL);

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM MYSTERY HOUSE TESTS PASSED ===\n");
    return 0;
}
