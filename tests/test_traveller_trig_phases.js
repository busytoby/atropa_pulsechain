// Low-level unit tests for Z-Traveller x-quaternion Phase Trigonometries
const assert = require("assert");

console.log("=============================================================");
console.log("Z-Traveller x-quaternion Phase Trigonometry Verification Suite");
console.log("=============================================================");

// Simulated trigonometric state engine
function calculateTravellerModulation(playerX, playerZ, inPhase) {
    const qX = playerX / 300.0;
    const qZ = playerZ / 300.0;

    const basePitch = 120.0;
    const centerFreq = 800.0;

    // Pitch Shift: Modulate pitch based on x-quaternion offset
    const pitchMod = 1.0 + (qX * 0.3);
    const calculatedPitch = basePitch * pitchMod;

    // Formant Shift: Shift formant filters dynamically along the Z traveller trajectory
    // Phase dictates if Z tracks constructively (in phase) or inversely (out of phase) to X
    const phaseMultiplier = inPhase ? 1.0 : -1.0;
    const formantMod = 1.0 + (qZ * phaseMultiplier * 0.2);
    const calculatedFormant = centerFreq * formantMod;

    // Stereo Panner Node: Pan coordinate maps directly to spatial x-position of traveller
    const panVal = Math.max(-1.0, Math.min(1.0, qX));

    return {
        pitch: calculatedPitch,
        formant: calculatedFormant,
        pan: panVal
    };
}

// 1. Test In-Phase Positive Telemetry
console.log("[TEST] Verifying In-Phase Positive Telemetry...");
const res1 = calculateTravellerModulation(150, 150, true);
assert.strictEqual(res1.pan, 0.5, "Pan should map to 0.5");
assert.strictEqual(res1.pitch, 120 * 1.15, "Pitch should be modulated by +15%");
assert.strictEqual(res1.formant, 800 * 1.1, "Formant should be modulated by +10%");

// 2. Test In-Phase Negative Telemetry
console.log("[TEST] Verifying In-Phase Negative Telemetry...");
const res2 = calculateTravellerModulation(-150, -150, true);
assert.strictEqual(res2.pan, -0.5, "Pan should map to -0.5");
assert.strictEqual(res2.pitch, 120 * 0.85, "Pitch should be modulated by -15%");
assert.strictEqual(res2.formant, 800 * 0.9, "Formant should be modulated by -10%");

// 3. Test Out-of-Phase Positive Telemetry
console.log("[TEST] Verifying Out-of-Phase Positive Telemetry...");
const res3 = calculateTravellerModulation(150, 150, false);
assert.strictEqual(res3.pan, 0.5, "Pan should map to 0.5");
assert.strictEqual(res3.pitch, 120 * 1.15, "Pitch should be modulated by +15%");
assert.strictEqual(res3.formant, 800 * 0.9, "Formant should be modulated by -10% due to out of phase shift");

// 4. Test Panning Limits / Saturation
console.log("[TEST] Verifying Panning Limits...");
const resMax = calculateTravellerModulation(450, 0, true);
assert.strictEqual(resMax.pan, 1.0, "Pan should saturate at 1.0");

const resMin = calculateTravellerModulation(-450, 0, true);
assert.strictEqual(resMin.pan, -1.0, "Pan should saturate at -1.0");

console.log("\n=============================================================");
console.log("SUCCESS: All Z-Traveller phase trigonometry unit tests passed!");
console.log("=============================================================");
