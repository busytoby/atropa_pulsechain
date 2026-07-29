/* Auncient XPL GPR Skeleton Layout and Transaction Controller (gpr_skeleton.xpl) */

/* 1. Core General Purpose Register Memory Offsets */
DECLARE GPR_BASE            LITERALLY '65500'; /* Base memory address of GPR skeleton */
DECLARE REG_R0              LITERALLY '65500';
DECLARE REG_R1              LITERALLY '65504';
DECLARE REG_R2              LITERALLY '65508';
DECLARE REG_R3              LITERALLY '65512';
DECLARE REG_R4              LITERALLY '65516';
DECLARE REG_R5              LITERALLY '65520';
DECLARE REG_R6              LITERALLY '65524';
DECLARE REG_R7              LITERALLY '65528';
DECLARE REG_R8              LITERALLY '65532';
DECLARE REG_R9              LITERALLY '65536';
DECLARE REG_R10             LITERALLY '65540';
DECLARE REG_R11             LITERALLY '65544';
DECLARE REG_R12             LITERALLY '65548';
DECLARE REG_R13             LITERALLY '65552';
DECLARE REG_R14             LITERALLY '65556';
DECLARE REG_R15             LITERALLY '65560';

/* 2. Capstan Hardware Registers for Transaction Control */
DECLARE CAPSTAN_CONTROL     LITERALLY '65000';
DECLARE CAPSTAN_SOLENOID    LITERALLY '65001';
DECLARE CAPSTAN_ENCODER     LITERALLY '65002';
DECLARE CAPSTAN_BRAKE       LITERALLY '65003';
DECLARE RAW_HEAD_STATUS     LITERALLY '65004';
DECLARE SECTOR_DATA_REG     LITERALLY '65005';

/* 3. Transactional GPR Initialization and Verification */
INITIALIZE_GPR_SKELETON: PROCEDURE FIXED;
    DECLARE (VERIFY_RETRIES, I, REG_ADDR) FIXED;
    
    VERIFY_RETRIES = 0;
    
    GPR_WRITE_ATTEMPT:
    /* Physical step 1: Engage roller clamp and release mechanical brake */
    BYTE(CAPSTAN_SOLENOID) = 1;
    BYTE(CAPSTAN_BRAKE) = 0;
    BYTE(CAPSTAN_CONTROL) = 1; /* Forward spin */
    
    /* Zero all 16 general purpose registers in memory */
    I = 0;
    DO WHILE I < 16;
        REG_ADDR = GPR_BASE + (I * 4);
        BYTE(REG_ADDR) = 0;
        BYTE(REG_ADDR + 1) = 0;
        BYTE(REG_ADDR + 2) = 0;
        BYTE(REG_ADDR + 3) = 0;
        
        I = I + 1;
    END;
    
    /* Write initialization confirmation log to target tape sector 10 */
    IF BYTE(CAPSTAN_ENCODER) != 10 THEN
        /* Wait for encoder sector alignment */
    END;
    BYTE(SECTOR_DATA_REG) = 0xAA; /* Hex check token */
    
    /* Physical step 6: RAW verification check */
    IF BYTE(RAW_HEAD_STATUS) == 0 THEN
        /* Abort current transaction, brake capstan */
        BYTE(CAPSTAN_CONTROL) = 0;
        BYTE(CAPSTAN_BRAKE) = 1;
        BYTE(CAPSTAN_SOLENOID) = 0;
        
        VERIFY_RETRIES = VERIFY_RETRIES + 1;
        IF VERIFY_RETRIES < 3 THEN
            /* Rewind tape back 1 sector to retry initialization */
            BYTE(CAPSTAN_SOLENOID) = 1;
            BYTE(CAPSTAN_BRAKE) = 0;
            BYTE(CAPSTAN_CONTROL) = 2; /* Reverse motor */
            
            IF BYTE(CAPSTAN_ENCODER) == 9 THEN
                BYTE(CAPSTAN_CONTROL) = 0;
                BYTE(CAPSTAN_BRAKE) = 1;
            END;
            
            GOTO GPR_WRITE_ATTEMPT;
        END;
        
        RETURN 0; /* GPR Transaction failed and rolled back */
    END;
    
    /* Physical step 7: Commit transaction */
    BYTE(CAPSTAN_CONTROL) = 0;
    BYTE(CAPSTAN_BRAKE) = 1;
    BYTE(CAPSTAN_SOLENOID) = 0;
    
    RETURN 1; /* GPR Skeleton committed successfully */
END INITIALIZE_GPR_SKELETON;
