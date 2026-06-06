const http = require("http");
const fs = require("fs");
const path = require("path");

const PORT = 3000;
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const MIME_TYPES = {
    ".html": "text/html",
    ".css": "text/css",
    ".js": "text/javascript",
    ".json": "application/json",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".gif": "image/gif",
    ".svg": "image/svg+xml"
};

const server = http.createServer((req, res) => {
    // API endpoint to serve config keys
    if (req.url === "/api/config") {
        if (fs.existsSync(CONFIG_PATH)) {
            const data = fs.readFileSync(CONFIG_PATH, "utf8");
            res.writeHead(200, { 
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(404, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "Config file not found" }));
        }
        return;
    }

    // API endpoint to serve the Markdown documentation
    if (req.url === "/api/docs") {
        const DOCS_PATH = path.join(__dirname, "../frontend/local_deployment_guide.md");
        if (fs.existsSync(DOCS_PATH)) {
            const data = fs.readFileSync(DOCS_PATH, "utf8");
            res.writeHead(200, { 
                "Content-Type": "text/markdown",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(404, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "Docs not found" }));
        }
        return;
    }

    // Serve static files from frontend directory
    const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    let filePath = parsedUrl.pathname === "/" ? "/index.html" : parsedUrl.pathname;
    let absolutePath = path.join(__dirname, "../frontend", filePath);
    
    // Safety check to prevent directory traversal
    if (parsedUrl.pathname.startsWith("/Wallet/bin/Contracts/")) {
        // Expose Solidity compiled output files securely
        absolutePath = path.join(__dirname, "..", filePath);
    } else if (!absolutePath.startsWith(path.join(__dirname, "../frontend"))) {
        res.writeHead(403, { "Content-Type": "text/plain" });
        res.end("Forbidden");
        return;
    }

    fs.exists(absolutePath, (exists) => {
        if (exists) {
            const ext = path.extname(absolutePath);
            const contentType = MIME_TYPES[ext] || "application/octet-stream";
            res.writeHead(200, { "Content-Type": contentType });
            fs.createReadStream(absolutePath).pipe(res);
        } else {
            res.writeHead(404, { "Content-Type": "text/plain" });
            res.end("404 Not Found");
        }
    });
});

server.listen(PORT, "127.0.0.1", () => {
    console.log(`==================================================`);
    console.log(`DApp Dashboard Server running at http://127.0.0.1:${PORT}`);
    console.log(`==================================================`);
});
