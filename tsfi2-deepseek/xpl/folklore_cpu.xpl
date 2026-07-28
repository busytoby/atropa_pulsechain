/* XplOS Folklore CPU Simulator Script */
/* Emulates core MOS 6502 instructions inside the RAU */

/* 1. Initialize Coaxial Memory block */
INIT_RAU

/* 2. Setup virtual registers */
REG A = 0
REG X = 0
REG Y = 0
REG PC = 0x0200
REG SP = 0x01FF
REG SR = 0

/* 3. Run CPU instruction loop step */
READ_BYTE
IF (OPCODE == 0xA9) {
    /* LDA Immediate: Load A with byte at PC+1 */
    PC = PC + 1
    A = MEM[PC]
    PC = PC + 1
}

IF (OPCODE == 0xAD) {
    /* LDA Absolute: Load A from 16-bit address at PC+1 */
    PC = PC + 1
    ADDR = MEM[PC]
    A = MEM[ADDR]
    PC = PC + 1
}

IF (OPCODE == 0x8D) {
    /* STA Absolute: Store A to 16-bit address at PC+1 */
    PC = PC + 1
    ADDR = MEM[PC]
    MEM[ADDR] = A
    PC = PC + 1
}

IF (OPCODE == 0x69) {
    /* ADC Immediate: Add byte at PC+1 to A with carry */
    PC = PC + 1
    A = A + MEM[PC]
    PC = PC + 1
}

IF (OPCODE == 0x4C) {
    /* JMP Absolute: Jump to 16-bit address at PC+1 */
    PC = PC + 1
    PC = MEM[PC]
}
