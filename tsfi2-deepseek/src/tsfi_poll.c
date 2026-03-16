#include "lau_telemetry.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (!id) id = "cockpit";
    
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", id);

    int fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd == -1) return 1;
    
    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) return 1;
    
    if (strlen((char*)telem->request_cmd) > 0) {
        if (((char*)telem->request_cmd)[0] != '{') {
            tsfi_io_printf(stdout, "%s", (char*)telem->request_cmd);
            memset((void*)telem->request_cmd, 0, sizeof(telem->request_cmd));
            return 0;
        }
    }
    
    return 1; // No command
}
