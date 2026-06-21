const puppeteer = require('puppeteer-core');
const net = require('net');

// Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".

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
    102: 'Home', 103: 'ArrowUp', 104: 'PageUp', 105: 'ArrowLeft', 106: 'ArrowRight', 107: 'End', 108: 'ArrowDown', 109: 'PageDown'
};

async function connectToBrowser(port) {
    console.log(`[Auncient Bridge] Connecting to browser on port ${port}...`);
    try {
        const browser = await puppeteer.connect({
            browserURL: `http://127.0.0.1:${port}`,
            defaultViewport: null
        });
        const pages = await browser.pages();
        // Return first valid page
        return { browser, page: pages[0] };
    } catch (e) {
        console.error(`[Auncient Bridge ERR] Failed to connect to port ${port}:`, e.message);
        return null;
    }
}

async function startBridge() {
    const mainConnection = await connectToBrowser(33661);
    const ytConnection = await connectToBrowser(44135);

    if (!mainConnection && !ytConnection) {
        console.error("[Auncient Bridge ERR] No active browser connections established. Exiting.");
        process.exit(1);
    }

    async function handleCommand(line, isYouTube) {
        const conn = isYouTube ? ytConnection : mainConnection;
        if (!conn) return;

        const parts = line.replace(/,/g, ' ').split(/\s+/).filter(Boolean);
        if (parts.length === 0) return;

        const cmd = parts[0];
        if (cmd === 'KEY_DOWN' || cmd === 'KEY_UP') {
            const key = parseInt(parts[1]);
            const keyName = linuxKeyMap[key];
            if (keyName) {
                console.log(`[Auncient Bridge TCP] Routing event to ${isYouTube ? 'YouTube' : 'Main'}: ${cmd} ${keyName}`);
                try {
                    // Only focus search/query bar if it is NOT a navigation key
                    const isNavigationKey = ['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'PageUp', 'PageDown', 'Home', 'End'].includes(keyName);
                    if (!isNavigationKey) {
                        const searchInputSelector = 'input[name="search_query"], input#search, input.query-bar';
                        await conn.page.focus(searchInputSelector).catch(() => {});
                    }
                    
                    if (cmd === 'KEY_DOWN') {
                        await conn.page.keyboard.down(keyName);
                    } else {
                        await conn.page.keyboard.up(keyName);
                    }
                } catch (err) {
                    console.error("[Auncient Bridge Routing ERR]", err.message);
                }
            }
        } else if (cmd === 'MOUSE_SCROLL') {
            const axis = parseInt(parts[1]); // 0 = Y, 1 = X
            const value = parseInt(parts[2]);
            const multiplier = isYouTube ? 55 : 25;
            const deltaY = (axis === 0) ? value * multiplier : 0;
            const deltaX = (axis === 1) ? value * multiplier : 0;
            console.log(`[Auncient Bridge TCP] Routing scroll event to ${isYouTube ? 'YouTube' : 'Main'}: deltaX=${deltaX}, deltaY=${deltaY}`);
            try {
                await conn.page.evaluate((dx, dy) => {
                    window.scrollBy(dx, dy);
                }, deltaX, deltaY).catch(() => {});
            } catch (err) {
                console.error("[Auncient Bridge Scroll ERR]", err.message);
            }
        }
    }

    function createServerForPort(tcpPort, isYouTube) {
        const server = net.createServer((socket) => {
            let buffer = '';
            socket.on('data', (data) => {
                buffer += data.toString();
                let boundary = buffer.indexOf('\n');
                while (boundary !== -1) {
                    const line = buffer.substring(0, boundary).trim();
                    buffer = buffer.substring(boundary + 1);
                    if (line) {
                        handleCommand(line, isYouTube);
                    }
                    boundary = buffer.indexOf('\n');
                }
            });
        });
        server.listen(tcpPort, '127.0.0.1', () => {
            console.log(`[Auncient Bridge Server] Listening for Yul hardware events on port ${tcpPort} -> ${isYouTube ? 'YouTube' : 'Main'}`);
        });
    }

    createServerForPort(18080, false);
    createServerForPort(18081, true);
}

startBridge().catch(console.error);
