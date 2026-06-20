const puppeteer = require('puppeteer-core');
const { spawn } = require('child_process');
const { PNG } = require('pngjs');
const path = require('path');
const readline = require('readline');

const linuxKeyMap = {
    1: 'Escape',
    2: '1', 3: '2', 4: '3', 5: '4', 6: '5', 7: '6', 8: '7', 9: '8', 10: '9', 11: '0',
    12: '-', 13: '=', 14: 'Backspace',
    15: 'Tab',
    16: 'q', 17: 'w', 18: 'e', 19: 'r', 20: 't', 21: 'y', 22: 'u', 23: 'i', 24: 'o', 25: 'p',
    26: '[', 27: ']', 28: 'Enter',
    29: 'Control',
    30: 'a', 31: 's', 32: 'd', 33: 'f', 34: 'g', 35: 'h', 36: 'j', 37: 'k', 38: 'l',
    39: ';', 40: "'", 41: '`',
    42: 'Shift', 43: '\\',
    44: 'z', 45: 'x', 46: 'c', 47: 'v', 48: 'b', 49: 'n', 50: 'm',
    51: ',', 52: '.', 53: '/',
    54: 'Shift', 56: 'Alt', 57: ' ',
    103: 'ArrowUp', 105: 'ArrowLeft', 106: 'ArrowRight', 108: 'ArrowDown'
};

const linuxButtonMap = {
    272: 'left',
    273: 'right',
    274: 'middle'
};

async function main() {
    const url = process.argv[2] || "https://youtube.com";
    console.log("=== Auncient ROOTED Node.js Browser Controller ===");
    console.log(`Target URL: ${url}`);

    // 1. Launch presenter process
    const presenterPath = path.join(__dirname, "../tsfi2-deepseek/tests/rooted_frame_presenter");
    console.log(`[PRESENTER] Spawning ${presenterPath}`);
    const presenter = spawn(presenterPath, [], {
        env: {
            ...process.env,
            WAYLAND_DISPLAY: process.env.WAYLAND_DISPLAY || "wayland-1"
        }
    });

    presenter.stderr.on("data", data => {
        console.error(`[PRESENTER ERR] ${data.toString().trim()}`);
    });

    // 2. Launch Puppeteer
    console.log("[PUPPETEER] Launching system Google Chrome...");
    const browser = await puppeteer.launch({
        executablePath: "/usr/bin/google-chrome",
        headless: true,
        args: [
            "--no-sandbox",
            "--disable-setuid-sandbox",
            "--disable-gpu",
            "--window-size=800,600",
            "--autoplay-policy=no-user-gesture-required"
        ]
    });

    const page = await browser.newPage();
    await page.setViewport({ width: 800, height: 600 });
    
    console.log(`[PUPPETEER] Navigating to ${url}...`);
    await page.goto(url, { waitUntil: "networkidle2" });
    console.log("[PUPPETEER] Navigation complete. Streaming frames...");

    try {
        const consentButton = await page.$('button[aria-label*="Accept"], button[aria-label*="Agree"], ytd-button-renderer yt-button-shape button');
        if (consentButton) {
            console.log("[PUPPETEER] Clicking consent/agree button...");
            await consentButton.click();
            await new Promise(resolve => setTimeout(resolve, 2000));
        }
    } catch (e) {
        console.log("[PUPPETEER] Consent check skipped: " + e.message);
    }

    try {
        console.log("[PUPPETEER] Identifying search input box...");
        const searchInputSelector = 'input[name="search_query"], input#search';
        await page.waitForSelector(searchInputSelector, { timeout: 10000 });
        console.log("[PUPPETEER] Search input identified. Typing 'Atropa'...");
        await page.focus(searchInputSelector);
        await page.type(searchInputSelector, "Atropa", { delay: 150 });
        console.log("[PUPPETEER] Performing search by pressing Enter...");
        await page.keyboard.press("Enter");
    } catch (e) {
        console.log("[PUPPETEER] Automatic search input identification failed: " + e.message);
    }

    let active = true;
    presenter.on("exit", (code, signal) => {
        console.log(`[PRESENTER] Exited with code=${code}, signal=${signal}. Stopping stream.`);
        active = false;
        browser.close();
    });

    // Read input events from presenter stdout and route them to Puppeteer
    const rl = readline.createInterface({
        input: presenter.stdout,
        terminal: false
    });

    rl.on('line', async (line) => {
        const parts = line.split(' ');
        const cmd = parts[0];
        try {
            if (cmd === 'MOUSE_MOVE') {
                const x = parseInt(parts[1]);
                const y = parseInt(parts[2]);
                await page.mouse.move(x, y);
            } else if (cmd === 'MOUSE_DOWN') {
                const btn = parseInt(parts[1]);
                const button = linuxButtonMap[btn] || 'left';
                await page.mouse.down({ button });
            } else if (cmd === 'MOUSE_UP') {
                const btn = parseInt(parts[1]);
                const button = linuxButtonMap[btn] || 'left';
                await page.mouse.up({ button });
            } else if (cmd === 'KEY_DOWN') {
                const key = parseInt(parts[1]) + 8; // Adjust Wayland keycode offset (evdev keycode - 8)
                const keyName = linuxKeyMap[key];
                if (keyName) {
                    await page.keyboard.down(keyName);
                }
            } else if (cmd === 'KEY_UP') {
                const key = parseInt(parts[1]) + 8; // Adjust Wayland keycode offset (evdev keycode - 8)
                const keyName = linuxKeyMap[key];
                if (keyName) {
                    await page.keyboard.up(keyName);
                }
            } else {
                console.log(`[PRESENTER OUT] ${line}`);
            }
        } catch (err) {
            // Suppress error if Puppeteer is closed or navigating
        }
    });

    // Frame capture loop
    const frameInterval = 1000 / 5; // 5 FPS target (concept demo)
    let frameCount = 0;

    async function captureLoop() {
        while (active) {
            const startTime = Date.now();
            try {
                const pngBuffer = await page.screenshot({ type: 'png' });
                const rawData = await new Promise((resolve, reject) => {
                    new PNG().parse(pngBuffer, (err, parsed) => {
                        if (err) reject(err);
                        else resolve(parsed.data);
                    });
                });

                if (active && presenter.stdin.writable) {
                    const ok = presenter.stdin.write(rawData);
                    if (!ok) {
                        await new Promise(resolve => presenter.stdin.once('drain', resolve));
                    }
                    frameCount++;
                    if (frameCount % 30 === 0) {
                        console.log(`[STREAM] Sent ${frameCount} frames.`);
                    }
                }
            } catch (err) {
                console.error("[CAPTURE ERR]", err);
            }

            const elapsed = Date.now() - startTime;
            const delay = Math.max(0, frameInterval - elapsed);
            await new Promise(resolve => setTimeout(resolve, delay));
        }
    }

    captureLoop();
}

main().catch(err => {
    console.error("FATAL ERROR:", err);
});
