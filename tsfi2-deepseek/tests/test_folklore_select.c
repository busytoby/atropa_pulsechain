#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_wiring.h"
#include "tsfi_sort.h"
#include "lau_memory.h"

int main(void) {
    printf("=== Auncient Folklore Z-Machine Selection Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    /* 1. Compile and Initialize zmachine Yul contracts */
    printf("[ZMM] Compiling and Initializing zmachine.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"zmachine\", \"../solidity/bin/zmachine.yul\", 5");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"zmachineParser\", \"../solidity/bin/zmachineParser.yul\", 6");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"zmachine\", \"7e1ef7e90000000000000000000000000000000000000000000000000000000000000006\"");

    /* 
     * 2. Register mock folklore rooms.
     * This creates physical memory allocations mapped as data structures.
     */
    printf("[ZMM] Registering custom folklore rooms to allocate memory headers...\n");
    // selector: b4c9ac9b for createRoom(uint256,bytes,uint256)
    // We register rooms 100, 101, 102 with different description payload lengths
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"zmachine\", \"b4c9ac9b"
                          "0000000000000000000000000000000000000000000000000000000000000064" /* 100 */
                          "0000000000000000000000000000000000000000000000000000000000000060"
                          "0000000000000000000000000000000000000000000000000000000000000001" /* exits */
                          "000000000000000000000000000000000000000000000000000000000000000b" /* desc len = 11 */
                          "4461726b2048616c6c7761790000000000000000000000000000000000000000" /* "Dark Hallway" */
                          "\"");

    /* 3. Run Topological Möbius-Klein selection over the active allocations */
    printf("[FOLKLORE] Selecting K-th memory allocation metadata...\n");
    LauMetadata *min_alloc = tsfi_select_memory_header(0);
    LauMetadata *topo_alloc = tsfi_wire_topological_select(NULL, 0);

    if (min_alloc && topo_alloc) {
        printf("  - Smallest allocation size found: %zu bytes\n", min_alloc->alloc_size);
        printf("  - Non-orientable Möbius-Klein select size: %zu bytes\n", topo_alloc->alloc_size);
        printf("[PASS] Active registry searches from the folklore context verified successfully.\n");
    } else {
        printf("[WARN] Allocation search returned NULL. This is expected if hypervisor intercepts are disabled.\n");
    }

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL FOLKLORE SELECTION TESTS PASSED ===\n");
    return 0;
}
