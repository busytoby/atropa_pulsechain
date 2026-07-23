/* Advanced Auncient Hardware Utilities in XPL */

DECLARE SRAM_BASE  LITERALLY '63000'; /* 0xF600 */
DECLARE GEL_BASE   LITERALLY '63232'; /* 0xF700 */
DECLARE INT_BASE   LITERALLY '64896'; /* 0xFD80 */

/* 1. Backup SRAM Integrity Checksum */
SRAM_CHECK: PROCEDURE(START_ADDR, LENGTH) FIXED;
    DECLARE (START_ADDR, LENGTH, I, HASH) FIXED;
    I = 0;
    HASH = 0;
    DO WHILE I < LENGTH;
        HASH = HASH XOR BYTE(START_ADDR + I);
        I = I + 1;
    END;
    RETURN HASH;
END SRAM_CHECK;

/* 2. PC-FX Gel Block DMA Frame Copier */
GEL_DMA: PROCEDURE(SRC_ADDR, DST_ADDR, BLOCKS) FIXED;
    DECLARE (SRC_ADDR, DST_ADDR, BLOCKS, I, VAL) FIXED;
    I = 0;
    DO WHILE I < BLOCKS;
        /* Perform block copy mimicking DMA hardware loops */
        VAL = BYTE(SRC_ADDR + I);
        BYTE(DST_ADDR + I) = VAL;
        I = I + 1;
    END;
    RETURN 1;
END GEL_DMA;

/* 3. Dynamic Interrupt Vector Router */
INT_ROUTE: PROCEDURE(VECTOR_ID, HANDLER_ADDR) FIXED;
    DECLARE (VECTOR_ID, HANDLER_ADDR) FIXED;
    /* Registers target handler address into hardware vector map */
    BYTE(INT_BASE + VECTOR_ID) = HANDLER_ADDR;
    RETURN 1;
END INT_ROUTE;
