const { ethers } = require("ethers");

const PULSECHAIN_RPC = "https://rpc.pulsechain.com";
const LOG_EVENT_SIGNATURE = "LogEvent(string,uint64,uint64,string)";
const TOPIC_0 = ethers.id(LOG_EVENT_SIGNATURE);

async function main() {
    console.log("Connecting to PulseChain RPC...");
    const provider = new ethers.JsonRpcProvider(PULSECHAIN_RPC);

    // Get current block
    const latestBlock = await provider.getBlockNumber();
    console.log(`Current Block: ${latestBlock}`);

    // PulseChain block time is exactly 10 seconds.
    // 5 days = 5 * 24 * 60 * 60 = 432,000 seconds.
    // At 10 seconds per block, 5 days is 43,200 blocks.
    const blocksFor5Days = 43200;
    const fromBlock = latestBlock - blocksFor5Days;

    console.log(`Querying logs from block ${fromBlock} to ${latestBlock} (range of ${blocksFor5Days} blocks)...`);
    console.log(`Event signature hash (Topic 0): ${TOPIC_0}`);

    try {
        // Fetch logs matching topic 0
        const logs = await provider.getLogs({
            fromBlock: "0x" + fromBlock.toString(16),
            toBlock: "0x" + latestBlock.toString(16),
            topics: [TOPIC_0]
        });

        console.log(`\nFound ${logs.length} chat events in the last 5 days.`);

        if (logs.length > 0) {
            console.log("\nSample of events found:");
            const iface = new ethers.Interface([
                `event ${LOG_EVENT_SIGNATURE}`
            ]);

            logs.slice(0, 10).forEach((log, index) => {
                try {
                    const parsed = iface.parseLog(log);
                    console.log(`[${index + 1}] Tx: ${log.transactionHash}`);
                    console.log(`    Contract: ${log.address}`);
                    console.log(`    Username: ${parsed.args[0]}`);
                    console.log(`    Soul:     ${parsed.args[1].toString()}`);
                    console.log(`    Aura:     ${parsed.args[2].toString()}`);
                    console.log(`    Message:  "${parsed.args[3]}"`);
                } catch (err) {
                    console.log(`[${index + 1}] Failed to parse log at ${log.address}: ${err.message}`);
                }
            });
        }
    } catch (error) {
        console.error("RPC Query failed:", error.message);
    }
}

main().catch(console.error);
