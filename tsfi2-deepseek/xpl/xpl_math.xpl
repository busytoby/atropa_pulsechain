/* Auncient XPL System Math and Hardware Mapping Library (xpl_math.xpl) */

/* 1. Standard Graphic Primitives Map */
DECLARE SPRITE_SIZE   LITERALLY '256'; /* 16 * 16 pixels */
DECLARE TILE_SIZE     LITERALLY '64';  /* 8 * 8 pixels */
DECLARE VRAM_SPRITES  LITERALLY '62464'; /* 0xF400 - VRAM Sprite attribute base */

/* 2. Fixed-Point and Boundary Math Functions */
ABS: PROCEDURE(X) FIXED;
    DECLARE X FIXED;
    IF X < 0 THEN
        RETURN -X;
    END;
    RETURN X;
END ABS;

FP_MUL: PROCEDURE(X, Y, SCALE) FIXED;
    DECLARE (X, Y, SCALE) FIXED;
    RETURN (X * Y) / SCALE;
END FP_MUL;

/* 3. Sprite VRAM Offset Calculator */
GET_SPRITE_VRAM_OFFSET: PROCEDURE(SPRITE_IDX) FIXED;
    DECLARE SPRITE_IDX FIXED;
    /* Calculates memory address offset for standard sprite */
    RETURN VRAM_SPRITES + (SPRITE_IDX * SPRITE_SIZE);
END GET_SPRITE_VRAM_OFFSET;

/* 4. Reserved Interrupt Vector Mapper */
/* Vector 0: Boot, Vector 1: VBlank, Vector 2: HBlank, Vector 3: SCSI */
DECLARE INT_VECTOR_BASE LITERALLY '64896'; /* 0xFD80 */

GET_INTERRUPT_HANDLER: PROCEDURE(VECTOR_ID) FIXED;
    DECLARE VECTOR_ID FIXED;
    RETURN BYTE(INT_VECTOR_BASE + VECTOR_ID);
END GET_INTERRUPT_HANDLER;
