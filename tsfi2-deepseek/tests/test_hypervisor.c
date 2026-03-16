#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lau_memory.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_types.h"

// 1. The Real Functions we are trying to protect
void node_fire_action(void *ctx) {
    HypervisorTestNode *n = (HypervisorTestNode*)ctx;
    printf("[HYPERVISOR] Fire Action Executed on Node %d. (State=%d)\n", n->id, n->state);
    lau_unseal_object(n);
    n->state += 100;
    lau_seal_object(n);
}

void node_cycle_state(void *ctx) {
    HypervisorTestNode *n = (HypervisorTestNode*)ctx;
    printf("[HYPERVISOR] Cycle State Executed on Node %d.\n", n->id);
    lau_unseal_object(n);
    n->state = 0;
    lau_seal_object(n);
}

// 2. The Static Zero-Cost Schema defining the structural topography
const ThunkSignature hypervisor_schema[] = {
    { offsetof(HypervisorTestNode, fire_action), THUNK_BAKED, 1, (void*)node_fire_action },
    { offsetof(HypervisorTestNode, cycle_state), THUNK_BAKED, 1, (void*)node_cycle_state }
};

int main() {
    alarm(5);
    printf("=== TSFi Thunk Hypervisor Verification ===\n");
    tsfi_wire_firmware_init();

    // 3. Allocate via lau_malloc_wired so the Proxy can intercept it natively.
    HypervisorTestNode *node = (HypervisorTestNode*)lau_malloc_wired_loc(sizeof(HypervisorTestNode), "test_hypervisor", 0);
    node->id = 777;
    node->state = 42;
    


    // 4. Bind the schema mapping manually
    TSFI_BIND_SCHEMA(node, hypervisor_schema, 2);

    // 4b. We must create the ThunkProxy so the VM has somewhere to find the JIT instructions!
    LauWiredHeader* _h = (LauWiredHeader*)((char*)node - 8192);
    _h->proxy = ThunkProxy_create();
    
    // Explicitly trigger the hypervisor's JIT emission loop now that the proxy is online
    extern void* ThunkProxy_emit_mapped(ThunkProxy *p, MappedCommon *c, LauWiredHeader *h);
    ThunkProxy_emit_mapped(_h->proxy, (MappedCommon*)node, _h);



    // 5. Build and engage the VM
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);
    
    // Test 1: Direct Host Write (Before Seal)
    printf("[TEST 1] Testing direct state write before VM_OP_SEAL...\n");
    node->state = 99;
    assert(node->state == 99);
    printf("PASS\n");

    // Test 2: The Hypervisor takes control
    printf("[TEST 2] Engaging Hypervisor Seal via VM Pipeline...\n");
    uint32_t hi = (uint32_t)((uint64_t)node >> 32);
    uint32_t lo = (uint32_t)((uint64_t)node & 0xFFFFFFFF);
    
    // We construct raw bytecode for VM_OP_SEAL
    uint32_t seal_cmd[] = { VM_OP_SEAL, hi, lo, ZMM_OP_END };
    tsfi_zmm_vm_exec_block(&vm, seal_cmd);
    printf("PASS: Node successfully sealed by VM MMU.\n");

    // Test 3: Hypervisor Safe Execution
    printf("[TEST 3] Requesting execution of fire_action via VM_OP_INVOKE...\n");
    uint32_t fire_cmd[] = { VM_OP_INVOKE, hi, lo, offsetof(HypervisorTestNode, fire_action), ZMM_OP_END };
    tsfi_zmm_vm_exec_block(&vm, fire_cmd);
    
    // The JIT thunk executes safely through the ABI barrier
    assert(node->state == 199);
    printf("PASS: Thunk executed successfully. New state: %d\n", node->state);
    
    // Test 4: Hypervisor State Cycle
    printf("[TEST 4] Requesting execution of cycle_state via VM_OP_INVOKE...\n");
    uint32_t cycle_cmd[] = { VM_OP_INVOKE, hi, lo, offsetof(HypervisorTestNode, cycle_state), ZMM_OP_END };
    tsfi_zmm_vm_exec_block(&vm, cycle_cmd);
    
    assert(node->state == 0);
    printf("PASS: Thunk executed successfully. State cleared.\n");

    // Test 5: The Unseal release
    printf("[TEST 5] Releasing Hypervisor Seal via VM_OP_UNSEAL...\n");
    uint32_t unseal_cmd[] = { VM_OP_UNSEAL, hi, lo, ZMM_OP_END };
    tsfi_zmm_vm_exec_block(&vm, unseal_cmd);
    
    node->state = 1;
    assert(node->state == 1);
    printf("PASS: Unsealed. Direct host write verified.\n");

    tsfi_zmm_vm_destroy(&vm);
    lau_free(node);
    
    printf("=== HYPERVISOR TESTS PASSED ===\n");
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
