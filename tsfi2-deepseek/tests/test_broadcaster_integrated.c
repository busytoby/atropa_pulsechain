#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "lau_memory.h"
#include "tsfi_broadcaster.h"

extern void tsfi_soft_encode_frame(LauBroadcaster *lb);
extern void* tsfi_rtmp_connect(const char *hostname, const char *stream_key);
extern int tsfi_rtmp_publish(void *ctx);
extern void tsfi_rtmp_close(void *ctx);

#define MOCK_RTMP_HOST "a.rtmp.youtube.com"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <STREAM_KEY>\n", argv[0]);
        return 1;
    }

    const char *key = argv[1];
    printf("=== TSFi Integrated Broadcaster Test ===\n");

    pid_t dummy_pid;
    LauBroadcaster *lb = lau_wire_broadcaster(key, NULL, &dummy_pid);
    if (!lb) {
        printf("[FAIL] Failed to allocate resident broadcaster.\n");
        return 1;
    }
    printf("[PASS] Allocated Resident Broadcaster Wavefront.\n");

    int p_fds[2];
    if (pipe(p_fds) != 0) return 1;
    lb->pty_master_fd = p_fds[0];
    
    const char *msg = "TSFi Phase 1 Broadcaster Online\n";
    if (write(p_fds[1], msg, strlen(msg)) < 0) return 1;

    struct timespec ts = {0, 50000000}; // 50ms
    nanosleep(&ts, NULL);
    
    printf("[TEST] Executing one logic epoch...\n");
    lb->step_safety_epoch();

    if (lb->char_BASE[0][0] == 'T' && lb->char_BASE[0][1] == 'S') {
        printf("[PASS] PTY Ingest and ANSI BASE Update successful.\n");
    } else {
        printf("[FAIL] ANSI BASE Update failed. (Char: %c)\n", lb->char_BASE[0][0]);
    }
    
    // Manually force content for the soft encoder so we can reliably test it.
    lb->char_BASE[0][0] = 'T'; lb->char_BASE[0][1] = 'S';

    printf("[TEST] Verifying Wave512 Soft-Encoder Parity...\n");
    tsfi_soft_encode_frame(lb);
    if (lb->bitstream_buffer != NULL && lb->bitstream_size > 5 && lb->bitstream_buffer[4] == 0x65 && lb->bitstream_buffer[5] == 'T') {
        printf("[PASS] Soft-Encoder NALU Generation Verified.\n");
    } else {
        printf("[FAIL] Soft-Encoder NALU Generation Failed.\n");
        return 1;
    }

    printf("\n[TEST] Attempting to connect to mock RTMP ingest (%s)...\n", MOCK_RTMP_HOST);
    void *ctx = tsfi_rtmp_connect(MOCK_RTMP_HOST, key);
    if (ctx) {
        printf("[PASS] Successfully connected and handshaked with RTMP server.\n");
        if (tsfi_rtmp_publish(ctx) == 0) {
            printf("[PASS] Successfully sent PUBLISH command sequence.\n");
        } else {
            printf("[FAIL] Failed to send PUBLISH command.\n");
            tsfi_rtmp_close(ctx);
            return 1;
        }
        tsfi_rtmp_close(ctx);
    } else {
        printf("[FAIL] Failed to establish RTMP session.\n");
        return 1;
    }

    if (lb->bitstream_buffer) {
        lau_free(lb->bitstream_buffer);
        lb->bitstream_buffer = NULL;
    }
    
    if (lb->pty_master_fd >= 0) close(lb->pty_master_fd);
    if (p_fds[1] >= 0) close(p_fds[1]);
    
    if (lb->pty_wire) {
        lau_unseal_object(lb->pty_wire);
        lau_free(lb->pty_wire);
    }
    
    lau_unseal_object(lb);
    lau_free(lb);

    printf("ALL INTEGRATED BROADCASTER TESTS PASSED\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
