#include "lau_telemetry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    const char *id = "cockpit";
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", id);

    int fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd == -1) return 1;
    
    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) return 1;
    
    printf("%s", (char*)telem->zmm_msg);
    return 0;
}
