#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* 1. X-MACRO (Initially Empty) */
#define CONFIG_ENTRIES 

/* 2. AUTO-GENERATE NATIVE STRUCT */
typedef struct {
#define X(name, val) int name;
    CONFIG_ENTRIES
#undef X
    int _reserved; 
} AppConfig;

/* 3. B-TRIE NODE (LIMB) STRUCTURE */
typedef struct ConfigLimb {
    char label;
    int value;
    int is_end;
    struct ConfigLimb *child, *sibling;
} ConfigLimb;

/* 4. SEARCH & INSERT LOGIC */

static void** _btrie_find_limb(void **root, const char *key, size_t c_off, size_t s_off) {
    void **curr = root;
    for (const char *k = key; *k; k++) {
        while (*curr && (*(char*)*curr) < *k) 
            curr = (void**)((char*)*curr + s_off);
        if (!*curr || (*(char*)*curr) != *k) return NULL;
        if (*(k + 1) != '\0') curr = (void**)((char*)*curr + c_off);
    }
    return curr;
}

void btrie_insert(ConfigLimb **root, const char *key, int val) {
    ConfigLimb **curr = root;
    for (const char *k = key; *k; k++) {
        while (*curr && (*curr)->label < *k) curr = &((*curr)->sibling);
        if (!*curr || (*curr)->label != *k) {
            ConfigLimb *n = calloc(1, sizeof(ConfigLimb));
            n->label = *k; n->sibling = *curr; *curr = n;
        }
        if (*(k + 1) == '\0') { (*curr)->value = val; (*curr)->is_end = 1; }
        curr = &((*curr)->child);
    }
}

/* 5. STRICT ACCESSOR MACRO (NO FALLBACK) */
// This version returns 0 if the key is missing entirely.
#define CONFIG_GET(root, cfg, member) ({ \
    int __ret = 0; \
    ConfigLimb **__res = (ConfigLimb**)_btrie_find_limb((void**)&(root), #member, \
                          offsetof(ConfigLimb, child), offsetof(ConfigLimb, sibling)); \
    if (__res && *__res && (*__res)->is_end) { \
        __ret = (*__res)->value; \
    } else { \
        __ret = _Generic(&(cfg), AppConfig*: (cfg)._reserved, default: 0); \
    } \
    __ret; \
})

/* 6. EXECUTION */
int main() {
    ConfigLimb *root = NULL;
    AppConfig my_cfg = {0};

    // Runtime Dynamic Insertion
    btrie_insert(&root, "api_port", 8080);
    btrie_insert(&root, "timeout_ms", 5000);

    // Strict Access (Raw tokens, no fallback argument needed)
    int port    = CONFIG_GET(root, my_cfg, api_port);
    int timeout = CONFIG_GET(root, my_cfg, timeout_ms);
    int unknown = CONFIG_GET(root, my_cfg, some_missing_key);

    printf("--- 2026 Strict Hybrid B-trie ---\n");
    printf("API Port: %d\n", port);    // Result: 8080
    printf("Timeout:  %d\n", timeout); // Result: 5000
    printf("Missing:  %d\n", unknown); // Result: 0 (Default strict return)

    return 0;
}
