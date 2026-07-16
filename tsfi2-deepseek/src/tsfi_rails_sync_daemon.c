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
    if (pid > 0) exit(EXIT_SUCCESS); // Parent exits
    
    if (setsid() < 0) exit(EXIT_FAILURE);
    
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // First child exits
    
    umask(0);
    
    // Change to root directory or workspace root
    if (chdir("/home/mariarahel/src/tsfi2/atropa_pulsechain") < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(void) {
    // Initialize daemon process
    daemonize();
    
    // Open log file
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) exit(EXIT_FAILURE);
    
    fprintf(log_fp, "[%s] Sync Daemon successfully started in background.\n", DAEMON_NAME);
    fflush(log_fp);
    
    // Initialize root trie for our BTC DAT 2-3 Tree
    tsfi_trie_node *trie_root = tsfi_trie_create_node('\0');
    
    // Loop polling loop
    while (true) {
        // In a real execution loop, this daemon polls the Anvil JSON-RPC endpoint
        // (similar to the Python listener) to catch event logs and perform:
        // 1. tsfi_trie_insert mappings for new unified address bindings.
        // 2. tsfi_dat_compile and tsfi_dat_save_bin updates to disk.
        // 3. Spawning test_eye_of_the_tiger for audio performance playback.
        
        fprintf(log_fp, "[%s] Listening for BTC Rails transactions...\n", DAEMON_NAME);
        fflush(log_fp);
        
        // Keep polling intervals clean
        sleep(5);
    }
    
    tsfi_trie_destroy(trie_root);
    fclose(log_fp);
    return 0;
}
