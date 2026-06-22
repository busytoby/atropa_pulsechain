# CABS Security Audit & Recommendations

While the current CABS-Folklore architecture is secured by namespaced memory isolation and complies with the **Checks-Effects-Interactions** pattern, moving the system to a production mainnet requires a few defense-in-depth improvements.

Below is an audit of potential vulnerabilities and recommendations for hardening the contracts:

---

## 1. Diyat Fee Front-Running (High Priority)

### Vulnerability Analysis
In `CABSMarketMachine.sol`, the Diyat fee is routed to `tx.origin`:
```solidity
if (diyatFee > 0) {
    token.transfer(tx.origin, diyatFee); // Diyat fees routed to deployer treasury
}
```
Because `evaluateAndSettle(uint256 tradeId)` is a public function that anyone can trigger, MEV (Maximal Extractable Value) searchers or front-running bots will monitor the mempool for pending settlement transactions. 
* A bot can copy the transaction data, place a higher gas fee, and replace the transaction.
* The bot becomes `tx.origin`, stealing the Diyat fee that was intended for the contract owner or the player who triggered the transaction.

### Recommendation
Redirect fees to a hardcoded, immutable **Treasury Wallet** (or a set fee collector address) instead of `tx.origin`.

```diff
- token.transfer(tx.origin, diyatFee);
+ address treasury = 0xYourTreasuryAddress...;
+ token.transfer(treasury, diyatFee);
```

---

## 2. Reentrancy Guard (Medium Priority)

### Vulnerability Analysis
The contract currently follows the Checks-Effects-Interactions pattern:
```solidity
trade.active = false; // Effect
...
token.transfer(trade.recipient, payout); // Interaction
```
This protects the code from standard reentrancy. However, if standard ERC-20 tokens are replaced by **ERC-777** tokens or tokens with transfer hooks (like `tokensToSend` or `tokensReceived`), the transfer can call back into a malicious merchant contract before the transfer returns. 
Although `trade.active` is set to `false`, adding a Reentrancy Guard provides a robust second layer of defense.

### Recommendation
Inherit a standard `ReentrancyGuard` and apply the `nonReentrant` modifier to `stageEscrow`, `evaluateAndSettle`, and `refundEscrow`.

```solidity
abstract contract ReentrancyGuard {
    uint256 private constant _NOT_ENTERED = 1;
    uint256 private constant _ENTERED = 2;
    uint256 private _status;

    constructor() { _status = _NOT_ENTERED; }

    modifier nonReentrant() {
        require(_status != _ENTERED, "ReentrancyGuard: reentrant call");
        _status = _ENTERED;
        _;
        _status = _NOT_ENTERED;
    }
}
```

---

## 3. Owner Key Compromise and Oracle Upgradability

### Vulnerability Analysis
The `setBoundAddresses` function is restricted to owners:
```solidity
function setBoundAddresses(address folklore, address zmachine) public onlyOwners {
    folkloreCPUAddress = folklore;
    zmachineAddress = zmachine;
}
```
If an owner's private key is compromised, an attacker can point `zmachineAddress` to a malicious contract that they control, which returns the target value for every single pending escrow trade, allowing them to drain all locked funds in `Account 1200`.

### Recommendation
1. **Multisig Control**: Assign contract ownership to a **Multisig Wallet** (e.g., Gnosis Safe) requiring consensus among multiple team keys.
2. **Timelock**: Wrap changes to crucial addresses in a **Timelock contract** (e.g., 48-hour delay) to allow players to safely exit escrows if a malicious address change is queued.
