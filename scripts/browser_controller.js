const { spawn } = require("child_process");
const path = require("path");

const fs = require("fs");
const COMPOSITOR_SOCKET = "wayland-tsfi";
const BROWSER_PATH = fs.existsSync("/home/mariarahel/src/mozilla/obj-tsfi/dist/bin/firefox")
    ? "/home/mariarahel/src/mozilla/obj-tsfi/dist/bin/firefox"
    : "/usr/bin/firefox";
const DEFAULT_URL = process.argv[2] || `file://${path.resolve(__dirname, '../frontend/hub_portal.html')}`;

console.log("=== Auncient TSFi2 Browser Controller ===");

// 1. Launch the native Vulkan Wayland compositor
console.log(`[COMPOSITOR] Launching ./tsfi2 display compositor on socket: ${COMPOSITOR_SOCKET}`);
const compositor = spawn("./tsfi2", [], {
    cwd: path.join(__dirname, "../tsfi2-deepseek"),
    env: {
        ...process.env,
        WAYLAND_DISPLAY: COMPOSITOR_SOCKET
    }
});

compositor.stdout.on("data", data => {
    console.log(`[COMPOSITOR LOG] ${data.toString().trim()}`);
});

compositor.stderr.on("data", data => {
    console.error(`[COMPOSITOR ERR] ${data.toString().trim()}`);
});

// 2. Launch the custom-built Firefox engine once compositor is initialized
setTimeout(() => {
    console.log(`[BROWSER] Launching browser engine on Vulkan/Wayland: ${DEFAULT_URL}`);
    
    const args = [];
    if (fs.existsSync("/home/mariarahel/src/mozilla/tsfi_profile")) {
        args.push("-profile", "/home/mariarahel/src/mozilla/tsfi_profile");
    }
    args.push("-no-remote", "--new-window", DEFAULT_URL);

    const browser = spawn(BROWSER_PATH, args, {
        env: {
            ...process.env,
            WAYLAND_DISPLAY: COMPOSITOR_SOCKET,
            MOZ_ENABLE_WAYLAND: "1",
            MOZ_WEBRENDER: "1" // Force WebRender graphics pipeline
        }
    });

    browser.stdout.on("data", data => {
        console.log(`[BROWSER OUT] ${data.toString().trim()}`);
    });

    browser.stderr.on("data", data => {
        console.error(`[BROWSER ERR] ${data.toString().trim()}`);
    });

}, 2000);
