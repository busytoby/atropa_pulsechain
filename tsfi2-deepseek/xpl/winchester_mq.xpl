/* XplOS WinchesterMQ SCSI Handshake Loop */
/* Emulates low-level hardware handshake transitions and aligns DisplacementShader offsets */

/* 1. Mount virtual hardware register pages */
INIT_RAU

/* 2. Setup WinchesterMQ registers (offsets 0x0500-0x050F) */
REG SCSI_REQ = 0
REG SCSI_ACK = 0
REG SCSI_DATA = 0

/* 3. Setup XDC Debug control word and PKI register */
REG XDC_AUTH = 0
REG DISPLACEMENT_X = 0

/* 4. Perform SCSI Handshake loop */
SCSI_INITIATE:
SCSI_REQ = 1
MEM[0x0500] = SCSI_REQ /* Assert REQ */

SCSI_WAIT_ACK:
SCSI_ACK = MEM[0x0501]
IF (SCSI_ACK == 0) {
    JUMP SCSI_WAIT_ACK /* Loop until ACK assert */
}

/* 5. Transfer data byte and align DisplacementShader */
SCSI_DATA = 0xAA
MEM[0x0502] = SCSI_DATA
DISPLACEMENT_X = SCSI_DATA * 2 /* Vertex scale factor */
MEM[0x0503] = DISPLACEMENT_X

/* 6. Release Handshake */
SCSI_REQ = 0
MEM[0x0500] = SCSI_REQ
