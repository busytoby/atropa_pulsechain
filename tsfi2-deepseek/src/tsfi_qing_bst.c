#include "tsfi_qing_bst.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static inline int max_val(int a, int b) {
    return (a > b) ? a : b;
}

static inline int get_height(tsfi_qing_bst_node *node) {
    return node ? node->height : 0;
}

static inline int get_balance(tsfi_qing_bst_node *node) {
    return node ? (get_height(node->left) - get_height(node->right)) : 0;
}

static tsfi_qing_bst_node* rotate_right(tsfi_qing_bst_node *y) {
    tsfi_qing_bst_node *x = y->left;
    tsfi_qing_bst_node *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max_val(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_val(get_height(x->left), get_height(x->right)) + 1;

    return x;
}

static tsfi_qing_bst_node* rotate_left(tsfi_qing_bst_node *x) {
    tsfi_qing_bst_node *y = x->right;
    tsfi_qing_bst_node *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max_val(get_height(x->left), get_height(x->right)) + 1;
    y->height = max_val(get_height(y->left), get_height(y->right)) + 1;

    return y;
}

tsfi_qing_bst_node* tsfi_qing_bst_insert(tsfi_qing_bst_node *root, uint64_t virtual_address, CachedContract *contract) {
    if (!root) {
        tsfi_qing_bst_node *node = (tsfi_qing_bst_node*)malloc(sizeof(tsfi_qing_bst_node));
        if (node) {
            node->virtual_address = virtual_address;
            node->contract = contract;
            node->height = 1;
            node->left = NULL;
            node->right = NULL;
        }
        return node;
    }

    if (virtual_address < root->virtual_address) {
        root->left = tsfi_qing_bst_insert(root->left, virtual_address, contract);
    } else if (virtual_address > root->virtual_address) {
        root->right = tsfi_qing_bst_insert(root->right, virtual_address, contract);
    } else {
        return root; // Equal keys are not allowed in BST
    }

    root->height = max_val(get_height(root->left), get_height(root->right)) + 1;

    int balance = get_balance(root);

    // Left Left Case
    if (balance > 1 && virtual_address < root->left->virtual_address) {
        return rotate_right(root);
    }

    // Right Right Case
    if (balance < -1 && virtual_address > root->right->virtual_address) {
        return rotate_left(root);
    }

    // Left Right Case
    if (balance > 1 && virtual_address > root->left->virtual_address) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }

    // Right Left Case
    if (balance < -1 && virtual_address < root->right->virtual_address) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }

    return root;
}

CachedContract* tsfi_qing_bst_find(tsfi_qing_bst_node *root, uint64_t virtual_address) {
    if (!root) {
        return NULL;
    }
    if (virtual_address == root->virtual_address) {
        return root->contract;
    }
    if (virtual_address < root->virtual_address) {
        return tsfi_qing_bst_find(root->left, virtual_address);
    }
    return tsfi_qing_bst_find(root->right, virtual_address);
}

tsfi_qing_bst_node* tsfi_qing_bst_populate(void) {
    tsfi_qing_bst_node *root = NULL;
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (g_cached_contracts[i].virtual_address > 0) {
            root = tsfi_qing_bst_insert(root, g_cached_contracts[i].virtual_address, &g_cached_contracts[i]);
        }
    }
    return root;
}

void tsfi_qing_bst_destroy(tsfi_qing_bst_node *root) {
    if (!root) {
        return;
    }
    tsfi_qing_bst_destroy(root->left);
    tsfi_qing_bst_destroy(root->right);
    free(root);
}

tsfi_qing_bst_node *g_runtime_qing_bst = NULL;
static pthread_rwlock_t g_bst_rwlock = PTHREAD_RWLOCK_INITIALIZER;

#define LOAD_BALANCER_SLOTS 16
typedef struct {
    uint64_t virtual_address;
    CachedContract *routes[4];
    int route_count;
    int next_index;
    uint64_t call_count;
} UMCGRLoadBalancer;

static UMCGRLoadBalancer g_umcgr_balancer[LOAD_BALANCER_SLOTS];
static int g_umcgr_balancer_count = 0;

void tsfi_umcgr_register_route(uint64_t virtual_address, CachedContract *contract) {
    pthread_rwlock_wrlock(&g_bst_rwlock);
    for (int i = 0; i < g_umcgr_balancer_count; i++) {
        if (g_umcgr_balancer[i].virtual_address == virtual_address) {
            // Avoid duplicate route registration
            for (int r = 0; r < g_umcgr_balancer[i].route_count; r++) {
                if (g_umcgr_balancer[i].routes[r] == contract) {
                    pthread_rwlock_unlock(&g_bst_rwlock);
                    return;
                }
            }
            if (g_umcgr_balancer[i].route_count < 4) {
                g_umcgr_balancer[i].routes[g_umcgr_balancer[i].route_count++] = contract;
            }
            pthread_rwlock_unlock(&g_bst_rwlock);
            return;
        }
    }
    if (g_umcgr_balancer_count < LOAD_BALANCER_SLOTS) {
        int idx = g_umcgr_balancer_count++;
        g_umcgr_balancer[idx].virtual_address = virtual_address;
        g_umcgr_balancer[idx].routes[0] = contract;
        g_umcgr_balancer[idx].route_count = 1;
        g_umcgr_balancer[idx].next_index = 0;
        g_umcgr_balancer[idx].call_count = 0;
    }
    pthread_rwlock_unlock(&g_bst_rwlock);
}

CachedContract* tsfi_qing_bst_find_safe(uint64_t virtual_address) {
    pthread_rwlock_rdlock(&g_bst_rwlock);
    for (int i = 0; i < g_umcgr_balancer_count; i++) {
        if (g_umcgr_balancer[i].virtual_address == virtual_address) {
            UMCGRLoadBalancer *lb = &g_umcgr_balancer[i];
            pthread_rwlock_unlock(&g_bst_rwlock);
            
            pthread_rwlock_wrlock(&g_bst_rwlock);
            CachedContract *selected = lb->routes[lb->next_index];
            lb->next_index = (lb->next_index + 1) % lb->route_count;
            lb->call_count++;
            pthread_rwlock_unlock(&g_bst_rwlock);
            return selected;
        }
    }
    CachedContract *res = tsfi_qing_bst_find(g_runtime_qing_bst, virtual_address);
    pthread_rwlock_unlock(&g_bst_rwlock);
    
    if (res) {
        tsfi_umcgr_register_route(virtual_address, res);
    }
    return res;
}

void tsfi_qing_bst_invalidate_safe(void) {
    pthread_rwlock_wrlock(&g_bst_rwlock);
    if (g_runtime_qing_bst) {
        tsfi_qing_bst_destroy(g_runtime_qing_bst);
        g_runtime_qing_bst = NULL;
    }
    pthread_rwlock_unlock(&g_bst_rwlock);
}

void tsfi_qing_bst_populate_safe(void) {
    if (g_runtime_qing_bst) return;
    pthread_rwlock_wrlock(&g_bst_rwlock);
    if (!g_runtime_qing_bst) {
        g_runtime_qing_bst = tsfi_qing_bst_populate();
    }
    pthread_rwlock_unlock(&g_bst_rwlock);
}
