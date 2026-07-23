/* Auncient XPL SCSI, DECnet, and STANAG Network Routing Library (xpl_network.xpl) */

/* 1. Network Register Base Offsets */
DECLARE LOCAL_STACK_BASE   LITERALLY '63744'; /* 0xF900 */
DECLARE SHARED_STACK_BASE  LITERALLY '63000'; /* 0xF600 */
DECLARE SCSI_STATUS_REG    LITERALLY '64256'; /* 0xFB00 */

/* Standard Node IDs */
DECLARE LOCAL_NODE_ID      LITERALLY '5';
DECLARE BRIDGE_NODE_ID     LITERALLY '12';

/* 2. DECnet Frame Routing Procedure */
ROUTE_DECNET_PACKET: PROCEDURE(PACKET_ADDR) FIXED;
    DECLARE PACKET_ADDR FIXED;
    DECLARE (DST_NODE, PAYLOAD_VAL, STATUS) FIXED;
    
    /* Decode Destination Node from packet header (position 1) */
    DST_NODE = BYTE(PACKET_ADDR + 1);
    PAYLOAD_VAL = BYTE(PACKET_ADDR + 2);
    
    IF DST_NODE == LOCAL_NODE_ID THEN
        /* Route directly to Local stack buffer */
        BYTE(LOCAL_STACK_BASE) = PAYLOAD_VAL;
        STATUS = 1;
    ELSE
        /* Route to Shared SRAM stack for DECnet bridge node */
        BYTE(SHARED_STACK_BASE) = PAYLOAD_VAL;
        
        /* Trigger SCSI bus transfer handshake */
        BYTE(SCSI_STATUS_REG) = 2; /* Data Out phase */
        STATUS = 2;
    END;
    
    RETURN STATUS;
END ROUTE_DECNET_PACKET;

/* 3. STANAG 5516 Tactical Frame Encapsulation */
ENCAPSULATE_STANAG: PROCEDURE(RAW_ADDR, FRAME_ADDR) FIXED;
    DECLARE (RAW_ADDR, FRAME_ADDR) FIXED;
    DECLARE (I, CHECKSUM) FIXED;
    
    /* Write STANAG 5516 Magic Identifier */
    BYTE(FRAME_ADDR) = 21782; /* 0x5516 */
    
    /* Copy Source and Destination nodes */
    BYTE(FRAME_ADDR + 2) = BYTE(RAW_ADDR);
    BYTE(FRAME_ADDR + 3) = BYTE(RAW_ADDR + 1);
    
    /* Copy payload data and calculate checksum */
    CHECKSUM = 0;
    I = 0;
    DO WHILE I < 4;
        BYTE(FRAME_ADDR + 4 + I) = BYTE(RAW_ADDR + 2 + I);
        CHECKSUM = CHECKSUM + BYTE(RAW_ADDR + 2 + I);
        I = I + 1;
    END;
    
    /* Write Checksum */
    BYTE(FRAME_ADDR + 8) = CHECKSUM;
    
    RETURN 1;
END ENCAPSULATE_STANAG;
