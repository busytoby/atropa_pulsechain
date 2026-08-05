#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../tsfi2-deepseek/src/tsfi_ast_optimizer.c"

int main(void) {
    printf("=============================================================\n");
    printf("AST COMPILER OPTIMIZER TEST RUNNER (FORTRAN H MODEL)\n");
    printf("=============================================================\n");

    // Construct mock AST for: IF (5 * 0) THEN (CONST 99) ELSE (10 + 20)
    printf("[TEST] Constructing mock AST tree...\n");
    
    ASTNode *cond = create_binary_node(NODE_MUL, create_const_node(5), create_const_node(0));
    ASTNode *then_br = create_const_node(99);
    ASTNode *else_br = create_binary_node(NODE_ADD, create_const_node(10), create_const_node(20));
    
    ASTNode *root = create_if_node(cond, then_br, else_br);

    printf("[TEST] Running AST optimization pass...\n");
    ASTNode *optimized_root = tsfi_optimize_ast(root);

    // Verify optimized tree properties
    printf("\n[TEST] Verifying optimization invariants...\n");
    assert(optimized_root != NULL);
    
    // The IF shell should be pruned, returning only the folded else branch: NODE_CONST(30)
    assert(optimized_root->type == NODE_CONST);
    assert(optimized_root->value == 30);
    printf("   ✓ Folded Node Type: %d (Expected: %d/NODE_CONST)\n", optimized_root->type, NODE_CONST);
    printf("   ✓ Folded Value    : %ld (Expected: 30)\n", optimized_root->value);

    free_ast(optimized_root);

    printf("=============================================================\n");
    printf("AST COMPILER OPTIMIZER TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
