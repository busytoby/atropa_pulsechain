#include "tsfi_block_monitor.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    printf("=== Auncient PulseChain Pricing Router Daemon ===\n");
    printf("Initializing EventMiner Knowledge Graph registry...\n");
    tsfi_block_monitor_init();
    
    TsfiZmmVmState state;
    memset(&state, 0, sizeof(state));
    
    printf("Daemon started. Polling mainnet block index continuously...\n");
    while (1) {
        tsfi_block_monitor_tick(&state);
        sleep(3); // poll every 3 seconds
    }
    return 0;
}
