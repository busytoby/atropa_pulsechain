#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

// 1. Master Header with 3 Logic Varieties
typedef struct {
    int version;
    char *resonance_as_status; 
    bool ftw; 
    int counter;
    bool is_autonomous_excuse_active;
    void (*execute_logic)(void *self); 
    void (*execute_logic2)(int *counter, bool *ftw);
    void (*execute_logic3)(int *counter, bool *ftw, bool *excuse);
} InternalHeader;

// 2. Mapped Struct with Thunk Pool
#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct name { \
        uint8_t thunk_pool[256]; /* Space for ~10 generalized thunks */ \
        uint8_t *thunk_cursor;   \
        struct { \
            int * const version; \
            char ** const resonance_as_status; \
            bool * const ftw; \
            int * const counter; \
            bool * const is_autonomous_excuse_active; \
            void (*execute)(); \
            void (*execute2)(); \
            void (*execute3)(); \
        }; \
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

// 3. Logic Implementations
void master_logic1(void *ctx) {
    WaveSystem *s = (WaveSystem *)ctx;
    (*s->counter)++;
    printf("[Execute1] ID: %d | Counter: %d\n", s->system_id, *s->counter);
}

void master_logic2(int *cnt, bool *ftw) {
    *cnt += 10;
    *ftw = !(*ftw);
    printf("[Execute2] Counter: %d | FTW Toggled: %d\n", *cnt, *ftw);
}

void master_logic3(int *cnt, bool *ftw, bool *excuse) {
    *cnt += 100;
    *excuse = true;
    printf("[Execute3] Counter: %d | FTW: %d | Excuse Set: %d\n", *cnt, *ftw, *excuse);
}

// 4. The Variadic Thunk Generator
static inline void* emit_thunk(uint8_t **cursor, void *fn, int argc, ...) {
    uint8_t *c = *cursor;
    void *start = (void*)c;
    va_list args;
    va_start(args, argc);

    // Opcodes for MOVABS [IMM64], REG
    // %rdi=bf, %rsi=be, %rdx=ba, %rcx=b9, %r8=b8, %r9=b9
    static const uint8_t arg_regs[] = { 0xbf, 0xbe, 0xba, 0xb9, 0x38, 0x39 };

    for (int i = 0; i < argc && i < 6; i++) {
        *c++ = 0x48; *c++ = arg_regs[i]; 
        *(void**)c = va_arg(args, void*); 
        c += 8;
    }
    va_end(args);

    // movabs fn, %rax; jmp *%rax
    *c++ = 0x48; *c++ = 0xb8; *(void**)c = fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0;

    *cursor = c; // Advance for next attachment
    return start;
}

// 5. Generalized Wiring Macro
#define WIRE_BIJECTION(d, h) ( \
    *(int**)&(d)->version = &(h).version, \
    *(char***)&(d)->resonance_as_status = &(h).resonance_as_status, \
    *(bool**)&(d)->ftw = &(h).ftw, \
    *(int**)&(d)->counter = &(h).counter, \
    *(bool**)&(d)->is_autonomous_excuse_active = &(h).is_autonomous_excuse_active, \
    \
    (d)->execute  = (void(*)())emit_thunk(&(d)->thunk_cursor, (h).execute_logic,  1, (d)), \
    (d)->execute2 = (void(*)())emit_thunk(&(d)->thunk_cursor, (h).execute_logic2, 2, &(h).counter, &(h).ftw), \
    (d)->execute3 = (void(*)())emit_thunk(&(d)->thunk_cursor, (h).execute_logic3, 3, &(h).counter, &(h).ftw, &(h).is_autonomous_excuse_active), \
    \
    mprotect((void*)((uintptr_t)(d) & ~(sysconf(_SC_PAGESIZE) - 1)), sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE | PROT_EXEC) \
)

int main() {
    WaveSystem *child = WaveSystem_create();
    if (!child) return 1;
    child->system_id = 1337;

    InternalHeader header = { 
        .counter = 0, .ftw = false, .is_autonomous_excuse_active = false,
        .execute_logic  = master_logic1,
        .execute_logic2 = master_logic2,
        .execute_logic3 = master_logic3 
    };

    if (WIRE_BIJECTION(child, header) == 0) {
        printf("--- Starting Bijective Execution ---\n");
        
        child->execute();  // Logic 1: Whole struct access
        child->execute2(); // Logic 2: Counter + FTW access
        child->execute3(); // Logic 3: Counter + FTW + Excuse access
        
        printf("--- Execution Complete ---\n");
        printf("Final Header State -> Counter: %d, FTW: %d, Excuse: %d\n", 
                header.counter, header.ftw, header.is_autonomous_excuse_active);
    } else {
        perror("mprotect");
    }

    free(child);
    return 0;
}
