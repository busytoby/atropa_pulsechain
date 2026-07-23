/* Auncient XPL Ocean Loader for Hudson Soft HuC (huc_ocean_loader.xpl) */

/* 1. Hudson Soft HuC Hardware Register Constants */
DECLARE PSG_BASE           LITERALLY '61696'; /* 0xF100 - PSG Audio channels */
DECLARE VCE_PALETTE_REG    LITERALLY '62208'; /* 0xF300 - VCE Border palette */
DECLARE SCSI_DATA_REG      LITERALLY '64256'; /* 0xFB00 - WinchesterMQ SCSI stream */

/* Ocean Loader Colors */
DECLARE COLOR_RED          LITERALLY '255';   /* 0x00FF */
DECLARE COLOR_CYAN         LITERALLY '65535'; /* 0xFFFF */
DECLARE COLOR_BLUE         LITERALLY '16711680'; /* 0xFF0000 */

/* 2. Split-Raster Border Modulator */
CYCLE_BORDER_RASTER: PROCEDURE(FRAME_COUNT) FIXED;
    DECLARE (FRAME_COUNT, COLOR_VAL) FIXED;
    
    /* Cycle colors sequentially based on frame count */
    IF (FRAME_COUNT % 3) == 0 THEN
        COLOR_VAL = COLOR_RED;
    ELSE IF (FRAME_COUNT % 3) == 1 THEN
        COLOR_VAL = COLOR_CYAN;
    ELSE
        COLOR_VAL = COLOR_BLUE;
    END;
    
    /* Write to HuC VCE border color register */
    BYTE(VCE_PALETTE_REG) = COLOR_VAL;
    
    RETURN COLOR_VAL;
END CYCLE_BORDER_RASTER;

/* 3. PSG Loading Music Chiptune Arpeggiator */
PLAY_LOADER_ARPEGGIO: PROCEDURE(FRAME_COUNT) FIXED;
    DECLARE (FRAME_COUNT, NOTE_INDEX, FREQ) FIXED;
    
    /* C-Major Triad Arpeggio: C (261Hz), E (329Hz), G (392Hz) */
    NOTE_INDEX = FRAME_COUNT % 3;
    
    IF NOTE_INDEX == 0 THEN
        FREQ = 261;
    ELSE IF NOTE_INDEX == 1 THEN
        FREQ = 329;
    ELSE
        FREQ = 392;
    END;
    
    /* Write frequency to HuC PSG Channel 1 */
    BYTE(PSG_BASE) = FREQ;
    
    RETURN FREQ;
END PLAY_LOADER_ARPEGGIO;
