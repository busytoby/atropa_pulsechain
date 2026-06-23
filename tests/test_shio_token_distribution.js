const assert = require("assert");
const http = require("http");

console.log("=============================================================");
const msg = "Auncient ZMM VM SHIO Token Distribution Integration Tests";
console.log(msg);
console.log("=============================================================");

const zmmUrl = "http://127.0.0.1:3000/api/zmm-exec";

function postJSON(url, payload) {
    return new Promise((resolve, reject) => {
        const u = new URL(url);
        const data = JSON.stringify(payload);
        const options = {
            hostname: u.hostname,
            port: u.port,
            path: u.pathname,
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Content-Length': Buffer.byteLength(data)
            }
        };
        const req = http.request(options, (res) => {
            let body = '';
            res.on('data', (chunk) => body += chunk);
            res.on('end', () => {
                try {
                    resolve(JSON.parse(body));
                } catch (e) {
                    reject(new Error(`Failed to parse response: ${body}`));
                }
            });
        });
        req.on('error', reject);
        req.write(data);
        req.end();
    });
}

async function getDynamicTokenAddress(parentName, selector, wordIndex) {
    const res = await postJSON(zmmUrl, {
        name: parentName,
        calldata: selector.replace("0x", "")
    });
    assert.ok(res && !res.error, `ZMM call for resolving dynamic address from parent ${parentName} failed`);
    assert.ok(res.result && res.result.output, `No output returned from parent ${parentName}`);
    
    const output = res.result.output.trim();
    let resolvedAddr = "";
    if (wordIndex !== undefined) {
        const offset = wordIndex * 64;
        if (output.length >= offset + 64) {
            resolvedAddr = "0x" + output.substring(offset + 24, offset + 64);
        } else {
            resolvedAddr = "0x" + output.substring(output.length - 40);
        }
    } else {
        resolvedAddr = "0x" + output.substring(output.length - 40);
    }
    return resolvedAddr.toLowerCase();
}

async function getBalance(tokenName) {
    const res = await postJSON(zmmUrl, {
        name: tokenName,
        calldata: "70a0823100000000000000000000000000000000000000000000000000000000000004cc"
    });
    assert.ok(res && !res.error, `ZMM call for querying balance of ${tokenName} failed`);
    assert.ok(res.result && res.result.output, `No balance output returned for ${tokenName}`);
    
    const output = res.result.output.trim();
    return BigInt("0x" + output);
}

async function run() {
    try {
        const tokensToTest = [
            { name: 'Psi_SHIO', parent: 'yi', selector: '0x76fade90' },
            { name: 'Theta_SHIO', parent: 'yau', selector: '0x3b297416', wordIndex: 4 },
            { name: 'Lai_SHIO', parent: 'yang', selector: '0xa4566950', wordIndex: 12 },
            { name: 'User_SHIO', parent: 'lau', selector: '0x85bcb519', wordIndex: 4 }
        ];

        for (const token of tokensToTest) {
            console.log(`Resolving dynamic address for ${token.name} via parent ${token.parent}...`);
            const addr = await getDynamicTokenAddress(token.parent, token.selector, token.wordIndex);
            assert.ok(addr && addr !== "0x0000000000000000000000000000000000000000", `Resolved address for ${token.name} is invalid`);
            console.log(`   ✓ Resolved Address: ${addr}`);

            const zmmName = `dynamic_${addr.replace("0x", "")}`;
            console.log(`Querying balance for ${token.name} (${zmmName}) in ZMM...`);
            const balance = await getBalance(zmmName);
            console.log(`   ✓ Balance: ${balance} Wei`);
            assert.ok(balance > 0n, `${token.name} balance is zero! Balance should be distributed when initcode is run.`);
        }

        console.log("\n=============================================================");
        console.log("ALL SHIO TOKEN DISTRIBUTION UNIT TESTS PASSED SUCCESSFULLY");
        console.log("=============================================================");
        process.exit(0);
    } catch (err) {
        console.error("\nTEST RUN FAILURE:", err.message);
        process.exit(1);
    }
}

run();
