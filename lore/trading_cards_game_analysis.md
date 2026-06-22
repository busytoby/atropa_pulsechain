# NoNukes Trading Cards Game (TCG) Analysis

This document outlines the core architecture, mathematical formulas, and on-chain mechanics governing the Dysnomia NoNukes Trading Cards Game (TCG).

---

## 1. System Architecture

The game leverages the **SEI** coordinator contract to connect a player's wallet to their unique **LAU** identity card and **YUE** reaction engine.

```mermaid
graph TD
    A[Partner Token JSON Metadata] -->|Loaded by Dashboard| B[Voxel Art & Lore Renderer]
    B -->|MD5 Address Hash| C[Procedural Card Stats]
    B -->|LangChain Expanded Lore| D[Cyber Retro Visual Card]
    C -->|ATK / DEF / LIQ| D
    
    E[User Wallet (tx.origin)] -->|Calls Start| F[SEI.sol Contract]
    F -->|Registers LAU| G[Cho Registry]
    F -->|Deploys| H[YUE.sol Contract]
    E -->|Direct Ownership / Controls| H
    H -->|Calls React| I[XIE.sol Contract]
    I -->|Power Query| H
    H -->|Saves On-Chain| J[Hypobar & Epibar Stats]
```

---

## 2. Card Metadata & Procedural Stat Engine

Every collectible card in the game is represented by a partner token mapped under [solidity/dysnomia/domain/data/](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/data/) and has a mandatory attached **QING** contract. 
When rendering the card art via [batch_generate_art.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/batch_generate_art.py), the asset pipeline uses a cryptographic seed derived from the token's address to procedurally assign stats:

1. **Seed Derivation**: An MD5 hash of the card's address is computed:
   $$h_{val} = \text{MD5}(address)$$
2. **ATK / Burst**: Determined by the lower byte values:
   $$\text{ATK} = (h_{val} \pmod 8) + 3 \quad \in [3, 10]$$
3. **DEF / Shield**: Determined by the next nibble shifted:
   $$\text{DEF} = ((h_{val} \gg 4) \pmod 8) + 3 \quad \in [3, 10]$$
4. **Liquidity Power**: Determined by the next byte shifted:
   $$\text{LIQ} = ((h_{val} \gg 8) \pmod 8) + 3 \quad \in [3, 10]$$

---

## 3. On-Chain Game Mechanics (SEI, YUE & XIE)

Collectible cards are activated, owned, and modified using the **SEI**, **YUE**, and **XIE** smart contracts:

### Identity & Coordination ([SEI.sol](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/tang/01_sei.sol))
The coordinator contract maps the player's wallet to their assets:
- **`Start(address LauToken, ...)`**: Binds the player's wallet (`tx.origin`) to their **LAU** card inside the `Cho` registry and deploys their personalized **YUE** reaction contract.
- **`Chi()`**: Resolves and returns the player's active `Yue` and `UserToken` (LAU) contract addresses.
- **Ownership Link**: The player's wallet address (`tx.origin`) is granted owner status on the deployed `YUE` contract, giving the player exclusive control to execute reactions.

### Reaction Mechanics ([YUE.sol](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/yue.sol))
When a player initiates a reaction on a target QING card pool, they execute the `React` function from their owner-verified wallet:
```solidity
function React(address Qing) public onlyOwners returns (uint256 Charge) {
    if(balanceOf(tx.origin) == 0) revert ZeroHoldings(tx.origin);
    QINGINTERFACE _qing = QINGINTERFACE(Qing);
    uint256 Omicron;
    uint256 Omega;
    (Charge, Omicron, Omega) = Chan.Xie().Power(_qing.Waat());        
    Hypobar[Qing] += Omega;
    Epibar[Qing] += Omicron;
}
```

- **Omicron (Epibar gain)**: Proportional to the balance of the `Fornax` token in the user's active session divided by the user's entropy.
- **Omega (Hypobar gain)**: Proportional to the `Fornax` balance in the target pool contract divided by the pool's entropy.

### Raising Reaction Stats Above Zero
To raise the reaction numbers (Hypobar and Epibar) above zero, specific on-chain environmental and player-state conditions must be achieved:

1. **Fornax (SHIO) Seeding**: Both stats depend on the ERC-20 `Fornax` token balances:
   * **For Epibar (Omicron)**: `Fornax` must be transferred or generated in the player's active room/coordinate slot (`Alpha.On.Phi`).
   * **For Hypobar (Omega)**: `Fornax` must be held by the target `Qing` contract address (`address(Qing)`).
2. **Entropy Minimization**: The yields are scaled inversely by entropy:
   * Lower player entropy (`Alpha.Entropy`) increases Epibar gains.
   * Lower target pool entropy (`Qing.Entropy()`) increases Hypobar gains.
3. **YUE Balance Verification**: The player's wallet must hold a positive balance of the custom `YUE` token to authorize and execute the reaction.

---

## 4. Art Generation Pipeline

The TCG features a high-fidelity visual pipeline that renders 1280x720 3D cards:
1. **Voxelizer**: [render_vlm_synthesizer.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/render_vlm_synthesizer.py) parses the token's description to generate a unique 3D isometric representation.
2. **Stable Diffusion Styling**: The voxel render is piped into the Stable Diffusion worker `tsfi_sd_worker` to apply a vibrant 1980s retro cyber aesthetic.
3. **Lore Master**: LangChain prompts expand simple card descriptions (e.g. `blotter paper currency token`) into lore lines via Gemini or GPT.

---

## 5. Dashboard Integration & Real-Time Querying

The frontend dashboard has been updated to query these on-chain parameters dynamically using the active player's profile:
- **Interactive Stats Display**: Displays **Hypobar (Omega)** and **Epibar (Omicron)** reaction values inside the pool details modal when a valid YUE contract is resolved or loaded.
- **On-Chain Queries**: Uses MetaMask RPC `eth_call` to query `YUE.Bar(Qing)` (selector `0xe76caed5`) against the player's active contract, ensuring stats update instantly post-transaction or upon manually changing YUE credentials.
- **Tethered to LAU**: The active YUE domain is resolved directly via `SEI.Chi()` mapping the user's active LAU identity on-chain.
