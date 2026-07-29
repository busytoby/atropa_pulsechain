# Hogan Bank Block-Lattice Account-Chain Architecture

The Hogan Bank block-lattice architecture defines how individual qualifying participants (newborn Stuffed Teddy Bears and verified citizens) maintain private, independent blockchains on the Dysnomia VM to enforce decentralized ledger auditability.

```
+---------------------------------------------------------------+
|                    HOGAN BLOCK-LATTICE                        |
+---------------------------------------------------------------+
|  Account A Chain: [OPEN] -> [SEND (to B)]                     |
|                                \                              |
|  Account B Chain:               +---> [RECEIVE (from A)]      |
+---------------------------------------------------------------+
```

## Glossary Definitions

### HoganAccountChain
* **VM Register Context:** The virtual account-level ledger mapping the private chain-tip hash (`chain_head`) and transaction sequence counter (`tx_sequence`) registers.
* **Mathematical Function:** The state vector $\vec{C}_a$ tracking the blockchain head updates: $\vec{C}_a = \text{Hash}(Block_{prev} \parallel TX\_Params) \pmod{MotzkinPrime}$.
* **Visual / Geometric Manifestation:** Governs the size of individual orbital coordinate spheres representing active account balances on the Vulkan vector canvas.

### BlockLatticeSend Block
* **VM Transition Context:** The transaction state transition routing outbound Saat funds to a recipient account.
* **Mathematical Operation:** Deducts the transfer amount from the sender's balance, increments `tx_sequence` by 1, and appends a `SEND` block chain-link pointing to the previous `chain_head` hash.
* **Visual / Geometric Shift:** Animates a coordinate projection vector starting from the sender's coordinate node and traveling outward along the orbital flight vectors.

### BlockLatticeReceive Block
* **VM Transition Context:** The transaction state transition admitting incoming Saat funds to a recipient account.
* **Mathematical Operation:** Adds the transfer amount to the recipient's balance, increments `tx_sequence` by 1, and appends a `RECEIVE` block pointing to the recipient's previous `chain_head` while checking link validity against the matching `SEND` block.
* **Visual / Geometric Shift:** Animates a coordinate vector absorption point, flashing the target node glowing cyan as the transaction registers.
