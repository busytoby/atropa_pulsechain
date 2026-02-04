#ifndef LAU_MEMORY_H
#define LAU_MEMORY_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdalign.h>
#include "lau_thunk.h"

#define DLE 16
#define LAU_SCRAMBLE_MODE_RANDOM DLE

typedef enum {
    LAU_TYPE_BASIC = 0x10,
    LAU_TYPE_WIRED = 0x20
} LauHeaderType;

// Packed Footer (8 bytes)
// Layout: [Magic(1)][Type(1)][Line(2)][Checksum(4)]
typedef struct {
    uint8_t magic;
    uint8_t type;
    uint16_t alloc_line;
    uint32_t checksum;
} LauFooter;

// Metadata (40 bytes)
typedef struct LauMetadata {
    struct LauMetadata *next;
    struct LauMetadata *prev;
    const char *alloc_file;
    size_t alloc_size;
    void *actual_start;
} LauMetadata;

// Basic Header (48 bytes, 16-byte aligned)
typedef struct {
    LauMetadata meta;
    LauFooter footer;
} LauHeader;

// Wired Header (Full Logic)
// Meta(40) + Fields + Footer(8) -> Must align to 16
typedef struct {
    LauMetadata meta;
    
    // Logic & State
    ThunkProxy *proxy;
    int version;
    char *resonance_as_status; 
    int counter;
    bool sealed;
    bool ftw; 
    bool is_autonomous_excuse_active;
    uint8_t _pad[5]; // Pad to align pointers
    
    void (*logic_epoch)(int*);
    void (*logic_state)(bool*);
    void (*logic_directive)(int*, char*);
    void (*logic_scramble)(void*);
    void (*logic_provenance)(void*);

    // Padding ensures Footer ends at 16-byte boundary
    // Current size calc: 40 + 8+4+8+4+1+1+1+5(pad=32) + 40(5 ptrs) = 112. 
    // + 8 (Footer) = 120. 
    // 120 is not 16-byte aligned (120/16 = 7.5).
    // We need 8 bytes padding before footer.
    uint64_t _align_pad;

    LauFooter footer;
} LauWiredHeader;

// Use LauWiredHeader as the "SystemHeader"
typedef LauWiredHeader LauSystemHeader;

// Macro to recover the Payload pointer from a field inside LauWiredHeader (e.g. version)
#define LAU_GET_SYSTEM(field_ptr, field_name, system_type) \
    ((system_type*)((char*)(field_ptr) - offsetof(LauWiredHeader, field_name) + sizeof(LauWiredHeader)))

void *lau_malloc_loc(size_t size, const char *file, int line);
void *lau_malloc_wired_loc(size_t size, const char *file, int line); // New
void *lau_memalign_wired_loc(size_t alignment, size_t size, const char *file, int line);
void *lau_memalign_loc(size_t alignment, size_t size, const char *file, int line);
void *lau_realloc_loc(void *ptr, size_t size, const char *file, int line);
char *lau_strdup_loc(const char *s, const char *file, int line);

#define lau_malloc(size) lau_malloc_loc(size, __FILE__, __LINE__)
#define lau_malloc_wired(size) lau_malloc_wired_loc(size, __FILE__, __LINE__)
#define lau_memalign_wired(align, size) lau_memalign_wired_loc(align, size, __FILE__, __LINE__)
#define lau_memalign(align, size) lau_memalign_loc(align, size, __FILE__, __LINE__)
#define lau_realloc(ptr, size) lau_realloc_loc(ptr, size, __FILE__, __LINE__)
#define lau_strdup(s) lau_strdup_loc(s, __FILE__, __LINE__)

void lau_free(void *ptr);
void lau_quarantine_drain(void); // New
void lau_seal_object(void *ptr);
void lau_report_memory_metrics(void);
void lau_report_memory_to_log(void); // New: Write to lau_audit.log
void lau_mem_scramble(void* ptr, size_t size, char set);

size_t lau_get_active_count(void); // New: Accessor for tracking
size_t lau_get_max_active_bytes(void); // New: Accessor for peak usage

// Hausdorff Operator: Verify distinctness and validity
bool lau_verify_hausdorff_property(void *ptr);

// Magic constants
#define LAU_MAGIC 0x42
#define LAU_MAGIC_FREED 0xDEAD

#endif // LAU_MEMORY_H