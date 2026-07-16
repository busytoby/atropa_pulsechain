#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"

#define DAEMON_NAME "tsfi_rails_sync_daemon"
#define LOG_FILE "tmp/tsfi_rails_sync.log"

static void daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    if (setsid() < 0) exit(EXIT_FAILURE);
    
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    umask(0);
    
    if (chdir("/home/mariarahel/src/tsfi2/atropa_pulsechain") < 0) {
        exit(EXIT_FAILURE);
    }
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(void) {
    daemonize();
    
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) exit(EXIT_FAILURE);
    
    fprintf(log_fp, "[%s] Sync Daemon successfully started in background.\n", DAEMON_NAME);
    fflush(log_fp);
    
    tsfi_trie_node *trie_root = tsfi_trie_create_node('\0');
    int loops = 0;
    
    while (loops < 5) {
        fprintf(log_fp, "[%s] Checking for new logs...\n", DAEMON_NAME);
        fflush(log_fp);
        
        // Simulating catching OP_PING_DAEMON (DaemonPing) event:
        fprintf(log_fp, "       [EVENT] Caught DaemonPing event (nonce: %d)\n", loops + 100);
        
        // Simulating generating verification proof via OP_CHALLENGE_STATE:
        char proof_key[128];
        snprintf(proof_key, sizeof(proof_key), "challenge/state_root/nonce_%d", loops + 100);
        tsfi_trie_insert(trie_root, proof_key, "VERIFIED_STATE_PROOF");
        
        // Flush updated tree state to DAT .dat.bin slice
        tsfi_dat *dat = tsfi_dat_compile(trie_root);
        if (dat) {
            tsfi_dat_save_bin(dat, "tmp/test_unified_addr.dat.bin");
            tsfi_dat_destroy(dat);
            fprintf(log_fp, "       [DAT] Flushed verified state proof to tmp/test_unified_addr.dat.bin\n");
        }
        
        fflush(log_fp);
        loops++;
        sleep(2);
    }
    
    fprintf(log_fp, "[%s] Completed verification cycles. Exiting.\n", DAEMON_NAME);
    fclose(log_fp);
    tsfi_trie_destroy(trie_root);
    return 0;
}
