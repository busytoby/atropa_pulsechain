/* Auncient xpl Coaxial RAU Gating Unit Test (test_coax_rau.xpl) */

/* 1. Coaxial RAU Register Base Mapping offsets */
DECLARE RAU_V1 LITERALLY '16416'; /* 0x4020 - Register V1 */
DECLARE RAU_V2 LITERALLY '16448'; /* 0x4040 - Register V2 */
DECLARE RAU_V3 LITERALLY '16480'; /* 0x4060 - Register V3 */

TEST_RAU_PEEK_POKE: PROCEDURE() FIXED;
    DECLARE SUCCESS FIXED;
    SUCCESS = 0;

    /* Poke values into registers via direct memory-mapped byte writes */
    BYTE(RAU_V1) = 111;
    BYTE(RAU_V2) = 222;

    /* Peek and assert values matches write values */
    IF BYTE(RAU_V1) == 111 THEN
        IF BYTE(RAU_V2) == 222 THEN
            /* Perform a data transfer to V3 register */
            BYTE(RAU_V3) = BYTE(RAU_V1) + BYTE(RAU_V2);
            IF BYTE(RAU_V3) == 77 THEN /* 111 + 222 = 333 (mod 256 for byte) = 77 */
                SUCCESS = 1; /* Test passed */
            END;
        END;
    END;

    RETURN SUCCESS;
END TEST_RAU_PEEK_POKE;
