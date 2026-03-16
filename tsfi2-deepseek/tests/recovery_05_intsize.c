#include <stdio.h>
int main() {
    printf("[RECOVERY] Int Size: %zu\n", sizeof(int));
    return (sizeof(int) >= 4) ? 0 : 1;
}
