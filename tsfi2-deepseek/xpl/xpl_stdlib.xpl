/* Auncient XPL Standard Library and System Interface (xpl_stdlib.xpl) */

/* 1. Standard Virtual Register Constants */
DECLARE PSG_BASE        LITERALLY '61696'; /* 0xF100 */
DECLARE VCE_BASE        LITERALLY '62208'; /* 0xF300 */
DECLARE VRAM_BASE       LITERALLY '62464'; /* 0xF400 */
DECLARE SRAM_BASE       LITERALLY '63000'; /* 0xF600 */
DECLARE SCSI_PHASE      LITERALLY '64256'; /* 0xFB00 */
DECLARE SYSTEM_STATUS   LITERALLY '64513'; /* 0xFC01 - Global Error/Status Register */

/* Standardized Error Codes */
DECLARE SUCCESS                  LITERALLY '1';
DECLARE ERR_GOVERNANCE_VIOLATION LITERALLY '109';
DECLARE ERR_PHYSICS_CONSTRAINT   LITERALLY '110';
DECLARE ERR_SCSI_TIMED_OUT       LITERALLY '111';

/* 2. Standardized System Calls */

VCE_WRITE_PALETTE: PROCEDURE(IDX, PACKED_COLOR) FIXED;
    DECLARE (IDX, PACKED_COLOR) FIXED;
    
    /* Write packed 9-bit color to shadow registers */
    BYTE(VCE_BASE + IDX) = PACKED_COLOR;
    BYTE(SYSTEM_STATUS) = SUCCESS;
    RETURN 1;
END VCE_WRITE_PALETTE;

PSG_SET_VOLUME: PROCEDURE(CHANNEL, VOLUME) FIXED;
    DECLARE (CHANNEL, VOLUME) FIXED;
    DECLARE VOL_ADDR FIXED;
    VOL_ADDR = PSG_BASE + 1 + (CHANNEL * 16);
    
    IF VOLUME > 15 THEN
        /* Limit to 4-bit volume range */
        BYTE(SYSTEM_STATUS) = ERR_PHYSICS_CONSTRAINT;
        RETURN 0;
    END;
    
    BYTE(VOL_ADDR) = VOLUME;
    BYTE(SYSTEM_STATUS) = SUCCESS;
    RETURN 1;
END PSG_SET_VOLUME;

SCSI_EXECUTE_HANDSHAKE: PROCEDURE(TARGET_ID) FIXED;
    DECLARE TARGET_ID FIXED;
    
    /* Set Selection Phase */
    BYTE(SCSI_PHASE) = 1;
    
    /* Simulate WinchesterMQ 6-loop REQ/ACK handshake */
    DECLARE H FIXED;
    H = 0;
    DO WHILE H < 6;
        BYTE(SCSI_PHASE) = 2; /* Data In */
        H = H + 1;
    END;
    
    /* Reset to Bus Free */
    BYTE(SCSI_PHASE) = 0;
    
    BYTE(SYSTEM_STATUS) = SUCCESS;
    RETURN 1;
END SCSI_EXECUTE_HANDSHAKE;
