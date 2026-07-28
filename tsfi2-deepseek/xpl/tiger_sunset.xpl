/* XplOS Tiger Sunset Software Rasterizer */
/* Draws a sunset curve into the RAU frame-buffer space */

/* 1. Mount virtual video RAM page */
INIT_RAU

/* 2. Clear video memory (addresses 0x0400 to 0x07FF) */
REG ADDR = 0x0400
LOOP:
IF (ADDR < 0x0800) {
    MEM[ADDR] = 32 /* ASCII Space */
    ADDR = ADDR + 1
    JUMP LOOP
}

/* 3. Draw sunset horizon at Y = 10 (horizontal line) */
REG X = 0
HORIZON_LOOP:
IF (X < 40) {
    /* Offset = 0x0400 + Y*40 + X */
    REG OFFSET = 0x0400 + 400 + X
    MEM[OFFSET] = 61 /* ASCII '=' */
    X = X + 1
    JUMP HORIZON_LOOP
}

/* 4. Draw sun at center (X=20, Y=5) */
MEM[0x0400 + 200 + 20] = 79 /* ASCII 'O' (Sun) */
MEM[0x0400 + 200 + 19] = 40 /* ASCII '(' */
MEM[0x0400 + 200 + 21] = 41 /* ASCII ')' */

/* 5. Draw tiger silhouette near horizon (X=15, Y=9) */
MEM[0x0400 + 360 + 15] = 94 /* ASCII '^' (Tiger ear) */
MEM[0x0400 + 360 + 16] = 94 /* ASCII '^' (Tiger ear) */
MEM[0x0400 + 360 + 17] = 84 /* ASCII 'T' (Tiger body) */
