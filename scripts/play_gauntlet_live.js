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

// AI Director state
let activeDirective = "MONITORING LORE TELEMETRY...";
let enemySpeedMultiplier = 1.0;
let spawnRateInterval = 24;

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
        projectiles.push({
            x: hero.x,
            y: hero.y,
            vx: 22,
            vy: 0,
            life: 25
        });
        score = Math.max(0, score - 2); // Small consumption cost
    }
});

function resolveWallCollisions() {
    for (let w of walls) {
        if (hero.x + hero.size/2 > w.x && hero.x - hero.size/2 < w.x + w.w &&
            hero.y + hero.size/2 > w.y && hero.y - hero.size/2 < w.y + w.h) {
            // Push out based on overlap
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
    
    // Start game loop (60ms tick for high speed responsiveness)
    const interval = setInterval(() => {
        if (gameOver || gameWon) {
            clearInterval(interval);
            return;
        }
        aiDirectorTick();
        updateGame();
        renderGame();
    }, 60);
});

ws.on("error", (err) => {
    console.error("[GAUNTLET LIVE ERROR]:", err.message);
    cleanupAndExit();
});

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
            // Spawn a powerful projectile sweep
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

    // Health timer decay
    health = Math.max(0, health - 1);
    if (health <= 0) {
        gameOver = true;
    }

    // Spawn ghosts from active spawner
    if (frameCount % spawnRateInterval === 0 && ghosts.length < 15 && spawner.health > 0) {
        ghosts.push({
            x: spawner.x + (Math.random() * 40 - 20),
            y: spawner.y + (Math.random() * 40 - 20),
            speed: (1.8 + Math.random() * 1.4) * enemySpeedMultiplier,
            health: 1
        });
    }

    // Move ghosts towards hero with wall avoidance
    for (let g of ghosts) {
        let dx = hero.x - g.x;
        let dy = hero.y - g.y;
        let dist = Math.sqrt(dx*dx + dy*dy);
        if (dist > 5) {
            g.x += (dx / dist) * g.speed;
            g.y += (dy / dist) * g.speed;
        }

        // Damage hero on contact
        if (dist < 14) {
            health = Math.max(0, health - 8);
        }
    }

    // Collect chests
    for (let c of chests) {
        if (!c.active) continue;
        let dx = hero.x - c.x;
        let dy = hero.y - c.y;
        if (Math.sqrt(dx*dx + dy*dy) < 22) {
            c.active = false;
            score += 500;
            health = Math.min(1500, health + 200); // Chest chest provides food/health
        }
    }

    // Trigger alchemical traps (damage hero if stepped on)
    for (let t of traps) {
        if (!t.active) continue;
        let dx = hero.x - t.x;
        let dy = hero.y - t.y;
        if (Math.sqrt(dx*dx + dy*dy) < 18) {
            t.active = false;
            health = Math.max(0, health - 250); // Big explosion damage
        }
    }

    // Move projectiles and check collisions
    projectiles = projectiles.filter(p => {
        p.x += p.vx;
        p.y += p.vy;
        p.life--;

        // Check collision with ghosts
        for (let i = ghosts.length - 1; i >= 0; i--) {
            let dx = p.x - ghosts[i].x;
            let dy = p.y - ghosts[i].y;
            let dist = Math.sqrt(dx*dx + dy*dy);
            if (dist < 16) {
                ghosts.splice(i, 1);
                score += 100;
                return false; // destroy projectile
            }
        }

        // Check collision with spawner
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
                return false; // destroy projectile
            }
        }

        return p.life > 0;
    });
}

function renderGame() {
    const ops = [];

    // 1. Draw procedural Cobblestone floor texture overlay (repeating grid)
    for (let y = 60; y < 520; y += 40) {
        for (let x = 60; x < 1210; x += 40) {
            // Procedurally shade each cobble tile to look like grey stone
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

    // 2. Draw 3D-shaded alchemical Brick Walls
    for (let w of walls) {
        ops.push({
            type: "draw_rect",
            x: w.x,
            y: w.y,
            w: w.w,
            h: w.h,
            r: 105,
            g: 55,
            b: 45 // Brick base color
        });
        // 3D highlights on the wall borders
        ops.push({
            type: "draw_rect",
            x: w.x,
            y: w.y,
            w: w.w,
            h: 3,
            r: 155,
            g: 95,
            b: 85 // Highlight top
        });
    }

    // 3. Draw active chests (wooden boxes with gold lock)
    for (let c of chests) {
        if (!c.active) continue;
        ops.push({
            type: "draw_rect",
            x: c.x - 10,
            y: c.y - 8,
            w: 20,
            h: 16,
            r: 139,
            g: 90,
            b: 43 // Wood
        });
        ops.push({
            type: "draw_rect",
            x: c.x - 4,
            y: c.y - 2,
            w: 8,
            h: 4,
            r: 255,
            g: 215,
            b: 0 // Gold latch
        });
    }

    // 4. Draw alchemical traps (pulsing purple traps)
    const trapPulse = Math.floor(4 * Math.sin(frameCount * 0.2));
    for (let t of traps) {
        if (!t.active) continue;
        ops.push({
            type: "draw_rect",
            x: t.x - 8 - trapPulse/2,
            y: t.y - 8 - trapPulse/2,
            w: 16 + trapPulse,
            h: 16 + trapPulse,
            r: 147,
            g: 50,
            b: 220 // Purple magic
        });
    }

    // 5. Draw Spawner (volumetric red/orange generator portal)
    if (spawner.health > 0) {
        const portalPulse = Math.floor(6 * Math.sin(frameCount * 0.35));
        ops.push({
            type: "draw_rect",
            x: spawner.x - 18 - portalPulse/2,
            y: spawner.y - 18 - portalPulse/2,
            w: 36 + portalPulse,
            h: 36 + portalPulse,
            r: 220,
            g: 40,
            b: 20 // Outer red glow
        });
        ops.push({
            type: "draw_rect",
            x: spawner.x - 10,
            y: spawner.y - 10,
            w: 20,
            h: 20,
            r: 255,
            g: 165,
            b: 0 // Orange core
        });
    }

    // 6. Draw Ghosts (flashing grey sprites)
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

    // 7. Draw Hero (bright blue warrior box with glowing center)
    ops.push({
        type: "draw_rect",
        x: Math.floor(hero.x - hero.size/2),
        y: Math.floor(hero.y - hero.size/2),
        w: hero.size,
        h: hero.size,
        r: 0,
        g: 140,
        b: 255 // Blue plate armor
    });
    ops.push({
        type: "draw_rect",
        x: Math.floor(hero.x - 3),
        y: Math.floor(hero.y - 3),
        w: 6,
        h: 6,
        r: 200,
        g: 240,
        b: 255 // Glowing helmet visor
    });

    // 8. Draw Projectiles (axes / spinning yellow lines)
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

    // 9. HUD panel overlay
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

    // Health bar representation
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

    // HUD Text highlight indicators (draw purple bar for directives)
    ops.push({
        type: "draw_rect",
        x: 420,
        y: 545,
        w: 780,
        h: 12,
        r: 147,
        g: 50,
        b: 220 // Directive purple bar
    });

    // Game Over / Victory banners
    if (gameWon) {
        ops.push({
            type: "draw_rect",
            x: 440,
            y: 200,
            w: 400,
            h: 80,
            r: 46,
            g: 204,
            b: 113 // Green victory
        });
    } else if (gameOver) {
        ops.push({
            type: "draw_rect",
            x: 440,
            y: 200,
            w: 400,
            h: 80,
            r: 231,
            g: 76,
            b: 60 // Red defeat
        });
    }

    // Package payload and dispatch
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
