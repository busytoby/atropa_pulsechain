#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lau_memory.h"

int main() {
    printf("=== TSFi Tech Test: Memory Scrambling Allocator ===\n");
    
    // Allocate a wired object
    void *ptr = lau_malloc_wired(1024);
    if (!ptr) return 1;
    
    printf("[AUDIT] Allocated 1024 bytes (Wired). Pointer: %p\n", ptr);
    
    // Verify magic
    LauWiredHeader *h = (LauWiredHeader*)((char*)ptr - 8192);
    printf("[AUDIT] Header Magic: 0x%x\n", h->footer.magic);
    
    lau_free(ptr);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
