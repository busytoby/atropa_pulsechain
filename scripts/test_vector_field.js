/**
 * Unit Tests & Benchmarks for "Velocity" in Agentic Vector Fields
 * TSFi2 Platform validation suite
 */

// Simple 2D Vector operations
function add(v1, v2) { return { x: v1.x + v2.x, y: v1.y + v2.y }; }
function sub(v1, v2) { return { x: v1.x - v2.x, y: v1.y - v2.y }; }
function mul(v, s) { return { x: v.x * s, y: v.y * s }; }
function mag(v) { return Math.sqrt(v.x * v.x + v.y * v.y); }
function norm(v) {
    const m = mag(v);
    return m === 0 ? { x: 0, y: 0 } : { x: v.x / m, y: v.y / m };
}

// 1. Resolve Potential Field Forces
// Models goals as attractors (type=0) and obstacles/history as repellers (type=1)
function resolveVelocity(entity, nodes, currentEpoch, decayRate = 5) {
    let netFx = 0;
    let netFy = 0;

    for (const node of nodes) {
        let weight = node.weight;

        // Apply decay to history nodes (type = 1, age > 0)
        if (node.type === 1 && node.age > 0) {
            const elapsed = currentEpoch - node.age;
            const decay = elapsed * decayRate;
            weight = Math.max(0, weight - decay);
        }

        if (weight <= 0) continue;

        // Manhattan distance calculation
        const dx = node.x - entity.x;
        const dy = node.y - entity.y;
        const dist = Math.abs(dx) + Math.abs(dy) || 1;

        // Radius cutoff
        if (node.radius > 0 && dist > node.radius) continue;

        // Force = (weight * 100) / distance
        const forceMag = (weight * 100) / dist;
        const dirX = dx > 0 ? 1 : (dx < 0 ? -1 : 0);
        const dirY = dy > 0 ? 1 : (dy < 0 ? -1 : 0);

        // Attractor pulls, Repeller pushes
        const multiplier = node.type === 0 ? 1 : -1;

        netFx += forceMag * dirX * multiplier;
        netFy += forceMag * dirY * multiplier;
    }

    return { x: netFx, y: netFy };
}

// 2. Accumulator Vector Field (EMA Integration)
function accumulateVelocity(currentAcc, incomingForce, alpha = 0.75) {
    const scale = 1.0;
    return {
        x: currentAcc.x * alpha + incomingForce.x * (scale - alpha),
        y: currentAcc.y * alpha + incomingForce.y * (scale - alpha)
    };
}

// ============================================================================
// RUN TESTS & BENCHMARKS
// ============================================================================

function runTests() {
    console.log("=== RUNNING AGENTIC VECTOR FIELD VELOCITY SUITE ===\n");

    const entity = { x: 0, y: 0 };

    // TEST 1: Attractor Pull
    console.log("[TEST 1] Attractor Pull Test...");
    const goalNode = { weight: 50, type: 0, x: 100, y: 0, radius: 200, age: 0 };
    const v1 = resolveVelocity(entity, [goalNode], 0);
    console.log(`  Velocity Vector: (${v1.x}, ${v1.y})`);
    if (v1.x > 0 && v1.y === 0) {
        console.log("  SUCCESS: Attractor pulled agent forward.\n");
    } else {
        console.error("  FAIL: Incorrect attractor pull direction.\n");
        process.exit(1);
    }

    // TEST 2: Repeller Push
    console.log("[TEST 2] Repeller Push Test...");
    const hazardNode = { weight: 80, type: 1, x: 50, y: 0, radius: 100, age: 0 };
    const v2 = resolveVelocity(entity, [hazardNode], 0);
    console.log(`  Velocity Vector: (${v2.x}, ${v2.y})`);
    if (v2.x < 0 && v2.y === 0) {
        console.log("  SUCCESS: Repeller pushed agent backward.\n");
    } else {
        console.error("  FAIL: Incorrect repeller push direction.\n");
        process.exit(1);
    }

    // TEST 3: History Node Decay Test (Tabu Repeller)
    console.log("[TEST 3] Tabu Repeller Decay Test...");
    const historyNode = { weight: 50, type: 1, x: 0, y: 0, radius: 50, age: 10 }; // Visited at epoch 10
    
    // Check force at epoch 10 (no decay yet)
    const v3_t10 = resolveVelocity({ x: 10, y: 0 }, [historyNode], 10);
    // Check force at epoch 15 (partial decay: weight = 50 - (5 * 5) = 25)
    const v3_t15 = resolveVelocity({ x: 10, y: 0 }, [historyNode], 15);
    // Check force at epoch 20 (total decay: weight = 0)
    const v3_t20 = resolveVelocity({ x: 10, y: 0 }, [historyNode], 20);

    console.log(`  Force at t=10: (${v3_t10.x}, ${v3_t10.y})`);
    console.log(`  Force at t=15: (${v3_t15.x}, ${v3_t15.y})`);
    console.log(`  Force at t=20: (${v3_t20.x}, ${v3_t20.y})`);

    if (Math.abs(v3_t10.x) > Math.abs(v3_t15.x) && v3_t20.x === 0) {
        console.log("  SUCCESS: History weight correctly decayed to zero.\n");
    } else {
        console.error("  FAIL: Decaying weight calculation incorrect.\n");
        process.exit(1);
    }

    // TEST 4: Accumulator Vector Momentum (EMA)
    console.log("[TEST 4] EMA Accumulator Velocity Integration...");
    let acc = { x: 0, y: 0 };
    const force = { x: 100, y: 50 };
    
    console.log("  Simulating 5 ticks of constant force pull...");
    for (let t = 1; t <= 5; t++) {
        acc = accumulateVelocity(acc, force, 0.75);
        console.log(`    Tick ${t} Accumulated Velocity: (${acc.x.toFixed(2)}, ${acc.y.toFixed(2)})`);
    }
    
    if (acc.x > 50 && acc.x < 100) {
        console.log("  SUCCESS: Accumulator integrated velocity momentum correctly.\n");
    } else {
        console.error("  FAIL: Accumulator integration values out of bounds.\n");
        process.exit(1);
    }

    // ============================================================================
    // BENCHMARK: 10,000 Velocity Evaluations
    // ============================================================================
    console.log("=== BENCHMARKING VECTOR RESOLVER PERFORMANCE ===");
    const iterations = 10000;
    const benchmarkNodes = [
        { weight: 60, type: 0, x: 200, y: 300, radius: 500, age: 0 },
        { weight: 40, type: 1, x: 50, y: 80, radius: 100, age: 0 },
        { weight: 30, type: 1, x: 20, y: 20, radius: 50, age: 5 },
        { weight: 80, type: 0, x: 400, y: 10, radius: 600, age: 0 }
    ];

    const start = process.hrtime.bigint();
    let dummySumX = 0;
    let dummySumY = 0;

    for (let i = 0; i < iterations; i++) {
        const dummyEntity = { x: (i % 300), y: (i % 200) };
        const vel = resolveVelocity(dummyEntity, benchmarkNodes, i % 50);
        dummySumX += vel.x;
        dummySumY += vel.y;
    }

    const end = process.hrtime.bigint();
    const durationMs = Number(end - start) / 1000000;
    const opsPerSec = (iterations / durationMs) * 1000;

    console.log(`  Executed ${iterations} velocity resolutions in: ${durationMs.toFixed(3)} ms`);
    console.log(`  Throughput: ${opsPerSec.toFixed(0)} operations/second`);
    console.log(`  Velocity Checksum (Sum): (${dummySumX.toFixed(0)}, ${dummySumY.toFixed(0)})`);
    console.log("\n  SUCCESS: Benchmark completed cleanly.\n");
}

runTests();
