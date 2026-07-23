/* R. W. Bemer ASCII and Escape Sequence Utilities in XPL */

DECLARE TERM_BASE LITERALLY '62208'; /* 0xF300 - VCE terminal display base */

/* 1. ASCII Escape Sequence Router */
ESC_ROUTE: PROCEDURE(STATE, CHAR_VAL) FIXED;
    DECLARE (STATE, CHAR_VAL) FIXED;
    
    /* State 0: Normal character read */
    /* State 1: ESC encountered */
    /* State 2: ESC [ sequence */
    
    IF STATE == 0 THEN
        IF CHAR_VAL == 27 THEN
            RETURN 1; /* Transited to ESC state */
        END;
        RETURN 0;
    END;
    
    IF STATE == 1 THEN
        IF CHAR_VAL == 91 THEN
            RETURN 2; /* Transited to ESC [ state */
        END;
        RETURN 0; /* Reset state */
    END;
    
    IF STATE == 2 THEN
        IF CHAR_VAL == 74 THEN
            /* 'J' character: Trigger screen clear register */
            BYTE(TERM_BASE) = 0;
        END;
        RETURN 0; /* Reset state */
    END;
    
    RETURN 0;
END ESC_ROUTE;

/* 2. ASCII Character Sanitizer */
ASCII_SAN: PROCEDURE(CHAR_VAL) FIXED;
    DECLARE CHAR_VAL FIXED;
    
    /* Standard printable range: 32 (space) to 126 (tilde) */
    IF CHAR_VAL >= 32 THEN
        IF CHAR_VAL <= 126 THEN
            RETURN CHAR_VAL;
        END;
    END;
    
    /* Allow CR and LF Control Characters */
    IF CHAR_VAL == 10 THEN RETURN 10; END;
    IF CHAR_VAL == 13 THEN RETURN 13; END;
    
    RETURN 0; /* Nullify invalid control character */
END ASCII_SAN;

/* 3. Screen Reset Escape Generator */
ESC_RESET: PROCEDURE(TERM_ADDR) FIXED;
    DECLARE TERM_ADDR FIXED;
    
    /* Emits ESC [ 2 J to clean screen register */
    BYTE(TERM_ADDR + 0) = 27; /* ESC */
    BYTE(TERM_ADDR + 1) = 91; /* [ */
    BYTE(TERM_ADDR + 2) = 50; /* 2 */
    BYTE(TERM_ADDR + 3) = 74; /* J */
    
    RETURN 1;
END ESC_RESET;
