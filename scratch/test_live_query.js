const https = require("https");

function rpcCall(to, data) {
    return new Promise((resolve, reject) => {
        const postData = JSON.stringify({
            jsonrpc: "2.0",
            method: "eth_call",
            params: [
                {
                    to: to,
                    data: data
                },
                "latest"
            ],
            id: 1
        });

        const options = {
            hostname: "rpc.pulsechain.com",
            port: 443,
            path: "/",
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Content-Length": postData.length
            }
        };

        const req = https.request(options, (res) => {
            let body = "";
            res.on("data", chunk => body += chunk);
            res.on("end", () => {
                try {
                    const parsed = JSON.parse(body);
                    if (parsed.error) {
                        reject(new Error(parsed.error.message));
                    } else {
                        resolve(parsed.result);
                    }
                } catch (e) {
                    reject(e);
                }
            });
        });

        req.on("error", (e) => reject(e));
        req.write(postData);
        req.end();
    });
}

async function testLiveQuery() {
    const bzo = "0x008fc4bbb1998bfee060f780be7688f0cec66bff";
    console.log("Querying BZO decimals...");
    try {
        const result = await rpcCall(bzo, "0x313ce567"); // decimals()
        console.log("Raw hex result:", result);
        const decimals = parseInt(result, 16);
        console.log("Decimals:", decimals);
    } catch (e) {
        console.error("Error:", e);
    }
}

testLiveQuery();
