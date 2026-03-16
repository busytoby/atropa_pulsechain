#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/signalfd.h>
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "lau_memory.h"

uint32_t g_telem_val = 0;

typedef struct {
    int pipe_fd;
    int sig_fd;
} TargetData;

void* firmware_stimulator(void* arg) {
    TargetData *td = (TargetData*)arg;
    
    usleep(300000); 
    printf("[STIM] Triggering Memory Update...\n");
    g_telem_val = 100;
    
    usleep(300000);
    printf("[STIM] Triggering Stdin (Pipe)...\n");
    if (write(td->pipe_fd, "HELLO\n", 6) < 0) perror("write");
    
    usleep(300000);
    printf("[STIM] Triggering Signal...\n");
    kill(getpid(), SIGUSR1);
    
    return NULL;
}

int main() {
    printf("=== TSFi Firmware Wait Wavefront Test ===\n");

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) { perror("pipe"); return 1; }
    fcntl(pipe_fds[0], F_SETFL, O_NONBLOCK);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    int sfd = signalfd(-1, &mask, SFD_NONBLOCK);

    TargetData td = { .pipe_fd = pipe_fds[1], .sig_fd = sfd };
    pthread_t thread;
    pthread_create(&thread, NULL, firmware_stimulator, &td);

    int fds[2] = { pipe_fds[0], sfd };
    void* mem_addrs[1] = { &g_telem_val };
    uint32_t mem_masks[1] = { 0 };

    int events_found = 0;
    while (events_found < 3) {
        int res = tsfi_io_wait_universal(fds, 2, mem_addrs, mem_masks, 1, 2000);
        
        if (res == 0) { 
            char buf[64];
            if (read(pipe_fds[0], buf, sizeof(buf)) < 0) perror("read");
            printf("[PASS] Detected Stdin Event: %s", buf);
            events_found++;
        } else if (res == 1) { 
            struct signalfd_siginfo fdsi;
            if (read(sfd, &fdsi, sizeof(fdsi)) < 0) perror("read sig");
            printf("[PASS] Detected Signal Event: %d\n", fdsi.ssi_signo);
            events_found++;
        } else if (res == 2) { 
            printf("[PASS] Detected Memory Event: g_telem_val=%u\n", g_telem_val);
            mem_masks[0] = g_telem_val; 
            events_found++;
        } else if (res == -1) {
            printf("[FAIL] Timeout waiting for events. found=%d\n", events_found);
            break;
        }
    }

    pthread_join(thread, NULL);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    close(sfd);

    if (events_found == 3) {
        printf("=== Firmware Wait Wavefront Passed ===\n");

        extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    return 0;
    }
    return 1;
}
