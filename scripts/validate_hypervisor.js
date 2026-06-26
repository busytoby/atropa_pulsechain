const fs = require('fs');
const path = require('path');
const http = require('http');

// Auncient spelling mandate followed in output text
console.log("=============================================================");
console.log("Auncient ZMM VM vs Anvil EVM Hypervisor Validation Suite");
console.log("=============================================================");

// Helper to make POST requests via built-in http module
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
                if (res.statusCode >= 200 && res.statusCode < 300) {
                    try {
                        resolve(JSON.parse(body));
                    } catch (e) {
                        reject(new Error(`Failed to parse response: ${body}`));
                    }
                } else {
                    reject(new Error(`HTTP status ${res.statusCode}: ${body}`));
                }
            });
        });
        req.on('error', (e) => reject(e));
        req.write(data);
        req.end();
    });
}

// Load configurations
const CONFIG_PATH = path.join(__dirname, '../config/user_config.json');
const ADDRESSES_PATH = path.join(__dirname, '../scripts/deployed_addresses_localhost.json');

let userConfig = { networks: { localhost: {} } };
let deployedAddresses = {};

try {
    userConfig = JSON.parse(fs.readFileSync(CONFIG_PATH, 'utf8'));
} catch (e) {
    console.error(`Warning: Could not read ${CONFIG_PATH}:`, e.message);
}

try {
    deployedAddresses = JSON.parse(fs.readFileSync(ADDRESSES_PATH, 'utf8'));
} catch (e) {
    console.warn(`Warning: Could not read ${ADDRESSES_PATH}:`, e.message);
}

// Contract configuration arrays
const tokens = [
    { name: 'YI', type: 'Token', zmmName: 'yi' },
    { name: 'ZHENG', type: 'Token', zmmName: 'zheng' },
    { name: 'ZHOU', type: 'Token', zmmName: 'zhou' },
    { name: 'YAU', type: 'Token', zmmName: 'yau' },
    { name: 'YANG', type: 'Token', zmmName: 'yang' },
    { name: 'SIU', type: 'Token', zmmName: 'siu' },
    { name: 'VOID', type: 'Token', zmmName: 'void' },
    { name: 'QI', type: 'Token', zmmName: 'qi' },
    { name: 'MAI', type: 'Token', zmmName: 'mai' },
    { name: 'XIA', type: 'Token', zmmName: 'xia' },
    { name: 'XIE', type: 'Token', zmmName: 'xie' },
    { name: 'CHAN', type: 'Token', zmmName: 'chan' },
    { name: 'SEI', type: 'Token', zmmName: 'sei' },
    { name: 'CHOA', type: 'Token', zmmName: 'choa' },
    { name: 'CHEON', type: 'Token', zmmName: 'cheon' },
    { name: 'mariarahelLau', type: 'Token', zmmName: 'lau' },
    { name: 'oregonTrailToken', type: 'Token', zmmName: 'oregonTrailToken' },
    { name: 'ERIS', type: 'Token', zmmName: 'eris', dynamicResolver: { parent: 'QI', selector: '0x05d3d596' } },
    { name: 'FOMALHAUTE', type: 'Token', zmmName: 'fomalhaute', dynamicResolver: { parent: 'XIA', selector: '0xfff3be7d' } },
    { name: 'FORNAX', type: 'Token', zmmName: 'fornax', dynamicResolver: { parent: 'XIE', selector: '0xa60d57c8' } },
    { name: 'Psi_SHIO', type: 'Token', zmmName: 'psi_shio', dynamicResolver: { parent: 'YI', selector: '0x76fade90' } },
    { name: 'Theta_SHIO', type: 'Token', zmmName: 'theta_shio', dynamicResolver: { parent: 'YAU', selector: '0x3b297416', wordIndex: 4 } },
    { name: 'Lai_SHIO', type: 'Token', zmmName: 'lai_shio', dynamicResolver: { parent: 'YANG', selector: '0xa4566950', wordIndex: 12 } },
    { name: 'User_SHIO', type: 'Token', zmmName: 'user_shio', dynamicResolver: { parent: 'mariarahelLau', selector: '0x85bcb519', wordIndex: 4 } }
];

const yulSystems = [
    { name: 'zmachine', type: 'Yul System', zmmName: 'zmachine' },
    { name: 'zmachineParser', type: 'Yul System', zmmName: 'zmachineParser' },
    { name: 'keySystem', type: 'Yul System', zmmName: 'keySystem' },
    { name: 'bGraph', type: 'Yul System', zmmName: 'bGraph' },
    { name: 'batcher', type: 'Yul System', zmmName: 'batcher' },
    { name: 'folklore', type: 'Yul System', zmmName: 'folklore' },
    { name: 'diyat', type: 'Yul System', zmmName: 'diyat' },
    { name: 'ledger', type: 'Yul System', zmmName: 'ledger' },
    { name: 'biorhythm', type: 'Yul System', zmmName: 'biorhythm' },
    { name: 'dragonsLair', type: 'Yul System', zmmName: 'dragonsLair' },
    { name: 'starCastle', type: 'Yul System', zmmName: 'starCastle' },
    { name: 'consensusPki', type: 'Yul System', zmmName: 'consensusPki' },
    { name: 'arenaProcessor', type: 'Yul System', zmmName: 'arenaProcessor' },
    { name: 'ImmutableFactory', type: 'Yul System', zmmName: 'shafactory' }
];

const allContracts = [...tokens, ...yulSystems];

// Address lookup helper
function getEVMAddress(item) {
    if (item.dynamicResolver) {
        return item.resolvedEVMAddress || null;
    }
    const loc = userConfig.networks.localhost;
    if (item.name === 'zmachine') return deployedAddresses.zmachine || loc.zmachineAddress || null;
    if (item.name === 'zmachineParser') return deployedAddresses.zmachineParser || loc.zmachineParserAddress || null;
    if (item.name === 'keySystem') return loc.keySystemAddress || null;
    if (item.name === 'bGraph') return loc.bGraphAddress || null;
    if (item.name === 'batcher') return loc.batcherAddress || null;
    if (item.name === 'folklore') return loc.folkloreAddress || null;
    if (item.name === 'diyat') return loc.diyatAddress || null;
    if (item.name === 'ledger') return loc.ledgerAddress || null;
    if (item.name === 'biorhythm') return loc.biorhythmAddress || null;
    if (item.name === 'dragonsLair') return loc.dragonsLairAddress || null;
    if (item.name === 'starCastle') return loc.starCastleAddress || null;
    if (item.name === 'consensusPki') return loc.consensusPkiAddress || null;
    if (item.name === 'arenaProcessor') return loc.arenaProcessorAddress || null;
    if (item.name === 'ImmutableFactory') return loc.immutableFactoryAddress || null;
    
    return loc[item.name] || null;
}

function decodeString(hex) {
    if (!hex || hex === "0x" || hex === "") return "";
    try {
        const clean = hex.replace(/^0x/, "");
        if (clean.length < 128) return "";
        const len = parseInt(clean.substring(64, 128), 16);
        const strHex = clean.substring(128, 128 + len * 2);
        return Buffer.from(strHex, "hex").toString("utf8").trim();
    } catch (e) {
        return "";
    }
}

function decodeUint8(hex) {
    if (!hex || hex === "0x" || hex === "") return 0;
    try {
        return parseInt(hex.replace(/^0x/, ""), 16);
    } catch (e) {
        return 0;
    }
}

// Main execution routine
async function main() {
    const anvilUrl = "http://127.0.0.1:8545";
    const zmmUrl = "http://127.0.0.1:3000/api/zmm-exec";
    
    const defaultAnvilWallet = "0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266";
    const defaultZMMWallet = "0x00000000000000000000000000000000000004cc";
    
    // Resolve dynamic path-based contract addresses
    for (const item of allContracts) {
        if (item.dynamicResolver) {
            const parentAddr = getEVMAddress(allContracts.find(c => c.name === item.dynamicResolver.parent));
            if (parentAddr) {
                try {
                    const res = await postJSON(anvilUrl, {
                        jsonrpc: "2.0",
                        method: "eth_call",
                        params: [{
                            to: parentAddr,
                            data: item.dynamicResolver.selector
                        }, "latest"],
                        id: 99
                    });
                    if (res && res.result && res.result !== "0x" && res.result !== "") {
                        if (item.dynamicResolver.wordIndex !== undefined) {
                            const hex = res.result.replace("0x", "");
                            const offset = item.dynamicResolver.wordIndex * 64;
                            if (hex.length >= offset + 64) {
                                item.resolvedEVMAddress = "0x" + hex.substring(offset + 24, offset + 64);
                            } else {
                                item.resolvedEVMAddress = "0x" + res.result.substring(res.result.length - 40);
                            }
                        } else {
                            item.resolvedEVMAddress = "0x" + res.result.substring(res.result.length - 40);
                        }
                    }
                } catch (e) {}
            }
            try {
                const parentContract = allContracts.find(c => c.name === item.dynamicResolver.parent);
                const parentZmmName = parentContract ? parentContract.zmmName : item.dynamicResolver.parent.toLowerCase();
                const zmmRes = await postJSON(zmmUrl, {
                    name: parentZmmName,
                    calldata: item.dynamicResolver.selector.replace("0x", "")
                });
                if (zmmRes && !zmmRes.error && zmmRes.result && zmmRes.result.output) {
                    const output = zmmRes.result.output;
                    if (item.dynamicResolver.wordIndex !== undefined) {
                        const offset = item.dynamicResolver.wordIndex * 64;
                        if (output.length >= offset + 64) {
                            item.resolvedZMMAddress = "0x" + output.substring(offset + 24, offset + 64);
                        } else {
                            item.resolvedZMMAddress = "0x" + output.substring(output.length - 40);
                        }
                    } else {
                        item.resolvedZMMAddress = "0x" + output.substring(output.length - 40);
                    }
                    item.zmmName = `dynamic_${item.resolvedZMMAddress.replace("0x", "").toLowerCase()}`;
                }
            } catch (e) {}
        }
    }

    const results = [];
    
    console.log("Analyzing local network hypervisors...");
    
    for (const item of allContracts) {
        const evmAddr = getEVMAddress(item);
        let evmStatus = "Not Deployed";
        let evmBalance = "-";
        let evmMetadata = null;
        
        // 1. Query Anvil EVM (port 8545)
        if (evmAddr) {
            try {
                // Check deployment code
                const codeRes = await postJSON(anvilUrl, {
                    jsonrpc: "2.0",
                    method: "eth_getCode",
                    params: [evmAddr, "latest"],
                    id: 1
                });
                
                if (codeRes && codeRes.result && codeRes.result !== "0x" && codeRes.result !== "") {
                    evmStatus = "Deployed";
                    
                    // If it is a token, query balance and metadata
                    if (item.type === "Token") {
                        const balRes = await postJSON(anvilUrl, {
                            jsonrpc: "2.0",
                            method: "eth_call",
                            params: [{
                                to: evmAddr,
                                data: `0x70a08231000000000000000000000000${defaultAnvilWallet.replace("0x", "").toLowerCase()}`
                            }, "latest"],
                            id: 2
                        });
                        
                        if (balRes && balRes.result) {
                            try {
                                evmBalance = BigInt(balRes.result).toString();
                            } catch (err) {
                                evmBalance = "0";
                            }
                        } else {
                            evmBalance = "0";
                        }

                        // Query metadata
                        const nameRes = await postJSON(anvilUrl, {
                            jsonrpc: "2.0",
                            method: "eth_call",
                            params: [{ to: evmAddr, data: "0x06fdde03" }, "latest"],
                            id: 3
                        });
                        const symRes = await postJSON(anvilUrl, {
                            jsonrpc: "2.0",
                            method: "eth_call",
                            params: [{ to: evmAddr, data: "0x95d89b41" }, "latest"],
                            id: 4
                        });
                        const decRes = await postJSON(anvilUrl, {
                            jsonrpc: "2.0",
                            method: "eth_call",
                            params: [{ to: evmAddr, data: "0x313ce567" }, "latest"],
                            id: 5
                        });

                        evmMetadata = {
                            name: decodeString(nameRes.result),
                            symbol: decodeString(symRes.result),
                            decimals: decodeUint8(decRes.result)
                        };
                    }
                }
            } catch (err) {
                // Connection or method error
                evmStatus = "Error (Unreachable)";
            }
        }
        
        // 2. Query ZMM VM (port 3000 /api/zmm-exec)
        let zmmStatus = "Not Deployed";
        let zmmBalance = "-";
        let zmmMetadata = null;
        
        try {
            // Query token balance or execution to test deployment
            const zmmCd = `70a08231000000000000000000000000${defaultZMMWallet.replace("0x", "").toLowerCase()}`;
            const zmmRes = await postJSON(zmmUrl, {
                name: item.zmmName,
                calldata: zmmCd
            });
            
            // If the server processes and returns a valid result thunk
            if (zmmRes && !zmmRes.error) {
                zmmStatus = "Deployed";
                
                if (item.type === "Token") {
                    const output = (zmmRes.result && zmmRes.result.output) || "";
                    if (output && output !== "") {
                        try {
                            zmmBalance = BigInt("0x" + output).toString();
                        } catch (err) {
                            zmmBalance = "0";
                        }
                    } else {
                        zmmBalance = "0";
                    }

                    // Query metadata
                    const zmmNameRes = await postJSON(zmmUrl, { name: item.zmmName, calldata: "06fdde03" });
                    const zmmSymRes = await postJSON(zmmUrl, { name: item.zmmName, calldata: "95d89b41" });
                    const zmmDecRes = await postJSON(zmmUrl, { name: item.zmmName, calldata: "313ce567" });

                    zmmMetadata = {
                        name: decodeString("0x" + ((zmmNameRes.result && zmmNameRes.result.output) || "")),
                        symbol: decodeString("0x" + ((zmmSymRes.result && zmmSymRes.result.output) || "")),
                        decimals: decodeUint8("0x" + ((zmmDecRes.result && zmmDecRes.result.output) || ""))
                    };
                }
            }
        } catch (err) {
            zmmStatus = "Not Deployed";
        }
        
        results.push({
            name: item.name,
            type: item.type,
            evmAddr: evmAddr || "N/A",
            evmStatus,
            evmBalance,
            evmMetadata,
            zmmName: item.zmmName,
            zmmStatus,
            zmmBalance,
            zmmMetadata
        });
    }
    
    // Print ASCII test report table
    console.log("\n+----------------------+------------+--------------------------------------------+--------------+---------------+--------------+---------------+");
    console.log("| Contract Name        | Type       | EVM Address                                | EVM Status   | EVM Balance   | ZMM Status   | ZMM Balance   |");
    console.log("+----------------------+------------+--------------------------------------------+--------------+---------------+--------------+---------------+");
    
    for (const r of results) {
        const namePad = r.name.padEnd(20);
        const typePad = r.type.padEnd(10);
        const addrPad = r.evmAddr.padEnd(42);
        const evmStatPad = r.evmStatus.padEnd(12);
        const evmBalPad = r.evmBalance.padEnd(13);
        const zmmStatPad = r.zmmStatus.padEnd(12);
        const zmmBalPad = r.zmmBalance.padEnd(13);
        
        console.log(`| ${namePad} | ${typePad} | ${addrPad} | ${evmStatPad} | ${evmBalPad} | ${zmmStatPad} | ${zmmBalPad} |`);
    }
    console.log("+----------------------+------------+--------------------------------------------+--------------+---------------+--------------+---------------+");
    
    // Check validation consistency
    console.log("\n=============================================================");
    console.log("Validation Diagnostics & Consistency Analysis:");
    
    let matchedCount = 0;
    let mismatchedCount = 0;
    
    for (const r of results) {
        if (r.type === "Token") {
            if (r.evmStatus === "Deployed" && r.zmmStatus === "Deployed") {
                const balancesMatch = r.evmBalance === r.zmmBalance;
                let metadataMatch = false;
                if (r.evmMetadata && r.zmmMetadata) {
                    metadataMatch = r.evmMetadata.name === r.zmmMetadata.name &&
                                    r.evmMetadata.symbol === r.zmmMetadata.symbol &&
                                    r.evmMetadata.decimals === r.zmmMetadata.decimals;
                }
                
                if (balancesMatch && metadataMatch) {
                    console.log(`[PASS] Consistency verified for ${r.name}: balances match (${r.evmBalance}) & metadata matches ("${r.evmMetadata.name}", "${r.evmMetadata.symbol}", ${r.evmMetadata.decimals} dec).`);
                    matchedCount++;
                } else {
                    if (!balancesMatch) {
                        console.warn(`[WARNING] Balances differ for ${r.name}: EVM=${r.evmBalance}, ZMM=${r.zmmBalance}.`);
                    }
                    if (!metadataMatch) {
                        console.warn(`[WARNING] Metadata differs for ${r.name}: EVM=${JSON.stringify(r.evmMetadata)}, ZMM=${JSON.stringify(r.zmmMetadata)}.`);
                    }
                    mismatchedCount++;
                }
            } else if (r.evmStatus !== r.zmmStatus) {
                console.log(`[INFO] Deployment split for ${r.name}: EVM=${r.evmStatus}, ZMM=${r.zmmStatus}.`);
            }
        } else {
            // Yul System
            if (r.evmStatus === r.zmmStatus) {
                matchedCount++;
            } else {
                console.log(`[INFO] System deployment split for ${r.name}: EVM=${r.evmStatus}, ZMM=${r.zmmStatus}.`);
            }
        }
    }
    
    console.log("-------------------------------------------------------------");
    console.log(`Summary: Verified ${matchedCount} consistent deployment states.`);
    if (mismatchedCount > 0) {
        console.log(`Alert: Found ${mismatchedCount} token distribution/metadata inconsistencies.`);
    }
    console.log("Validation execution completed successfully.");
    process.exit(0);
}

main().catch(err => {
    console.error("Critical error running hypervisor validation:", err);
    process.exit(1);
});
