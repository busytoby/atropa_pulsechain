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

### Open Representative Voting (ORV)
* **VM Register Context:** The representative selection register (`representative_id`) binding account balance weights.
* **Mathematical Function:** The delegated consensus power $W_{rep}$ calculated as the sum of all balances selecting representative $R$: $W_{rep} = \sum_{a \in A, rep(a) = R} Balance_a$.
* **Visual / Geometric Manifestation:** Modulates the orbital line thickness of the connection links between representative node spheres and delegate nodes.

### Transaction Proof-of-Work
* **VM Register Context:** The mining difficulty register checking that block hashes satisfy leading/trailing zero-bit properties.
* **Mathematical Function:** The mining nonce check verifying that the compiled block hash terminates with 3 zero bits: $Hash(Block \parallel Nonce) \equiv 0 \pmod 8$.
* **Visual / Geometric Manifestation:** Determines the rotation rate of coordinate grid vertices during block integration animation frames.

### Dual-Block Balance Audit
* **VM Transition Context:** The global ledger audit check validating fund conservation across transaction channels.
* **Mathematical Operation:** Asserts that for every block of type `SEND` (type 1) referencing amount $X$ to recipient $B$, there exists a corresponding block of type `RECEIVE` (type 2) on $B$'s chain referencing amount $X$ from sender $A$.
* **Visual / Geometric Shift:** Renders solid green flow lines between sending and receiving nodes on the grid; any mismatched block highlights in blinking red.
