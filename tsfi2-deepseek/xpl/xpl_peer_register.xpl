/* Auncient XPL DECnet and STANAG Peer Registration Library (xpl_peer_register.xpl) */

/* 1. Register Database Buffer Layout */
DECLARE DB_MAGIC_OFFSET    LITERALLY '0';
DECLARE DB_COUNT_OFFSET    LITERALLY '4';
DECLARE DB_RECORD_SIZE     LITERALLY '8';

/* Address Types */
DECLARE TYPE_DECNET        LITERALLY '1';
DECLARE TYPE_STANAG        LITERALLY '2';

/* 2. Register Peer Entry Procedure */
REGISTER_PEER: PROCEDURE(DB_ADDR, PEER_ID, ADDR_TYPE, ADDR_VAL) FIXED;
    DECLARE (DB_ADDR, PEER_ID, ADDR_TYPE, ADDR_VAL) FIXED;
    DECLARE (PEER_COUNT, RECORD_ADDR) FIXED;
    
    /* Write Magic Header if empty */
    IF BYTE(DB_ADDR + DB_MAGIC_OFFSET) == 0 THEN
        BYTE(DB_ADDR + DB_MAGIC_OFFSET) = 21782; /* 0x5516 Magic */
    END;
    
    /* Read current peer count */
    PEER_COUNT = BYTE(DB_ADDR + DB_COUNT_OFFSET);
    
    /* Calculate registration target record address base */
    RECORD_ADDR = DB_ADDR + 8 + (PEER_COUNT * DB_RECORD_SIZE);
    
    /* Write entry details */
    BYTE(RECORD_ADDR) = PEER_ID;
    BYTE(RECORD_ADDR + 1) = ADDR_TYPE;
    BYTE(RECORD_ADDR + 2) = ADDR_VAL;
    BYTE(RECORD_ADDR + 4) = 0xFFFF; /* Registration Status Flag */
    
    /* Update peer count */
    BYTE(DB_ADDR + DB_COUNT_OFFSET) = PEER_COUNT + 1;
    
    RETURN PEER_COUNT + 1;
END REGISTER_PEER;
