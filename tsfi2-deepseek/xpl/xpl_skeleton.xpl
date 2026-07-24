/* Auncient XPL System Bootstrap and Routing Skeleton (xpl_skeleton.xpl) */

/* 1. Core Hardware Registers and Memory Layout */
DECLARE TX_BUFFER_BASE     LITERALLY '63000'; /* 0xF600 - Transmit SRAM */
DECLARE RX_BUFFER_BASE     LITERALLY '63100'; /* 0xF664 - Receive SRAM */
DECLARE DEVICE_STATUS      LITERALLY '64000'; /* 0xFA00 - Device State Control */
DECLARE SCSI_HANDSHAKE     LITERALLY '64256'; /* 0xFB00 - SCSI Bus Status */

/* 2. Device Initialization Wrapper */
SETUP_SYSTEM_DEVICE: PROCEDURE(PORT_ID) FIXED;
    DECLARE PORT_ID FIXED;
    
    /* Transition state to CONFIGURING */
    BYTE(DEVICE_STATUS) = 1;
    
    /* Write local Port configuration */
    BYTE(TX_BUFFER_BASE) = PORT_ID;
    
    /* Transition state to ACTIVE */
    BYTE(DEVICE_STATUS) = 2;
    
    RETURN 1;
END SETUP_SYSTEM_DEVICE;

/* 3. Unified Packet Demux and Dispatch Loop */
DISPATCH_INCOMING_PACKET: PROCEDURE(PACKET_ADDR) FIXED;
    DECLARE PACKET_ADDR FIXED;
    DECLARE (ETHER_TYPE, PORT, PAYLOAD_OFFSET) FIXED;
    
    /* Extract EtherType from packet header */
    ETHER_TYPE = BYTE(PACKET_ADDR + 12);
    
    IF ETHER_TYPE == 14130 THEN /* Custom Auncient STANAG: 0x8B32 */
        PORT = BYTE(PACKET_ADDR + 14);
        PAYLOAD_OFFSET = PACKET_ADDR + 16;
        
        /* Dispatch payload to target registered procedure based on Port */
        BYTE(TX_BUFFER_BASE + 10) = BYTE(PAYLOAD_OFFSET);
        RETURN 1;
    END;
    
    IF ETHER_TYPE == 24579 THEN /* DECnet Phase IV: 0x6003 */
        /* Route directly to DECnet point-to-point state machine */
        BYTE(TX_BUFFER_BASE + 20) = BYTE(PACKET_ADDR + 20);
        RETURN 2;
    END;
    
    /* Discard unknown frames */
    RETURN 0;
END DISPATCH_INCOMING_PACKET;
