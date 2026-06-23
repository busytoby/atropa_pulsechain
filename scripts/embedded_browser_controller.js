const path = require('path');
const vulkanInterop = require(path.join(__dirname, 'vulkan_interop.node'));

console.log("==================================================");
console.log("   EMBEDDED ROOTED BROWSER CONTROLLER (IN-MEMORY) ");
console.log("==================================================");

function formatInputEvent(line) {
    line = line.trim();
    if (line.startsWith("{")) {
        try {
            const js = JSON.parse(line);
            if (js.method === "input.mouse_move") {
                return `Y:MM ${js.params.x} ${js.params.y}`;
            }
            if (js.method === "input.mouse_button") {
                return `Y:${js.params.state ? 'MD' : 'MU'} ${js.params.button} ${js.params.x} ${js.params.y}`;
            }
            if (js.method === "input.keyboard") {
                return `Y:${js.params.state ? 'KD' : 'KU'} ${js.params.keycode}`;
            }
        } catch (e) {}
    }

    if (line.startsWith("YOUTUBE:")) line = line.substring(8);
    else if (line.startsWith("Y:")) line = line.substring(2);

    const parts = line.split(/\s+/).filter(Boolean);
    if (parts.length === 0) return null;
    const cmd = parts[0];
    const args = parts.slice(1).join(" ");

    let fullCmd = `${cmd} ${args}`.trim();
    fullCmd = "Y:" + fullCmd
        .replace("MOUSE_MOVE", "MM")
        .replace("MOUSE_DOWN", "MD")
        .replace("MOUSE_UP", "MU")
        .replace("MOUSE_SCROLL", "MS")
        .replace("KEY_DOWN", "KD")
        .replace("KEY_UP", "KU");

    return fullCmd;
}

const EventEmitter = require('events');

class VulkanInputEmitter extends EventEmitter {
    constructor(interopModule) {
        super();
        this.interop = interopModule;
        this.pollInterval = 8;
        this.logInterval = 16;
        this.active = false;
    }

    start() {
        if (this.active) return;
        this.active = true;
        this._pollInput();
        this._pollLogs();
    }

    _pollInput() {
        if (!this.active) return;
        try {
            while (true) {
                const inputLine = this.interop.getNextInputEvent();
                if (!inputLine) break;
                this.emit('input', inputLine);
            }
        } catch (err) {
            this.emit('error', err);
        }
        setTimeout(() => this._pollInput(), this.pollInterval);
    }

    _pollLogs() {
        if (!this.active) return;
        try {
            while (true) {
                const logStr = this.interop.getNextLogEvent();
                if (!logStr) break;
                this.emit('log', logStr);
            }
        } catch (err) {
            this.emit('error', err);
        }
        setTimeout(() => this._pollLogs(), this.logInterval);
    }
}

// Instantiate standard Node.js input event emitter
const emitter = new VulkanInputEmitter(vulkanInterop);

// Register input receiver via standard EventEmitter setup function (.on)
emitter.on('input', (event) => {
    if (!event.includes("MOUSE_MOVE") && !event.includes("MM")) {
        console.log(`[Node V8 Input Received] Raw: ${event}`);
    }
    const formatted = formatInputEvent(event);
    if (formatted) {
        if (!formatted.includes("MM")) {
            console.log(`[Node V8 Input Received] Formatted: ${formatted}`);
        }
        try {
            vulkanInterop.pushEvent(formatted);
        } catch (err) {
            console.error("[Embedded Controller ERR] pushEvent failed:", err);
        }
    }
});

// Register log receiver
emitter.on('log', (logStr) => {
    console.log(`[Embedded Controller WinchesterMQ Log] Event: ${logStr}`);
});

emitter.on('error', (err) => {
    console.error("[Embedded Controller ERR] Emitter error:", err);
});

// Start the standard Node.js event setup
emitter.start();

// Keep event loop alive
setInterval(() => {}, 60000);
