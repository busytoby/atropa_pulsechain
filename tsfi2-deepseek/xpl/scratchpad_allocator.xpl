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
