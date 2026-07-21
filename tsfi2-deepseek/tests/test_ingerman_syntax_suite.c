#include "tsfi_parc_ingerman_syntax.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING PETER Z INGERMAN SYNTAX COMPILER AUDIT (1966) \n");
    printf("=======================================================\n");

    tsfi_ingerman_syntax_compiler_t compiler;
    assert(tsfi_ingerman_syntax_init(&compiler) == 0);
    printf("[PASS] Syntax Compiler Initialized\n");

    // Register Syntax Rules
    assert(tsfi_ingerman_syntax_register_rule(&compiler, "SCSI_COMMAND", "SCSI", 101) == 0);
    assert(tsfi_ingerman_syntax_register_rule(&compiler, "SCHUR_TRANSFORM", "SCHUR", 102) == 0);
    assert(tsfi_ingerman_syntax_register_rule(&compiler, "TAPE_CHECK", "TAPE CHECK", 103) == 0);
    printf("[PASS] Registered 3 Syntax Grammar Productions\n");

    // Parse tokens using Pushdown Automata
    uint32_t action = 0;
    assert(tsfi_ingerman_syntax_parse_token(&compiler, "SCSI", &action) == 0);
    assert(action == 101);
    printf("[PASS] Token 'SCSI' Parsed -> Action 101\n");

    assert(tsfi_ingerman_syntax_parse_token(&compiler, "SCHUR", &action) == 0);
    assert(action == 102);
    printf("[PASS] Token 'SCHUR' Parsed -> Action 102\n");

    assert(compiler.stack_top == 1);
    printf("[PASS] Pushdown Automata Stack Height: %d\n", compiler.stack_top + 1);

    printf("=======================================================\n");
    printf(" ALL PETER Z INGERMAN SYNTAX COMPILER TESTS PASSED     \n");
    printf("=======================================================\n");

    return 0;
}
