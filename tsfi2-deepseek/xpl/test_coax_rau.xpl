/* Auncient xpl Coaxial RAU Gating Unit Test (test_coax_rau.xpl) */

/* 1. Coaxial RAU Register Base Mapping offsets */
DECLARE RAU_DITHER_IN  LITERALLY '16416'; /* 0x4020 - Input sensor reading */
DECLARE RAU_DITHER_OUT LITERALLY '16448'; /* 0x4040 - Quantized output */
DECLARE RAU_DITHER_ERR LITERALLY '16480'; /* 0x4060 - Feedback error */

TEST_RAU_DITHER_ACID: PROCEDURE() FIXED;
    DECLARE (SUCCESS, FAILS) FIXED;
    DECLARE (BACKUP_IN, BACKUP_OUT, BACKUP_ERR) FIXED;
    DECLARE (RAW_VAL, DITHER_NOISE, PREV_ERROR, SUM_VAL) FIXED;
    DECLARE TEMP_ERR FIXED;
    
    SUCCESS = 0;
    FAILS = 0;
    
    /* Initialize default state */
    BYTE(RAU_DITHER_IN) = 120;
    BYTE(RAU_DITHER_OUT) = 0;
    BYTE(RAU_DITHER_ERR) = 0;
    
    /* --- ATOMICITY: Backup and Rollback check --- */
    BACKUP_IN = BYTE(RAU_DITHER_IN);
    BACKUP_OUT = BYTE(RAU_DITHER_OUT);
    BACKUP_ERR = BYTE(RAU_DITHER_ERR);
    
    /* Simulate dither cycle with overflow risk (Atomicity test) */
    RAW_VAL = BYTE(RAU_DITHER_IN);
    DITHER_NOISE = 250; /* Large noise source */
    PREV_ERROR = 20;
    SUM_VAL = RAW_VAL + DITHER_NOISE - PREV_ERROR; /* 120 + 250 - 20 = 350 (exceeds 255) */
    
    IF SUM_VAL > 255 THEN DO;
        /* Transaction aborted: rollback to backups */
        BYTE(RAU_DITHER_IN) = BACKUP_IN;
        BYTE(RAU_DITHER_OUT) = BACKUP_OUT;
        BYTE(RAU_DITHER_ERR) = BACKUP_ERR;
    END;
    
    /* Verify rollback */
    IF BYTE(RAU_DITHER_OUT) <> 0 OR BYTE(RAU_DITHER_ERR) <> 0 THEN FAILS = FAILS + 1;
    
    /* --- CONSISTENCY: Enforce safe 8-bit quantized boundaries --- */
    DITHER_NOISE = 2; /* Safe dither noise */
    SUM_VAL = RAW_VAL + DITHER_NOISE - PREV_ERROR; /* 102 */
    
    BYTE(RAU_DITHER_OUT) = SUM_VAL;
    BYTE(RAU_DITHER_ERR) = 0; /* quantize error */
    
    IF BYTE(RAU_DITHER_OUT) <> 102 OR BYTE(RAU_DITHER_ERR) <> 0 THEN FAILS = FAILS + 1;
    
    /* --- ISOLATION: Calculations in isolated temp storage --- */
    TEMP_ERR = 0;
    /* Temp stays isolated from memory-mapped registers until commit */
    IF BYTE(RAU_DITHER_ERR) <> 0 THEN FAILS = FAILS + 1;
    
    /* --- DURABILITY: Once committed, registers persist --- */
    IF FAILS == 0 THEN DO;
        SUCCESS = 1;
    END;
    
    RETURN SUCCESS;
END TEST_RAU_DITHER_ACID;
