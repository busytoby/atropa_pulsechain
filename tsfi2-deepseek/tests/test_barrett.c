#include <stdint.h>
#include <stdio.h>

#define TSFI_LIMB_BITS 52
#define TSFI_LIMB_MASK 0xFFFFFFFFFFFFF

static inline uint64_t barrett_mul_scalar(uint64_t a, uint64_t b, uint64_t m, uint64_t mu) {
    unsigned __int128 T = (unsigned __int128)a * b;
    uint64_t T_lo = (uint64_t)T & TSFI_LIMB_MASK;
    uint64_t T_hi = (uint64_t)(T >> TSFI_LIMB_BITS);
    
    // T_0 * mu
    unsigned __int128 T0_mu = (unsigned __int128)T_lo * mu;
    uint64_t tlo_mu_hi = (uint64_t)(T0_mu >> TSFI_LIMB_BITS);
    
    // T_1 * mu + (T_0 * mu) >> 52
    unsigned __int128 T1_mu = (unsigned __int128)T_hi * mu;
    uint64_t thi_mu_lo = (uint64_t)T1_mu & TSFI_LIMB_MASK;
    uint64_t thi_mu_hi = (uint64_t)(T1_mu >> TSFI_LIMB_BITS);
    
    uint64_t mid_sum = tlo_mu_hi + thi_mu_lo;
    uint64_t carry = mid_sum >> TSFI_LIMB_BITS;
    
    uint64_t q = thi_mu_hi + carry;
    
    unsigned __int128 qm = (unsigned __int128)q * m;
    uint64_t qm_lo = (uint64_t)qm & TSFI_LIMB_MASK;
    uint64_t qm_hi = (uint64_t)(qm >> TSFI_LIMB_BITS);
    
    uint64_t sub_lo = (T_lo - qm_lo) & TSFI_LIMB_MASK;
    uint64_t borrow = (T_lo < qm_lo) ? 1 : 0;
    uint64_t sub_hi = T_hi - qm_hi - borrow;
    
    uint64_t res = sub_lo | (sub_hi << TSFI_LIMB_BITS);
    
    int iters = 0;
    while (res >= m) { res -= m; iters++; }
    
    printf("Reduction iters: %d\n", iters);
    return res;
}

int main() {
    uint64_t m = 953467954114362ULL;
    unsigned __int128 two_104 = (unsigned __int128)1 << 104;
    uint64_t mu = (uint64_t)(two_104 / m);
    
    uint64_t a = 123456789012345ULL;
    uint64_t b = 987654321098765ULL;
    
    uint64_t expected = ((unsigned __int128)a * b) % m;
    uint64_t actual = barrett_mul_scalar(a, b, m, mu);
    
    printf("Expected: %lu\n", expected);
    printf("Actual:   %lu\n", actual);
    
    if (expected == actual) printf("PASS\n");
    else printf("FAIL\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
