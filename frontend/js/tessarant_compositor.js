/**
 * tessarant_compositor.js
 * Handles 4D rotating tessarant vertex geometry and canvas composition algorithms.
 * Adheres to the Auncient VM depth specs.
 */

(function() {
    const baseVertices = [
        {x:-1, y:-1, z:-1, w:-1}, {x:-1, y:-1, z:-1, w:1}, {x:-1, y:-1, z:1, w:-1}, {x:-1, y:-1, z:1, w:1},
        {x:-1, y:1, z:-1, w:-1}, {x:-1, y:1, z:-1, w:1}, {x:-1, y:1, z:1, w:-1}, {x:-1, y:1, z:1, w:1},
        {x:1, y:-1, z:-1, w:-1}, {x:1, y:-1, z:-1, w:1}, {x:1, y:-1, z:1, w:-1}, {x:1, y:-1, z:1, w:1},
        {x:1, y:1, z:-1, w:-1}, {x:1, y:1, z:-1, w:1}, {x:1, y:1, z:1, w:-1}, {x:1, y:1, z:1, w:1}
    ];

    const edges = [];
    for (let i = 0; i < 16; i++) {
        for (let j = i + 1; j < 16; j++) {
            let diffs = 0;
            if (baseVertices[i].x !== baseVertices[j].x) diffs++;
            if (baseVertices[i].y !== baseVertices[j].y) diffs++;
            if (baseVertices[i].z !== baseVertices[j].z) diffs++;
            if (baseVertices[i].w !== baseVertices[j].w) diffs++;
            if (diffs === 1) edges.push([i, j]);
        }
    }

    const cards = [
        { name: "TIM EXPLORER", maxHP: 120, color: "rgba(0, 210, 255, 0.8)", wireColor: "rgba(255, 235, 59, 0.9)", attacks: [{name: "SCSI Probe", dmg: 40}, {name: "4D Warp Spin", dmg: 80}] },
        { name: "MINER 2049ER", maxHP: 150, color: "rgba(39, 174, 96, 0.8)", wireColor: "rgba(46, 204, 113, 0.9)", attacks: [{name: "Pickaxe Swing", dmg: 35}, {name: "Nuclear Charge", dmg: 90}] },
        { name: "CIDER SPIDER", maxHP: 100, color: "rgba(230, 126, 34, 0.8)", wireColor: "rgba(241, 196, 15, 0.9)", attacks: [{name: "Web Constrict", dmg: 45}, {name: "Venom Bite", dmg: 75}] }
    ];

    const opponents = [
        { name: "PTERO-TESSARANT", maxHP: 110, color: "rgba(255, 0, 127, 0.8)", wireColor: "rgba(155, 89, 182, 0.9)", attacks: [{name: "Dampening Wave", dmg: 30}, {name: "Hyper-Projection", dmg: 70}] },
        { name: "GORGON PHASER", maxHP: 130, color: "rgba(155, 89, 182, 0.8)", wireColor: "rgba(231, 76, 60, 0.9)", attacks: [{name: "Phaser Blast", dmg: 40}, {name: "Soft Diffusion", dmg: 80}] }
    ];

    const defaultSpritePatterns = {
        0: [0x000c00, 0x000c00, 0x001e00, 0x001e00, 0x003300, 0x003300, 0x007f80, 0x00ffff, 0x01ffe0, 0x03fff0, 0x07ffc0, 0x07ffc0, 0x07ffc0, 0x03ffc0, 0x01ff80, 0x00ff80, 0x007e00, 0x003c00, 0x001800, 0x001800, 0x000000],
        1: [0x000180, 0x0003c0, 0x0007e0, 0x000ff0, 0x001ff8, 0x003ffc, 0x007ffe, 0x00ff7f, 0x01e03c, 0x03c01e, 0x03c01e, 0x01e03c, 0x00ff7f, 0x007ffe, 0x003ffc, 0x001ff8, 0x000ff0, 0x0007e0, 0x0003c0, 0x000180, 0x000000],
        2: [0x0003c0, 0x0007e0, 0x000ff0, 0x001ff8, 0x003c3c, 0x007c3e, 0x00fcf3, 0x00ffc0, 0x00ffd0, 0x00ffd8, 0x00fbe4, 0x00f9fc, 0x00fc3e, 0x007c1e, 0x003c00, 0x001800, 0x001800, 0x003c00, 0x000000, 0x000000, 0x000000],
        3: [0x0003c0, 0x000ff0, 0x001ff8, 0x003e7c, 0x007c3e, 0x00f81f, 0x00f81f, 0x00ffdf, 0x00ffd8, 0x00ffd8, 0x00ffd8, 0x00fbd8, 0x00f8d8, 0x00fc78, 0x007c30, 0x003c00, 0x001800, 0x001800, 0x003c00, 0x000000, 0x000000]
    };

    function drawTessarant(ctx, cx, cy, radius, color, frame) {
        const t = frame * 0.04;
        const cosT = Math.cos(t);
        const sinT = Math.sin(t);
        const cosT2 = Math.cos(t * 1.5);
        const sinT2 = Math.sin(t * 1.5);
        const d = 2.0;

        const projected = baseVertices.map(v => {
            let x1 = v.x * cosT - v.w * sinT;
            let w1 = v.x * sinT + v.w * cosT;
            let y1 = v.y * cosT2 - v.z * sinT2;
            let z1 = v.y * sinT2 + v.z * cosT2;
            const denom = d - w1;
            return {
                x: cx + (x1 / denom) * radius,
                y: cy + (y1 / denom) * radius
            };
        });

        ctx.strokeStyle = color;
        ctx.lineWidth = 0.5;
        ctx.beginPath();
        for (const edge of edges) {
            ctx.moveTo(projected[edge[0]].x, projected[edge[0]].y);
            ctx.lineTo(projected[edge[1]].x, projected[edge[1]].y);
        }
        ctx.stroke();
    }

    function drawEggCard(ctx, cardX, cardY, frame, eggIndex) {
        // Draw holographic card border
        ctx.strokeStyle = "rgba(0, 210, 255, 0.8)";
        ctx.lineWidth = 1;
        ctx.strokeRect(cardX - 5, cardY - 8, 10, 14);
        ctx.fillStyle = "rgba(0, 210, 255, 0.15)";
        ctx.fillRect(cardX - 5, cardY - 8, 10, 14);

        // Project and draw tessarant wireframe
        const t = (frame + eggIndex * 20) * 0.05;
        const cosT = Math.cos(t);
        const sinT = Math.sin(t);
        const cosT2 = Math.cos(t * 1.3);
        const sinT2 = Math.sin(t * 1.3);
        const d = 2.2;

        const projectedPoints = baseVertices.map(v => {
            let x1 = v.x * cosT - v.w * sinT;
            let w1 = v.x * sinT + v.w * cosT;
            let y1 = v.y * cosT2 - v.z * sinT2;
            let z1 = v.y * sinT2 + v.z * cosT2;
            
            const denom = d - w1;
            const x3 = x1 / denom;
            const y3 = y1 / denom;
            
            return {
                x: cardX + x3 * 3,
                y: cardY + y3 * 3
            };
        });

        ctx.strokeStyle = "rgba(255, 255, 255, 0.9)";
        ctx.lineWidth = 0.5;
        ctx.beginPath();
        for (const edge of edges) {
            const p1 = projectedPoints[edge[0]];
            const p2 = projectedPoints[edge[1]];
            ctx.moveTo(p1.x, p1.y);
            ctx.lineTo(p2.x, p2.y);
        }
        ctx.stroke();
    }

    function drawSpriteIcon(ctx, index, x, y, color, hasCollision, zDepth, dnaFrame, customPatterns, currentGame) {
        const mappedX = (x % 320);
        const mappedY = (y % 200);

        let stretchOffset = 0;
        let displayColor = color;
        let diffusionOpacity = 0.25;
        let pulseScale = 1.0;

        if (dnaFrame) {
            displayColor = dnaFrame.color || color;
            stretchOffset = dnaFrame.stretch * 4;
            diffusionOpacity = Math.min(0.8, 0.2 + dnaFrame.light * 0.5);
            pulseScale = 1.0 + Math.sin(dnaFrame.pulse * Math.PI) * 0.1;
        }

        // Try local storage for hot updates
        try {
            const stored = localStorage.getItem(`c64_shared_sprite_${index}`);
            if (stored && customPatterns) {
                customPatterns[index] = JSON.parse(stored);
            }
        } catch (e) {}

        let pattern = (customPatterns && customPatterns[index]) || defaultSpritePatterns[index] || [];
        
        // Shadow layer
        if (zDepth > 0) {
            ctx.fillStyle = "rgba(0, 0, 0, 0.4)";
            const offset = Math.min(8, zDepth / 4);
            for (let r = 0; r < 21; r++) {
                const row_val = pattern[r] || 0;
                const drawY = mappedY + r + stretchOffset * (r / 21);
                for (let c = 0; c < 24; c++) {
                    if ((row_val & (1 << (23 - c))) !== 0) {
                        ctx.fillRect(mappedX + c + offset, drawY + offset, 1, 1);
                    }
                }
            }
        }

        // Main Layer
        for (let r = 0; r < 21; r++) {
            const row_val = pattern[r] || 0;
            const drawY = mappedY + r + stretchOffset * (r / 21);

            if (index === 3 && currentGame === "gorf") {
                const laserHue = (Date.now() / 3 + r * 8) % 360;
                const gradient = ctx.createLinearGradient(mappedX - 25, drawY, mappedX + 49, drawY);
                gradient.addColorStop(0, "rgba(0,0,0,0)");
                gradient.addColorStop(0.3, `hsla(${laserHue}, 100%, 50%, 0.1)`);
                gradient.addColorStop(0.5, `hsla(${laserHue}, 100%, 80%, 0.9)`);
                gradient.addColorStop(0.7, `hsla(${laserHue}, 100%, 50%, 0.1)`);
                gradient.addColorStop(1, "rgba(0,0,0,0)");
                ctx.fillStyle = gradient;
                ctx.fillRect(mappedX - 25, drawY, 74, 1);
                
                if (r === 10) {
                    const time = Date.now() / 200;
                    const pulseRadius = (time % 1) * 18;
                    ctx.strokeStyle = `hsla(${laserHue}, 100%, 70%, ${1 - (time % 1)})`;
                    ctx.lineWidth = 1;
                    ctx.beginPath();
                    ctx.arc(mappedX + 12, drawY, pulseRadius, 0, Math.PI * 2);
                    ctx.stroke();
                }
            }

            for (let c = 0; c < 24; c++) {
                if ((row_val & (1 << (23 - c))) !== 0) {
                    if (index === 3 && currentGame === "gorf") {
                        const hue = (Date.now() / 2 + r * 12 + c * 4) % 360;
                        ctx.fillStyle = `hsl(${hue}, 100%, 75%)`;
                    } else if (index === 3) {
                        if (r <= 5) ctx.fillStyle = "#e6dfd3";
                        else if (r <= 15) ctx.fillStyle = "#3a86c8";
                        else ctx.fillStyle = "#8c7241";
                    } else if (index === 2) {
                        if (r <= 6) ctx.fillStyle = "#800080";
                        else ctx.fillStyle = "#25201b";
                    } else if (index === 1) {
                        if (r === 4 && (c === 11 || c === 12)) ctx.fillStyle = "#ff0000";
                        else ctx.fillStyle = "#0d0c0a";
                    } else {
                        ctx.fillStyle = displayColor;
                    }
                    ctx.fillRect(mappedX + c, drawY, 1, 1);
                }
            }
        }

        // Volumetric Soft Diffusion Texture Overlay
        if (zDepth > 0) {
            for (let r = 0; r < 21; r++) {
                const row_val = pattern[r] || 0;
                const drawY = mappedY + r + stretchOffset * (r / 21);
                for (let c = 0; c < 24; c++) {
                    if ((row_val & (1 << (23 - c))) !== 0) {
                        const noise = Math.sin((mappedX + c) * 12.9898 + (drawY) * 78.233) * 43758.5453;
                        const noiseFract = noise - Math.floor(noise);
                        if (noiseFract > 0.6) {
                            ctx.fillStyle = dnaFrame ? dnaFrame.eyeColor : `rgba(255, 255, 255, ${diffusionOpacity})`;
                            ctx.fillRect(mappedX + c, drawY, 1, 1);
                        }
                    }
                }
            }
        }
    }

    function drawScrollingBanner(ctx, message, yPos, frame, speed = 1.5, textFont = "bold 9px monospace") {
        const textWidth = ctx.measureText(message).width || 300;
        const xOffset = (frame * speed) % (textWidth + ctx.canvas.width);
        const startX = ctx.canvas.width - xOffset;

        ctx.font = textFont;
        ctx.textAlign = "left";

        for (let i = 0; i < message.length; i++) {
            const charX = startX + i * 8;
            if (charX >= -10 && charX <= ctx.canvas.width + 10) {
                const yOffset = Math.sin(frame * 0.1 + i * 0.3) * 3;
                const charHue = (frame + i * 12) % 360;
                ctx.fillStyle = `hsl(${charHue}, 100%, 75%)`;
                ctx.fillText(message[i], charX, yPos + yOffset);
            }
        }
    }

    function drawCRTScanlines(ctx, width, height, frame) {
        // Horizontal Scanlines
        ctx.fillStyle = "rgba(0, 0, 0, 0.08)";
        for (let y = 0; y < height; y += 2) {
            ctx.fillRect(0, y, width, 1);
        }

        // CRT Shadow Mask / Vignette
        const gradient = ctx.createRadialGradient(width/2, height/2, width/3, width/2, height/2, width/2);
        gradient.addColorStop(0, "rgba(0,0,0,0)");
        gradient.addColorStop(0.8, "rgba(0,0,0,0.05)");
        gradient.addColorStop(1, "rgba(0,0,0,0.3)");
        ctx.fillStyle = gradient;
        ctx.fillRect(0, 0, width, height);

        // Screen flicker overlay
        if (Math.sin(frame * 0.5) > 0.98) {
            ctx.fillStyle = "rgba(255, 255, 255, 0.02)";
            ctx.fillRect(0, 0, width, height);
        }
    }

    function drawScreenTransition(ctx, frame, maxFrames = 30) {
        const ratio = frame / maxFrames;
        if (ratio >= 1.0) return;

        ctx.fillStyle = "#000000";
        // Grid wipe dissolve
        const gridSize = 16;
        for (let y = 0; y < ctx.canvas.height; y += gridSize) {
            for (let x = 0; x < ctx.canvas.width; x += gridSize) {
                const cellProgress = (Math.sin(x * 0.01 + y * 0.02) + 1) / 2;
                if (cellProgress > ratio) {
                    ctx.fillRect(x, y, gridSize, gridSize);
                }
            }
        }
    }

    function drawHUD(ctx, score, lives, title, level, extraText, frame) {
        ctx.fillStyle = "#39ff14";
        ctx.font = "bold 8px monospace";
        
        // Draw Left title
        ctx.textAlign = "left";
        ctx.fillText(title.toUpperCase(), 10, 12);

        // Draw Right score & lives
        ctx.textAlign = "right";
        const paddedScore = String(score || 0).padStart(6, '0');
        ctx.fillText(`LIVES: ${String(lives || 3).padStart(2, '0')}  SCORE: ${paddedScore}`, ctx.canvas.width - 10, 12);

        // Draw Center Level / Extra info
        ctx.textAlign = "center";
        const glowingHue = (frame * 2) % 360;
        ctx.fillStyle = `hsl(${glowingHue}, 100%, 75%)`;
        ctx.fillText(extraText ? extraText.toUpperCase() : `LEVEL ${level || 1}`, ctx.canvas.width / 2, 12);
    }

    function checkAABBCollision(x1, y1, w1, h1, x2, y2, w2, h2) {
        return x1 < x2 + w2 &&
               x1 + w1 > x2 &&
               y1 < y2 + h2 &&
               y1 + h1 > y2;
    }

    function checkCircleCollision(x1, y1, r1, x2, y2, r2) {
        const dx = x1 - x2;
        const dy = y1 - y2;
        const dist = Math.sqrt(dx * dx + dy * dy);
        return dist < (r1 + r2);
    }

    class ParticleEngine {
        constructor() {
            this.particles = [];
        }

        spawn(x, y, vx, vy, color, size, life) {
            this.particles.push({ x, y, vx, vy, color, size, initialLife: life, life });
        }

        updateAndDraw(ctx) {
            const neighborDist = 45;
            const separationDist = 10;
            const maxSpeed = 2.5;
            const maxForce = 0.15;

            // Pre-calculate flocking forces for each particle to avoid in-place update artifacts
            const steerForces = this.particles.map((p, idx) => {
                let sepX = 0, sepY = 0;
                let alignX = 0, alignY = 0;
                let cohX = 0, cohY = 0;
                let sepCount = 0, neighborCount = 0;

                for (let j = 0; j < this.particles.length; j++) {
                    if (idx === j) continue;
                    const other = this.particles[j];
                    const dx = p.x - other.x;
                    const dy = p.y - other.y;
                    const dist = Math.sqrt(dx * dx + dy * dy);

                    if (dist > 0 && dist < neighborDist) {
                        // Alignment accumulation
                        alignX += other.vx;
                        alignY += other.vy;

                        // Cohesion accumulation
                        cohX += other.x;
                        cohY += other.y;

                        neighborCount++;

                        if (dist < separationDist) {
                            // Separation: force is inversely proportional to distance
                            sepX += dx / dist;
                            sepY += dy / dist;
                            sepCount++;
                        }
                    }
                }

                let steerX = 0, steerY = 0;

                if (sepCount > 0) {
                    sepX /= sepCount;
                    sepY /= sepCount;
                    const mag = Math.sqrt(sepX * sepX + sepY * sepY);
                    if (mag > 0) {
                        sepX = (sepX / mag) * maxSpeed - p.vx;
                        sepY = (sepY / mag) * maxSpeed - p.vy;
                        steerX += sepX * 1.5;
                        steerY += sepY * 1.5;
                    }
                }

                if (neighborCount > 0) {
                    // Alignment force
                    alignX /= neighborCount;
                    alignY /= neighborCount;
                    const alignMag = Math.sqrt(alignX * alignX + alignY * alignY);
                    if (alignMag > 0) {
                        alignX = (alignX / alignMag) * maxSpeed - p.vx;
                        alignY = (alignY / alignMag) * maxSpeed - p.vy;
                        steerX += alignX * 1.0;
                        steerY += alignY * 1.0;
                    }

                    // Cohesion force
                    cohX /= neighborCount;
                    cohY /= neighborCount;
                    let targetX = cohX - p.x;
                    let targetY = cohY - p.y;
                    const cohMag = Math.sqrt(targetX * targetX + targetY * targetY);
                    if (cohMag > 0) {
                        targetX = (targetX / cohMag) * maxSpeed - p.vx;
                        targetY = (targetY / cohMag) * maxSpeed - p.vy;
                        steerX += targetX * 0.8;
                        steerY += targetY * 0.8;
                    }
                }

                // Limit steering force
                const forceMag = Math.sqrt(steerX * steerX + steerY * steerY);
                if (forceMag > maxForce) {
                    steerX = (steerX / forceMag) * maxForce;
                    steerY = (steerY / forceMag) * maxForce;
                }

                return { fx: steerX, fy: steerY };
            });

            for (let i = this.particles.length - 1; i >= 0; i--) {
                const p = this.particles[i];
                const sf = steerForces[i];

                // Update velocity with steering forces
                p.vx += sf.fx;
                p.vy += sf.fy;

                // Limit speed
                const speed = Math.sqrt(p.vx * p.vx + p.vy * p.vy);
                if (speed > maxSpeed) {
                    p.vx = (p.vx / speed) * maxSpeed;
                    p.vy = (p.vy / speed) * maxSpeed;
                }

                p.x += p.vx;
                p.y += p.vy;
                p.life--;

                if (p.life <= 0) {
                    this.particles.splice(i, 1);
                    continue;
                }

                ctx.fillStyle = p.color;
                const alpha = p.life / p.initialLife;
                ctx.globalAlpha = alpha;
                ctx.fillRect(p.x, p.y, p.size, p.size);
            }
            ctx.globalAlpha = 1.0; // Reset
        }
    }

    const HighscoreManager = {
        get(gameId) {
            try {
                return Number(localStorage.getItem(`c64_highscore_${gameId}`) || 0);
            } catch (e) {
                return 0;
            }
        },
        save(gameId, score) {
            try {
                const current = this.get(gameId);
                if (score > current) {
                    localStorage.setItem(`c64_highscore_${gameId}`, String(score));
                    return true; // New Highscore
                }
            } catch (e) {}
            return false;
        }
    };

    function drawBattleScreen(ctx, canvas, frame) {
        if (!window.dinoBattleState) {
            window.dinoBattleState = {
                stage: "select", // "select" or "battle"
                playerHP: 120,
                playerMaxHP: 120,
                opponentHP: 100,
                opponentMaxHP: 100,
                playerCard: null,
                opponentCard: null,
                log: "CHOOSE YOUR CHAMPION TO DEPLOY!",
                turn: "player"
            };
        }

        const bs = window.dinoBattleState;

        // Initialize key listener for card selection and battle
        if (!window.dinoBattleKeyListenerAddedGlobal) {
            window.dinoBattleKeyListenerAddedGlobal = true;
            window.addEventListener('keydown', (e) => {
                if (!window.dinoBattleMode) return;
                
                if (bs.stage === "select") {
                    let idx = -1;
                    if (e.key === '1') idx = 0;
                    if (e.key === '2') idx = 1;
                    if (e.key === '3') idx = 2;

                    if (idx !== -1) {
                        bs.playerCard = cards[idx];
                        bs.playerHP = bs.playerCard.maxHP;
                        bs.playerMaxHP = bs.playerCard.maxHP;

                        // Random opponent
                        bs.opponentCard = opponents[Math.floor(Math.random() * opponents.length)];
                        bs.opponentHP = bs.opponentCard.maxHP;
                        bs.opponentMaxHP = bs.opponentCard.maxHP;

                        bs.stage = "battle";
                        bs.log = `A WILD ${bs.opponentCard.name} APPEARS! GO, ${bs.playerCard.name}!`;
                        bs.turn = "player";
                    }
                    return;
                }

                if (bs.playerHP <= 0 || bs.opponentHP <= 0) {
                    if (e.key === 'r' || e.key === 'R') {
                        window.dinoBattleState = null;
                    }
                    return;
                }

                if (e.key === 'e' || e.key === 'E') {
                    window.dinoBattleMode = false;
                    return;
                }

                if (bs.turn !== "player") return;

                let atkIdx = -1;
                if (e.key === '1') atkIdx = 0;
                if (e.key === '2') atkIdx = 1;

                if (atkIdx !== -1) {
                    const atk = bs.playerCard.attacks[atkIdx];
                    bs.opponentHP = Math.max(0, bs.opponentHP - atk.dmg);
                    bs.log = `${bs.playerCard.name} USED ${atk.name.toUpperCase()} FOR ${atk.dmg} DMG!`;
                    bs.turn = "opponent";
                    
                    if (bs.opponentHP <= 0) {
                        bs.log = `${bs.opponentCard.name} FAINTED! VICTORY! (PRESS R)`;
                    } else {
                        setTimeout(() => {
                            if (!window.dinoBattleMode) return;
                            const oppAtk = bs.opponentCard.attacks[Math.floor(Math.random() * bs.opponentCard.attacks.length)];
                            bs.playerHP = Math.max(0, bs.playerHP - oppAtk.dmg);
                            bs.log = `${bs.opponentCard.name} USED ${oppAtk.name.toUpperCase()} FOR ${oppAtk.dmg} DMG!`;
                            if (bs.playerHP <= 0) {
                                bs.log = `${bs.playerCard.name} FAINTED! DEFEAT! (PRESS R)`;
                            } else {
                                bs.turn = "player";
                            }
                        }, 1200);
                    }
                }
            });
        }

        ctx.fillStyle = "#0c0812"; // Deep space void
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        if (bs.stage === "select") {
            ctx.fillStyle = "#ffffff";
            ctx.font = "bold 9px monospace";
            ctx.textAlign = "center";
            ctx.fillText("4D TESSARANT CARD SELECTION", canvas.width / 2, 20);

            cards.forEach((card, i) => {
                const cx = 10 + i * 100;
                const cy = 40;
                const cw = 85;
                const ch = 110;

                ctx.strokeStyle = card.color;
                ctx.lineWidth = 1.5;
                ctx.strokeRect(cx, cy, cw, ch);
                ctx.fillStyle = "rgba(0, 210, 255, 0.05)";
                ctx.fillRect(cx, cy, cw, ch);

                ctx.fillStyle = "#ffffff";
                ctx.font = "bold 6px monospace";
                ctx.fillText(card.name, cx + cw/2, cy + 12);
                ctx.fillText(`HP: ${card.maxHP}`, cx + cw/2, cy + 22);

                drawTessarant(ctx, cx + cw/2, cy + 55, 12, card.wireColor, frame + i * 20);

                ctx.fillStyle = "#39ff14";
                ctx.font = "bold 7px monospace";
                ctx.fillText(`PRESS [${i + 1}]`, cx + cw/2, cy + 95);
            });

            ctx.fillStyle = "#a2b4c2";
            ctx.font = "7px monospace";
            ctx.fillText(bs.log, canvas.width/2, 175);
            return;
        }

        // Draw Battle Stage
        // Render Player Card
        const cardLeftX = 40;
        const cardY = 25;
        const cardW = 90;
        const cardH = 125;

        ctx.strokeStyle = bs.playerCard.color;
        ctx.lineWidth = 2;
        ctx.strokeRect(cardLeftX, cardY, cardW, cardH);
        ctx.fillStyle = "rgba(0, 210, 255, 0.15)";
        ctx.fillRect(cardLeftX, cardY, cardW, cardH);

        ctx.fillStyle = "#ffffff";
        ctx.font = "bold 7px monospace";
        ctx.textAlign = "center";
        ctx.fillText(bs.playerCard.name, cardLeftX + cardW/2, cardY + 12);
        ctx.fillText(`HP: ${bs.playerHP}/${bs.playerMaxHP}`, cardLeftX + cardW/2, cardY + 22);

        drawTessarant(ctx, cardLeftX + cardW/2, cardY + 65, 15, bs.playerCard.wireColor, frame);

        // Render Opponent Card
        const cardRightX = 170;
        ctx.strokeStyle = bs.opponentCard.color;
        ctx.lineWidth = 2;
        ctx.strokeRect(cardRightX, cardY, cardW, cardH);
        ctx.fillStyle = "rgba(255, 0, 127, 0.15)";
        ctx.fillRect(cardRightX, cardY, cardW, cardH);

        ctx.fillStyle = "#ffffff";
        ctx.fillText(bs.opponentCard.name, cardRightX + cardW/2, cardY + 12);
        ctx.fillText(`HP: ${bs.opponentHP}/${bs.opponentMaxHP}`, cardRightX + cardW/2, cardY + 22);

        drawTessarant(ctx, cardRightX + cardW/2, cardY + 65, 15, bs.opponentCard.wireColor, frame + 30);

        // Draw status log box at bottom
        ctx.fillStyle = "#110b29";
        ctx.strokeStyle = "#4a154b";
        ctx.lineWidth = 1;
        ctx.fillRect(20, 158, canvas.width - 40, 25);
        ctx.strokeRect(20, 158, canvas.width - 40, 25);

        ctx.fillStyle = "#00ffcc";
        ctx.font = "8px monospace";
        ctx.textAlign = "center";
        ctx.fillText(bs.log, canvas.width/2, 174);

        if (bs.playerHP > 0 && bs.opponentHP > 0) {
            ctx.fillStyle = "#a2b4c2";
            ctx.font = "7px monospace";
            const atk1 = bs.playerCard.attacks[0].name.toUpperCase();
            const atk2 = bs.playerCard.attacks[1].name.toUpperCase();
            ctx.fillText(`ATTACKS: [1] ${atk1}   [2] ${atk2}   [E] ESCAPE`, canvas.width/2, 192);
        } else {
            ctx.fillStyle = "#ff007f";
            ctx.font = "8px monospace";
            ctx.fillText("PRESS [R] TO RE-INITIALIZE STATE", canvas.width/2, 192);
        }
    }

    window.TessarantCompositor = {
        baseVertices,
        edges,
        drawTessarant,
        drawEggCard,
        drawSpriteIcon,
        drawScrollingBanner,
        drawCRTScanlines,
        drawScreenTransition,
        drawHUD,
        checkAABBCollision,
        checkCircleCollision,
        ParticleEngine,
        HighscoreManager,
        drawBattleScreen
    };
})();
