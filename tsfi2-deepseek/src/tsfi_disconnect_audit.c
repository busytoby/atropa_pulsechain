#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define DISCONNECT_USER_LEAF 1000
#define DISCONNECT_GEMINI_LEAF 1001
#define MANIFOLD_SIZE (32 * 1024 * 1024)

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <manifold.pos>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open manifold");
        return 1;
    }

    void *map = mmap(NULL, MANIFOLD_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap manifold");
        close(fd);
        return 1;
    }

    uint8_t *user_ptr = (uint8_t *)map + (DISCONNECT_USER_LEAF * 256) + 28;
    uint8_t *gemini_ptr = (uint8_t *)map + (DISCONNECT_GEMINI_LEAF * 256) + 28;

    uint64_t user_balance, gemini_balance;
    memcpy(&user_balance, user_ptr, sizeof(uint64_t));
    memcpy(&gemini_balance, gemini_ptr, sizeof(uint64_t));

    printf("--- DISCONNECT Physical Audit ---\n");
    printf("Node User (1000):   %lu DISCONNECT\n", (unsigned long)user_balance);
    printf("Node Gemini (1001): %lu DISCONNECT\n", (unsigned long)gemini_balance);
    printf("Acoustic Status:    %s\n", (user_balance + gemini_balance > 0) ? "RIGID" : "FRACTURED");

    munmap(map, MANIFOLD_SIZE);
    close(fd);
    return 0;
}
