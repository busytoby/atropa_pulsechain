#include "tsfi_zmm_rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char output[1024];
    tsfi_zmm_rpc_dispatch(NULL, "{\"method\": \"math.motzkin\", \"id\": 1}", output, sizeof(output));
    printf("RPC OUTPUT: %s\n", output);
    return 0;
}
