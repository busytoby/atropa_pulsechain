#include "tsfi_parc_ingerman_translator.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static uint64_t sample_jensen_thunk(void *env) {
    int *val = (int*)env;
    return (uint64_t)((*val) * (*val) + 7);
}

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING PETER Z INGERMAN TRANSLATOR & THUNK AUDIT     \n");
    printf("=======================================================\n");

    tsfi_ingerman_translator_t translator;
    assert(tsfi_ingerman_translator_initialize(&translator) == 0);
    assert(translator.is_initialized == true);
    printf("[PASS] Ingerman Syntax Translator Initialization verified\n");

    // Test Production Rule Translation (Ingerman 1966)
    char yul_out[512];
    assert(tsfi_ingerman_translate_syntax(&translator, "x := 42", yul_out, sizeof(yul_out)) == 0);
    assert(strstr(yul_out, "sstore(x, 42)") != NULL);
    assert(strstr(yul_out, "dynamic_0x7343d8afa9d6e3376873ea24ccba7c7230aab14b") != NULL);
    printf("[PASS] Syntax-Oriented Production Rule Translation verified\n");

    // Test Ingerman Call-By-Name Thunk Registration & Evaluation (Ingerman 1961)
    int env_val = 5;
    uint32_t thunk_id = 0;
    assert(tsfi_ingerman_register_thunk(&translator, "i * i + 7", sample_jensen_thunk, &env_val, &thunk_id) == 0);
    assert(thunk_id == 1);

    uint64_t thunk_result = tsfi_ingerman_evaluate_thunk(&translator, thunk_id);
    assert(thunk_result == 32); // 5*5 + 7 = 32
    printf("[PASS] Ingerman Call-By-Name Thunk Evaluation verified (Result: %lu)\n", thunk_result);

    printf("=======================================================\n");
    printf(" ALL PETER Z INGERMAN TRANSLATOR TESTS PASSED (100%%)   \n");
    printf("=======================================================\n");

    return 0;
}
