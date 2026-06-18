const fs = require("fs");
const path = require("path");

// ============================================================================
// SPRITE-SPECIALIZED PROGRESSIVE RAG PIPELINE
// ============================================================================

// Simulated database of 500 game sprites/projectiles/enemies
const DATABASE_SIZE = 500;
const SEED = 1337;

// Simple seedable pseudo-random generator
function random(seed) {
    let x = Math.sin(seed) * 10000;
    return x - Math.floor(x);
}

// Generate normalized vector representing a sprite's physics & visual profile
function generateSpriteVector(dimensions, seedOffset) {
    const vec = [];
    for (let i = 0; i < dimensions; i++) {
        vec.push(random(SEED + seedOffset + i) * 2 - 1);
    }
    const mag = Math.sqrt(vec.reduce((sum, v) => sum + v * v, 0));
    return vec.map(v => v / (mag || 1));
}

// Compute cosine similarity between two vectors
function cosineSimilarity(vecA, vecB) {
    let dot = 0.0, magA = 0.0, magB = 0.0;
    for (let i = 0; i < vecA.length; i++) {
        dot += vecA[i] * vecB[i];
        magA += vecA[i] * vecA[i];
        magB += vecB[i] * vecB[i];
    }
    return dot / (Math.sqrt(magA) * Math.sqrt(magB) || 1);
}

// Compute Hamming distance for 16-bit color profile masks
function hammingDistance(mask1, mask2) {
    let diff = mask1 ^ mask2;
    let count = 0;
    while (diff > 0) {
        if (diff & 1) count++;
        diff >>= 1;
    }
    return count;
}

// Generate the database of procedural sprites
const spriteClasses = ["Warrior", "Valkyrie", "Wizard", "Elf", "Troll", "Ghost", "Spawner", "Projectile"];
const spriteDatabase = [];

console.log("===============================================================");
console.log("🧬 INITIALIZING SPRITE-SPECIALIZED PROGRESSIVE RAG DATABASE");
console.log(`Indexing ${DATABASE_SIZE} procedural sprite genome profiles...`);
console.log("===============================================================");

for (let i = 0; i < DATABASE_SIZE; i++) {
    const cls = spriteClasses[Math.floor(random(i + 1) * spriteClasses.length)];
    
    // Generate full-fidelity 512-dim visual/physical embedding
    const highFidelityVec = generateSpriteVector(512, i * 20);
    const midFidelityVec = highFidelityVec.slice(0, 64);
    const matryoshkaVec = highFidelityVec.slice(0, 16);

    // Map high-fidelity vector features to a simulated 12-byte bear/sprite DNA genome
    const fur_r = Math.floor(Math.abs(highFidelityVec[0]) * 255);
    const fur_g = Math.floor(Math.abs(highFidelityVec[1]) * 255);
    const fur_b = Math.floor(Math.abs(highFidelityVec[2]) * 255);
    const sickness = Math.floor(Math.abs(highFidelityVec[3]) * 100);
    const scale = Math.floor(Math.abs(highFidelityVec[4]) * 200 + 50); // Scale range 50-250
    
    const dnaGenome = Buffer.alloc(12);
    dnaGenome.writeUInt8(fur_r, 0);
    dnaGenome.writeUInt8(fur_g, 1);
    dnaGenome.writeUInt8(fur_b, 2);
    dnaGenome.writeUInt8(sickness, 6);
    dnaGenome.writeUInt8(scale, 7);

    // Stage 2: 16-bit color profile mask
    // Bits 0-3: Blue, 4-7: Red, 8-11: Green, 12-15: Dark/Gothic tones
    let colorMask = 0;
    if (fur_b > 120) colorMask |= 0x000F; // Blue dominant
    if (fur_r > 120) colorMask |= 0x00F0; // Red dominant
    if (fur_g > 120) colorMask |= 0x0F00; // Green dominant
    if (sickness > 50) colorMask |= 0xF000; // Sick/Dark tint

    // Stage 1: 8-bit Cluster ID (Class hashing)
    const clusterId = spriteClasses.indexOf(cls);

    // Stage 4: Relation graph coordinates (connected entities)
    const graphNode = {
        id: `sprite_${i}`,
        weapon: cls === "Warrior" ? "Axe" : cls === "Valkyrie" ? "Sword" : cls === "Wizard" ? "Fireball" : cls === "Elf" ? "Arrow" : "Melee"
    };

    spriteDatabase.push({
        id: i,
        name: `${cls}_Genome_${i.toString(16).toUpperCase()}`,
        cls,
        clusterId,
        colorMask,
        matryoshkaVec,
        midFidelityVec,
        highFidelityVec,
        dnaGenome,
        graphNode
    });
}

// RAG Search Pipeline Query
function querySpriteRag(queryText) {
    console.log(`\n🔍 USER QUERY: "${queryText}"`);
    console.log("---------------------------------------------------------------");

    // Natural Language Tokenizer / Query Encoder
    const queryLower = queryText.toLowerCase();
    
    // Default Query Profile
    let targetClassIdx = 0; // Default to Warrior
    if (queryLower.includes("valkyrie") || queryLower.includes("thyra")) targetClassIdx = 1;
    else if (queryLower.includes("wizard") || queryLower.includes("merlin")) targetClassIdx = 2;
    else if (queryLower.includes("elf") || queryLower.includes("questor")) targetClassIdx = 3;
    else if (queryLower.includes("troll") || queryLower.includes("vaesen")) targetClassIdx = 4;
    else if (queryLower.includes("ghost")) targetClassIdx = 5;
    else if (queryLower.includes("spawner")) targetClassIdx = 6;
    else if (queryLower.includes("projectile") || queryLower.includes("axe") || queryLower.includes("missile")) targetClassIdx = 7;

    // Simulated query color extraction
    let queryColorMask = 0;
    if (queryLower.includes("blue")) queryColorMask |= 0x000F;
    if (queryLower.includes("red")) queryColorMask |= 0x00F0;
    if (queryLower.includes("green")) queryColorMask |= 0x0F00;
    if (queryLower.includes("sick") || queryLower.includes("gothic")) queryColorMask |= 0xF000;

    // Encode query vector profile
    const queryHighVec = generateSpriteVector(512, 9999);
    const queryMidVec = queryHighVec.slice(0, 64);
    const queryMatryoshkaVec = queryHighVec.slice(0, 16);

    console.log(`[STAGES 1 & 2] Coarse Filtering by Cluster & Color Profile...`);
    // Pass 1: Filter by exact Cluster ID (Class)
    const stage1Candidates = spriteDatabase.filter(s => s.clusterId === targetClassIdx);
    console.log(`  - Stage 1 (Cluster ID: ${targetClassIdx} (${spriteClasses[targetClassIdx]})): Retrieved ${stage1Candidates.length} sprites.`);

    // Pass 2: Sort by Hamming distance on 16-bit color profile mask
    const stage2Candidates = stage1Candidates.map(s => {
        return { sprite: s, distance: hammingDistance(s.colorMask, queryColorMask) };
    }).sort((a, b) => a.distance - b.distance).slice(0, 15); // Top 15 candidates
    console.log(`  - Stage 2 (Hamming Distance): Reranked top 15 candidates.`);

    console.log(`[STAGE 3] Cosine Similarity on 16-Dimensional Matryoshka Vector...`);
    // Pass 3: Evaluate cosine similarity on matryoshka sub-vector slice
    const stage3Candidates = stage2Candidates.map(c => {
        const sim = cosineSimilarity(c.sprite.matryoshkaVec, queryMatryoshkaVec);
        return { sprite: c.sprite, similarity: sim };
    }).sort((a, b) => b.similarity - a.similarity).slice(0, 5); // Top 5 candidates
    console.log(`  - Stage 3 (Matryoshka Slice): Refined to top 5 candidates.`);

    console.log(`[STAGE 4 & 5] Mid-Fidelity & Graph Relational Verification...`);
    // Pass 4: Evaluate mid-fidelity vector + display graph neighbor logic
    const finalCandidates = stage3Candidates.map(c => {
        const finalSim = cosineSimilarity(c.sprite.highFidelityVec, queryHighVec);
        return { sprite: c.sprite, similarity: finalSim };
    }).sort((a, b) => b.similarity - a.similarity);

    console.log("\n🏆 TOP RETRIEVED GENOMES & REGISTER MAPPINGS:");
    console.log("===============================================================");
    finalCandidates.forEach((c, idx) => {
        const s = c.sprite;
        const fur_r = s.dnaGenome.readUInt8(0);
        const fur_g = s.dnaGenome.readUInt8(1);
        const fur_b = s.dnaGenome.readUInt8(2);
        const sickness = s.dnaGenome.readUInt8(6);
        const scale = s.dnaGenome.readUInt8(7);

        // Convert RGB to Atari Color code (same algorithm as Yul pmgSystem)
        const luma = Math.floor((fur_r + fur_g + fur_b) / 3);
        const hue = (fur_r + fur_g * 2) % 16;
        const atariColor = (hue << 4) | (luma >> 4);

        console.log(`${idx + 1}. Sprite: ${s.name.padEnd(20)} | Similarity: ${c.similarity.toFixed(4)}`);
        console.log(`   - DNA Tracks: RGB(${fur_r},${fur_g},${fur_b}) | Sickness: ${sickness}% | Scale: ${scale}`);
        console.log(`   - PMG Registers: COLPM0=0x${atariColor.toString(16).toUpperCase().padStart(2, '0')} | Weapon: ${s.graphNode.weapon}`);
        console.log("---------------------------------------------------------------");
    });
}

// Run test queries
const userQuery = process.argv[2];
if (userQuery) {
    querySpriteRag(userQuery);
} else {
    querySpriteRag("Retrieve a blue sick troll with high mutation traits");
    querySpriteRag("Fetch a red wizard carrying an active fireball");
}
