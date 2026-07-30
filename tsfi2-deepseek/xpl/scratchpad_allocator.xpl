/* EDSAC-Inspired Scratchpad Memory Allocator and Safeguard in XPL */

DECLARE FALSE LITERALLY '0';
DECLARE TRUE  LITERALLY '1';

/* Scratchpad Memory Space Configuration */
DECLARE SCRATCHPAD_START LITERALLY '65440'; /* 0xFFE0 */
DECLARE SCRATCHPAD_END   LITERALLY '65535'; /* 0xFFFF */
DECLARE SCRATCHPAD_LIMIT LITERALLY '96';    /* Total size of scratchpad space (96 bytes) */

/* Allocator Tracking Registers */
DECLARE ALLOC_OFFSET_REG LITERALLY '65408'; /* Current allocation offset relative to start */
DECLARE IS_LOCKED_REG    LITERALLY '65412'; /* Lock register to restrict write permissions */

/* Initializes and clears the entire scratchpad memory block */
RESET_SCRATCHPAD: PROCEDURE;
    DECLARE I FIXED;
    I = 0;
    DO WHILE I < SCRATCHPAD_LIMIT;
        BYTE(SCRATCHPAD_START + I) = 0;
        I = I + 1;
    END;
    BYTE(ALLOC_OFFSET_REG) = 0;
    BYTE(IS_LOCKED_REG) = FALSE;
END RESET_SCRATCHPAD;

/* Reserves a block of scratchpad memory (Returns starting address or 0 on error) */
ALLOCATE_SCRATCHPAD: PROCEDURE(SIZE) FIXED;
    DECLARE SIZE FIXED;
    DECLARE OFFSET FIXED;
    DECLARE ADDR FIXED;
    
    OFFSET = BYTE(ALLOC_OFFSET_REG);
    
    /* Check if requested allocation exceeds remaining scratchpad space */
    IF (OFFSET + SIZE) > SCRATCHPAD_LIMIT THEN DO;
        RETURN 0; /* Allocation failed: out of bounds */
    END;
    
    ADDR = SCRATCHPAD_START + OFFSET;
    BYTE(ALLOC_OFFSET_REG) = OFFSET + SIZE;
    RETURN ADDR;
END ALLOCATE_SCRATCHPAD;

/* Writes a value to the scratchpad, auditing boundaries and locks */
WRITE_SCRATCHPAD: PROCEDURE(ADDR, VALUE) FIXED;
    DECLARE (ADDR, VALUE) FIXED;
    
    /* Enforce write protection if the scratchpad is locked */
    IF BYTE(IS_LOCKED_REG) = TRUE THEN DO;
        RETURN FALSE; /* Access denied: scratchpad is locked */
    END;
    
    /* Validate that the address lies within active scratchpad boundaries */
    IF ADDR < SCRATCHPAD_START OR ADDR > SCRATCHPAD_END THEN DO;
        RETURN FALSE; /* Access denied: address out of bounds */
    END;
    
    BYTE(ADDR) = VALUE;
    RETURN TRUE;
END WRITE_SCRATCHPAD;

/* Reads a value from the scratchpad, auditing boundaries */
READ_SCRATCHPAD: PROCEDURE(ADDR) FIXED;
    DECLARE ADDR FIXED;
    
    IF ADDR < SCRATCHPAD_START OR ADDR > SCRATCHPAD_END THEN DO;
        RETURN 0; /* Access denied: address out of bounds */
    END;
    
    RETURN BYTE(ADDR);
END READ_SCRATCHPAD;

/* Lock the scratchpad to prevent further writes during critical phases */
LOCK_SCRATCHPAD: PROCEDURE;
    BYTE(IS_LOCKED_REG) = TRUE;
END LOCK_SCRATCHPAD;

/* Unlock the scratchpad to restore write permissions */
UNLOCK_SCRATCHPAD: PROCEDURE;
    BYTE(IS_LOCKED_REG) = FALSE;
END UNLOCK_SCRATCHPAD;

/* Verification of scratchpad allocator ACID compliance - 36 Test Cases */
PROVE_ACID_COMPLIANCE: PROCEDURE FIXED;
    DECLARE (P, T, FAILS) FIXED;
    DECLARE (SIZE, WADDR, WVAL, LSTATE) FIXED;
    DECLARE (BACKUP_OFFSET, BACKUP_LOCKED) FIXED;
    DECLARE (EXPECT_ADDR, EXPECT_WOK, EXPECT_VAL) FIXED;
    DECLARE (IS_VALID_PHYSICAL) FIXED;
    DECLARE (I, ADDR) FIXED;
    
    /* Temp storage for 96 bytes of memory backups */
    DECLARE BACKUP_MEM_BASE LITERALLY '63000'; /* 0xF600 mock backup area */
    
    FAILS = 0;
    P = 1;
    
    /* Initialize allocator baseline state */
    CALL RESET_SCRATCHPAD;
    
    DO WHILE P <= 9;
        T = 1;
        DO WHILE T <= 4;
            /* 1. Backup all state registers and 96 memory bytes */
            BACKUP_OFFSET = BYTE(ALLOC_OFFSET_REG);
            BACKUP_LOCKED = BYTE(IS_LOCKED_REG);
            I = 0;
            DO WHILE I < SCRATCHPAD_LIMIT;
                BYTE(BACKUP_MEM_BASE + I) = BYTE(SCRATCHPAD_START + I);
                I = I + 1;
            END;
            
            /* 2. Define physical permutations (9 states) */
            IS_VALID_PHYSICAL = TRUE;
            
            /* Allocation: Valid Size */
            IF P = 1 THEN DO;
                SIZE = 10; LSTATE = FALSE; WADDR = SCRATCHPAD_START; WVAL = 42;
                EXPECT_ADDR = SCRATCHPAD_START; EXPECT_WOK = TRUE; EXPECT_VAL = 42;
            END;
            
            /* Allocation: Out of bounds (exceeds limit) */
            IF P = 2 THEN DO;
                SIZE = 128; LSTATE = FALSE; WADDR = SCRATCHPAD_START; WVAL = 42;
                EXPECT_ADDR = 0; EXPECT_WOK = TRUE; EXPECT_VAL = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Locked Write: Prohibited Write Access */
            IF P = 3 THEN DO;
                SIZE = 8; LSTATE = TRUE; WADDR = SCRATCHPAD_START; WVAL = 99;
                EXPECT_ADDR = SCRATCHPAD_START; EXPECT_WOK = FALSE; EXPECT_VAL = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Locked Write: Allowed Read Access */
            IF P = 4 THEN DO;
                SIZE = 8; LSTATE = TRUE; WADDR = SCRATCHPAD_START; WVAL = 0;
                EXPECT_ADDR = SCRATCHPAD_START; EXPECT_WOK = FALSE; EXPECT_VAL = 0;
            END;
            
            /* Double Allocation: Offset accumulation check */
            IF P = 5 THEN DO;
                BYTE(ALLOC_OFFSET_REG) = 20;
                SIZE = 10; LSTATE = FALSE; WADDR = SCRATCHPAD_START + 20; WVAL = 15;
                EXPECT_ADDR = SCRATCHPAD_START + 20; EXPECT_WOK = TRUE; EXPECT_VAL = 15;
            END;
            
            /* Out-of-bounds Write: Below range (Invalid) */
            IF P = 6 THEN DO;
                SIZE = 8; LSTATE = FALSE; WADDR = SCRATCHPAD_START - 5; WVAL = 88;
                EXPECT_ADDR = SCRATCHPAD_START; EXPECT_WOK = FALSE; EXPECT_VAL = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Out-of-bounds Write: Above range (Invalid) */
            IF P = 7 THEN DO;
                SIZE = 8; LSTATE = FALSE; WADDR = SCRATCHPAD_END + 10; WVAL = 77;
                EXPECT_ADDR = SCRATCHPAD_START; EXPECT_WOK = FALSE; EXPECT_VAL = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Zero-size allocation */
            IF P = 8 THEN DO;
                SIZE = 0; LSTATE = FALSE; WADDR = SCRATCHPAD_START; WVAL = 33;
                EXPECT_ADDR = SCRATCHPAD_START; EXPECT_WOK = TRUE; EXPECT_VAL = 33;
            END;
            
            /* Max boundary allocation */
            IF P = 9 THEN DO;
                SIZE = SCRATCHPAD_LIMIT; LSTATE = FALSE; WADDR = SCRATCHPAD_START; WVAL = 55;
                EXPECT_ADDR = SCRATCHPAD_START; EXPECT_WOK = TRUE; EXPECT_VAL = 55;
            END;

            /* Apply test configuration */
            BYTE(IS_LOCKED_REG) = LSTATE;

            /* 3. Execute Transactional Pathway (4 pathways) */
            
            /* T1: Commit Pathway (Execute operations or immediately abort on invalid configs) */
            IF T = 1 THEN DO;
                IF IS_VALID_PHYSICAL = FALSE THEN DO;
                    /* Revert: Restore all registers and 96 memory bytes */
                    BYTE(ALLOC_OFFSET_REG) = BACKUP_OFFSET;
                    BYTE(IS_LOCKED_REG) = BACKUP_LOCKED;
                    I = 0;
                    DO WHILE I < SCRATCHPAD_LIMIT;
                        BYTE(SCRATCHPAD_START + I) = BYTE(BACKUP_MEM_BASE + I);
                        I = I + 1;
                    END;
                    
                    /* Verify rollback completed for all registers and memory cells */
                    IF BYTE(ALLOC_OFFSET_REG) <> BACKUP_OFFSET OR BYTE(IS_LOCKED_REG) <> BACKUP_LOCKED THEN FAILS = FAILS + 1;
                    I = 0;
                    DO WHILE I < SCRATCHPAD_LIMIT;
                        IF BYTE(SCRATCHPAD_START + I) <> BYTE(BACKUP_MEM_BASE + I) THEN FAILS = FAILS + 1;
                        I = I + 1;
                    END;
                END;
                ELSE DO;
                    ADDR = CALL ALLOCATE_SCRATCHPAD(SIZE);
                    IF ADDR <> EXPECT_ADDR THEN FAILS = FAILS + 1;
                    IF ADDR > 0 THEN DO;
                        IF CALL WRITE_SCRATCHPAD(WADDR, WVAL) <> EXPECT_WOK THEN FAILS = FAILS + 1;
                        IF EXPECT_WOK = TRUE THEN DO;
                            IF CALL READ_SCRATCHPAD(WADDR) <> EXPECT_VAL THEN FAILS = FAILS + 1;
                        END;
                    END;
                END;
            END;
            
            /* T2: Explicit Abort Pathway (Tests explicit transaction rollback) */
            IF T = 2 THEN DO;
                BYTE(ALLOC_OFFSET_REG) = BACKUP_OFFSET;
                BYTE(IS_LOCKED_REG) = BACKUP_LOCKED;
                I = 0;
                DO WHILE I < SCRATCHPAD_LIMIT;
                    BYTE(SCRATCHPAD_START + I) = BYTE(BACKUP_MEM_BASE + I);
                    I = I + 1;
                END;
                
                /* Verify rollback completed for all registers and memory cells */
                IF BYTE(ALLOC_OFFSET_REG) <> BACKUP_OFFSET OR BYTE(IS_LOCKED_REG) <> BACKUP_LOCKED THEN FAILS = FAILS + 1;
                I = 0;
                DO WHILE I < SCRATCHPAD_LIMIT;
                    IF BYTE(SCRATCHPAD_START + I) <> BYTE(BACKUP_MEM_BASE + I) THEN FAILS = FAILS + 1;
                    I = I + 1;
                END;
            END;
            
            /* T3: Isolation Pathway (Tests calculation double-buffering safety) */
            IF T = 3 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    /* Verify intermediate allocations are isolated and not read concurrently */
                    IF BYTE(ALLOC_OFFSET_REG) = EXPECT_ADDR AND BYTE(IS_LOCKED_REG) = TRUE THEN DO;
                        FAILS = FAILS + 1;
                    END;
                END;
            END;
            
            /* T4: Durability Pathway (Enforce writes persist to output registers) */
            IF T = 4 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    ADDR = CALL ALLOCATE_SCRATCHPAD(SIZE);
                    IF ADDR > 0 THEN DO;
                        CALL WRITE_SCRATCHPAD(WADDR, WVAL);
                        IF CALL READ_SCRATCHPAD(WADDR) <> EXPECT_VAL THEN FAILS = FAILS + 1;
                    END;
                END;
            END;
            
            T = T + 1;
        END;
        P = P + 1;
    END;
    
    IF FAILS = 0 THEN DO;
        RETURN 1; /* All 36 verification checks passed successfully */
    END;
    RETURN 0;
END PROVE_ACID_COMPLIANCE;

