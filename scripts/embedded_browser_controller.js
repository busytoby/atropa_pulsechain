const path = require('path');
const vulkanInterop = require(path.join(__dirname, 'vulkan_interop.node'));

console.log("==================================================");
console.log("   EMBEDDED ROOTED BROWSER CONTROLLER (IN-MEMORY) ");
console.log("==================================================");

function formatInputEvent(line) {
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

// 1. Poll input events from Vulkan presenter in-memory, format them, and push them back (WinchesterMQ)
function pollInputEvents() {
    try {
        while (true) {
            const inputLine = vulkanInterop.getNextInputEvent();
            if (!inputLine) break;
            
            const formatted = formatInputEvent(inputLine);
            if (formatted) {
                // Route directly to Yul CPU in-memory via WinchesterMQ
                vulkanInterop.pushEvent(formatted);
            }
        }
    } catch (err) {
        console.error("[Embedded Controller ERR] pollInputEvents failed:", err);
    }
    setTimeout(pollInputEvents, 8); // Low latency poll (8ms)
}

// 2. Poll WinchesterMQ logs in-memory and print/process them
function pollLogEvents() {
    try {
        while (true) {
            const logStr = vulkanInterop.getNextLogEvent();
            if (!logStr) break;
            
            console.log(`[Embedded Controller WinchesterMQ Log] Event: ${logStr}`);
        }
    } catch (err) {
        console.error("[Embedded Controller ERR] pollLogEvents failed:", err);
    }
    setTimeout(pollLogEvents, 16);
}

// Start poll loops
pollInputEvents();
pollLogEvents();

// Keep event loop alive
setInterval(() => {}, 60000);
