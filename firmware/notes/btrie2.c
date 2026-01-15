#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* --- 1. DATA STRUCTURES --- */
#define BTRIE_STRUCT_DECL(name, ...) typedef struct { __VA_ARGS__ } name
BTRIE_STRUCT_DECL(UserPayload, int id; float score;);

#define CONFIG_SCHEMA(X, target) \
    X(api_port,    int,         target) \
    X(api_host,    char*,       target) \
    X(user_data,   UserPayload, target)

/* --- 2. TYPE RESOLUTION --- */
#define TYPE_PICKER(name, type, target) \
    __builtin_choose_expr(__builtin_strcmp(#target, #name) == 0, (type){0},
#define TYPE_CLOSER(name, type, target) )
#define RESOLVE_TYPE(target) typeof( \
    CONFIG_SCHEMA(TYPE_PICKER, target) (void*)0 CONFIG_SCHEMA(TYPE_CLOSER, target) \
)

typedef struct ConfigLimb {
    char label; void* value; int is_ptr; int is_end;
    struct ConfigLimb *child, *sibling;
} ConfigLimb;

/* --- 3. SEARCH AND ROBUST INSERT --- */
static ConfigLimb* _btrie_find(ConfigLimb **root, const char *key) {
    if (!root || !*root) return NULL;
    ConfigLimb **curr = root;
    for (const char *k = key; *k; k++) {
        while (*curr && (*curr)->label < *k) curr = &((*curr)->sibling);
        if (!*curr || (*curr)->label != *k) return NULL;
        if (*(k+1) != '\0') curr = &((*curr)->child);
    }
    return (*curr && (*curr)->is_end) ? *curr : NULL;
}

static void _btrie_ins_robust(ConfigLimb **root, const char *key, void *val_ptr, size_t sz, int is_p) {
    ConfigLimb **curr = root;
    for (const char *k = key; *k; k++) {
        while (*curr && (*curr)->label < *k) curr = &((*curr)->sibling);
        if (!*curr || (*curr)->label != *k) {
            ConfigLimb *n = (ConfigLimb*)calloc(1, sizeof(ConfigLimb));
            n->label = *k; n->sibling = *curr; *curr = n;
        }
        if (*(k+1) == '\0') {
            ConfigLimb *leaf = *curr;
            if (leaf->value) free(leaf->value);
            leaf->is_ptr = is_p;
            if (is_p) {
                const char *input_str = *(const char**)val_ptr;
                leaf->value = input_str ? strdup(input_str) : NULL;
            } else {
                leaf->value = malloc(sz);
                if (leaf->value) memcpy(leaf->value, val_ptr, sz);
            }
            leaf->is_end = 1;
        }
        curr = &((*curr)->child);
    }
}

/* --- 4. THE MACRO INTERFACE --- */
#define BTRIE_INSERT(root, member, val) do { \
    typeof(val) __v = (val); \
    int __is_p = _Generic((val), char*: 1, const char*: 1, default: 0); \
    _btrie_ins_robust(&(root), #member, &__v, sizeof(__v), __is_p); \
} while (0)

#define SAFE_CAST(type, limb) (type)__builtin_choose_expr( \
    __builtin_types_compatible_p(type, char*) || __builtin_types_compatible_p(type, const char*), \
    (limb->value), \
    *(type*)(limb->value) \
)

#define BTRIE_GET(root, member) ({ \
    typedef RESOLVE_TYPE(member) __T; \
    ConfigLimb *__l = _btrie_find(&(root), #member); \
    __T __ret = (__T){0}; \
    if (__l && __l->value) __ret = SAFE_CAST(__T, __l); \
    __ret; \
})

/* --- 5. EXECUTION --- */
int main() {
    ConfigLimb *root = NULL;

    BTRIE_INSERT(root, api_port, 9090);
    char* host_str = calloc(11, 1); // char consts aren't functional but allocated strings are
    strcpy(host_str, "127.0.0.1");
    BTRIE_INSERT(root, api_host, host_str);
    
    UserPayload u_in = { .id = 2026, .score = 100.0f };
    BTRIE_INSERT(root, user_data, u_in);

    char* host = BTRIE_GET(root, api_host);
    printf("--- 2026 B-trie Final Verification ---\n");
    printf("Host (Heap Duplicate): %s\n", host);
    printf("Port: %d\n", BTRIE_GET(root, api_port));

    return 0;
}
