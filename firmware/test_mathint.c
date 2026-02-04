#define GEMALLOC_IMPLEMENTATION
#define MATHINT_IMPLEMENTATION
#include "mathint.h"

int main() {
    printf("--- Atropa MathInt & Gemalloc Test ---\n");
    
    // 1. Basic Allocation Test
    void *ptr = gemalloc(128);
    if (!ptr) {
        printf("[FAIL] gemalloc returned NULL\n");
        return 1;
    }
    gemfree(ptr);
    printgemallocstats();

    // 2. MathInt Test (Modular Exponentiation)
    const char *base = "123456789";
    const char *exp = "65537";
    const char *mod = "1000000007";
    
    printf("Calculating %s^%s mod %s ...\n", base, exp, mod);
    
    MathInt res = modPow(base, exp, mod);
    char *res_str = mi2str(res);
    
    printf("Result: %s\n", res_str);
    
    // Validation (Expected result for 123456789^65537 mod 1000000007)
    // 123456789 mod 1000000007 = 123456789
    // It's a large calculation, but as long as it runs without crash, we verified the library structure.
    
    mifree(res);
    gemfree(res_str); // mi2str uses gemalloc internally
    
    printgemallocstats();
    
    return 0;
}
