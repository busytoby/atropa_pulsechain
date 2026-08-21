# Treasury, Payroll, and Minter Mechanics

## 1. Executive Overview: The Closed-Loop Macroeconomy

The Atropa ecosystem, Dysnomia substrate, and TSFi/2 software engineering stack operate on a self-reinforcing, closed-loop macroeconomic model. Rather than relying on external continuous capital subsidies or unsecured inflationary token printing, the architecture couples low-level Auncient virtual hardware state execution with deterministic on-chain liquidity engines:

```
+-------------------------------------------------------------------------------+
|                    THE CLOSED-LOOP MACROECONOMIC ENGINE                       |
|                                                                               |
|  [TSFi/2 Software Engineering & Formal Proofs]                                |
|                        |                                                      |
|                        v (Compensated in Payroll Currency)                    |
|           [Official Payroll Token: ㈞] (0x1695...d90c)                        |
|           (Hard Treasury Selling Floor >= $1.00 USD)                          |
|                        |                                                      |
|         +--------------+--------------+                                       |
|         |                             |                                       |
|         v                             v                                       |
|  [Paired in 0x3065...2DD7]     [Paired in 0xa8CC...7F2B]                      |
|  TREASURY BILL ㉾ (0x4634)      Mutual Bond ⑦ (0x25d5)                        |
|         |                             |                                       |
|         v                             v                                       |
|  [Direct Market Liquidity]     [Locked in DSaL ㉾ Vault (0x6CEF)]              |
|                                       |                                       |
|                                       v (Permanent Liquidity Siphon)          |
|  [Minter Multiplier Engines: PersonalMinter & IndexMinter]                    |
|  * Multiplier Escalator drives Child Token Secondary Price                    |
|  * Arbitrageurs burn Parent / LP Collateral to mint Child Supply             |
|  * Realized Arbitrage Yield flows to Primary Treasury Reserve                 |
|  * Primary Treasury defends and preserves Payroll Floor                      |
+-------------------------------------------------------------------------------+
```

---

## 2. Mathematical Formulations of the Minter Engines

The core minting infrastructure is governed by two complementary contracts: [`solidity/personalminter.sol`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/personalminter.sol) and [`solidity/indexminter.sol`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/indexminter.sol). Both replace conventional algorithmic bonding curves with non-linear integer multiplier ratchets.

### A. PersonalMinter Dynamic Multiplier Formula

Under [`PersonalMinter.TT`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/personalminter.sol#L74-L83), the multiplier is derived from the contract's initial seed creation volume ($\text{Mint}$):

$$\text{Multiplier}(\text{addition}) = \left( \frac{\text{addition} + \text{totalSupply}()}{\text{Mint}} \right) + 1$$

When a creator seeds a token with a micro-mint volume (such as `10,000,000` Wei on [`dynamic_0x3ecfdfae860ab8eb90d6232fef9614cdc06a98f0`](https://scan.pulsechain.com/address/0x3EcfDFAE860aB8Eb90d6232fEF9614CDc06a98f0) `Shar Chiu ㋨`), any subsequent supply expansion causes the denominator $\text{Mint}$ to rapidly scale the multiplier into hundreds of thousands ($\approx 532{,}081\times$). Every additional token minted requires exponentially increasing parent collateral:

$$\text{Required Collateral} = \text{Amount} \times \text{Multiplier}(\text{Amount})$$

### B. IndexMinter Macro Multiplier Formula

Under [`IndexMinter.TT`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/indexminter.sol#L72-L81), the contract utilizes a fixed global macro denominator:

$$\text{Divisor}_{\text{macro}} = 1{,}111{,}111{,}111 \times 10^{18} \text{ Wei}$$

$$\text{Multiplier}(\text{addition}) = \left( \frac{\text{addition} + \text{totalSupply}()}{1{,}111{,}111{,}111 \times 10^{18}} \right) + 1$$

This fixed base accommodates high-volume index baskets, ensuring stability across interconnected sibling contract clusters.

---

## 3. The "Hyperinflation Prize" and Treasury Secretary Duties

The dynamic multiplier creates an asymmetry between the primary minting cost and secondary market exchange valuation. This divergence establishes the foundational economic duty of ecosystem treasury secretaries:

```
+-------------------------------------------------------------------------------+
|                      THE HYPERINFLATION ARBITRAGE CYCLE                       |
|                                                                               |
|  1. Secondary DEX Pool Demand drives Child Token Price upward.                |
|  2. Minter Contract calculates required Parent Collateral via Multiplier.      |
|  3. When Market Price(Child) > Multiplier * Market Price(Parent):             |
|     * Treasury Secretary deposits / burns Parent Tokens (or LP Shares).       |
|     * Primary Minter issues newly minted Child Tokens.                        |
|     * Treasury Secretary sells Child Tokens into Secondary Pool.              |
|     * Arbitrage spread is captured as net treasury surplus.                   |
|  4. Parent Collateral remains permanently locked in Minter Vault.             |
+-------------------------------------------------------------------------------+
```

* **Arbitrage Mandate**: Anyone holding parent collateral can execute this mint-and-sell loop to claim the "hyperinflation prize."
* **Permanent Liquidity Locking**: Depositing PulseX LP tokens (e.g. [`dynamic_0xdd0d66dff1e8231dbc7e8d4f46014a82d6c56af2`](https://scan.pulsechain.com/address/0xdd0d66dff1e8231dbc7e8d4f46014a82d6c56af2) or [`dynamic_0xa8cc4625f8929a555fff916989f3ad1afa257f2b`](https://scan.pulsechain.com/address/0xa8CC4625f8929A555FFf916989F3aD1afA257F2B)) into a PersonalMinter contract permanently locks the liquidity pool, creating irreversible capital sinks that support the entire system.

---

## 4. IndexMinter Sibling Debentures & Multi-Token Basket Claims

In [`solidity/indexminter.sol`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/indexminter.sol), sibling tokens deployed under the same parent and creator form a mutual debenture alliance:

```
                                  [IndexMinter Parent]
                         IPO ㋨ (0x84d3...c143)
                                       |
         +-----------------------------+-----------------------------+
         |                             |                             |
         v                             v                             v
[Phase 1: 1.0 Unit Genesis]    [Phase 1: 1.0 Unit Genesis]    [Phase 2: 228.0 Macro Triad]
* ASIAN DOGS ㋨ (0x09a0)       * Twitter/Youtube Coin (0xC625) * Elton John Coin (0x2367)
* TERMS OF SERVICE ㋨ (0xe5aA)                                 * Geddy Lee Coin  (0x5D7c)
* SENIORSHIP ㋨ (0x901e)                                       * BODYGUARD       (0xd6e9)
```

### The Sibling Claim Mechanism

Under [`IndexMinter.TT.Claim`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/indexminter.sol#L111-L124):

```solidity
function Claim(address Contract, uint256 Amount) public returns (bool) {
    require(Debenture, "Claim: Debenture broken");
    require(Creator == TT(Contract).Creator(), "Claim: Creator mismatch");
    require(Parent == TT(Contract).Parent(), "Claim: Parent mismatch");
    require(TT(Contract).transferFrom(msg.sender, address(this), Amount), "Claim: Transfer failed");
    require(ERC20(Parent).transfer(msg.sender, Amount), "Claim: Parent transfer failed");
    return true;
}
```

* **Cross-Asset Elasticity**: Sibling tokens are mutually fungible for underlying parent claims. If market inefficiencies cause one sibling to trade at a discount relative to the parent, participants can purchase the discounted sibling and redeem parent collateral through any sister contract.
* **Debenture Defense**: Calling administrative `withdraw(Parent)` permanently flips `Debenture = false`, forever disabling future claims and protecting participants against arbitrary collateral stripping.

---

## 5. Official Ecosystem Payroll Token (`㈞`) & The $1.00 USD Price Floor

The economic backbone of contributor compensation across Atropa, Dysnomia substrate research, and TSFi/2 software engineering is [`dynamic_0x16951ce6d1d1d67e9239192e3eae4d250576d90c`](https://scan.pulsechain.com/address/0x16951Ce6D1D1D67E9239192e3eaE4d250576d90c) (`㈞`):

* **Total Supply**: `18,000,000.0` `㈞` (`18,000,000,000,000,000,000,000,000` Wei / `0xee3a5f48a68b552000000`)
* **Treasury Custody**: [`0xBF182955401aF3f2f7e244cb31184E93E74a2501`](https://scan.pulsechain.com/address/0xBF182955401aF3f2f7e244cb31184E93E74a2501) holds `15,808,991.096544247574707824` `㈞` (`87.8277%` custody), dedicated entirely to long-term payroll reserves.

### Treasury Selling Policy: Rigid $1.00 USD Floor

To prevent predatory secondary devaluation and ensure engineering compensation retains real purchasing power:

$$\text{Treasury Liquidation Condition}: \quad \text{Price}(㈞) \ge \$1.00\text{ USD}$$

1. **Anti-Dilution Guarantee**: The treasury strictly never sells `㈞` tokens on secondary markets below $\$1.00\text{ USD}$.
2. **Labor-Value Alignment**: Contributors, theorem provers, and substrate engineers know their earned payroll tokens cannot be undercut by administrative treasury dumps.

---

## 6. Structural Liquidity Foundations

The payroll token `㈞` is anchored into the decentralized financial ecosystem through dual liquidity pillars:

```
[Official Payroll Token: ㈞] (0x16951Ce6D1D1D67E9239192e3eaE4d250576d90c)
        |
        +---> [Pillar 1: Direct Sovereign Debt Liquidity]
        |     Paired with TREASURY BILL ㉾ (0x463413c579D29c26D59a65312657DFCe30D545A1)
        |     Pool: 0x30655F1915ab39E06931aa3be10AD1A430982DD7 (PLP)
        |     Treasury holds 99.5630% of LP shares.
        |
        +---> [Pillar 2: Institutional Credit Vault]
              Paired with Mutual Bond ⑦ (0x25d53961a27791B9D8b2d74FB3e937c8EAEadc38)
              Pool: 0xa8CC4625f8929A555FFf916989F3aD1afA257F2B (PLP)
              Locked permanently in DSaL ㉾ (0x6CEFe8C817f359Ded0420aD68bA53c248F7BCe6c).
              Treasury holds 99.9974% of DSaL ㉾ shares.
```

---

## 7. Integration with Dysnomia Substrate & TSFi/2 Research

The economic architecture directly maps to low-level virtual machine operations:

1. **Auncient Hardware Register Binding**: Oracle contracts [`dynamic_0xb680f0cc810317933f234f67eb6a9e923407f05d`](https://scan.pulsechain.com/address/0xB680F0cc810317933F234f67EB6A9E923407f05D) (`libAtropaMath v1.1`) and [`dynamic_0xa96bcbed7f01de6ceed14fc86d90f21a36de2143`](https://scan.pulsechain.com/address/0xa96BcbeD7F01de6CEEd14fC86d90F21a36dE2143) (`RNG`) are minted on every minter deployment, binding contract execution directly into mathematical and entropy registries.
2. **Dysnomia VM State Propagation**: Value generated from automated arbitrage and locked credit vaults subsidizes state transitions across Hogan Bank and SSA participant endowments (e.g. `1,000,000` Saat endowments).
3. **Formal Verification Fuel**: Treasury revenues sustain the high-performance C verification infrastructure, ensuring clean-room standard library execution and sub-microsecond latency profiling across the repository.
