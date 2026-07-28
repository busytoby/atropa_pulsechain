#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    NODE_CONST,
    NODE_VAR,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_IF
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    int64_t value;         // For NODE_CONST
    char var_name[16];     // For NODE_VAR
    struct ASTNode *left;  // Operands or Condition
    struct ASTNode *right; // Operands or Then branch
    struct ASTNode *els;   // Else branch
} ASTNode;

// Helper to create constant node
ASTNode *create_const_node(int64_t val) {
    ASTNode *n = malloc(sizeof(ASTNode));
    if (n) {
        n->type = NODE_CONST;
        n->value = val;
        n->left = n->right = n->els = NULL;
    }
    return n;
}

// Helper to create binary op node
ASTNode *create_binary_node(ASTNodeType type, ASTNode *left, ASTNode *right) {
    ASTNode *n = malloc(sizeof(ASTNode));
    if (n) {
        n->type = type;
        n->left = left;
        n->right = right;
        n->els = NULL;
    }
    return n;
}

// Helper to create IF node
ASTNode *create_if_node(ASTNode *cond, ASTNode *then_br, ASTNode *else_br) {
    ASTNode *n = malloc(sizeof(ASTNode));
    if (n) {
        n->type = NODE_IF;
        n->left = cond;
        n->right = then_br;
        n->els = else_br;
    }
    return n;
}

// Recursively free AST nodes
void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->els);
    free(node);
}

// AST Optimizer Pass: Constant Folding & Dead Code Pruning (FORTRAN H Model)
ASTNode *tsfi_optimize_ast(ASTNode *node) {
    if (!node) return NULL;

    // Optimize children recursively
    node->left = tsfi_optimize_ast(node->left);
    node->right = tsfi_optimize_ast(node->right);
    node->els = tsfi_optimize_ast(node->els);

    // 1. Constant Folding for binary operators
    if (node->type == NODE_ADD || node->type == NODE_SUB || node->type == NODE_MUL) {
        if (node->left && node->right && node->left->type == NODE_CONST && node->right->type == NODE_CONST) {
            int64_t val = 0;
            if (node->type == NODE_ADD) val = node->left->value + node->right->value;
            else if (node->type == NODE_SUB) val = node->left->value - node->right->value;
            else if (node->type == NODE_MUL) val = node->left->value * node->right->value;

            printf("[OPTIMIZER] Folding expression: ");
            if (node->type == NODE_ADD) printf("%ld + %ld -> %ld\n", node->left->value, node->right->value, val);
            else if (node->type == NODE_SUB) printf("%ld - %ld -> %ld\n", node->left->value, node->right->value, val);
            else if (node->type == NODE_MUL) printf("%ld * %ld -> %ld\n", node->left->value, node->right->value, val);

            // Free children and replace this node with constant node
            free_ast(node->left);
            free_ast(node->right);
            node->type = NODE_CONST;
            node->value = val;
            node->left = node->right = NULL;
        }
    }

    // 2. Dead Code Pruning for IF conditional branches
    if (node->type == NODE_IF) {
        if (node->left && node->left->type == NODE_CONST) {
            bool cond_val = (node->left->value != 0);
            printf("[OPTIMIZER] Pruning conditional block: constant condition resolves to %s\n", cond_val ? "TRUE" : "FALSE");

            ASTNode *result_branch = NULL;
            if (cond_val) {
                // If condition is True, keep then branch and discard else
                result_branch = node->right;
                free_ast(node->left);
                free_ast(node->els);
            } else {
                // If condition is False, keep else branch and discard then
                result_branch = node->els;
                free_ast(node->left);
                free_ast(node->right);
            }

            // Free current IF node shell and return the optimized branch
            free(node);
            return result_branch;
        }
    }

    return node;
}
