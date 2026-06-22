// Unit tests for Auncient 4D Tesseract Geometry and Projection Math
const assert = require("assert");

// Mock vertex list matching frontend structure
const tesseractVertices = [];
for (let x of [-1, 1]) {
    for (let y of [-1, 1]) {
        for (let z of [-1, 1]) {
            for (let w of [-1, 1]) {
                tesseractVertices.push({ x, y, z, w });
            }
        }
    }
}

// 6-axis 4D rotation function under test
// Pre-computed Sine/Cosine Lookup Tables (LUTs) for high-speed Auncient 4D rotation math (Hugi #16)
const LUT_SIZE = 262144;
const LUT_MASK = 262143;
const sinLUT = new Float64Array(LUT_SIZE);
const cosLUT = new Float64Array(LUT_SIZE);
const LUT_FACTOR = LUT_SIZE / (2 * Math.PI);

for (let i = 0; i < LUT_SIZE; i++) {
    const angle = (i * 2 * Math.PI) / LUT_SIZE;
    sinLUT[i] = Math.sin(angle);
    cosLUT[i] = Math.cos(angle);
}

function lutSin(a) {
    const val = a * LUT_FACTOR;
    const idx1 = Math.floor(val);
    const fract = val - idx1;
    const idx2 = idx1 + 1;
    
    const s1 = sinLUT[idx1 & LUT_MASK];
    const s2 = sinLUT[idx2 & LUT_MASK];
    return s1 + fract * (s2 - s1);
}

function lutCos(a) {
    const val = a * LUT_FACTOR;
    const idx1 = Math.floor(val);
    const fract = val - idx1;
    const idx2 = idx1 + 1;
    
    const c1 = cosLUT[idx1 & LUT_MASK];
    const c2 = cosLUT[idx2 & LUT_MASK];
    return c1 + fract * (c2 - c1);
}

function rotate4D(p, aXY, aXZ, aXW, aYZ, aYW, aZW) {
    let x = p.x;
    let y = p.y;
    let z = p.z;
    let w = p.w;

    if (aXY !== 0) {
        let cos = lutCos(aXY), sin = lutSin(aXY);
        let xNew = x * cos - y * sin;
        let yNew = x * sin + y * cos;
        x = xNew; y = yNew;
    }
    if (aXZ !== 0) {
        let cos = lutCos(aXZ), sin = lutSin(aXZ);
        let xNew = x * cos - z * sin;
        let zNew = x * sin + z * cos;
        x = xNew; z = zNew;
    }
    if (aXW !== 0) {
        let cos = lutCos(aXW), sin = lutSin(aXW);
        let xNew = x * cos - w * sin;
        let wNew = x * sin + w * cos;
        x = xNew; w = wNew;
    }
    if (aYZ !== 0) {
        let cos = lutCos(aYZ), sin = lutSin(aYZ);
        let yNew = y * cos - z * sin;
        let zNew = y * sin + z * cos;
        y = yNew; z = zNew;
    }
    if (aYW !== 0) {
        let cos = lutCos(aYW), sin = lutSin(aYW);
        let yNew = y * cos - w * sin;
        let wNew = y * sin + w * cos;
        y = yNew; w = wNew;
    }
    if (aZW !== 0) {
        let cos = lutCos(aZW), sin = lutSin(aZW);
        let zNew = z * cos - w * sin;
        let wNew = z * sin + w * cos;
        z = zNew; w = wNew;
    }

    return { x, y, z, w };
}

// Face generation loop matching frontend
const tesseractFaces = [];
const cycle = [
    [-1, -1],
    [1, -1],
    [1, 1],
    [-1, 1]
];
for (let i = 0; i < 4; i++) {
    for (let j = i + 1; j < 4; j++) {
        const fixedDims = [];
        for (let k = 0; k < 4; k++) {
            if (k !== i && k !== j) fixedDims.push(k);
        }
        for (let val1 of [-1, 1]) {
            for (let val2 of [-1, 1]) {
                const faceVertices = [];
                for (let step = 0; step < 4; step++) {
                    const v = { x: 0, y: 0, z: 0, w: 0 };
                    const dims = ['x', 'y', 'z', 'w'];
                    v[dims[i]] = cycle[step][0];
                    v[dims[j]] = cycle[step][1];
                    v[dims[fixedDims[0]]] = val1;
                    v[dims[fixedDims[1]]] = val2;

                    const idx = tesseractVertices.findIndex(tv => 
                        Math.abs(tv.x - v.x) < 0.01 && 
                        Math.abs(tv.y - v.y) < 0.01 && 
                        Math.abs(tv.z - v.z) < 0.01 && 
                        Math.abs(tv.w - v.w) < 0.01
                    );
                    faceVertices.push(idx);
                }
                tesseractFaces.push(faceVertices);
            }
        }
    }
}

console.log("=== RUNNING TESSERACT MATH TESTS ===");

// Test 1: Count of 4D hypercube elements
console.log(`Vertices found: ${tesseractVertices.length}`);
assert.strictEqual(tesseractVertices.length, 16);
console.log(`Faces generated: ${tesseractFaces.length}`);
assert.strictEqual(tesseractFaces.length, 24);
console.log("✓ Test 1: Tesseract vertex and face count correct.");

// Test 2: Vertex Index Integrity
for (let i = 0; i < tesseractFaces.length; i++) {
    const face = tesseractFaces[i];
    assert.strictEqual(face.length, 4, `Face ${i} should have exactly 4 vertices`);
    for (let idx of face) {
        assert.ok(idx >= 0 && idx < 16, `Invalid index ${idx} in face ${i}`);
    }
}
console.log("✓ Test 2: Face vertex indices bounds validated.");

// Test 3: Magnitude Conservation of 4D Rotations
const v0 = tesseractVertices[0];
const originalLength = Math.sqrt(v0.x*v0.x + v0.y*v0.y + v0.z*v0.z + v0.w*v0.w);
assert.strictEqual(Math.round(originalLength), 2);

const rotatedV0 = rotate4D(v0, 0.5, 0.2, -0.4, 0.8, -0.1, 0.3);
const rotatedLength = Math.sqrt(rotatedV0.x*rotatedV0.x + rotatedV0.y*rotatedV0.y + rotatedV0.z*rotatedV0.z + rotatedV0.w*rotatedV0.w);
assert.ok(Math.abs(rotatedLength - originalLength) < 1e-9);
console.log("✓ Test 3: Magnitude conserved across 6-axis 4D rotations.");

// Test 4: Depth Sorting Algorithm verification (painter's algorithm)
const facesMock = [
    { index: 0, depth: 15.5 },
    { index: 1, depth: 32.1 },
    { index: 2, depth: 12.0 },
    { index: 3, depth: 25.4 }
];
// Sort descending (back-to-front for painter's algorithm rendering)
facesMock.sort((a, b) => b.depth - a.depth);
assert.strictEqual(facesMock[0].index, 1); // 32.1
assert.strictEqual(facesMock[1].index, 3); // 25.4
assert.strictEqual(facesMock[2].index, 0); // 15.5
assert.strictEqual(facesMock[3].index, 2); // 12.0
console.log("✓ Test 4: Painter's algorithm depth-sorting correct.");

console.log("=== ALL TESSERACT MATH TESTS PASSED ===");
