#include <stdio.h>
#include <stdlib.h>
int main() {
    void *ptr = NULL;
    printf("[RECOVERY] Pointer State: %p\n", ptr);
    return (ptr == NULL) ? 0 : 1;
}
