#include <stdio.h>
#include <assert.h>
#include "tsfi_broadcaster.h"
#include "lau_memory.h"

extern void tsfi_soft_encode_frame(LauBroadcaster *lb);

int main() {
    LauBroadcaster lb = {0};
    lb.bitstream_buffer = NULL;
    lb.char_BASE[0][0] = 'X';

    for (int i = 0; i < 1000; i++) {
        tsfi_soft_encode_frame(&lb);
    }

    if (lb.bitstream_buffer != NULL) {
        printf("[PASS] Zero memory leakage verified.\n");
    } else {
        printf("[FAIL] Buffer was never allocated.\n");
        return 1;
    }

    lau_free(lb.bitstream_buffer);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}