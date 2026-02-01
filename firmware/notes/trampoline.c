#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// 1. Data Source (Master Header)
typedef struct {
    int counter;
    bool ftw;
    bool is_autonomous_excuse_active;
    void (*logic1)(void *self); 
    void (*logic2)(int *cnt, bool *ftw);
    void (*logic3)(int *cnt, bool *ftw, bool *excuse);
    void (*logic4)(int *cnt, int amount);
} InternalHeader;

// 2. Intermediary Executable Proxy
typedef struct {
    uint8_t *thunk_pool;
    uint8_t *thunk_cursor;
    size_t pool_size;
} ThunkProxy;

static inline ThunkProxy* ThunkProxy_create() {
    size_t pg = sysconf(_SC_PAGESIZE);
    ThunkProxy *proxy = malloc(sizeof(ThunkProxy));
    if (posix_memalign((void**)&proxy->thunk_pool, pg, pg) != 0) return NULL;
    memset(proxy->thunk_pool, 0, pg);
    proxy->thunk_cursor = proxy->thunk_pool;
    proxy->pool_size = pg;
    return proxy;
}

// 3. Behavior Object (Child Struct)
#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct name { \
        ThunkProxy *proxy; \
        void (*execute)(); \
        void (*execute2)(); \
        void (*execute3)(); \
        void (*execute4)(int amount); \
        __VA_ARGS__ \
    } name;

DEFINE_MAPPED_STRUCT(WaveSystem, int system_id;)

// 4. Master Logic Implementations
void master_logic1(void *ctx) { printf("[Logic 1] System ID: %d\n", ((WaveSystem*)ctx)->system_id); }
void master_logic2(int *cnt, bool *ftw) { *cnt += 1; *ftw = !(*ftw); printf("[Logic 2] Cnt: %d | FTW: %d\n", *cnt, *ftw); }
void master_logic3(int *cnt, bool *ftw, bool *exc) { *cnt += 100; *exc = true; printf("[Logic 3] Cnt: %d | Excuse: %d\n", *cnt, *exc); }
void master_logic4(int *cnt, int amount) { *cnt += amount; printf("[Logic 4] +%d | Total: %d\n", amount, *cnt); }

// 5. Specialized Thunk Generators
static inline void* emit_baked(ThunkProxy *p, void *fn, int argc, ...) {
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    va_list args; va_start(args, argc);
    static const uint8_t regs[] = { 0xbf, 0xbe, 0xba };
    for (int i = 0; i < argc; i++) {
        *c++ = 0x48; *c++ = regs[i]; *(void**)c = va_arg(args, void*); c += 8;
    }
    va_end(args);
    *c++ = 0x48; *c++ = 0xb8; *(void**)c = fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0;
    p->thunk_cursor = c; return start;
}

static inline void* emit_mixed(ThunkProxy *p, void *fn, void *baked_ptr) {
    uint8_t *c = p->thunk_cursor; void *start = (void*)c;
    *c++ = 0x89; *c++ = 0xfe; // mov %edi, %esi
    *c++ = 0x48; *c++ = 0xbf; *(void**)c = baked_ptr; c += 8;
    *c++ = 0x48; *c++ = 0xb8; *(void**)c = fn; c += 8;
    *c++ = 0xff; *c++ = 0xe0;
    p->thunk_cursor = c; return start;
}

// 6. Final Wiring Interface
#define WIRE_BIJECTION(d, h, p) ({ \
    (d)->proxy = p; \
    mprotect((p)->thunk_pool, (p)->pool_size, PROT_READ | PROT_WRITE); \
    (d)->execute  = (void(*)())emit_baked(p, (h).logic1, 1, (d)); \
    (d)->execute2 = (void(*)())emit_baked(p, (h).logic2, 2, &(h).counter, &(h).ftw); \
    (d)->execute3 = (void(*)())emit_baked(p, (h).logic3, 3, &(h).counter, &(h).ftw, &(h).is_autonomous_excuse_active); \
    (d)->execute4 = (void(*)(int))emit_mixed(p, (h).logic4, &(h).counter); \
    __builtin___clear_cache((char*)(p)->thunk_pool, (char*)(p)->thunk_cursor); \
    mprotect((p)->thunk_pool, (p)->pool_size, PROT_READ | PROT_EXEC); \
})

// 7. Execution in Main
int main() {
    WaveSystem *child = malloc(sizeof(WaveSystem));
    child->system_id = 42;
    ThunkProxy *proxy = ThunkProxy_create();
    InternalHeader header = { .counter = 0, .ftw = false, .logic1=master_logic1, .logic2=master_logic2, .logic3=master_logic3, .logic4=master_logic4 };

    if (proxy && WIRE_BIJECTION(child, header, proxy) == 0) {
        printf("--- Sequence Start ---\n");
        child->execute();
        child->execute2();
        child->execute3();
        child->execute4(50);
        printf("--- Final Header: Counter=%d, FTW=%d, Excuse=%d ---\n", header.counter, header.ftw, header.is_autonomous_excuse_active);

        mprotect(proxy->thunk_pool, proxy->pool_size, PROT_READ | PROT_WRITE);
        free(proxy->thunk_pool);
        free(proxy);
    }
    free(child);
    return 0;
}
