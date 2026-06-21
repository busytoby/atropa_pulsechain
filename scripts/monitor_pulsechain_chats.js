const { ethers } = require("ethers");
const net = require("net");

const PULSECHAIN_RPC = "https://rpc.pulsechain.com";
const LOG_EVENT_SIGNATURE = "LogEvent(string,uint64,uint64,string)";
const TOPIC_0 = ethers.id(LOG_EVENT_SIGNATURE);

const C = {
    reset: "\x1b[0m",
    bright: "\x1b[1m",
    dim: "\x1b[2m",
    blue: "\x1b[34m",
    green: "\x1b[32m",
    cyan: "\x1b[36m",
    yellow: "\x1b[33m",
    magenta: "\x1b[35m",
    red: "\x1b[31m"
};

const iface = new ethers.Interface([
    `event ${LOG_EVENT_SIGNATURE}`
]);

const nameCache = {};

async function getContractName(provider, address) {
    if (nameCache[address]) return nameCache[address];
    try {
        const contract = new ethers.Contract(address, ["function name() view returns (string)"], provider);
        const name = await contract.name();
        nameCache[address] = name;
        return name;
    } catch {
        nameCache[address] = "Unknown QING";
        return "Unknown QING";
    }
}

function logToZmmMcp(event, source, details) {
    const payload = {
        jsonrpc: "2.0",
        method: "wave512.dilemma_log",
        params: {
            event: event,
            source: source,
            details: details
        },
        id: 1
    };
    
    const client = new net.Socket();
    client.setTimeout(2000);
    client.connect(10042, "127.0.0.1", () => {
        client.write(JSON.stringify(payload));
    });
    client.on("data", () => {
        client.destroy();
    });
    client.on("error", () => {
        client.destroy();
    });
}

function formatMessage(username, soul, aura, msg, qingName, timestampText) {
    const timeStr = timestampText ? `[${timestampText}] ` : "";
    return `${C.dim}${timeStr}${C.reset}${C.cyan}${C.bright}${qingName}${C.reset} | ${C.green}${C.bright}${username}${C.reset} ${C.dim}(S:${soul} A:${aura})${C.reset}\n  ${C.bright}${msg}${C.reset}\n`;
}

async function main() {
    console.log(`${C.blue}Connecting to PulseChain RPC at ${PULSECHAIN_RPC}...${C.reset}`);
    const provider = new ethers.JsonRpcProvider(PULSECHAIN_RPC);

    try {
        const latestBlock = await provider.getBlockNumber();
        console.log(`${C.green}Connected! Current Block: ${latestBlock}${C.reset}\n`);

        // Get 5 days of history
        const blocksFor5Days = 43200;
        const fromBlock = Math.max(0, latestBlock - blocksFor5Days);

        console.log(`${C.dim}Fetching last 5 days of historical logs...${C.reset}`);
        let logs = await provider.getLogs({
            fromBlock: "0x" + fromBlock.toString(16),
            toBlock: "0x" + latestBlock.toString(16),
            topics: [TOPIC_0]
        });

        if (logs.length === 0) {
            console.log(`${C.yellow}No chat logs found in last 5 days. Scanning back 30 days...${C.reset}`);
            const blocksFor30Days = 259200;
            const fromBlock30 = Math.max(0, latestBlock - blocksFor30Days);
            logs = await provider.getLogs({
                fromBlock: "0x" + fromBlock30.toString(16),
                toBlock: "0x" + latestBlock.toString(16),
                topics: [TOPIC_0]
            });
            
            if (logs.length === 0) {
                console.log(`${C.yellow}No chat logs found in last 30 days. Scanning back 90 days...${C.reset}`);
                const blocksFor90Days = 777600;
                const fromBlock90 = Math.max(0, latestBlock - blocksFor90Days);
                logs = await provider.getLogs({
                    fromBlock: "0x" + fromBlock90.toString(16),
                    toBlock: "0x" + latestBlock.toString(16),
                    topics: [TOPIC_0]
                });
            }
        }

        console.log(`${C.blue}=== Historical Feed (${logs.length} events) ===${C.reset}\n`);
        for (const log of logs) {
            try {
                const parsed = iface.parseLog(log);
                const qingName = await getContractName(provider, log.address);
                let timeStr = "";
                try {
                    const block = await provider.getBlock(log.blockNumber);
                    timeStr = new Date(block.timestamp * 1000).toLocaleTimeString();
                } catch {
                    timeStr = `Block ${log.blockNumber}`;
                }
                console.log(formatMessage(parsed.args[0], parsed.args[1].toString(), parsed.args[2].toString(), parsed.args[3], qingName, timeStr));
                
                const isVoid = qingName.toLowerCase().includes("void");
                const eventName = isVoid ? "M:VOID_CHAT" : "M:QING_CHAT";
                logToZmmMcp(eventName, qingName, `${parsed.args[0]} (S:${parsed.args[1]} A:${parsed.args[2]}) | ${parsed.args[3]}`);
            } catch (e) {}
        }

        console.log(`\n${C.green}=== Live Feed Monitoring Started ===${C.reset}\n`);
        
        let lastCheckedBlock = latestBlock;
        setInterval(async () => {
            try {
                const currentBlock = await provider.getBlockNumber();
                if (currentBlock > lastCheckedBlock) {
                    const freshLogs = await provider.getLogs({
                        fromBlock: "0x" + (lastCheckedBlock + 1).toString(16),
                        toBlock: "0x" + currentBlock.toString(16),
                        topics: [TOPIC_0]
                    });

                    for (const log of freshLogs) {
                        try {
                            const parsed = iface.parseLog(log);
                            const qingName = await getContractName(provider, log.address);
                            const timeStr = new Date().toLocaleTimeString();
                            console.log(formatMessage(parsed.args[0], parsed.args[1].toString(), parsed.args[2].toString(), parsed.args[3], qingName, timeStr));
                            
                            const isVoid = qingName.toLowerCase().includes("void");
                            const eventName = isVoid ? "M:VOID_CHAT" : "M:QING_CHAT";
                            logToZmmMcp(eventName, qingName, `${parsed.args[0]} (S:${parsed.args[1]} A:${parsed.args[2]}) | ${parsed.args[3]}`);
                        } catch (e) {}
                    }
                    lastCheckedBlock = currentBlock;
                }
            } catch (err) {
                // Ignore transient network errors
            }
        }, 5000);

    } catch (err) {
        console.error(`${C.red}Initialization failed: ${err.message}${C.reset}`);
    }
}

main();
