#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi ZMM VM MVS Tape Certification Test Runner ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and Initialize mvs_tape_certifier.yul
    printf("[ZMM] Compiling and Initializing mvs_tape_certifier.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"mvs_tape_certifier\", \"../solidity/dysnomia/domain/bin/mvs_tape_certifier.yul\", 1");

    // 2. Validate Register Sequence Rule (Solenoid engages before brake releases)
    // verifyRegisterSequence(1, 0) -> selector: d76fa120
    printf("[ZMM] Verifying valid register sequence (Solenoid=1, Brake=0)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"d76fa120"
                          "0000000000000000000000000000000000000000000000000000000000000001"
                          "0000000000000000000000000000000000000000000000000000000000000000\"");
    printf("Result: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);

    printf("[ZMM] Verifying invalid register sequence (Solenoid=0, Brake=0)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"d76fa120"
                          "0000000000000000000000000000000000000000000000000000000000000000"
                          "0000000000000000000000000000000000000000000000000000000000000000\"");
    printf("Result: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") != NULL);

    // 3. Validate Caliper Brake Latency Rule (<= 1 cycle)
    // verifyBrakeLatency(uint256 cycleOffset) -> selector: 4e23cf01
    printf("[ZMM] Verifying brake latency within limits (1 cycle offset)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"4e23cf01"
                          "0000000000000000000000000000000000000000000000000000000000000001\"");
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);

    printf("[ZMM] Verifying brake latency exceeding limits (2 cycle offset)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"4e23cf01"
                          "0000000000000000000000000000000000000000000000000000000000000002\"");
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") != NULL);

    // 4. Validate RAW Checksum Mismatch
    // verifyRAWVerification(uint256 checksumVal) -> selector: 0bf8923a
    printf("[ZMM] Verifying RAW verification with correct checksum...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"0bf8923a"
                          "0000000000000000000000000000000000000000000000000000000000001234\"");
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);

    printf("[ZMM] Verifying RAW verification with mismatching checksum (0xDEAD)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"0bf8923a"
                          "000000000000000000000000000000000000000000000000000000000000dead\"");
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") != NULL);

    // 5. Validate Atomic Group Writes with rollback simulation
    // writeGroup(startSec, count, dataVal, failIndex) -> selector: e39fa210
    // Try writing 5 sectors, failing at index 3 (sector 13). Expect return 0 (failed/rolled back)
    printf("[ZMM] Executing group write with simulated verification failure...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"e39fa210"
                          "000000000000000000000000000000000000000000000000000000000000000a" // startSec = 10
                          "0000000000000000000000000000000000000000000000000000000000000005" // count = 5
                          "00000000000000000000000000000000000000000000000000000000000000ff" // dataVal = 255
                          "0000000000000000000000000000000000000000000000000000000000000003\""); // failIndex = 3
    printf("Group write outcome: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") != NULL);

    // 6. Validate Lock Isolation
    // acquireLock(uint256 sector, uint256 callerId) -> selector: 902d3412
    printf("[ZMM] Acquiring exclusive lock on sector 20 for Client 1...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"902d3412"
                          "0000000000000000000000000000000000000000000000000000000000000014" // sector = 20
                          "0000000000000000000000000000000000000000000000000000000000000001\""); // callerId = 1
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);

    printf("[ZMM] Attempting to acquire lock on sector 20 for Client 2 (should fail)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"mvs_tape_certifier\", \"902d3412"
                          "0000000000000000000000000000000000000000000000000000000000000014" // sector = 20
                          "0000000000000000000000000000000000000000000000000000000000000002\""); // callerId = 2
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") != NULL);

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL CBT MVS TAPE CERTIFICATION CHECKS PASSED ===\n");
    return 0;
}
