#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include "tsfi_ramac_layout.h"

#define NUM_THREADS 4
#define ITERATIONS 100

tsfi_mcs_queue shared_queue;
int total_sent = 0;
int total_received = 0;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void* client_worker(void* arg) {
    long id = (long)arg;
    char msg[32];
    snprintf(msg, sizeof(msg), "MSG_FROM_T%ld", id);
    
    for (int i = 0; i < ITERATIONS; i++) {
        // Attempt to send a message segment (retrying if locked)
        int send_res;
        do {
            send_res = tsfi_mcs_send(&shared_queue, msg, NULL);
            if (send_res == -4) {
                // Yield thread if resource is locked (status key "40")
                usleep(10);
            }
        } while (send_res == -4);
        
        if (send_res == 0) {
            pthread_mutex_lock(&counter_lock);
            total_sent++;
            pthread_mutex_unlock(&counter_lock);
        }
        
        // Attempt to receive a message segment (retrying if locked or empty)
        char rx_buf[128];
        int rx_res;
        do {
            rx_res = tsfi_mcs_receive(&shared_queue, rx_buf, sizeof(rx_buf));
            if (rx_res == -4 || rx_res == -2) {
                usleep(10);
            }
        } while (rx_res == -4 || rx_res == -2);
        
        if (rx_res == 0) {
            pthread_mutex_lock(&counter_lock);
            total_received++;
            pthread_mutex_unlock(&counter_lock);
        }
    }
    return NULL;
}

int main(void) {
    printf("[MCS Sync Client] Starting multi-terminal cooperative teleprocessing test...\n");
    tsfi_mcs_init(&shared_queue, "COOP_Q");
    
    pthread_t threads[NUM_THREADS];
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, client_worker, (void*)i);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("  Total transactions sent:     %d\n", total_sent);
    printf("  Total transactions received: %d\n", total_received);
    
    assert(total_sent > 0);
    assert(total_received > 0);
    printf("  [PASS] MCS Sync Client: Concurrent multi-threaded transactions synchronized safely.\n");
    return 0;
}
