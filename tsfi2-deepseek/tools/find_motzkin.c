#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static bool is_prime(uint64_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int main(void) {
    uint64_t m_prev2 = 1; // M_0
    uint64_t m_prev1 = 1; // M_1
    
    printf("M_0 = 1
");
    printf("M_1 = 1
");
    
    for (uint64_t n = 2; n < 60; n++) {
        // M_n = ((2n+1)*M_{n-1} + (3n-3)*M_{n-2}) / (n+2)
        uint64_t term1 = (2 * n + 1) * m_prev1;
        uint64_t term2 = (3 * n - 3) * m_prev2;
        uint64_t m_curr = (term1 + term2) / (n + 2);
        
        int bits = 0;
        uint64_t temp = m_curr;
        while (temp > 0) {
            bits++;
            temp >>= 1;
        }
        
        if (is_prime(m_curr)) {
            printf("M_%lu = %lu (0x%lX) - %d bits [PRIME]
", n, m_curr, m_curr, bits);
        }
        
        m_prev2 = m_prev1;
        m_prev1 = m_curr;
    }
    
    return 0;
}