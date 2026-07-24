/* Auncient XPL Autodin Dormant Client Session controller (autodin_dormant_client.xpl) */

/* 1. Hardware Register and Port Definitions */
DECLARE TX_RING_BASE       LITERALLY '63000'; /* 0xF600 */
DECLARE RX_RING_BASE       LITERALLY '63100'; /* 0xF664 */
DECLARE DEV_STATUS_REG     LITERALLY '64000'; /* 0xFA00 */
DECLARE STANAG_PORT        LITERALLY '2020';

/* 2. Device Initialization Procedure */
INIT_STANAG_DEVICE: PROCEDURE() FIXED;
    /* Write STANAG Port allocation to active registration buffer */
    BYTE(DEV_STATUS_REG) = 1; /* Status: CONFIGURING */
    
    /* Write target port to stack base config register */
    BYTE(TX_RING_BASE) = STANAG_PORT;
    
    /* Update Device Status to ACTIVE */
    BYTE(DEV_STATUS_REG) = 2; /* Status: ACTIVE */
    
    RETURN 1;
END INIT_STANAG_DEVICE;

/* 3. Send Transaction Payload */
TRANSMIT_PAYLOAD: PROCEDURE(DATA_VAL) FIXED;
    DECLARE DATA_VAL FIXED;
    
    /* Place transaction payload in TX ring buffer slot 0 */
    BYTE(TX_RING_BASE + 2) = DATA_VAL;
    
    /* Signal TX Ring update to direct PMD */
    BYTE(DEV_STATUS_REG + 1) = 1; /* Trigger Send */
    
    RETURN 1;
END TRANSMIT_PAYLOAD;

/* 4. Transition to Dormant State */
ENTER_DORMANCY: PROCEDURE() FIXED;
    /* Clear device active flags and save registers to Autodin buffer */
    BYTE(DEV_STATUS_REG) = 3; /* Status: DORMANT */
    
    RETURN 1;
END ENTER_DORMANCY;

/* 5. Resume from Dormancy */
RESUME_CONNECTION: PROCEDURE(RECOVERED_CHIN) FIXED;
    DECLARE RECOVERED_CHIN FIXED;
    
    /* Reconnect device using recovered register states */
    BYTE(TX_RING_BASE + 4) = RECOVERED_CHIN;
    BYTE(DEV_STATUS_REG) = 2; /* Status: ACTIVE */
    
    RETURN 1;
END RESUME_CONNECTION;
