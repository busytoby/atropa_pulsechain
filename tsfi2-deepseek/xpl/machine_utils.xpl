/* Machine-level Auncient Hardware Utilities in XPL */

DECLARE VCE_BASE   LITERALLY '62208'; /* 0xF300 */
DECLARE SCSI_PHASE LITERALLY '64256'; /* 0xFB00 */
DECLARE PSG_BASE    LITERALLY '61696'; /* 0xF100 */

/* 1. Hexadecimal Memory Examiner */
MEM_DUMP: PROCEDURE(START_ADDR, LENGTH) FIXED;
    DECLARE (START_ADDR, LENGTH, I, VAL) FIXED;
    I = 0;
    DO WHILE I < LENGTH;
        VAL = BYTE(START_ADDR + I);
        /* Simulate hex logging to console via XPLSM system call */
        OUTPUT(START_ADDR + I) = VAL;
        I = I + 1;
    END;
    RETURN 1;
END MEM_DUMP;

/* 2. WinchesterMQ SCSI Bus Diagnostic Loop */
SCSI_SCAN: PROCEDURE(TARGET_ID) FIXED;
    DECLARE TARGET_ID FIXED;
    
    /* 1. Selection Phase (set phase register to 1) */
    BYTE(SCSI_PHASE) = 1;
    
    /* 2. Execute SCSI REQ/ACK handshake simulation */
    /* 6 loop cycles required by WinchesterMQ specification */
    DECLARE H FIXED;
    H = 0;
    DO WHILE H < 6;
        /* Toggle REQ/ACK bit state */
        BYTE(SCSI_PHASE) = 2; /* Data In */
        H = H + 1;
    END;
    
    /* 3. Bus Free Phase */
    BYTE(SCSI_PHASE) = 0;
    RETURN 1; /* Success */
END SCSI_SCAN;

/* 3. PSG Wavetable Synthesizer Setup */
PSG_WAVE: PROCEDURE(CHAN_IDX, WAVE_TYPE) FIXED;
    DECLARE (CHAN_IDX, WAVE_TYPE, I) FIXED;
    DECLARE CHAN_OFFSET FIXED;
    CHAN_OFFSET = PSG_BASE + (CHAN_IDX * 16);
    
    I = 0;
    DO WHILE I < 16;
        IF WAVE_TYPE == 0 THEN
            /* Generate Sine wave step values (approx 0-15 scale) */
            BYTE(CHAN_OFFSET + I) = 8 + (7 * SIN(I * 0.39));
        ELSE
            /* Generate Square wave step values (alternating 0 and 15) */
            IF I < 8 THEN
                BYTE(CHAN_OFFSET + I) = 15;
            ELSE
                BYTE(CHAN_OFFSET + I) = 0;
        END;
        I = I + 1;
    END;
    RETURN 1;
END PSG_WAVE;
