// Unit tests for Guarded Port Knocking via Auncient Tunes
const assert = require("assert");

console.log("=============================================================");
console.log("Auncient Guarded Port Knocking (Tune Traps) Verification Suite");
console.log("=============================================================");

const MOTZKIN_PRIME = 953467954114363n;

// Simulated hypervisor/BIOS state machine
const biosState = {
    isLocked: true,
    base: 3n,
    signal: 0n,
    secret: 1782856474572n,
    channel: 0n,
    inputSequence: []
};

// Mathematical operation for Tune transition: Channel = Base^Signal mod MotzkinPrime
function bigModExp(base, exp, mod) {
    let res = 1n;
    base = base % mod;
    while (exp > 0n) {
        if (exp % 2n === 1n) res = (res * base) % mod;
        base = (base * base) % mod;
        exp = exp / 2n;
    }
    return res;
}

function processTuneTransition(signalValue) {
    biosState.signal = BigInt(signalValue);
    // Calculate the transmission channel frequency
    biosState.channel = bigModExp(biosState.base, biosState.signal, MOTZKIN_PRIME);
    
    // Output frequency mapped to Lissajous X frequency divider
    const freqMultiplier = Number(biosState.channel % 12n) + 1;
    
    return { channel: biosState.channel, freqMultiplier };
}

// Port Knocking: plays a sequence of signals ("tune") to unlock vectors
function playNoteOnSASI(noteSignal) {
    biosState.inputSequence.push(noteSignal);
    
    // Process the tune math for the current note
    const result = processTuneTransition(noteSignal);
    
    // The "Target Tune Key" is a sequence of derived frequencies: [Channel1, Channel2, Channel3]
    // Target sequence corresponds to signals: [101, 202, 303]
    const targetSequence = [101, 202, 303];
    
    if (biosState.inputSequence.length === targetSequence.length) {
        let isMatch = true;
        for (let i = 0; i < targetSequence.length; i++) {
            if (biosState.inputSequence[i] !== targetSequence[i]) {
                isMatch = false;
            }
        }
        
        if (isMatch) {
            biosState.isLocked = false; // System Unlocked!
        } else {
            biosState.inputSequence = []; // Reset on wrong sequence
        }
    } else if (biosState.inputSequence.length > targetSequence.length) {
        biosState.inputSequence = [noteSignal]; // Overflow reset
    }
}

function runAllTests() {
    try {
        console.log("1. Verifying basic Tune state transition calculations...");
        const step1 = processTuneTransition(101);
        assert.ok(step1.channel > 0n, "Channel should be calculated");
        console.log(`   ✓ Signal 101 resolved to Channel Frequency: ${step1.channel.toString()} (Lissajous X factor: ${step1.freqMultiplier}x).`);

        console.log("2. Verifying vector access block when system is Locked...");
        // Emulate guest attempting to jump to a hijacked vector
        let vectorAccessAllowed = !biosState.isLocked;
        assert.strictEqual(vectorAccessAllowed, false, "Hypervisor must block vector access while Locked");
        console.log("   ✓ Vector trap successfully blocked unauthorized jump.");

        console.log("3. Simulating incorrect tune sequence (Wrong port knock)...");
        playNoteOnSASI(123);
        playNoteOnSASI(456);
        playNoteOnSASI(789);
        assert.strictEqual(biosState.isLocked, true, "System must remain Locked on wrong tune");
        console.log("   ✓ Access denied for invalid tune sequence.");

        console.log("4. Simulating valid Auncient Tune knock sequence...");
        playNoteOnSASI(101);
        playNoteOnSASI(202);
        playNoteOnSASI(303);
        assert.strictEqual(biosState.isLocked, false, "System must unlock on correct tune sequence");
        console.log("   ✓ Access GRANTED. Hypervisor vector jump tables unlocked.");

        console.log("\n=============================================================");
        console.log("AUNCIENT TUNE-BASED PORT KNOCKING TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE DETECTED:");
        console.error(err);
        process.exit(1);
    }
}

runAllTests();
