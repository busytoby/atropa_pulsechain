#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

// 1. Source of truth (Data)
typedef struct {
    int counter;
    bool ftw;
    bool is_autonomous_excuse_active;
    void (*logic1)(void *self); 
    void (*logic2)(int *cnt, bool *ftw);
    void (*logic3)(int *cnt, bool *ftw, bool *excuse);
    void (*logic4)(int *cnt, int amount); // Context + Runtime Int
} InternalHeader;

// 2. Behavior Object (Child)
#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct name { \
        uint8_t thunk_pool[512]; /* Sufficient pool size */ \
        uint8_t *thunk_cursor;   \
        void (*execute)(); \
        void (*execute2)(); \
        void (*execute3)(); \
        void (*execute4)(int amount); \
        __VA_ARGS__ \
    } name; \
    static inline name* name##_create() { \
        name *ptr; \
        size_t pg = sysconf(_SC_PAGESIZE); \
        if (posix_memalign((void**)&ptr, pg, pg) != 0) return NULL; \
        ptr->thunk_cursor = ptr->thunk_pool; \
        return ptr; \
    }

DEFINE_MAPPED_STRUCT(WaveSystem, int system_id;)

// 3. Master Logic implementations
void master_logic1(void *ctx) {
    WaveSystem *s = (WaveSystem *)ctx;
    printf("[Logic 1] System ID: %d\n", s->system_id);
}

void master_logic2(int *cnt, bool *ftw) {
    *cnt += 1; *ftw = !(*ftw);
    printf("[Logic 2] Counter: %d | FTW: %d\n", *cnt, *ftw);
}

void master_logic3(int *cnt, bool *ftw, bool *excuse) {
    *cnt += 100; *excuse = true;
    printf("[Logic 3] Counter: %d | Excuse Set: %d\n", *cnt, *excuse);
}

void master_logic4(int *cnt, int amount) {
    *cnt += amount;
    printf("[Logic 4] Added %d | Total Counter: %d\n", amount, *cnt);
}

// 4. Thunk Generators
static inline void* emit_baked_thunk(uint8_t **cursor, void *fn, int argc, ...) {
    uint8_t *c = *cursor;
    void *start = (void*)c;
    va_list args;
    va_start(args, argc);
    static const uint8_t arg_regs[] = { 0xbf, 0xbe, 0xba }; // rdi, rsi, rdx
    for (int i = 0; i < argc; i++) {
        *c++ = 0x48; *c++ = arg_regs[i]; 
        *(void**)c = va_arg(args, void*); 
        c += 8;
    }
    va_end(args);
    *c++ = 0x48; *c++ = 0xb8; *(void**)c = fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0;
    *cursor = c;
    return start;
}

static inline void* emit_mixed_thunk(uint8_t **cursor, void *fn, void *baked_ptr) {
    uint8_t *c = *cursor;
    void *start = (void*)c;
    // Shuffle: mov %edi, %esi (moves runtime int out of 1st arg slot into 2nd)
    *c++ = 0x89; *c++ = 0xfe; 
    // Bake: movabs [baked_ptr], %rdi (injects header address into 1st arg slot)
    *c++ = 0x48; *c++ = 0xbf; *(void**)c = baked_ptr; c += 8;
    // Jump to logic
    *c++ = 0x48; *c++ = 0xb8; *(void**)c = fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0;
    *cursor = c;
    return start;
}

// 5. Bijective Wiring Macro
#define WIRE_BIJECTION(d, h) ( \
    (d)->execute  = (void(*)())emit_baked_thunk(&(d)->thunk_cursor, (h).logic1, 1, (d)), \
    (d)->execute2 = (void(*)())emit_baked_thunk(&(d)->thunk_cursor, (h).logic2, 2, &(h).counter, &(h).ftw), \
    (d)->execute3 = (void(*)())emit_baked_thunk(&(d)->thunk_cursor, (h).logic3, 3, &(h).counter, &(h).ftw, &(h).is_autonomous_excuse_active), \
    (d)->execute4 = (void(*)(int))emit_mixed_thunk(&(d)->thunk_cursor, (h).logic4, &(h).counter), \
    mprotect((void*)((uintptr_t)(d) & ~(sysconf(_SC_PAGESIZE) - 1)), sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE | PROT_EXEC) \
)

int main() {
    WaveSystem *child = WaveSystem_create();
    InternalHeader header = { 
        .counter = 0, .ftw = false, .is_autonomous_excuse_active = false,
        .logic1 = master_logic1, .logic2 = master_logic2, 
        .logic3 = master_logic3, .logic4 = master_logic4 
    };

    if (child && WIRE_BIJECTION(child, header) == 0) {
        child->system_id = 9000;
        
        printf("--- Initializing Sequence ---\n");
        child->execute();           // Prints ID 9000
        child->execute2();          // Counter: 1, FTW: 1
        child->execute3();          // Counter: 101, Excuse: 1
        child->execute4(42);        // Counter: 143 (101 + 42)
        child->execute4(7);         // Counter: 150 (143 + 7)

        printf("\nFinal Header -> Counter: %d, FTW: %d, Excuse: %d\n", 
                header.counter, header.ftw, header.is_autonomous_excuse_active);
    }

    free(child);
    return 0;
}
