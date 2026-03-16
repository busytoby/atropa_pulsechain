#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "tsfi_io.h"
#include "lau_memory.h"

uint32_t g_mem_signal = 0;

void* signal_trigger_thread(void* arg) {
    int mode = *(int*)arg;
    usleep(500000); 
    
    if (mode == 0) {
        printf("[THREAD] Triggering Memory Signal\n");
        g_mem_signal = 1;
    } else {
        printf("[THREAD] Triggering FD Signal\n");
    }
    return NULL;
}

int main() {
    printf("=== TSFi Hybrid Polling Unit Test ===\n");

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) { perror("pipe"); return 1; }
    fcntl(pipe_fds[0], F_SETFL, O_NONBLOCK);

    void *addrs[1];
    addrs[0] = &g_mem_signal;
    uint32_t masks[1];
    masks[0] = 0;

    printf("\n[TEST] Hybrid Select + Thunk\n");
    g_mem_signal = 0;
    int mode = 0;
    pthread_t thread;
    pthread_create(&thread, NULL, signal_trigger_thread, &mode);

    int found = 0;
    for (int i = 0; i < 100; i++) {
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(pipe_fds[0], &rset);
        struct timeval tv = {0, 0};
        
        if (select(pipe_fds[0] + 1, &rset, NULL, NULL, &tv) > 0) {
            printf("[PASS] Detected Pipe Event via Select\n");
            found = 1; break;
        }
        
        if (tsfi_io_wait_wavefront(addrs, masks, 1, 10) == 0) {
            printf("[PASS] Detected Memory Signal via Thunk (Select loop)\n");
            found = 1; break;
        }
    }
    pthread_join(thread, NULL);
    if (!found) { printf("[FAIL] Hybrid Select failed\n"); return 1; }

    printf("\n[TEST] Hybrid Poll + Thunk\n");
    g_mem_signal = 0;
    pthread_create(&thread, NULL, signal_trigger_thread, &mode);

    found = 0;
    for (int i = 0; i < 100; i++) {
        struct pollfd pfd = { pipe_fds[0], POLLIN, 0 };
        if (poll(&pfd, 1, 0) > 0) {
            printf("[PASS] Detected Pipe Event via Poll\n");
            found = 1; break;
        }
        
        if (tsfi_io_wait_wavefront(addrs, masks, 1, 10) == 0) {
            printf("[PASS] Detected Memory Signal via Thunk (Poll loop)\n");
            found = 1; break;
        }
    }
    pthread_join(thread, NULL);
    if (!found) { printf("[FAIL] Hybrid Poll failed\n"); return 1; }

    printf("\n[TEST] Hybrid Epoll + Thunk\n");
    int epfd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = pipe_fds[0];
    epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_fds[0], &ev);

    g_mem_signal = 0;
    pthread_create(&thread, NULL, signal_trigger_thread, &mode);

    found = 0;
    for (int i = 0; i < 100; i++) {
        struct epoll_event events[1];
        if (epoll_wait(epfd, events, 1, 0) > 0) {
            printf("[PASS] Detected Pipe Event via Epoll\n");
            found = 1; break;
        }
        
        if (tsfi_io_wait_wavefront(addrs, masks, 1, 10) == 0) {
            printf("[PASS] Detected Memory Signal via Thunk (Epoll loop)\n");
            found = 1; break;
        }
    }
    pthread_join(thread, NULL);
    if (!found) { printf("[FAIL] Hybrid Epoll failed\n"); return 1; }

    printf("\n[TEST] Hybrid FD Trigger (Select)\n");
    g_mem_signal = 0;
    if (write(pipe_fds[1], "X", 1) < 0) perror("write");

    found = 0;
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(pipe_fds[0], &rset);
    struct timeval tv = {0, 0};
    if (select(pipe_fds[0] + 1, &rset, NULL, NULL, &tv) > 0) {
        printf("[PASS] Immediate FD event detected\n");
        found = 1;
    }
    if (!found) { printf("[FAIL] FD trigger missed\n"); return 1; }

    close(pipe_fds[0]);
    close(pipe_fds[1]);
    close(epfd);

    printf("\n=== Hybrid Polling Tests Passed ===\n");

    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    return 0;
}
