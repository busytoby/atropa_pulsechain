#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    int version;
    char *resonance_as_status; 
    bool ftw; 
    int counter;
    bool is_autonomous_excuse_active;
    void (*execute_logic)(void *self); 
    void (*execute_logic2)(int *counter, bool *ftw); // Updated signature
} InternalHeader;

#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct name { \
        uint8_t thunk[32];   \
        uint8_t thunk2[48];  /* Increased size for more instructions */ \
        struct { \
            int * const version; \
            char ** const resonance_as_status; \
            bool * const ftw; \
            int * const counter; \
            bool * const is_autonomous_excuse_active; \
            void (*execute)(); \
            void (*execute2)(); \
        }; \
        __VA_ARGS__ \
    } name; \
    static inline name* name##_create() { \
        name *ptr; \
        size_t pg = sysconf(_SC_PAGESIZE); \
        if (posix_memalign((void**)&ptr, pg, sizeof(name)) != 0) return NULL; \
        return ptr; \
    }

DEFINE_MAPPED_STRUCT(WaveSystem, int system_id;)

void master_system_logic(void *ctx) {
    WaveSystem *self = (WaveSystem *)ctx;
    (*self->counter) += 1;
    printf("[Logic 1] ID: %d | Counter: %d\n", self->system_id, *self->counter);
}

void master_system_logic2(int *counter, bool *ftw) {
    *counter += 10;
    *ftw = !(*ftw); // Toggle the flag
    printf("[Logic 2] Counter: %d | FTW Toggled: %s\n", *counter, *ftw ? "TRUE" : "FALSE");
}

#define WIRE_BIJECTION(d, h) ( \
    *(int**)&(d)->version = &(h).version, \
    *(char***)&(d)->resonance_as_status = &(h).resonance_as_status, \
    *(bool**)&(d)->ftw = &(h).ftw, \
    *(int**)&(d)->counter = &(h).counter, \
    *(bool**)&(d)->is_autonomous_excuse_active = &(h).is_autonomous_excuse_active, \
    \
    /* Thunk 1: %rdi = d */ \
    (d)->thunk[0]=0x48, (d)->thunk[1]=0xbf, *(void**)&((d)->thunk[2])=(void*)(d), \
    (d)->thunk[10]=0x48, (d)->thunk[11]=0xb8, *(void**)&((d)->thunk[12])=(void*)((h).execute_logic), \
    (d)->thunk[20]=0xff, (d)->thunk[21]=0xe0, \
    (d)->execute = (void (*)())(d)->thunk, \
    \
    /* Thunk 2: %rdi = &counter, %rsi = &ftw */ \
    /* movabs &counter, %rdi (48 bf ...) */ \
    (d)->thunk2[0]=0x48, (d)->thunk2[1]=0xbf, *(void**)&((d)->thunk2[2])=(void*)(&(h).counter), \
    /* movabs &ftw, %rsi (48 be ...) */ \
    (d)->thunk2[10]=0x48, (d)->thunk2[11]=0xbe, *(void**)&((d)->thunk2[12])=(void*)(&(h).ftw), \
    /* movabs logic2, %rax (48 b8 ...) */ \
    (d)->thunk2[20]=0x48, (d)->thunk2[21]=0xb8, *(void**)&((d)->thunk2[22])=(void*)((h).execute_logic2), \
    /* jmp %rax */ \
    (d)->thunk2[30]=0xff, (d)->thunk2[31]=0xe0, \
    (d)->execute2 = (void (*)())(d)->thunk2, \
    \
    mprotect((void*)((uintptr_t)(d) & ~(sysconf(_SC_PAGESIZE) - 1)), sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE | PROT_EXEC) \
)

int main() {
    WaveSystem *child = WaveSystem_create();
    InternalHeader header = { 
        .counter = 0, .ftw = false, 
        .execute_logic = master_system_logic, 
        .execute_logic2 = master_system_logic2 
    };

    if (child && WIRE_BIJECTION(child, header) == 0) {
        child->system_id = 123;
        child->execute();  // +1
        child->execute2(); // +10, Toggle FTW
        child->execute2(); // +10, Toggle FTW
        free(child);
    }
    return 0;
}
