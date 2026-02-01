#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

// 1. InternalHeader remains the source of truth for data
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

// 2. WaveSystem no longer stores pointers to counter, ftw, etc.
#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct name { \
        uint8_t thunk_pool[512]; /* Increased pool for safety */ \
        uint8_t *thunk_cursor;   \
        void (*execute)(); \
        void (*execute2)(); \
        void (*execute3)(); \
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

// 3. Logic: Receives pointers directly from the thunk injection
void master_logic1(void *ctx) {
    WaveSystem *s = (WaveSystem *)ctx;
    printf("[Execute1] System ID: %d (No direct header access in struct)\n", s->system_id);
}

void master_logic2(int *cnt, bool *ftw) {
    *cnt += 10;
    *ftw = !(*ftw);
    printf("[Execute2] Counter: %d | FTW: %d\n", *cnt, *ftw);
}

void master_logic3(int *cnt, bool *ftw, bool *excuse) {
    *cnt += 100;
    *excuse = true;
    printf("[Execute3] Counter: %d | FTW: %d | Excuse: %d\n", *cnt, *ftw, *excuse);
}

// 4. The Machine Code Generator (x86_64)
static inline void* emit_thunk(uint8_t **cursor, void *fn, int argc, ...) {
    uint8_t *c = *cursor;
    void *start = (void*)c;
    va_list args;
    va_start(args, argc);
    static const uint8_t arg_regs[] = { 0xbf, 0xbe, 0xba, 0xb9, 0xb8, 0xb9 };

    for (int i = 0; i < argc && i < 6; i++) {
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

// 5. Simplified Wiring: Only binds functions to specific Header member addresses
#define WIRE_BIJECTION(d, h) ( \
    (d)->execute  = (void(*)())emit_thunk(&(d)->thunk_cursor, (h).execute_logic,  1, (d)), \
    (d)->execute2 = (void(*)())emit_thunk(&(d)->thunk_cursor, (h).execute_logic2, 2, &(h).counter, &(h).ftw), \
    (d)->execute3 = (void(*)())emit_thunk(&(d)->thunk_cursor, (h).execute_logic3, 3, &(h).counter, &(h).ftw, &(h).is_autonomous_excuse_active), \
    mprotect((void*)((uintptr_t)(d) & ~(sysconf(_SC_PAGESIZE) - 1)), sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE | PROT_EXEC) \
)

int main() {
    WaveSystem *child = WaveSystem_create();
    InternalHeader header = { 
        .counter = 0, .ftw = false, .is_autonomous_excuse_active = false,
        .execute_logic = master_logic1, .execute_logic2 = master_logic2, .execute_logic3 = master_logic3 
    };

    if (child && WIRE_BIJECTION(child, header) == 0) {
        child->system_id = 42;
        
        child->execute();  // Context: &child
        child->execute2(); // Context: &header.counter, &header.ftw
        child->execute3(); // Context: &header.counter, &header.ftw, &header.excuse
        
        printf("\nFinal Header State -> Counter: %d, FTW: %d, Excuse: %d\n", 
                header.counter, header.ftw, header.is_autonomous_excuse_active);
    }

    free(child);
    return 0;
}
