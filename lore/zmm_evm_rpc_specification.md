# ZMM EVM JSON-RPC Compatibility Specification

This document details the design for implementing standard Ethereum JSON-RPC compatibility over our Zero Memory Manager (ZMM) EVM proxy. This allows developer tools (e.g., `ethers.js`, `cast`, `Metamask`, `Hardhat`) to communicate directly with our local ZMM VM using standard Web3 protocols.

---

## 1. Architecture Overview

The RPC layer functions as a translation proxy inside [`scripts/server.js`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/server.js), converting standard JSON-RPC requests into TCP socket payloads for our TCP MCP server listening on port `10042`.

```
┌─────────────────────────────────┐
│ Web3 Client (ethers.js / cast)  │
└────────────────┬────────────────┘
                 │ (HTTP POST JSON-RPC)
                 ▼
┌─────────────────────────────────┐
│     Node.js Proxy Server        │
│    (Translates RPC to Yul)      │
└────────────────┬────────────────┘
                 │ (TCP Socket: JSON-RPC over Port 10042)
                 ▼
┌─────────────────────────────────┐
│     ZMM MCP Server (Port 10042) │
└─────────────────────────────────┘
```

---

## 2. Core RPC Method Mappings

To present our ZMM VM as a standard Ethereum network, we map the following JSON-RPC methods:

### A. `eth_chainId`
*   **Purpose**: Returns the unique chain identifier.
*   **Response**: `0x5a4d` (Dec: `23117` - representing **ZMM** in hex).

### B. `eth_blockNumber`
*   **Purpose**: Returns the current virtual block number.
*   **Response**: Query the ZMM database state for current block height (default: `0x1`).

### C. `eth_getCode`
*   **Purpose**: Returns the bytecode of a contract at a given address.
*   **Translation**: Queries the ZMM contract cache for the corresponding name and returns the registered hexadecimal compiled byte array.

### D. `eth_call`
*   **Purpose**: Read-only dry-run of a transaction.
*   **Translation**: 
    1. Extracts `to` (contract address) and `data` (calldata).
    2. Maps the address to the contract name (e.g. `cho` or `ledger`).
    3. Transmits `YULEXEC "contractName", "calldata"` to the MCP TCP socket.
    4. Returns the hex bytes from the execution buffer.

### E. `eth_sendTransaction`
*   **Purpose**: Submit state-changing transactions.
*   **Translation**: Natively executes the calldata on the ZMM TCP process, updates namespaced key storage parameters in `state.json`, and returns a mock transaction hash.

---

## 3. Proxy Server Implementation Blueprint

We can handle standard JSON-RPC payloads by intercepting `POST` requests to the root `/` or `/rpc` paths in `scripts/server.js`:

```javascript
// Example middleware addition for scripts/server.js
if (req.method === "POST" && (req.url === "/" || req.url === "/rpc")) {
    let body = "";
    req.on("data", chunk => body += chunk);
    req.on("end", () => {
        const rpcReq = JSON.parse(body);
        switch (rpcReq.method) {
            case "eth_chainId":
                res.end(JSON.stringify({ jsonrpc: "2.0", id: rpcReq.id, result: "0x5a4d" }));
                break;
            case "eth_blockNumber":
                res.end(JSON.stringify({ jsonrpc: "2.0", id: rpcReq.id, result: "0x1" }));
                break;
            case "eth_call":
                // Forward calldata to ZMM VM executor and return response
                break;
            default:
                // Fallback or forward to localhost Anvil node
        }
    });
}
```
