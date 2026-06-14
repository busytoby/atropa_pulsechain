# Multi-Network & Multi-VM Unified Ledger Registry

To track balances and inventories across distinct execution environments (ZMM VM, local EVM, PulseChain, and other networks), we can model the entire universe as a series of nested Z-machine objects. 

---

## 1. Unified Multi-Network Tree Hierarchy

Instead of maintaining separate state tables for different chains, we represent each network boundary as a **Root Room** at the top of our object tree.

```
                           [ ROOT SYSTEM ]
             /                    |                    \
   [ Room 1: ZMM VM ]    [ Room 2: Local EVM ]   [ Room 3: PulseChain ]
           |                      |                      |
     [ User Bag ]           [ User Bag ]           [ User Bag ]
     /    |     \           /    |     \           /    |     \
  [Pb]  [ZAL]  [Key]     [Pb]  [ZAL]  [Wagon]   [PLS]  [HEX]  [ATROPA]
```

### Environment IDs (Parent Rooms):
*   `Room 1`: **Z-Machine Memory (ZMM) VM**
*   `Room 2`: **Local EVM (Anvil localhost:8545)**
*   `Room 3`: **PulseChain Mainnet**
*   `Room 4`: **PulseChain Testnet v4**

---

## 2. On-Chain Storage Mapping for Network Nodes

We extend our Yul contract storage variables to include the network scope identifier:

| Function | Storage Layout | Purpose |
| :--- | :--- | :--- |
| `network(obj)` | `keccak256(obj, 90005)` | Returns the network boundary ID (`1` for ZMM, `2` for local EVM, etc.). |
| `register(netId, obj)` | `keccak256(netId, obj, 90006)` | Stores metadata/balances specific to an object on a given network. |

---

## 3. Cross-Chain / Cross-VM Asset Bridging (Move Logic)

Under this design, a "cross-chain bridge" or "VM migration" is simply a Z-machine `move(object, new_parent)` operation that shifts an asset from one network room to another:

### Example: Shifting 10 Lead (Pb) from ZMM VM to Local EVM
1.  **Debit / Unlink** from the player's ZMM inventory container (`parent = Room 1`).
2.  **Credit / Link** into the player's Local EVM wallet container (`parent = Room 2`).
3.  Emit a unified `BridgeTransfer(obj, amount, fromNetwork, toNetwork)` log for tracking.

---

## 4. Frontend Wallet Integration

The unified dashboard can query the `ZarrellaLedger` to reconstruct the entire asset tree. Rather than performing raw RPC calls to 5 different sources in ad-hoc scripts, the frontend queries:

```javascript
// Query all items under User Bag for each Network Room
const environments = [1, 2, 3]; // ZMM, Local EVM, PulseChain
for (const envId of environments) {
    const items = await ledgerContract.getContainerContents(envId, userAddress);
    // Display items dynamically in user inventory categorized by network
}
```
This is a highly structured way to represent a multi-network wallet inventory.
