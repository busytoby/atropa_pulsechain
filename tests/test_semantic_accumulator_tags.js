const assert = require("assert");

console.log("=============================================================");
console.log("Auncient Accumulator Vector Field & Tag Query Test Suite");
console.log("=============================================================");

// Simulated hardware tokenizer registers ($D6E0–$D6E3)
const registers = {
    55008: 0.0, // $D6E0: Semantic Centroid X
    55009: 0.0, // $D6E1: Semantic Centroid Y
    55010: 0,   // $D6E2: Active Category ID
    55011: 0    // $D6E3: Parse Trigger
};

// Preset semantic coordinates for our token/tag attractors
const semanticAttractors = {
    "[winchestermq]": { x: 10.0, y: 5.0, categoryId: 1 },
    "[zmachine]":     { x: -5.0, y: 12.0, categoryId: 2 },
    "[biotika]":      { x: 8.0,  y: -8.0, categoryId: 3 },
    "[joseon]":       { x: -12.0,y: -3.0, categoryId: 4 }
};

// Database of tokenized lore documents with associated tags and vectors
const loreDatabase = [
    {
        id: 101,
        title: "WinchesterMQ SCSI Handshake",
        tags: ["[winchestermq]"],
        vector: { x: 9.8, y: 4.9 }
    },
    {
        id: 102,
        title: "Z-Machine Story Memory Mapping",
        tags: ["[zmachine]"],
        vector: { x: -4.8, y: 11.5 }
    },
    {
        id: 103,
        title: "Biotika Neural Feedback Loops",
        tags: ["[biotika]"],
        vector: { x: 7.9, y: -7.5 }
    },
    {
        id: 104,
        title: "Joseon Historical Lore Map",
        tags: ["[joseon]"],
        vector: { x: -11.5, y: -2.8 }
    }
];

// Parser routine implementing the Accumulator Vector Field logic
function parseTokenWithAccumulator(token) {
    const normalized = token.toLowerCase();
    const attractor = semanticAttractors[normalized];
    
    if (attractor) {
        // Trigger register write: pull the centroid toward the attractor
        registers[55008] = attractor.x;
        registers[55009] = attractor.y;
        registers[55010] = attractor.categoryId;
        registers[55011] = 1; // Parse Trigger = 1
        return true;
    }
    return false;
}

// Relational query function matching the accumulator state to the lore database
function queryLoreWithAccumulator() {
    const activeCategory = registers[55010];
    const cx = registers[55008];
    const cy = registers[55009];

    // Filter database by the active category first (opt-in pre-filtering)
    const filtered = loreDatabase.filter(doc => {
        const matchingAttractor = Object.keys(semanticAttractors).find(key => 
            semanticAttractors[key].categoryId === activeCategory
        );
        return doc.tags.includes(matchingAttractor);
    });

    // Rank matching documents using Euclidean distance to the semantic centroid (Cx, Cy)
    let bestDoc = null;
    let closestDistance = Infinity;

    for (const doc of filtered) {
        const dx = doc.vector.x - cx;
        const dy = doc.vector.y - cy;
        const distance = Math.sqrt(dx * dx + dy * dy);
        
        if (distance < closestDistance) {
            closestDistance = distance;
            bestDoc = doc;
        }
    }
    return bestDoc;
}

function runTests() {
    try {
        console.log("1. Parsing opt-in TAG query '[WinchesterMQ]'...");
        const parseOk = parseTokenWithAccumulator("[WinchesterMQ]");
        assert.strictEqual(parseOk, true, "Tag should be recognized as a valid attractor");
        assert.strictEqual(registers[55008], 10.0, "Centroid X should match attractor coordinates");
        assert.strictEqual(registers[55009], 5.0, "Centroid Y should match attractor coordinates");
        assert.strictEqual(registers[55010], 1, "Active Category ID should be set to 1");
        console.log("   ✓ Centroid registers updated correctly.");

        console.log("2. Running similarity query against the lore database...");
        const match = queryLoreWithAccumulator();
        assert.ok(match, "Should find a matching document");
        assert.strictEqual(match.id, 101, "Should match WinchesterMQ SCSI Handshake");
        console.log(`   ✓ Correct lore document resolved: "${match.title}" (ID: ${match.id})`);

        console.log("3. Parsing opt-in TAG query '[Biotika]'...");
        parseTokenWithAccumulator("[Biotika]");
        assert.strictEqual(registers[55010], 3, "Active Category ID should change to 3");
        const matchBiotika = queryLoreWithAccumulator();
        assert.strictEqual(matchBiotika.id, 103, "Should match Biotika Neural Feedback Loops");
        console.log(`   ✓ Correct lore document resolved: "${matchBiotika.title}" (ID: ${matchBiotika.id})`);

        console.log("\n=============================================================");
        console.log("AUNCIENT TAG ACCUMULATOR TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE DETECTED:");
        console.error(err);
        process.exit(1);
    }
}

runTests();
