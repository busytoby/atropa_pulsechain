const WebSocket = require("ws");
const readline = require("readline");

const ws = new WebSocket("ws://127.0.0.1:9999");

// Game state variables
let hero = { x: 300, y: 350, r: 0, g: 191, b: 255, size: 8 }; // Warrior (blue circle/rect)
let spawner = { x: 640, y: 250, r: 230, g: 81, b: 0, size: 16 }; // Pulsing orange spawner
let ghosts = [];
let projectiles = [];
let health = 1500;
let score = 0;
let gameOver = false;
let gameWon = false;
let frameCount = 0;

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

    // Movement controls
    if (key.name === "w" || key.name === "up") {
        hero.y = Math.max(80, hero.y - 12);
    } else if (key.name === "s" || key.name === "down") {
        hero.y = Math.min(520, hero.y + 12);
    } else if (key.name === "a" || key.name === "left") {
        hero.x = Math.max(80, hero.x - 12);
    } else if (key.name === "d" || key.name === "right") {
        hero.x = Math.min(1200, hero.x + 12);
    }

    // Fire control
    if (key.name === "space" || key.name === "f") {
        // Fire axe in the direction of movement (or default right)
        projectiles.push({
            x: hero.x,
            y: hero.y,
            vx: 18,
            vy: 0,
            life: 30
        });
        score = Math.max(0, score - 5); // Axe consumption penalty
    }
});

ws.on("open", () => {
    console.log("[GAUNTLET LIVE] Connected to Coaxial WebSocket Server.");
    console.log("Controls: Arrow keys / WASD to move, Spacebar to shoot, Ctrl+C to exit.");
    
    // Start game loop (60ms tick for high speed responsiveness)
    const interval = setInterval(() => {
        if (gameOver || gameWon) {
            clearInterval(interval);
            return;
        }
        updateGame();
        renderGame();
    }, 60);
});

ws.on("error", (err) => {
    console.error("[GAUNTLET LIVE ERROR]:", err.message);
    cleanupAndExit();
});

function updateGame() {
    frameCount++;

    // Health timer decay
    health = Math.max(0, health - 1);
    if (health <= 0) {
        gameOver = true;
    }

    // Spawn ghosts from active spawner
    if (frameCount % 24 === 0 && ghosts.length < 12) {
        ghosts.push({
            x: spawner.x + (Math.random() * 40 - 20),
            y: spawner.y + (Math.random() * 40 - 20),
            speed: 2.2 + Math.random() * 1.5,
            health: 1
        });
    }

    // Move ghosts towards hero
    for (let g of ghosts) {
        let dx = hero.x - g.x;
        let dy = hero.y - g.y;
        let dist = Math.sqrt(dx*dx + dy*dy);
        if (dist > 5) {
            g.x += (dx / dist) * g.speed;
            g.y += (dy / dist) * g.speed;
        }

        // Damage hero on contact
        if (dist < 12) {
            health = Math.max(0, health - 8);
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
            if (dist < 14) {
                ghosts.splice(i, 1);
                score += 100;
                return false; // destroy projectile
            }
        }

        // Check collision with spawner
        let sdx = p.x - spawner.x;
        let sdy = p.y - spawner.y;
        let sdist = Math.sqrt(sdx*sdx + sdy*sdy);
        if (sdist < 20) {
            score += 1000;
            gameWon = true;
            return false; // destroy projectile
        }

        return p.life > 0;
    });
}

function renderGame() {
    const ops = [];

    // Clear game field area (overlay a semi-transparent black rectangle)
    ops.push({
        type: "draw_rect",
        x: 60,
        y: 60,
        w: 1160,
        h: 500,
        r: 10,
        g: 10,
        b: 15
    });

    // Draw Spawner (pulsing orange box)
    const pulse = 2 * Math.sin(frameCount * 0.25);
    ops.push({
        type: "draw_rect",
        x: Math.floor(spawner.x - spawner.size/2 - pulse/2),
        y: Math.floor(spawner.y - spawner.size/2 - pulse/2),
        w: Math.floor(spawner.size + pulse),
        h: Math.floor(spawner.size + pulse),
        r: spawner.r,
        g: spawner.g,
        b: spawner.b
    });

    // Draw Ghosts (grey boxes)
    for (let g of ghosts) {
        ops.push({
            type: "draw_rect",
            x: Math.floor(g.x - 4),
            y: Math.floor(g.y - 4),
            w: 8,
            h: 8,
            r: 180,
            g: 180,
            b: 180
        });
    }

    // Draw Hero (bright blue warrior box)
    ops.push({
        type: "draw_rect",
        x: Math.floor(hero.x - hero.size/2),
        y: Math.floor(hero.y - hero.size/2),
        w: hero.size,
        h: hero.size,
        r: hero.r,
        g: hero.g,
        b: hero.b
    });

    // Draw Projectiles (yellow axes)
    for (let p of projectiles) {
        ops.push({
            type: "draw_rect",
            x: Math.floor(p.x - 2),
            y: Math.floor(p.y - 2),
            w: 4,
            h: 4,
            r: 255,
            g: 223,
            b: 0
        });
    }

    // HUD background
    ops.push({
        type: "draw_rect",
        x: 60,
        y: 530,
        w: 1160,
        h: 30,
        r: 30,
        g: 30,
        b: 40
    });

    // Draw dynamic progress bar indicators for health on the screen
    const healthW = Math.floor(1140 * (health / 1500));
    if (healthW > 0) {
        ops.push({
            type: "draw_rect",
            x: 70,
            y: 540,
            w: healthW,
            h: 10,
            r: health < 300 ? 220 : 46,
            g: health < 300 ? 50 : 204,
            b: 64
        });
    }

    // Victory/Defeat text flashes
    if (gameWon) {
        ops.push({
            type: "draw_rect",
            x: 440,
            y: 220,
            w: 400,
            h: 80,
            r: 0,
            g: 180,
            b: 0
        });
    } else if (gameOver) {
        ops.push({
            type: "draw_rect",
            x: 440,
            y: 220,
            w: 400,
            h: 80,
            r: 180,
            g: 0,
            b: 0
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
