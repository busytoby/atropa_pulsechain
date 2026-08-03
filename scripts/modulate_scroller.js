const fs = require("fs");
const path = require("path");

const CONFIG_PATH = "/tmp/scroller_live_config.bin";

function writeLiveConfig(text, colorScheme = -1, shapeType = -1, speedScale = 1.0, bearCount = 3) {
    const buf = Buffer.alloc(32);
    
    // Write helix_text (16 bytes, null-padded)
    const textBuf = Buffer.from(text.substring(0, 15), "utf8");
    textBuf.copy(buf, 0);
    
    // Write color_scheme (4 bytes int32)
    buf.writeInt32LE(colorScheme, 16);
    
    // Write shape_type (4 bytes int32)
    buf.writeInt32LE(shapeType, 20);
    
    // Write speed_scale (4 bytes float)
    buf.writeFloatLE(speedScale, 24);
    
    // Write bear_count (4 bytes int32)
    buf.writeInt32LE(bearCount, 28);
    
    fs.writeFileSync(CONFIG_PATH, buf);
    console.log(`[COAXIAL MODULATION] Wrote live config: Text="${text}", Color=${colorScheme}, Shape=${shapeType}, Speed=${speedScale}, Bears=${bearCount}`);
}

const args = process.argv.slice(2);
if (args.length === 0) {
    console.log("Usage: node scripts/modulate_scroller.js <text> [color_scheme] [shape_type] [speed_scale] [bear_count]");
    process.exit(1);
}

const text = args[0];
const colorScheme = args[1] !== undefined ? parseInt(args[1]) : -1;
const shapeType = args[2] !== undefined ? parseInt(args[2]) : -1;
const speedScale = args[3] !== undefined ? parseFloat(args[3]) : 1.0;
const bearCount = args[4] !== undefined ? parseInt(args[4]) : 3;

writeLiveConfig(text, colorScheme, shapeType, speedScale, bearCount);
