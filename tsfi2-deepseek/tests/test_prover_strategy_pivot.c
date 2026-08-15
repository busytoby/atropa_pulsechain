#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../inc/tsfi_prover_strategy_pivot.h"
#include "../inc/tsfi_chancery_docket.h"

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: OUTPUT-STREAM-CENTRIC PROVER & STRATEGY REFINEMENT\n");
    printf("====================================================================\n\n");

    ProverStrategyPivotState pivot_state;
    tsfi_pivot_engine_init(&pivot_state, 4);

    const char *immutable_spec = 
        "function main() {\n"
        "    // Output hello world to standard stream\n"
        "    print \"Hello World\"\n"
        "}\n";

    printf("1. Running Output-Stream Refinement Loop with Immutable Spec:\n");
    printf("--------------------------------------------------------------------\n%s\n", immutable_spec);
    printf("--------------------------------------------------------------------\n");

    char final_code[2048] = {0};
    bool success = tsfi_pivot_engine_synthesize_code(
        &pivot_state,
        immutable_spec,
        "/home/mariarahel/src/tsfi2/assets/DeepSeek-Coder-6.7B.gguf",
        final_code,
        sizeof(final_code)
    );

    printf("\n2. Output Stream Pipeline Execution Summary:\n");
    printf("   Passes Evaluated:           %u\n", pivot_state.current_pass);
    printf("   Output Stream Transforms:   %u\n", pivot_state.stream_transform_count);
    printf("   Last Applied Strategy:      %s\n", pivot_state.triggered_strategy);
    printf("   Final Prover Decision Code: %d (0 = AUTHENTIC_CODE_STREAM)\n", pivot_state.last_prover_ruling);
    printf("   Formally Proven & Verified: %s\n", pivot_state.is_formally_proven ? "YES" : "NO");

    assert(success == true);
    assert(pivot_state.is_formally_proven == true);
    assert(pivot_state.last_prover_ruling == 0);

    printf("\n3. Formally Proven Final Output Stream:\n");
    printf("--------------------------------------------------------------------\n%s\n", final_code);
    printf("--------------------------------------------------------------------\n");

    assert(strstr(final_code, "#include <stdio.h>") != NULL);
    assert(strstr(final_code, "int main(void)") != NULL);
    assert(strstr(final_code, "printf(\"Hello World\\n\");") != NULL);
    assert(strstr(final_code, "return 0;") != NULL);

    // 4. Filing Resolution on Chancery Docket
    printf("\n4. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_id = tsfi_chancery_docket_file(
        &docket,
        "Output-Stream Prover Gating and Strategy Transform Verified (Zero Input Injections)",
        "solidity/dysnomia/domain/std/llm_code_diffusion_prover.algol61",
        2026
    );
    assert(doc_id == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_id, 0, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("OUTPUT-STREAM REFINEMENT TEST PASSED & SEALED ON CHANCERY\n");
    printf("====================================================================\n");

    return 0;
}
