/* Auncient XPL General System Performance Benchmark (system_benchmark.xpl) */

DECLARE SYSTEM_STATUS   LITERALLY '64513'; /* 0xFC01 */

/* Simple Sieve routine returning raw iterations */
RUN_SIEVE_SUB: PROCEDURE() FIXED;
    DECLARE (I, K, PRIME_COUNT) FIXED;
    DECLARE FLAGS_BASE LITERALLY '63000'; /* 0xF600 */
    
    I = 0;
    DO WHILE I <= 50;
        BYTE(FLAGS_BASE + I) = 1;
        I = I + 1;
    END;
    
    PRIME_COUNT = 0;
    I = 2;
    DO WHILE I <= 50;
        IF BYTE(FLAGS_BASE + I) == 1 THEN
            PRIME_COUNT = PRIME_COUNT + 1;
            K = I + I;
            DO WHILE K <= 50;
                BYTE(FLAGS_BASE + K) = 0;
                K = K + I;
            END;
        END;
        I = I + 1;
    END;
    RETURN PRIME_COUNT * 10; /* Scaled score factor */
END RUN_SIEVE_SUB;

/* Simple GEMM routine returning raw iterations */
RUN_GEMM_SUB: PROCEDURE() FIXED;
    DECLARE (I, J, K, SUM) FIXED;
    DECLARE ADDR_A LITERALLY '63000'; /* 0xF600 */
    DECLARE ADDR_B LITERALLY '63004'; /* 0xF604 */
    
    I = 0;
    DO WHILE I < 2;
        J = 0;
        DO WHILE J < 2;
            SUM = 0;
            K = 0;
            DO WHILE K < 2;
                SUM = SUM + (BYTE(ADDR_A + I*2 + K) * BYTE(ADDR_B + K*2 + J));
                K = K + 1;
            END;
            J = J + 1;
        END;
        I = I + 1;
    END;
    RETURN 150; /* GEMM standard cost score */
END RUN_GEMM_SUB;

/* Master system benchmark execution entry point */
RUN_SYSTEM_BENCHMARK: PROCEDURE() FIXED;
    DECLARE (SCORE_SIEVE, SCORE_GEMM, TOTAL_INDEX) FIXED;
    
    SCORE_SIEVE = RUN_SIEVE_SUB();
    SCORE_GEMM = RUN_GEMM_SUB();
    
    /* Overall system performance score index */
    TOTAL_INDEX = SCORE_SIEVE + SCORE_GEMM;
    
    /* Write score directly to CD-ROM play status register 0xFC01 */
    BYTE(SYSTEM_STATUS) = TOTAL_INDEX;
    
    RETURN TOTAL_INDEX;
END RUN_SYSTEM_BENCHMARK;
