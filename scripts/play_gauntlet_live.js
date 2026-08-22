const WebSocket = require("ws");
const readline = require("readline");
const fs = require("fs");
const path = require("path");

const ws = new WebSocket("ws://127.0.0.1:9999");

// Game state variables
let hero = { x: 200, y: 300, vx: 0, vy: 0, size: 10, speed: 12 };
let spawner = { x: 640, y: 280, health: 3 };
let ghosts = [];
let projectiles = [];
let chests = [
    { x: 300, y: 150, active: true },
    { x: 900, y: 150, active: true },
    { x: 640, y: 420, active: true }
];
let traps = [
    { x: 450, y: 220, active: true },
    { x: 800, y: 380, active: true }
];
let walls = [
    // Outer boundary walls
    { x: 60, y: 60, w: 1160, h: 10 },
    { x: 60, y: 520, w: 1160, h: 10 },
    { x: 60, y: 60, w: 10, h: 470 },
    { x: 1210, y: 60, w: 10, h: 470 },
    // Inner level partition walls
    { x: 400, y: 60, w: 20, h: 180 },
    { x: 860, y: 300, w: 20, h: 220 }
];

let health = 1500;
let score = 0;
let gameOver = false;
let gameWon = false;
let frameCount = 0;
let lastInputTime = Date.now();
let resetTimer = -1;

// AI Director state
let activeDirective = "MONITORING LORE TELEMETRY...";
let enemySpeedMultiplier = 1.0;
let spawnRateInterval = 24;

// Parse USDA rope spline path points (Pixar USD Asset Integration)
let usdaSplineFrames = [];
try {
    const usdaPath = path.join(__dirname, "../tsfi2_remorse_scene.usda");
    if (fs.existsSync(usdaPath)) {
        const content = fs.readFileSync(usdaPath, "utf8");
        // Regex to parse time-sampled Cubic Bezier curves with 5 control points
        const regex = /(\d+):\s*\[\(([\d.-]+),\s*([\d.-]+),\s*([\d.-]+)\),\s*\(([\d.-]+),\s*([\d.-]+),\s*([\d.-]+)\),\s*\(([\d.-]+),\s*([\d.-]+),\s*([\d.-]+)\),\s*\(([\d.-]+),\s*([\d.-]+),\s*([\d.-]+)\),\s*\(([\d.-]+),\s*([\d.-]+),\s*([\d.-]+)\)\]/g;
        let match;
        while ((match = regex.exec(content)) !== null) {
            const frameIdx = parseInt(match[1]);
            const pts = [
                { x: parseFloat(match[2]), y: parseFloat(match[3]) },
                { x: parseFloat(match[5]), y: parseFloat(match[6]) },
                { x: parseFloat(match[8]), y: parseFloat(match[9]) },
                { x: parseFloat(match[11]), y: parseFloat(match[12]) },
                { x: parseFloat(match[14]), y: parseFloat(match[15]) }
            ];
            usdaSplineFrames[frameIdx] = pts;
        }
        console.log(`[GAUNTLET LIVE] Successfully parsed ${usdaSplineFrames.length} USDA spline frames.`);
    }
} catch (e) {
    console.error("[GAUNTLET LIVE] Failed parsing USDA asset:", e.message);
}

// Parse Sewn Heart USDA mesh (Pixar USD Asset Integration)
let sewnHeartVertices = [];
let sewnHeartIndices = [];
try {
    const heartPath = path.join(__dirname, "../tsfi2-deepseek/assets/teddy_bear_sewnheart.usda");
    if (fs.existsSync(heartPath)) {
        const content = fs.readFileSync(heartPath, "utf8");
        // Extract points
        const pointsSection = content.match(/point3f\[\] points = \[\s*([\s\S]*?)\]/);
        if (pointsSection) {
            const ptsRegex = /\(([\d.-]+),\s*([\d.-]+),\s*([\d.-]+)\)/g;
            let match;
            while ((match = ptsRegex.exec(pointsSection[1])) !== null) {
                sewnHeartVertices.push({
                    x: parseFloat(match[1]),
                    y: parseFloat(match[2]),
                    z: parseFloat(match[3])
                });
            }
        }
        // Extract indices
        const indicesSection = content.match(/int\[\] faceVertexIndices = \[\s*([\s\S]*?)\]/);
        if (indicesSection) {
            sewnHeartIndices = indicesSection[1].split(",").map(x => parseInt(x.trim())).filter(x => !isNaN(x));
        }
        console.log(`[GAUNTLET LIVE] Successfully parsed SewnHeart mesh: ${sewnHeartVertices.length} vertices, ${sewnHeartIndices.length / 4} quads.`);
    }
} catch (e) {
    console.error("[GAUNTLET LIVE] Failed parsing SewnHeart USDA asset:", e.message);
}

// Parse Poly Haven CC0 3D model (Poly Haven Asset Integration)
let polyHavenVertices = [];
let polyHavenIndices = [];
let polyHavenAssetName = "None";
try {
    const polyPath = "/tmp/poly_haven_model.json";
    if (fs.existsSync(polyPath)) {
        const content = JSON.parse(fs.readFileSync(polyPath, "utf8"));
        polyHavenVertices = content.vertices || [];
        polyHavenIndices = content.indices || [];
        polyHavenAssetName = content.name || "Unknown";
        console.log(`[GAUNTLET LIVE] Successfully loaded Poly Haven asset "${polyHavenAssetName}": ${polyHavenVertices.length} vertices.`);
    }
} catch (e) {
    console.error("[GAUNTLET LIVE] Failed loading Poly Haven asset:", e.message);
}

function draw3DPolyHavenAsset(ops, cx, cy, angle) {
    if (polyHavenVertices.length === 0) return;

    const scale = 22.0; // scale factor
    const projected = [];

    const cosA = Math.cos(angle);
    const sinA = Math.sin(angle);

    for (let v of polyHavenVertices) {
        // Rotate around Y axis
        let x1 = v.x * cosA - v.z * sinA;
        let z1 = v.x * sinA + v.z * cosA;
        // Rotate around X axis slightly for 3D tilt
        let cosT = 0.866;
        let sinT = 0.5;
        let y2 = v.y * cosT - z1 * sinT;
        
        projected.push({
            x: Math.floor(cx + x1 * scale),
            y: Math.floor(cy - y2 * scale)
        });
    }

    // Draw edges (optimized downsampled wireframe for performance)
    for (let i = 0; i < polyHavenIndices.length; i += 48) {
        const p1 = projected[polyHavenIndices[i]];
        const p2 = projected[polyHavenIndices[i+1]];
        const p3 = projected[polyHavenIndices[i+2]];
        const p4 = projected[polyHavenIndices[i+3]];

        if (!p1 || !p2 || !p3 || !p4) continue;

        drawLine(ops, p1.x, p1.y, p2.x, p2.y, 147, 50, 220);
        drawLine(ops, p2.x, p2.y, p3.x, p3.y, 147, 50, 220);
        drawLine(ops, p3.x, p3.y, p4.x, p4.y, 147, 50, 220);
        drawLine(ops, p4.x, p4.y, p1.x, p1.y, 147, 50, 220);
    }
}

function draw3DThrone(ops, cx, cy, angle, health) {
    if (polyHavenVertices.length === 0) return;

    const scale = 54.0; 
    const projected = [];

    const cosA = Math.cos(angle);
    const sinA = Math.sin(angle);

    for (let v of polyHavenVertices) {
        let x1 = v.x * cosA - v.z * sinA;
        let z1 = v.x * sinA + v.z * cosA;
        let cosT = 0.866;
        let sinT = 0.5;
        let y2 = v.y * cosT - z1 * sinT;
        
        projected.push({
            x: Math.floor(cx + x1 * scale),
            y: Math.floor(cy - y2 * scale)
        });
    }

    const flash = (health < 3 && frameCount % 6 < 3) ? 100 : 0;

    for (let i = 0; i < polyHavenIndices.length; i += 48) {
        const idx1 = polyHavenIndices[i];
        const idx2 = polyHavenIndices[i+1];
        const idx3 = polyHavenIndices[i+2];
        const idx4 = polyHavenIndices[i+3];

        const p1 = projected[idx1];
        const p2 = projected[idx2];
        const p3 = projected[idx3];
        const p4 = projected[idx4];

        if (!p1 || !p2 || !p3 || !p4) continue;

        // Determine if vertex belongs to seat cushion region by 3D coordinates
        const v1 = polyHavenVertices[idx1];
        const v2 = polyHavenVertices[idx2];
        const avgY = (v1.y + v2.y) / 2;
        const avgX = (v1.x + v2.x) / 2;
        const avgZ = (v1.z + v2.z) / 2;

        let r = 255, g = 215 - flash, b = flash; // Gold frame
        if (avgY > 0.05 && avgY < 0.45 && Math.abs(avgX) < 0.35 && Math.abs(avgZ) < 0.35) {
            r = 186; g = 85; b = 211; // Royal Purple Velvet cushion
        }

        drawLine(ops, p1.x, p1.y, p2.x, p2.y, r, g, b);
        drawLine(ops, p2.x, p2.y, p3.x, p3.y, r, g, b);
        drawLine(ops, p3.x, p3.y, p4.x, p4.y, r, g, b);
        drawLine(ops, p4.x, p4.y, p1.x, p1.y, r, g, b);
    }
}

function draw3DSewnHeart(ops, cx, cy, angle) {
    if (sewnHeartVertices.length === 0) return;

    const scale = 36.0; // scale factor
    const projected = [];

    // Rotate and project vertices
    const cosA = Math.cos(angle);
    const sinA = Math.sin(angle);

    for (let v of sewnHeartVertices) {
        // Rotate around Y axis
        let x1 = v.x * cosA - v.z * sinA;
        let z1 = v.x * sinA + v.z * cosA;
        // Rotate around X axis slightly for 3D tilt
        let cosT = 0.866;
        let sinT = 0.5;
        let y2 = v.y * cosT - z1 * sinT;
        
        projected.push({
            x: Math.floor(cx + x1 * scale),
            y: Math.floor(cy - y2 * scale)
        });
    }

    // Draw quad edges
    for (let i = 0; i < sewnHeartIndices.length; i += 4) {
        const p1 = projected[sewnHeartIndices[i]];
        const p2 = projected[sewnHeartIndices[i+1]];
        const p3 = projected[sewnHeartIndices[i+2]];
        const p4 = projected[sewnHeartIndices[i+3]];

        if (!p1 || !p2 || !p3 || !p4) continue;

        drawLine(ops, p1.x, p1.y, p2.x, p2.y, 255, 60, 100);
        drawLine(ops, p2.x, p2.y, p3.x, p3.y, 255, 60, 100);
        drawLine(ops, p3.x, p3.y, p4.x, p4.y, 255, 60, 100);
        drawLine(ops, p4.x, p4.y, p1.x, p1.y, 255, 60, 100);
    }
}

function drawLine(ops, x1, y1, x2, y2, r, g, b) {
    const steps = Math.max(Math.abs(x2 - x1), Math.abs(y2 - y1));
    if (steps === 0) return;
    for (let i = 0; i <= steps; i += 10) {
        const t = i / steps;
        const x = Math.floor(x1 * (1 - t) + x2 * t);
        const y = Math.floor(y1 * (1 - t) + y2 * t);
        ops.push({
            type: "draw_rect",
            x: x,
            y: y,
            w: 2,
            h: 2,
            r: r,
            g: g,
            b: b
        });
    }
}

// Setup raw input to capture keystrokes from standard input
readline.emitKeypressEvents(process.stdin);
if (process.stdin.isTTY) {
    process.stdin.setRawMode(true);
}

process.stdin.on("keypress", (str, key) => {
    if (key.ctrl && key.name === "c") {
        cleanupAndExit();
    }

    if (gameOver || gameWon) return;

    lastInputTime = Date.now();

    // Movement controls (WASD/Arrows)
    if (key.name === "w" || key.name === "up") {
        hero.y = Math.max(80, hero.y - hero.speed);
    } else if (key.name === "s" || key.name === "down") {
        hero.y = Math.min(500, hero.y + hero.speed);
    } else if (key.name === "a" || key.name === "left") {
        hero.x = Math.max(80, hero.x - hero.speed);
    } else if (key.name === "d" || key.name === "right") {
        hero.x = Math.min(1190, hero.x + hero.speed);
    }

    // Resolve wall collisions instantly to slide smoothly
    resolveWallCollisions();

    // Fire control (Spacebar)
    if (key.name === "space" || key.name === "f") {
        fireAxe(22, 0);
    }
});

function fireAxe(vx, vy) {
    projectiles.push({
        x: hero.x,
        y: hero.y,
        vx: vx,
        vy: vy,
        life: 25
    });
    score = Math.max(0, score - 2); // Small consumption cost
}

function resolveWallCollisions() {
    for (let w of walls) {
        if (hero.x + hero.size/2 > w.x && hero.x - hero.size/2 < w.x + w.w &&
            hero.y + hero.size/2 > w.y && hero.y - hero.size/2 < w.y + w.h) {
            let overlapX = Math.min(hero.x + hero.size/2 - w.x, w.x + w.w - (hero.x - hero.size/2));
            let overlapY = Math.min(hero.y + hero.size/2 - w.y, w.y + w.h - (hero.y - hero.size/2));
            if (overlapX < overlapY) {
                hero.x += (hero.x < w.x + w.w/2) ? -overlapX : overlapX;
            } else {
                hero.y += (hero.y < w.y + w.h/2) ? -overlapY : overlapY;
            }
        }
    }
}

ws.on("open", () => {
    console.log("[GAUNTLET LIVE] Connected to Coaxial WebSocket Server.");
    console.log("Controls: Arrow keys / WASD to move, Spacebar to shoot, Ctrl+C to exit.");
    
    // Enable arcade mode (switches streamer to frame buffer display mode)
    ws.send(JSON.stringify({ arcade_mode: true }));

    const interval = setInterval(() => {
        if (gameOver || gameWon) {
            if (resetTimer === -1) {
                resetTimer = 80; // Wait 80 frames (~5 seconds) on screen before auto-restarting
            }
            resetTimer--;
            if (resetTimer <= 0) {
                resetGame();
            }
            renderGame();
            return;
        }
        aiDirectorTick();
        autoplayPilotTick();
        updateGame();
        renderGame();
    }, 60);
});

ws.on("error", (err) => {
    console.error("[GAUNTLET LIVE ERROR]:", err.message);
    cleanupAndExit();
});

// Autoplay pilot triggers if no keyboard input is received for 3 seconds
function autoplayPilotTick() {
    if (Date.now() - lastInputTime < 3000) return;

    // Direct movement toward target (prioritize Spawner, then closest Ghost)
    let targetX = spawner.x;
    let targetY = spawner.y;

    if (spawner.health <= 0 && ghosts.length > 0) {
        let closest = ghosts[0];
        let minDist = Infinity;
        for (let g of ghosts) {
            let dx = g.x - hero.x;
            let dy = g.y - hero.y;
            let dist = Math.sqrt(dx*dx + dy*dy);
            if (dist < minDist) {
                minDist = dist;
                closest = g;
            }
        }
        targetX = closest.x;
        targetY = closest.y;
    }

    // Move toward target
    let dx = targetX - hero.x;
    let dy = targetY - hero.y;
    let dist = Math.sqrt(dx*dx + dy*dy);
    if (dist > 15) {
        hero.x += (dx / dist) * (hero.speed * 0.4);
        hero.y += (dy / dist) * (hero.speed * 0.4);
        resolveWallCollisions();
    }

    // Auto-fire axes towards target periodically
    if (frameCount % 8 === 0) {
        let vx = (dx / dist) * 20;
        let vy = (dy / dist) * 20;
        fireAxe(vx, vy);
    }
}

function resetGame() {
    hero = { x: 200, y: 300, vx: 0, vy: 0, size: 10, speed: 12 };
    spawner = { x: 640, y: 280, health: 3 };
    ghosts = [];
    projectiles = [];
    chests = [
        { x: 300, y: 150, active: true },
        { x: 900, y: 150, active: true },
        { x: 640, y: 420, active: true }
    ];
    traps = [
        { x: 450, y: 220, active: true },
        { x: 800, y: 380, active: true }
    ];
    health = 1500;
    score = 0;
    gameOver = false;
    gameWon = false;
    resetTimer = -1;
}

// AI Director updates parameters dynamically by reading the live stream lore context
function aiDirectorTick() {
    if (frameCount % 45 !== 0) return;

    const loreFile = "/tmp/tsfi_block_run.md";
    if (!fs.existsSync(loreFile)) return;

    try {
        const text = fs.readFileSync(loreFile, "utf8").toLowerCase();
        
        if (text.includes("gold") || text.includes(" saat") || text.includes("account")) {
            activeDirective = "ALCHEMICAL DIRECTIVE: HARVEST COINS (+500 HEALTH BOOST)";
            health = Math.min(1500, health + 120);
        } else if (text.includes("hardware") || text.includes("scsi") || text.includes("winchester")) {
            activeDirective = "AUNCIENT CLOCK SPEED MULTIPLIER: ACCELERATING GHOSTS!";
            enemySpeedMultiplier = 1.6;
            spawnRateInterval = 14;
        } else if (text.includes("modulator") || text.includes("synchrodyne") || text.includes("filter")) {
            activeDirective = "SPECTRUM DIRECTIVE: RESIST ENEMY DAMAGE (+20% RESISTANCE)";
            enemySpeedMultiplier = 0.95;
        } else if (text.includes("subpoena") || text.includes("dna") || text.includes("recall")) {
            activeDirective = "CITIZEN DIRECTIVE: DESTRUCTIVE DEEPSEEK EVOLVE ACTIVE";
            for (let angle = 0; angle < Math.PI * 2; angle += Math.PI / 4) {
                projectiles.push({
                    x: hero.x,
                    y: hero.y,
                    vx: Math.cos(angle) * 16,
                    vy: Math.sin(angle) * 16,
                    life: 20
                });
            }
        } else {
            activeDirective = "MONITORING LORE TELEMETRY...";
            enemySpeedMultiplier = 1.0;
            spawnRateInterval = 24;
        }
    } catch (e) {}
}

function updateGame() {
    frameCount++;

    health = Math.max(0, health - 1);
    if (health <= 0) {
        gameOver = true;
    }

    if (frameCount % spawnRateInterval === 0 && ghosts.length < 15 && spawner.health > 0) {
        ghosts.push({
            x: spawner.x + (Math.random() * 40 - 20),
            y: spawner.y + (Math.random() * 40 - 20),
            speed: (1.8 + Math.random() * 1.4) * enemySpeedMultiplier,
            health: 1
        });
    }

    for (let g of ghosts) {
        let dx = hero.x - g.x;
        let dy = hero.y - g.y;
        let dist = Math.sqrt(dx*dx + dy*dy);
        if (dist > 5) {
            g.x += (dx / dist) * g.speed;
            g.y += (dy / dist) * g.speed;
        }

        if (dist < 14) {
            health = Math.max(0, health - 8);
        }
    }

    for (let c of chests) {
        if (!c.active) continue;
        let dx = hero.x - c.x;
        let dy = hero.y - c.y;
        if (Math.sqrt(dx*dx + dy*dy) < 22) {
            c.active = false;
            score += 500;
            health = Math.min(1500, health + 200);
        }
    }

    for (let t of traps) {
        if (!t.active) continue;
        let dx = hero.x - t.x;
        let dy = hero.y - t.y;
        if (Math.sqrt(dx*dx + dy*dy) < 18) {
            t.active = false;
            health = Math.max(0, health - 250);
        }
    }

    projectiles = projectiles.filter(p => {
        p.x += p.vx;
        p.y += p.vy;
        p.life--;

        for (let i = ghosts.length - 1; i >= 0; i--) {
            let dx = p.x - ghosts[i].x;
            let dy = p.y - ghosts[i].y;
            let dist = Math.sqrt(dx*dx + dy*dy);
            if (dist < 16) {
                ghosts.splice(i, 1);
                score += 100;
                return false;
            }
        }

        if (spawner.health > 0) {
            let sdx = p.x - spawner.x;
            let sdy = p.y - spawner.y;
            let sdist = Math.sqrt(sdx*sdx + sdy*sdy);
            if (sdist < 22) {
                spawner.health--;
                if (spawner.health <= 0) {
                    score += 1500;
                    gameWon = true;
                }
                return false;
            }
        }

        return p.life > 0;
    });
}

function renderGame() {
    const ops = [];

    // 1. Draw Cobblestone floor background
    for (let y = 60; y < 520; y += 40) {
        for (let x = 60; x < 1210; x += 40) {
            const stoneVal = 20 + ((x * 7 + y * 13) % 15);
            ops.push({
                type: "draw_rect",
                x: x,
                y: y,
                w: 38,
                h: 38,
                r: stoneVal,
                g: stoneVal + 4,
                b: stoneVal + 10
            });
        }
    }

    // 2. Draw Animating USDA Rope Spline (Pixar USD Asset Integration)
    if (usdaSplineFrames.length > 0) {
        const pts = usdaSplineFrames[frameCount % usdaSplineFrames.length];
        if (pts) {
            let prevX = pts[0].x;
            let prevY = pts[0].y;
            for (let t = 0.12; t <= 1.0; t += 0.12) {
                // Bezier curve interpolation over 5 points
                const omt = 1 - t;
                const x = omt*omt*omt*omt*pts[0].x + 4*omt*omt*omt*t*pts[1].x + 6*omt*omt*t*t*pts[2].x + 4*omt*t*t*t*pts[3].x + t*t*t*t*pts[4].x;
                const y = omt*omt*omt*omt*pts[0].y + 4*omt*omt*omt*t*pts[1].y + 6*omt*omt*t*t*pts[2].y + 4*omt*t*t*t*pts[3].y + t*t*t*t*pts[4].y;
                
                // Map/scale coordinate points onto the game board grid
                const gameX1 = 60 + (prevX * 1.8);
                const gameY1 = 60 + (prevY * 1.1);
                const gameX2 = 60 + (x * 1.8);
                const gameY2 = 60 + (y * 1.1);

                // Draw thick glowing cyan laser curve segments
                ops.push({
                    type: "draw_rect",
                    x: Math.floor(Math.min(gameX1, gameX2)),
                    y: Math.floor(Math.min(gameY1, gameY2)),
                    w: Math.floor(Math.abs(gameX2 - gameX1) + 2),
                    h: Math.floor(Math.abs(gameY2 - gameY1) + 2),
                    r: 0,
                    g: 220,
                    b: 255
                });

                prevX = x;
                prevY = y;
            }
        }
    }

    // 3. Draw 3D-shaded alchemical Brick Walls
    for (let w of walls) {
        ops.push({
            type: "draw_rect",
            x: w.x,
            y: w.y,
            w: w.w,
            h: w.h,
            r: 105,
            g: 55,
            b: 45
        });
        ops.push({
            type: "draw_rect",
            x: w.x,
            y: w.y,
            w: w.w,
            h: 3,
            r: 155,
            g: 95,
            b: 85
        });
    }

    // 4. Draw active chests (rotating 3D Pixar USD SewnHeart models)
    const heartRotation = frameCount * 0.08;
    for (let c of chests) {
        if (!c.active) continue;
        draw3DSewnHeart(ops, c.x, c.y, heartRotation);
    }

    // 5. Draw active traps (rotating 3D Poly Haven assets, e.g. ArmChair_01 / Concrete Barrier)
    const trapRotation = frameCount * 0.05;
    for (let t of traps) {
        if (!t.active) continue;
        draw3DPolyHavenAsset(ops, t.x, t.y, trapRotation);
    }

    // 6. Draw Spawner (Large Comfy Chair as the rotating Golden Throne, preserved on victory)
    if (spawner.health > 0 || gameWon) {
        const spinSpeed = gameWon ? 0.09 : 0.03;
        const displayHealth = gameWon ? 3 : spawner.health;
        draw3DThrone(ops, spawner.x, spawner.y, frameCount * spinSpeed, displayHealth);
    }

    // 7. Draw Ghosts
    for (let g of ghosts) {
        const flash = (frameCount % 4 < 2) ? 20 : 0;
        ops.push({
            type: "draw_rect",
            x: Math.floor(g.x - 6),
            y: Math.floor(g.y - 6),
            w: 12,
            h: 12,
            r: 160 + flash,
            g: 160 + flash,
            b: 170 + flash
        });
    }

    // 8. Draw Hero
    ops.push({
        type: "draw_rect",
        x: Math.floor(hero.x - hero.size/2),
        y: Math.floor(hero.y - hero.size/2),
        w: hero.size,
        h: hero.size,
        r: 0,
        g: 140,
        b: 255
    });
    ops.push({
        type: "draw_rect",
        x: Math.floor(hero.x - 3),
        y: Math.floor(hero.y - 3),
        w: 6,
        h: 6,
        r: 200,
        g: 240,
        b: 255
    });

    // 9. Draw Projectiles
    for (let p of projectiles) {
        ops.push({
            type: "draw_rect",
            x: Math.floor(p.x - 3),
            y: Math.floor(p.y - 3),
            w: 6,
            h: 6,
            r: 255,
            g: 215,
            b: 0
        });
    }

    // 10. HUD panel overlay
    ops.push({
        type: "draw_rect",
        x: 60,
        y: 530,
        w: 1160,
        h: 40,
        r: 20,
        g: 20,
        b: 28
    });

    const healthW = Math.floor(300 * (health / 1500));
    if (healthW > 0) {
        ops.push({
            type: "draw_rect",
            x: 80,
            y: 545,
            w: healthW,
            h: 12,
            r: health < 400 ? 220 : 46,
            g: health < 400 ? 50 : 204,
            b: 64
        });
    }

    ops.push({
        type: "draw_rect",
        x: 420,
        y: 545,
        w: 780,
        h: 12,
        r: 147,
        g: 50,
        b: 220
    });

    if (gameWon) {
        // Draw elegant glowing green border instead of a giant blocking box
        ops.push({ type: "draw_rect", x: 60, y: 60, w: 1160, h: 6, r: 46, g: 204, b: 113 });
        ops.push({ type: "draw_rect", x: 60, y: 520, w: 1160, h: 6, r: 46, g: 204, b: 113 });
        ops.push({ type: "draw_rect", x: 60, y: 60, w: 6, h: 470, r: 46, g: 204, b: 113 });
        ops.push({ type: "draw_rect", x: 1210, y: 60, w: 6, h: 470, r: 46, g: 204, b: 113 });
    } else if (gameOver) {
        // Draw elegant glowing red border
        ops.push({ type: "draw_rect", x: 60, y: 60, w: 1160, h: 6, r: 231, g: 76, b: 60 });
        ops.push({ type: "draw_rect", x: 60, y: 520, w: 1160, h: 6, r: 231, g: 76, b: 60 });
        ops.push({ type: "draw_rect", x: 60, y: 60, w: 6, h: 470, r: 231, g: 76, b: 60 });
        ops.push({ type: "draw_rect", x: 1210, y: 60, w: 6, h: 470, r: 231, g: 76, b: 60 });
    }

    const payload = {
        tts_enabled: false,
        draw_operations: ops
    };

    if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(payload));
    }
}

function cleanupAndExit() {
    if (process.stdin.isTTY) {
        process.stdin.setRawMode(false);
    }
    process.exit(0);
}
