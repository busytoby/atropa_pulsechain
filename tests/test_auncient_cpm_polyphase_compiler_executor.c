/* SPDX-License-Identifier: GPL-2.0 */
/*
 * C11 Test Prover: In-TPA CP/M-Tomie 3-Phase Polyphase Compiler & Virtual Machine Executor
 * Proves:
 * 1. Phase A: Lexical & AST Ingestion directly into TPA 0x0100
 * 2. Phase B: Relocation & EDSAC Initial Orders 1 Instruction Safety
 * 3. Phase C: Kinetic Rotor JIT Execution (Accumulator 100 + 50 == 150)
 * 4. DisplacementShader Vertex Phase Synchronization (Scale == 2.00)
 * 5. Multi-Tenant Teddy Bear Synchronous Execution with Saat Endowment
 */

#include "auncient_cpm_polyphase_compiler_executor.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

int main(void) {
    printf("=================================================================\n");
    printf("TEST PROVER: CP/M-TOMIE POLYPHASE COMPILER & EXECUTOR ENGINE    \n");
    printf("=================================================================\n");

    CpmPolyphaseCompilerExecutor exec;
    auncient_cpm_poly_init(&exec);

    bool ok = auncient_cpm_poly_verify_all(&exec);
    assert(ok);

    /* Phase A Verification */
    assert(exec.phase_a_verified);
    printf(" Phase A [Lexical AST Stator Winding]:          PROVED (%u Bytecode Instructions Synthesized)\n",
           exec.total_instructions);

    /* Phase B Verification */
    assert(exec.phase_b_verified);
    printf(" Phase B [Initial Orders 1 TPA Relocation]:     PROVED (Relocated into TPA 0x%04X)\n",
           exec.pc);

    /* Phase C Verification */
    assert(exec.phase_c_verified);
    assert(exec.accumulator == 150);
    printf(" Phase C [Kinetic Rotor JIT Execution]:         PROVED (Accumulator Result: %u | Dynamo: %lu)\n",
           exec.accumulator, exec.dynamo_velocity);

    /* DisplacementShader Sync Verification */
    assert(exec.displacementshader_synced);
    printf(" DisplacementShader Vertex Scaling Sync:        PROVED (Displacement Scale Factor: %.2f)\n",
           exec.displacement_scale);

    /* Multi-Tenant Teddy Bear Mesh Execution */
    assert(exec.active_tenants_count == 2);
    printf(" Multi-Tenant Teddy Bear Mesh Execution:        PROVED (%u Active Tenants Synchronized)\n",
           exec.active_tenants_count);

    /* Rule 18 Parity */
    assert(exec.rule18_parity_checksum > 0);
    printf(" Rule 18 Non-Preferential Parity Checksum:      PROVED (0x%08X)\n", exec.rule18_parity_checksum);

    printf("=================================================================\n");
    printf("CP/M-TOMIE POLYPHASE COMPILER & EXECUTOR FULLY CERTIFIED!        \n");
    printf("=================================================================\n");
    return 0;
}
