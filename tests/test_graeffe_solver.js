/**
 * Unit Test: Graeffe Root-Squaring Solver
 * 
 * Verifies the coefficient transformation and root magnitude extraction 
 * of the Graeffe root-squaring process for a quadratic (2-pole filter) polynomial.
 */

const assert = require("assert");

// Graeffe solver function for quadratic: a2*x^2 + a1*x + a0 = 0
function solveGraeffeQuadratic(a2, a1, a0, iterations = 3) {
    let coef = { a2, a1, a0 };
    
    for (let k = 0; k < iterations; k++) {
        const nextA2 = coef.a2 * coef.a2;
        const nextA1 = coef.a1 * coef.a1 - 2 * coef.a2 * coef.a0;
        const nextA0 = coef.a0 * coef.a0;
        
        coef = { a2: nextA2, a1: nextA1, a0: nextA0 };
    }
    
    // Extract root magnitudes:
    // r1^8 ≈ A1 / A2  => |r1| = (A1 / A2)^(1/8)
    // r2^8 ≈ A0 / A1  => |r2| = (A0 / A1)^(1/8)
    const power = Math.pow(2, iterations);
    const r1 = Math.pow(Math.abs(coef.a1 / coef.a2), 1 / power);
    const r2 = Math.pow(Math.abs(coef.a0 / coef.a1), 1 / power);
    
    return [r1, r2].sort((a, b) => b - a); // Return descending order
}

// Analytical roots of x^2 - 5x + 6 = 0 are r1 = 3, r2 = 2
console.log("[TEST] Testing Graeffe solver on: x^2 - 5x + 6 = 0");
const [r1, r2] = solveGraeffeQuadratic(1, -5, 6, 3);
console.log(`  |- Extracted Roots: r1 ≈ ${r1.toFixed(4)}, r2 ≈ ${r2.toFixed(4)}`);

try {
    assert(Math.abs(r1 - 3.0) < 0.05, `Root 1 extraction failed: expected ~3, got ${r1}`);
    assert(Math.abs(r2 - 2.0) < 0.05, `Root 2 extraction failed: expected ~2, got ${r2}`);
    console.log("[TEST] Success: Graeffe root extraction verified within tolerance limit.");
} catch (err) {
    console.error("[TEST] Failure:", err.message);
    process.exit(1);
}
