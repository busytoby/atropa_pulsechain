const fs = require("fs");
const path = require("path");

console.log("===============================================================");
console.log("EVOLVING TEDDY BEAR GENOME VIA HAMILTONIAN QUATERNION COMPOSITION");
console.log("===============================================================");

// 1. Hamiltonian Quaternion Multiplication
function multiplyQuaternions(q1, q2) {
    return {
        w: q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
        x: q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        y: q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        z: q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w
    };
}

// Normalize a quaternion to ensure it lies on the 3-sphere
function normalizeQuaternion(q) {
    const len = Math.sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    return {
        w: q.w / len,
        x: q.x / len,
        y: q.y / len,
        z: q.z / len
    };
}

// 2. Project composite quaternion to 12-byte genome
function projectQuaternionToGenome(q) {
    // Ensure inputs are within bounds
    const w = q.w;
    const x = q.x;
    const y = q.y;
    const z = q.z;

    // Mapping formulas:
    const fur_r = Math.floor((w + 1.0) * 127.5);
    const fur_g = Math.floor((x + 1.0) * 127.5);
    const fur_b = Math.floor((y + 1.0) * 127.5);

    const eye_r = Math.floor((Math.sin(z * Math.PI) + 1.0) * 127.5);
    const eye_g = Math.floor((Math.cos(w * Math.PI) + 1.0) * 127.5);
    const eye_b = Math.floor((Math.sin(x * Math.PI) + 1.0) * 127.5);

    const sickness = Math.floor(Math.abs(z) * 100);
    const scale = Math.floor(50 + (1.0 - Math.abs(x)) * 150);
    const fur_length = Math.floor(Math.abs(y) * 255);
    
    // atan2 projected to 0-255
    const angle = Math.atan2(y, x);
    const light_angle = Math.floor(((angle + Math.PI) / (2.0 * Math.PI)) * 255);
    
    const breathing_freq = Math.floor((w * w + z * z) * 255);
    const twitch_intensity = Math.floor((x * x + y * y) * 255);

    return {
        fur_r, fur_g, fur_b,
        eye_r, eye_g, eye_b,
        sickness,
        scale,
        fur_length,
        light_angle,
        breathing_freq,
        twitch_intensity
    };
}

function main() {
    // Define base spaceflight arena orientation (e.g. 45 degrees yaw rotation)
    const angleBase = Math.PI / 4;
    const qBase = normalizeQuaternion({
        w: Math.cos(angleBase / 2),
        x: 0,
        y: Math.sin(angleBase / 2),
        z: 0
    });

    // Define travelling union coordinates (e.g. rotation representing spacecraft roll/pitch)
    const angleTraveller = Math.PI / 6;
    const qTraveller = normalizeQuaternion({
        w: Math.cos(angleTraveller / 2),
        x: Math.sin(angleTraveller / 2),
        y: 0,
        z: 0
    });

    // Compose traveller upon the base space (Hamiltonian composition)
    const qCombined = normalizeQuaternion(multiplyQuaternions(qTraveller, qBase));

    console.log(`[BASE]      qBase:      { w: ${qBase.w.toFixed(4)}, x: ${qBase.x.toFixed(4)}, y: ${qBase.y.toFixed(4)}, z: ${qBase.z.toFixed(4)} }`);
    console.log(`[TRAVELLER] qTraveller: { w: ${qTraveller.w.toFixed(4)}, x: ${qTraveller.x.toFixed(4)}, y: ${qTraveller.y.toFixed(4)}, z: ${qTraveller.z.toFixed(4)} }`);
    console.log(`[COMPOSITE] qCombined:  { w: ${qCombined.w.toFixed(4)}, x: ${qCombined.x.toFixed(4)}, y: ${qCombined.y.toFixed(4)}, z: ${qCombined.z.toFixed(4)} }`);

    // Evolve genome
    const genome = projectQuaternionToGenome(qCombined);
    console.log("\n[GENOME] Evolved 12-Byte Teddy Bear Traits:");
    console.log(`   -> Fur Color (RGB):   [${genome.fur_r}, ${genome.fur_g}, ${genome.fur_b}]`);
    console.log(`   -> Eye Color (RGB):   [${genome.eye_r}, ${genome.eye_g}, ${genome.eye_b}]`);
    console.log(`   -> Sickness:          ${genome.sickness}%`);
    console.log(`   -> Model Scale:       ${genome.scale}`);
    console.log(`   -> Fur Length:        ${genome.fur_length}`);
    console.log(`   -> Light Angle:       ${genome.light_angle}`);
    console.log(`   -> Breathing Freq:    ${genome.breathing_freq}`);
    console.log(`   -> Twitch Intensity:  ${genome.twitch_intensity}`);

    // Serialize to binary assets/bear_genome.dna
    const dnaPath = path.join(__dirname, "../tsfi2-deepseek/assets/bear_genome.dna");
    const buffer = Buffer.alloc(12);
    buffer.writeUInt8(genome.fur_r, 0);
    buffer.writeUInt8(genome.fur_g, 1);
    buffer.writeUInt8(genome.fur_b, 2);
    buffer.writeUInt8(genome.eye_r, 3);
    buffer.writeUInt8(genome.eye_g, 4);
    buffer.writeUInt8(genome.eye_b, 5);
    buffer.writeUInt8(genome.sickness, 6);
    buffer.writeUInt8(genome.scale, 7);
    buffer.writeUInt8(genome.fur_length, 8);
    buffer.writeUInt8(genome.light_angle, 9);
    buffer.writeUInt8(genome.breathing_freq, 10);
    buffer.writeUInt8(genome.twitch_intensity, 11);

    fs.mkdirSync(path.dirname(dnaPath), { recursive: true });
    fs.writeFileSync(dnaPath, buffer);
    console.log(`\nGenome successfully serialized to: ${dnaPath}`);
}

main();
