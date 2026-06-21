const assert = require("assert");

// Keystroke dynamics and anomaly classifier
class KeystrokeClassifier {
    constructor() {
        this.keyDepressions = {}; // Maps keyName to down timestamp
        this.dwellTimes = [];
        this.flightTimes = [];
        this.lastUpTime = null;
    }

    onKeyDown(keyName, timestamp) {
        this.keyDepressions[keyName] = timestamp;
        if (this.lastUpTime !== null) {
            const flight = timestamp - this.lastUpTime;
            this.flightTimes.push(flight);
        }
    }

    onKeyUp(keyName, timestamp) {
        if (this.keyDepressions[keyName] !== undefined) {
            const dwell = timestamp - this.keyDepressions[keyName];
            this.dwellTimes.push(dwell);
            delete this.keyDepressions[keyName];
        }
        this.lastUpTime = timestamp;
    }

    getStats() {
        const mean = arr => arr.reduce((a, b) => a + b, 0) / (arr.length || 1);
        const stdDev = (arr, avg) => Math.sqrt(arr.map(x => Math.pow(x - avg, 2)).reduce((a, b) => a + b, 0) / (arr.length || 1));

        const avgDwell = mean(this.dwellTimes);
        const stdDwell = stdDev(this.dwellTimes, avgDwell);
        
        const avgFlight = mean(this.flightTimes);
        const stdFlight = stdDev(this.flightTimes, avgFlight);

        return {
            avgDwell,
            stdDwell,
            avgFlight,
            stdFlight
        };
    }

    classify(currentTimestamp = null) {
        // 1. Check for Stuck Key Anomaly
        if (currentTimestamp !== null) {
            for (const key in this.keyDepressions) {
                const heldDuration = currentTimestamp - this.keyDepressions[key];
                if (heldDuration > 3000) { // Held for >3 seconds
                    return "STUCK_KEY_ANOMALY";
                }
            }
        }

        const stats = this.getStats();

        // 2. Check for Contact Chatter/Key Bounce Anomaly
        const chatterCount = this.flightTimes.filter(t => t < 3.0).length;
        if (chatterCount > 3 && stats.avgFlight < 8.0) {
            return "CONTACT_CHATTER_ANOMALY";
        }

        // 3. Bot vs Human classification
        if (stats.stdFlight < 5.0 && stats.avgDwell < 50.0) {
            return "BOT/SYNTHETIC";
        }
        return "HUMAN";
    }
}

function runTests() {
    console.log("=== Auncient Keystroke Dynamics Classifier Unit Test ===");

    // 1. Simulate Human Profile
    const humanClassifier = new KeystrokeClassifier();
    let time = 1000;
    const text = "Auncient";

    for (let i = 0; i < text.length; i++) {
        const char = text[i];
        humanClassifier.onKeyDown(char, time);
        const dwell = 80 + Math.random() * 40;
        time += dwell;
        humanClassifier.onKeyUp(char, time);
        const flight = 100 + Math.random() * 150;
        time += flight;
    }
    const humanResult = humanClassifier.classify(time);
    console.log(`  |- Human Signature: ${humanResult}`);
    assert.strictEqual(humanResult, "HUMAN");

    // 2. Simulate Bot/Synthetic Profile
    const botClassifier = new KeystrokeClassifier();
    time = 1000;
    for (let i = 0; i < text.length; i++) {
        const char = text[i];
        botClassifier.onKeyDown(char, time);
        time += 10; // 10ms dwell
        botClassifier.onKeyUp(char, time);
        time += 50; // 50ms flight
    }
    const botResult = botClassifier.classify(time);
    console.log(`  |- Bot Signature: ${botResult}`);
    assert.strictEqual(botResult, "BOT/SYNTHETIC");

    // 3. Simulate Stuck Key Anomaly
    const stuckClassifier = new KeystrokeClassifier();
    time = 1000;
    stuckClassifier.onKeyDown("Ctrl", time);
    time += 4000; // Key held down for 4 seconds without KeyUp
    const stuckResult = stuckClassifier.classify(time);
    console.log(`  |- Stuck Key Anomaly: ${stuckResult}`);
    assert.strictEqual(stuckResult, "STUCK_KEY_ANOMALY");

    // 4. Simulate Contact Chatter/Key Bounce Anomaly
    const chatterClassifier = new KeystrokeClassifier();
    time = 1000;
    for (let i = 0; i < 5; i++) {
        chatterClassifier.onKeyDown("A", time);
        time += 2.0; // 2ms dwell
        chatterClassifier.onKeyUp("A", time);
        time += 1.5; // 1.5ms bounce flight
    }
    const chatterResult = chatterClassifier.classify(time);
    console.log(`  |- Contact Chatter Anomaly: ${chatterResult}`);
    assert.strictEqual(chatterResult, "CONTACT_CHATTER_ANOMALY");

    console.log("=== [SUCCESS] Extended Keystroke Classifier Unit Tests Passed! ===");
}

runTests();
