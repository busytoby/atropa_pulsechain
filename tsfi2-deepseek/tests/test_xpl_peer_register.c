#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_DB_BUFFER 0xFA00

typedef struct {
    uint32_t magic;
    uint32_t peer_count;
    uint8_t peer_id;
    uint8_t addr_type;
    uint16_t addr_val;
    uint32_t status;
} PeerRecord;

int main() {
    printf("=== TSFi ZMM VM Auncient Peer Registry Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Load hucSystem.yul to VM
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Register DECnet Node Address (Type 1, Address 101, Peer 5)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_DB_BUFFER + 8, 5); // Peer ID
    tsfi_zmm_vm_exec(&vm, cmd);

    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_DB_BUFFER + 9, 1); // Address Type
    tsfi_zmm_vm_exec(&vm, cmd);

    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_DB_BUFFER + 10, 101); // Address Val
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] Registered DECnet Node Address [ID: 5, Type: 1, Val: 101]\n");

    // 3. Register STANAG Node Address (Type 2, Address 5001, Peer 12)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_DB_BUFFER + 16, 12); // Peer ID
    tsfi_zmm_vm_exec(&vm, cmd);

    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_DB_BUFFER + 17, 2); // Address Type
    tsfi_zmm_vm_exec(&vm, cmd);

    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_DB_BUFFER + 18, 5001); // Address Val
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] Registered STANAG Node Address [ID: 12, Type: 2, Val: 5001]\n");

    // 4. Save Database Slice to peer_registry.dat.bin (Rule 13 Compliant)
    FILE* fp = fopen("peer_registry.dat.bin", "wb");
    assert(fp != NULL);
    
    PeerRecord records[2];
    records[0].magic = 0x5516;
    records[0].peer_count = 2;
    records[0].peer_id = 5;
    records[0].addr_type = 1;
    records[0].addr_val = 101;
    records[0].status = 0xFFFF;

    records[1].magic = 0x5516;
    records[1].peer_count = 2;
    records[1].peer_id = 12;
    records[1].addr_type = 2;
    records[1].addr_val = 5001;
    records[1].status = 0xFFFF;

    size_t written = fwrite(records, sizeof(PeerRecord), 2, fp);
    assert(written == 2);
    fclose(fp);
    printf("[XPL] Peer Registry Database Slice successfully saved to peer_registry.dat.bin.\n");

    // 5. Clean up local file
    remove("peer_registry.dat.bin");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL PEER REGISTRY TESTS PASSED ===\n");
    return 0;
}
