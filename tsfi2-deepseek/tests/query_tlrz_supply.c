#include <stdio.h>
#include <stdlib.h>
#include "tsfi_hogan.h"

int main(void) {
    char total_supply[256] = {0};
    char balance_of[256] = {0};
    
    // Query TLRz (Tellerz) total supply and balance of the minter contract
    int status = tsfi_hogan_query_token_facts("TLRz", total_supply, sizeof(total_supply), 
                                              balance_of, sizeof(balance_of), 
                                              "0xc15c5F699Daf5e1135732139f05D2c05b3EF4354");
    
    if (status == 0) {
        printf("[SUCCESS] Live TLRz Total Supply retrieved: %s\n", total_supply);
        printf("[SUCCESS] Live TLRz Balance Of Minter: %s\n", balance_of);
    } else {
        printf("[ERROR] Failed to query TLRz facts. Code: %d\n", status);
    }
    return 0;
}
