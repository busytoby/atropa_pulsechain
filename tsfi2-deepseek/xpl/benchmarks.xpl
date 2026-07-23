/* Auncient XPL System Standard Performance Benchmarks (benchmarks.xpl) */

/* 1. Sieve of Eratosthenes Benchmark */
/* Counts primes up to 100 using flag array starting at 0xF600 */
SIEVE_BENCH: PROCEDURE() FIXED;
    DECLARE (I, K, PRIME_COUNT) FIXED;
    DECLARE FLAGS_BASE LITERALLY '63000'; /* 0xF600 (SRAM) */
    
    /* Initialize flags to 1 */
    I = 0;
    DO WHILE I <= 100;
        BYTE(FLAGS_BASE + I) = 1;
        I = I + 1;
    END;
    
    PRIME_COUNT = 0;
    I = 2;
    DO WHILE I <= 100;
        IF BYTE(FLAGS_BASE + I) == 1 THEN
            PRIME_COUNT = PRIME_COUNT + 1;
            /* Clear multiples of prime I */
            K = I + I;
            DO WHILE K <= 100;
                BYTE(FLAGS_BASE + K) = 0;
                K = K + I;
            END;
        END;
        I = I + 1;
    END;
    
    RETURN PRIME_COUNT; /* Returns 25 primes (expected for n=100) */
END SIEVE_BENCH;

/* 2. General Matrix Multiplication (GEMM) Benchmark */
/* Performs 2x2 fixed-point matrix multiplication */
GEMM_BENCH: PROCEDURE(A_ADDR, B_ADDR, C_ADDR) FIXED;
    DECLARE (A_ADDR, B_ADDR, C_ADDR) FIXED;
    DECLARE (I, J, K, SUM) FIXED;
    
    I = 0;
    DO WHILE I < 2;
        J = 0;
        DO WHILE J < 2;
            SUM = 0;
            K = 0;
            DO WHILE K < 2;
                SUM = SUM + (BYTE(A_ADDR + (I * 2) + K) * BYTE(B_ADDR + (K * 2) + J));
                K = K + 1;
            END;
            BYTE(C_ADDR + (I * 2) + J) = SUM;
            J = J + 1;
        END;
        I = I + 1;
    END;
    
    RETURN 1; /* Success */
END GEMM_BENCH;

/* 3. Hypergeometric WZ Recurrence Solver Benchmark */
WZ_BENCH: PROCEDURE(STEPS) FIXED;
    DECLARE (STEPS, I, ACC) FIXED;
    ACC = 0;
    I = 0;
    DO WHILE I < STEPS;
        /* Mock WZ verification loop */
        ACC = ACC + (I * 2) - 1;
        I = I + 1;
    END;
    RETURN ACC;
END WZ_BENCH;
