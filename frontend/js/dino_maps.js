/**
 * dino_maps.js
 * Procedural map themes, hazards, visual layers, and elements for Dino Eggs.
 */

(function() {
    const THEMES = [
        {
            name: "VOLCANIC CORE",
            bgColor: "#1a0808",
            platformColor: "#d35400",
            vineColor: "#e67e22",
            pedestalColor: "#e74c3c",
            hazardColor: "#f1c40f",
            hasLava: true,
            windStrength: 0,
            friction: 0.8
        },
        {
            name: "AUNCIENT FOREST",
            bgColor: "#081c10",
            platformColor: "#27ae60",
            vineColor: "#2ecc71",
            pedestalColor: "#f39c12",
            hazardColor: "#1abc9c",
            hasLava: false,
            windStrength: 0.15, // drifting wind
            friction: 0.85
        },
        {
            name: "GLACIAL VOID",
            bgColor: "#0a192f",
            platformColor: "#5dade2",
            vineColor: "#a9cce3",
            pedestalColor: "#85c1e9",
            hazardColor: "#3498db",
            hasLava: false,
            windStrength: 0,
            friction: 0.95 // slippery ice
        }
    ];

    function generateMap(level) {
        const rand = (function(seed) {
            let s = seed;
            return function() {
                s = (s * 1103515245 + 12345) & 0x7fffffff;
                return s / 0x7fffffff;
            };
        })(level);

        const theme = THEMES[(level - 1) % THEMES.length];
        const numPlatforms = 3 + Math.floor(rand() * 3); // 3 to 5 platforms
        const platforms = [];
        platforms.push({ x: 40, y: 160, w: 240, type: "ground" }); // ground platform

        for (let i = 0; i < numPlatforms - 1; i++) {
            const y = 60 + i * 30;
            const w = 80 + Math.floor(rand() * 100);
            const x = 40 + Math.floor(rand() * (240 - w));
            platforms.push({ x, y, w, type: "floating" });
        }

        const vines = [];
        for (let i = 0; i < platforms.length - 1; i++) {
            const p1 = platforms[i];
            const p2 = platforms[i + 1];
            const minX = Math.max(p1.x, p2.x) + 10;
            const maxX = Math.min(p1.x + p1.w, p2.x + p2.w) - 10;
            if (maxX > minX) {
                const vx = minX + Math.floor(rand() * (maxX - minX));
                vines.push({ x: vx, y1: Math.min(p1.y, p2.y), y2: Math.max(p1.y, p2.y) });
            } else {
                const vx = p1.x + Math.floor(p1.w / 2);
                vines.push({ x: vx, y1: p1.y, y2: 170 });
            }
        }

        const nests = [];
        for (let i = 1; i < platforms.length; i++) {
            const p = platforms[i];
            if (rand() > 0.3) {
                const nx = p.x + 10 + Math.floor(rand() * (p.w - 20));
                nests.push({
                    x: nx,
                    y: p.y - 6,
                    eggs: 1 + Math.floor(rand() * 2),
                    collected: false,
                    hatchTimer: 400 + Math.floor(rand() * 500),
                    platformIndex: i
                });
            }
        }
        if (nests.length === 0) {
            nests.push({
                x: platforms[1].x + 15,
                y: platforms[1].y - 6,
                eggs: 2,
                collected: false,
                hatchTimer: 600,
                platformIndex: 1
            });
        }

        // Spawn crawling spiders on platforms
        const spiders = [];
        for (let i = 1; i < platforms.length; i++) {
            const p = platforms[i];
            spiders.push({
                x: p.x + p.w / 2,
                y: p.y - 4,
                vx: 0.5 + rand() * 0.5,
                plat: p
            });
        }

        // Add portals on higher levels
        const portals = [];
        if (level > 1) {
            portals.push({ x1: 50, y1: 50, x2: 270, y2: 150 });
        }

        return {
            level,
            theme,
            platforms,
            vines,
            nests,
            spiders,
            babyDinos: [],
            portals
        };
    }

    function drawMap(ctx, map, frame) {
        // Draw background theme color
        ctx.fillStyle = map.theme.bgColor;
        ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);

        // Draw volcanic lava if applicable
        if (map.theme.hasLava) {
            ctx.fillStyle = "rgba(231, 76, 60, 0.8)";
            ctx.fillRect(40, 180, 240, 20);
            for (let x = 40; x < 280; x += 10) {
                let waveY = 180 + Math.sin(frame * 0.1 + x * 0.2) * 3;
                ctx.fillStyle = "#f1c40f";
                ctx.fillRect(x, waveY, 5, 2);
            }
        }

        // Draw background theme particles
        if (!window.dinoParticles || window.dinoParticlesTheme !== map.theme.name) {
            window.dinoParticlesTheme = map.theme.name;
            window.dinoParticles = [];
            for (let i = 0; i < 20; i++) {
                window.dinoParticles.push({
                    x: Math.random() * 240 + 40,
                    y: Math.random() * 160 + 30,
                    size: Math.random() * 2 + 1,
                    speedY: Math.random() * 0.5 + 0.2,
                    speedX: (Math.random() - 0.5) * 0.3
                });
            }
        }
        ctx.fillStyle = map.theme.hazardColor;
        for (const p of window.dinoParticles) {
            p.y += p.speedY;
            p.x += p.speedX;
            if (map.theme.windStrength) {
                p.x += map.theme.windStrength * 0.5;
            }
            if (p.y > 180) {
                p.y = 30;
                p.x = Math.random() * 240 + 40;
            }
            if (p.x < 40 || p.x > 280) {
                p.x = Math.random() * 240 + 40;
            }
            ctx.fillRect(p.x, p.y, p.size, p.size);
        }

        // Draw platforms
        ctx.fillStyle = map.theme.platformColor;
        for (const p of map.platforms) {
            ctx.fillRect(p.x, p.y, p.w, 8);
        }

        // Draw green climbing vines (ladders)
        ctx.strokeStyle = map.theme.vineColor;
        ctx.lineWidth = 3;
        ctx.beginPath();
        for (const v of map.vines) {
            ctx.moveTo(v.x, v.y1);
            ctx.lineTo(v.x, v.y2);
        }
        ctx.stroke();

        // Draw vine leaves
        ctx.fillStyle = map.theme.vineColor;
        for (const v of map.vines) {
            for (let y = v.y1 + 5; y < v.y2; y += 10) {
                ctx.fillRect(v.x - 5, y, 4, 2);
                ctx.fillRect(v.x + 1, y + 4, 4, 2);
            }
        }

        // Draw portals
        ctx.lineWidth = 1.5;
        for (const p of map.portals) {
            ctx.strokeStyle = "rgba(155, 89, 182, 0.8)";
            ctx.beginPath();
            ctx.arc(p.x1, p.y1, 6, 0, Math.PI * 2);
            ctx.stroke();

            ctx.strokeStyle = "rgba(52, 152, 219, 0.8)";
            ctx.beginPath();
            ctx.arc(p.x2, p.y2, 6, 0, Math.PI * 2);
            ctx.stroke();
        }

        // Draw Crawling Spiders (Hazards)
        if (map.spiders) {
            ctx.fillStyle = "#e74c3c";
            for (const s of map.spiders) {
                // Draw body
                ctx.fillRect(s.x - 3, s.y - 3, 6, 4);
                // Draw blinking eyes
                ctx.fillStyle = "#ffffff";
                ctx.fillRect(s.x - 2, s.y - 2, 1, 1);
                ctx.fillRect(s.x + 1, s.y - 2, 1, 1);
                ctx.fillStyle = "#e74c3c";
                // Draw legs animation
                let legOffset = Math.sin(frame * 0.2 + s.x) * 2;
                ctx.fillRect(s.x - 5, s.y - 1 + legOffset, 2, 1);
                ctx.fillRect(s.x + 3, s.y - 1 - legOffset, 2, 1);
            }
        }

        // Draw Baby Dinosaurs
        if (map.babyDinos) {
            for (const b of map.babyDinos) {
                ctx.fillStyle = "#2ecc71"; // bright green baby dino
                ctx.fillRect(b.x - 4, b.y - 6, 8, 6);
                // Tail
                ctx.fillRect(b.x - 6, b.y - 4, 2, 2);
                // Head
                ctx.fillRect(b.x + (b.dir > 0 ? 3 : -5), b.y - 8, 3, 3);
            }
        }
    }

    function updatePlayer(win, map, interactiveKeys, isDemo, frame) {
        if (!win.vaultX) {
            win.vaultX = 144;
            win.vaultY = 120;
        }

        if (isDemo) {
            win.vaultX = 100 + Math.sin(frame * 0.02) * 50;
            win.vaultY = 140 - Math.abs(Math.sin(frame * 0.07)) * 12;
            return;
        }

        win.dinoVx = win.dinoVx || 0;
        win.dinoVy = win.dinoVy || 0;

        let onGround = false;
        let nearVine = false;

        // Check if player is overlapping with any vine for climbing
        for (const v of map.vines) {
            if (Math.abs(win.vaultX - v.x) < 8 && win.vaultY >= v.y1 && win.vaultY <= v.y2 + 4) {
                nearVine = true;
                break;
            }
        }

        // Check if player is on top of any platform
        for (const p of map.platforms) {
            if (win.vaultX >= p.x - 4 && win.vaultX <= p.x + p.w + 4) {
                if (win.vaultY >= p.y - 2 && win.vaultY <= p.y + 4 && win.dinoVy >= 0) {
                    onGround = true;
                    win.vaultY = p.y;
                    win.dinoVy = 0;
                    break;
                }
            }
        }

        // Move Left / Right with Friction
        let f = map.theme.friction || 0.85;
        let moveSpeed = 0.5;
        if (interactiveKeys.left) {
            win.dinoVx = -1.8;
        } else if (interactiveKeys.right) {
            win.dinoVx = 1.8;
        } else {
            win.dinoVx *= f;
        }

        // Wind drift force
        if (map.theme.windStrength) {
            win.dinoVx += map.theme.windStrength;
        }

        // Climb up or down if near vine
        if (nearVine) {
            if (interactiveKeys.up) {
                win.vaultY -= 1.5;
                win.dinoVy = 0;
                onGround = true;
            } else if (interactiveKeys.down) {
                win.vaultY += 1.5;
                win.dinoVy = 0;
                onGround = true;
            }
        }

        // Handle Jumping
        if (onGround && interactiveKeys.up && !nearVine) {
            win.dinoVy = -3.8;
            onGround = false;
            if (win.cartridgeAudio) win.cartridgeAudio.playJump();
        }

        // Apply Gravity
        if (!onGround && !nearVine) {
            win.dinoVy += 0.25; // gravity acceleration
            if (win.dinoVy > 4.5) win.dinoVy = 4.5; // terminal velocity
        }

        win.vaultX += win.dinoVx;
        win.vaultY += win.dinoVy;

        // Portal Teleportation
        win.dinoTeleportCooldown = win.dinoTeleportCooldown || 0;
        if (win.dinoTeleportCooldown > 0) {
            win.dinoTeleportCooldown--;
        } else {
            for (const p of map.portals) {
                if (Math.hypot(win.vaultX - p.x1, win.vaultY - p.y1) < 10) {
                    win.vaultX = p.x2;
                    win.vaultY = p.y2;
                    win.dinoTeleportCooldown = 45; // 45 frames cooldown
                    break;
                } else if (Math.hypot(win.vaultX - p.x2, win.vaultY - p.y2) < 10) {
                    win.vaultX = p.x1;
                    win.vaultY = p.y1;
                    win.dinoTeleportCooldown = 45;
                    break;
                }
            }
        }

        // Boundaries check
        if (win.vaultX < 40) { win.vaultX = 40; win.dinoVx = 0; }
        if (win.vaultX > 280) { win.vaultX = 280; win.dinoVx = 0; }
        if (win.vaultY < 30) { win.vaultY = 30; win.dinoVy = 0; }
        if (win.vaultY > 190) { win.vaultY = 190; win.dinoVy = 0; }

        // Lava Hazard resets player
        if (map.theme.hasLava && win.vaultY >= 170) {
            win.vaultX = 144;
            win.vaultY = 120;
            win.dinoVx = 0;
            win.dinoVy = 0;
            if (win.cartridgeAudio) win.cartridgeAudio.playExplosion();
        }

        // Update Crawling Spiders
        if (map.spiders) {
            for (const s of map.spiders) {
                s.x += s.vx;
                if (s.x < s.plat.x || s.x > s.plat.x + s.plat.w) {
                    s.vx = -s.vx;
                }
                // Collision with Tim
                if (Math.hypot(win.vaultX - s.x, win.vaultY - s.y) < 10) {
                    // reset player position
                    win.vaultX = 144;
                    win.vaultY = 120;
                    win.dinoVx = 0;
                    win.dinoVy = 0;
                    if (win.cartridgeAudio) win.cartridgeAudio.playExplosion();
                }
            }
        }

        // Update Egg Hatching
        for (const nest of map.nests) {
            if (!nest.collected && nest.eggs > 0) {
                nest.hatchTimer--;
                if (nest.hatchTimer <= 0) {
                    nest.eggs--;
                    nest.hatchTimer = 400 + Math.random() * 500;
                    // Spawn baby dino
                    map.babyDinos.push({
                        x: nest.x,
                        y: nest.y + 4,
                        vx: 0.5,
                        dir: 1,
                        plat: map.platforms[nest.platformIndex]
                    });
                }
            }
        }

        // Update Baby Dinos
        if (map.babyDinos) {
            for (let i = map.babyDinos.length - 1; i >= 0; i--) {
                const b = map.babyDinos[i];
                b.x += b.vx * b.dir;

                // Turn around at platform edges
                if (b.x < b.plat.x || b.x > b.plat.x + b.plat.w) {
                    b.dir = -b.dir;
                }

                // Rescue collision with player
                if (Math.hypot(win.vaultX - b.x, win.vaultY - b.y) < 12) {
                    map.babyDinos.splice(i, 1);
                    // Add score/rescue increments
                    win.dinoScore = (win.dinoScore || 0) + 250;
                    if (win.cartridgeAudio) win.cartridgeAudio.playPickup();
                }
            }
        }
    }

    window.DinoMapGenerator = {
        generateMap,
        drawMap,
        updatePlayer
    };
})();
