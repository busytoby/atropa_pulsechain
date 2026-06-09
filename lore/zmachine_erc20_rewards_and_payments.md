# Z-Machine ERC-20 Transaction Mapping

To allow Z-machine games to reward players with ERC-20 tokens or charge them for actions, we establish dynamic hooks in the opcode execution flow.

---

## 1. Interaction Mechanisms

We support two directions of ERC-20 interaction:
1.  **Rewards (Push)**: The contract transfers tokens *to* the player when they achieve an objective (e.g., finding the Brass Lantern or solving a puzzle).
2.  **Payments (Pull)**: The contract collects tokens *from* the player when they perform a purchase (e.g., buying a sword from a thief or paying a toll to cross a bridge).

---

## 2. On-Chain Binding in Yul (`zmachine.yul`)

We define custom opcodes or intercept existing property modifiers in Yul.

### Approach: Intercepting `put_prop` and `set_attr`
*   In the Z3 engine, altering object state is handled by opcodes like `set_attr` (set attribute) or `put_prop` (write object property).
*   We reserve special attributes or property IDs to represent ERC-20 contracts and values.

#### Example: Buying an Item
When the player buys an item:
*   The game code calls: `put_prop Sword Price 10`
*   Our Yul interpreter catches this modification. If the property being updated maps to an ERC-20 interaction:
    1.  It queries the player's balance on the bound token address using `staticcall` (`balanceOf`).
    2.  If the balance is sufficient, it triggers a `call` to `transferFrom` (pulling 10 tokens from the player to the game contract).
    3.  If the transfer succeeds, the Z-machine updates the object's parent pointer to the player's inventory.
    4.  If the transfer fails, the transaction reverts (or the game logic handles the failure state).

---

## 3. Dynamic Execution Flow (Yul Pseudo-code)

```yul
// Inside the VAR:put_prop execution handler
function executePutProp(objId, propId, value, playerAddress) {
    // Check if this object maps to an ERC-20 token reward/payment hook
    let isTokenHook := getStorageHookFlag(objId, propId)
    
    if isTokenHook {
        let tokenAddress := getTokenAddressForObject(objId)
        
        switch getHookType(objId)
        case 1 { // REWARD: Transfer tokens to player
            let success := erc20Transfer(tokenAddress, playerAddress, value)
            if iszero(success) { revert(0, 0) }
        }
        case 2 { // PAYMENT: Collect tokens from player
            let success := erc20TransferFrom(tokenAddress, playerAddress, address(), value)
            if iszero(success) { revert(0, 0) }
        }
    }
    
    // Fallback: Perform standard Z-machine property write
    writePropertyToMemory(objId, propId, value)
}
```
