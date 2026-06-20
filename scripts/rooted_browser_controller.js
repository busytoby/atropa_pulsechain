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

    let presenterReadyResolver;
    const presenterReady = new Promise((resolve) => {
        presenterReadyResolver = resolve;
    });

    // Define helper function to route input events to Puppeteer
    let page = null;
    let controlDown = false;
    async function handleInputCommand(line) {
        if (line.includes("Streaming starting...")) {
            console.log(`[PRESENTER OUT] ${line}`);
            if (presenterReadyResolver) {
                presenterReadyResolver();
            }
            return;
        }
        if (!page) return;
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
                if (keyName === 'Control') {
                    controlDown = true;
                }
                if (keyName === 'r' && controlDown) {
                    console.log("[PUPPETEER] Control+R detected. Reloading page...");
                    await page.reload({ waitUntil: "networkidle2" });
                } else if (keyName) {
                    await page.keyboard.down(keyName);
                }
            } else if (cmd === 'KEY_UP') {
                const key = parseInt(parts[1]) + 8; // Adjust Wayland keycode offset (evdev keycode - 8)
                const keyName = linuxKeyMap[key];
                if (keyName === 'Control') {
                    controlDown = false;
                }
                if (keyName) {
                    await page.keyboard.up(keyName);
                }
            } else {
                console.log(`[PRESENTER OUT] ${line}`);
            }
        } catch (err) {
            // Suppress error if Puppeteer is closed or navigating
        }
    }

    // 1. Read input events from presenter stdout (hooked early)
    const rl = readline.createInterface({
        input: presenter.stdout,
        terminal: false
    });
    rl.on('line', handleInputCommand);

    console.log("[PRESENTER] Waiting for presenter to initialize...");
    await presenterReady;
    console.log("[PRESENTER] Presenter is ready. Starting Puppeteer...");

    // 2. Launch Puppeteer
    console.log("[PUPPETEER] Launching system Google Chrome...");
    const browser = await puppeteer.launch({
        executablePath: "/usr/bin/google-chrome",
        headless: true,
        ignoreDefaultArgs: ["--mute-audio"],
        args: [
            "--no-sandbox",
            "--disable-setuid-sandbox",
            "--disable-gpu",
            "--window-size=800,600",
            "--autoplay-policy=no-user-gesture-required",
            "--enable-audio-service",
            "--disable-dev-shm-usage"
        ]
    });

    page = await browser.newPage();
    await page.setViewport({ width: 800, height: 600 });

    let active = true;
    presenter.on("exit", (code, signal) => {
        console.log(`[PRESENTER] Exited with code=${code}, signal=${signal}. Stopping stream.`);
        active = false;
        browser.close();
    });

    // Frame capture loop
    const frameInterval = 1000 / 30; // 30 FPS target (smooth)
    let frameCount = 0;

    async function captureLoop() {
        while (active) {
            const startTime = Date.now();
            try {
                const jpegBuffer = await page.screenshot({ type: 'jpeg', quality: 80 });

                if (active && presenter.stdin.writable) {
                    const lenBuf = Buffer.alloc(4);
                    lenBuf.writeUInt32LE(jpegBuffer.length, 0);

                    let ok = presenter.stdin.write(lenBuf);
                    if (ok) {
                        ok = presenter.stdin.write(jpegBuffer);
                    }
                    if (!ok) {
                        await new Promise(resolve => presenter.stdin.once('drain', resolve));
                    }
                    frameCount++;
                    if (frameCount % 30 === 0) {
                        try {
                            const fs = require('fs');
                            fs.writeFileSync(path.join(__dirname, "../frontend/latest_frame.jpg"), jpegBuffer);
                        } catch (writeErr) {
                            // ignore write errors
                        }
                    }
                    if (frameCount % 90 === 0) {
                        console.log(`[STREAM] Sent ${frameCount} JPEG frames.`);
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
    
    let lastReloadTime = 0;
    // Auto-unmute and auto-recover YouTube video player continuously in the background
    setInterval(async () => {
        try {
            const hasError = await page.evaluate(() => {
                // 1. Auto unmute
                const muteBtn = document.querySelector('.ytp-mute-button');
                if (muteBtn) {
                    const titleText = (muteBtn.getAttribute('title') || muteBtn.getAttribute('aria-label') || '').toLowerCase();
                    if (titleText.includes('unmute')) {
                        muteBtn.click();
                        console.log("[PUPPETEER] Triggered YouTube Player unmute button.");
                    }
                }
                
                // 2. Click retry button if visible (YouTube shows a retry button when playback fails)
                const retryBtn = document.querySelector('.ytp-error-message-button, .ytp-retry-button, button[aria-label="Retry"]');
                if (retryBtn) {
                    retryBtn.click();
                    console.log("[PUPPETEER] Clicked YouTube retry button.");
                    return false;
                }

                // Check if playback error overlay is active
                const errScreen = document.querySelector('.ytp-error, .ytp-playability-error-supported-renderers, #error-screen');
                if (errScreen && errScreen.offsetWidth > 0 && errScreen.offsetHeight > 0) {
                    return true;
                }
                return false;
            });

            if (hasError) {
                const now = Date.now();
                if (now - lastReloadTime > 15000) {
                    console.log("[PUPPETEER] YouTube error overlay detected. Reloading page...");
                    lastReloadTime = now;
                    await page.reload({ waitUntil: "networkidle2" });
                }
            }
        } catch (e) {
            // Suppress errors during page transitions
        }
    }, 1000);
    
    // Load Atropa Splash Screen first
    const splashPath = `file://${path.join(__dirname, "../frontend/atropa_splash.html")}`;
    console.log(`[PUPPETEER] Exhibiting splash screen: ${splashPath}`);
    await page.goto(splashPath, { waitUntil: "load" });
    
    // Display splash screen for 4 seconds
    await new Promise(resolve => setTimeout(resolve, 4000));
    
    console.log(`[PUPPETEER] Navigating to target URL: ${url}...`);
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

        // Wait for search results page
        console.log("[PUPPETEER] Waiting for search results to load...");
        await new Promise(resolve => setTimeout(resolve, 5000));

        // Click on filters button
        console.log("[PUPPETEER] Looking for 'Filters' button...");
        const filtersButtonSelector = await page.evaluate(() => {
            const elements = Array.from(document.querySelectorAll('ytd-toggle-button-renderer button, a[aria-label="Search filters"], button[aria-label="Search filters"]'));
            if (elements.length > 0) {
                elements[0].setAttribute('id', 'temp-filter-btn');
                return '#temp-filter-btn';
            }
            const all = Array.from(document.querySelectorAll('button, a, ytd-toggle-button-renderer'));
            const found = all.find(el => el.textContent && el.textContent.toLowerCase().includes('filter'));
            if (found) {
                const clickable = found.closest('button') || found.closest('a') || found;
                clickable.setAttribute('id', 'temp-filter-btn');
                return '#temp-filter-btn';
            }
            return null;
        });

        if (filtersButtonSelector) {
            console.log("[PUPPETEER] Clicking Filters button using page.click...");
            await page.click(filtersButtonSelector);
            await new Promise(resolve => setTimeout(resolve, 3000));

            // Click on "This week" option
            console.log("[PUPPETEER] Looking for 'This week' filter option...");
            const weekOptionSelector = await page.evaluate(() => {
                const elements = Array.from(document.querySelectorAll('ytd-search-filter-renderer a, ytd-search-filter-renderer, a.ytd-search-filter-renderer'));
                const found = elements.find(el => el.textContent && el.textContent.toLowerCase().includes('this week'));
                if (found) {
                    const clickable = found.closest('a') || found.closest('button') || found;
                    clickable.setAttribute('id', 'temp-this-week-opt');
                    return '#temp-this-week-opt';
                }
                return null;
            });

            if (weekOptionSelector) {
                console.log("[PUPPETEER] Clicking 'This week' filter option using page.click...");
                await page.click(weekOptionSelector);
                await new Promise(resolve => setTimeout(resolve, 5000));
            } else {
                console.log("[PUPPETEER] Failed to find 'This week' filter option.");
            }
        } else {
            console.log("[PUPPETEER] Failed to find 'Filters' button.");
        }
    } catch (e) {
        console.log("[PUPPETEER] Automatic search or filter flow failed: " + e.message);
    }

}

main().catch(err => {
    console.error("FATAL ERROR:", err);
});
