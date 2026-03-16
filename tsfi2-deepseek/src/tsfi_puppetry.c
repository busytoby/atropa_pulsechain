#include "tsfi_puppetry.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

TsfiPuppetState* tsfi_puppetry_shm_create() {
    int fd = shm_open(TSFI_PUPPET_SHM, O_RDWR | O_CREAT, 0666);
    if (fd < 0) return NULL;
    size_t size = sizeof(TsfiPuppetState);
    if (ftruncate(fd, size) == -1) { close(fd); return NULL; }
    TsfiPuppetState *state = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (state == MAP_FAILED) return NULL;
    
    memset(state, 0, size);
    state->magic = TSFI_PUPPET_MAGIC;
    state->version = 1;
    state->timestamp_ns = get_time_ns();
    return state;
}

TsfiPuppetState* tsfi_puppetry_shm_attach() {
    int fd = shm_open(TSFI_PUPPET_SHM, O_RDWR, 0666);
    if (fd < 0) return NULL;
    TsfiPuppetState *state = mmap(NULL, sizeof(TsfiPuppetState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (state == MAP_FAILED) return NULL;
    if (state->magic != TSFI_PUPPET_MAGIC) { munmap(state, sizeof(TsfiPuppetState)); return NULL; }
    return state;
}

void tsfi_puppetry_shm_close(TsfiPuppetState *state) {
    if (state) munmap(state, sizeof(TsfiPuppetState));
}
