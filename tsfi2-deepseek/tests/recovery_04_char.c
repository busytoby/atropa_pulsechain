#include <stdio.h>
int main() {
    printf("[RECOVERY] Char Size: %zu\n", sizeof(char));
    return (sizeof(char) == 1) ? 0 : 1;
}
