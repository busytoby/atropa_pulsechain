const fs = require("fs");
const path = require("path");

const STATE_PATH = "/tmp/tsfi_coaxial_state.json";

function writeCoaxialState(text, colorScheme = -1, shapeType = -1, speedScale = 1.0, bearCount = 3, ttsEnabled = true, tempoScale = 1.0, transposeOffset = 0, persistentTheme = true) {
    const state = {
        tts_enabled: ttsEnabled,
        scroller: {
            text: text,
            color_scheme: colorScheme,
            shape_type: shapeType,
            speed_scale: speedScale,
            bear_count: bearCount
        },
        audio: {
            tempo_scale: tempoScale,
            transpose_offset: transposeOffset,
            persistent_theme: persistentTheme
        }
    };
    
    fs.writeFileSync(STATE_PATH, JSON.stringify(state, null, 4));
    console.log(`[COAXIAL API] Wrote live coaxial state to ${STATE_PATH}:`, JSON.stringify(state));
}

const args = process.argv.slice(2);
if (args.length === 0) {
    console.log("Usage: node scripts/modulate_scroller.js <text> [color_scheme] [shape_type] [speed_scale] [bear_count] [tts_enabled] [tempo_scale] [transpose_offset] [persistent_theme]");
    process.exit(1);
}

const text = args[0];
const colorScheme = args[1] !== undefined ? parseInt(args[1]) : -1;
const shapeType = args[2] !== undefined ? parseInt(args[2]) : -1;
const speedScale = args[3] !== undefined ? parseFloat(args[3]) : 1.0;
const bearCount = args[4] !== undefined ? parseInt(args[4]) : 3;
const ttsEnabled = args[5] !== undefined ? (args[5] === "true" || args[5] === "1") : true;
const tempoScale = args[6] !== undefined ? parseFloat(args[6]) : 1.0;
const transposeOffset = args[7] !== undefined ? parseInt(args[7]) : 0;
const persistentTheme = args[8] !== undefined ? (args[8] === "true" || args[8] === "1") : true;

writeCoaxialState(text, colorScheme, shapeType, speedScale, bearCount, ttsEnabled, tempoScale, transposeOffset, persistentTheme);
