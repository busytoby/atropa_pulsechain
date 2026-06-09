# Epyx & Protecto Enterprises Integration

We have successfully integrated **Epyx Game Titles** within the **Protecto Enterprises Mail-Order Catalog Console** and on-chain ledger environment.

## 🦖 What is New?

1. **Protecto Order Strobe Implementation**:
   * Programmed the Yul-coded hardware ordering strobe in [cpu6502.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/cpu6502.yul) at address `$D66C` (register `54892`).
   * When an order is placed, the contract verifies the user's OTRT (Oregon Trail Token) balance, deducts the corresponding price + shipping tax, credits the treasury account, and dispatches the on-chain shipping log event.

2. **Expanded Catalog Items**:
   * Epyx's legendary games have been added to both the on-chain Yul pricing dispatcher and the frontend catalog options in [protecto.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/protecto.html):
     * **Item ID 5**: *Crush, Crumble and Chomp!* — Liquidation Price: **2 OTRT** (Original Retail: $29.95)
     * **Item ID 6**: *The Temple of Apshai* — Liquidation Price: **3 OTRT** (Original Retail: $39.95)

3. **On-Chain Event Verification**:
   * Placing an order on the Protecto console triggers the contract event:
     ```solidity
     event ProtectoOrder(address indexed buyer, uint256 itemId, uint256 pricePaid);
     ```
     This matches topic hash `0x9bcbf7ea2838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b999`, which is indexable on-chain.

---

## 💻 Try it Out!
1. Start the local server: `node scripts/server.js`
2. Navigate to `http://localhost:3000/protecto.html`.
3. Connect your wallet, select **Crush, Crumble and Chomp! (Epyx)** or **The Temple of Apshai (Epyx)**, and click **Place Order** to complete the on-chain purchase!
