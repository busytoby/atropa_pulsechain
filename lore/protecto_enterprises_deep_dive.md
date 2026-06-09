# Protecto Enterprises: Mail-Order Telemetry & B128 Emulation

This document provides a deep dive into the historical significance of **Protecto Enterprises** and details the virtual emulation of their Commodore mail-order catalog system, focusing on the liquidation of the **Commodore B128 (CBM-II)** series and how ordering transaction telemetry is logged on-chain.

---

## 1. Historical Background: The B128 Rescue

**Protecto Enterprises**, based in Barrington, Illinois, was one of the largest discount mail-order computer hardware and software distributors in the 1980s. While they sold a vast catalog of Commodore 64 and VIC-20 peripherals, their most famous achievement was single-handedly liquidating Commodore’s discontinued **B-Series (CBM-II)** inventory:

- **The CBM-II Flop**: Commodore intended the B-Series (including the B128) to replace the aging PET line for corporate users. Equipped with 128KB of RAM (massive for the era) and a custom MOS 6509 CPU with hardware bank switching, it was technically advanced but suffered from terrible market positioning, pricing blunders, and a lack of software. Commodore cancelled the line shortly after release.
- **The Liquidation Deal**: Protecto bought out Commodore's remaining stock of B128 computers, SFD-1001 floppy disk drives, and high-resolution monitors. They sold them as heavily discounted bundles (frequently under $250).
- **The Software Lifeline**: Since Commodore provided no software or support, Protecto wrote its own programmers' guides, created custom productivity applications, and published a dedicated catalog of compatible B128 software to keep the machines alive.

---

## 2. On-Chain Catalog Memory Map ($D660–$D66F / 54880-54895)

We emulate the physical Protecto Order Desk via a range of memory-mapped I/O registers inside the virtual machine. By reading and writing to these registers, user programs (or developers) inspect the active catalog and submit mail-orders to the blockchain.

| Register Address (Hex) | Register Address (Dec) | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D660` | `54880` | Read / Write | **Catalog Item ID Selector**: Poke an item ID (1-6) here to query. |
| `$D661`–`$D662` | `54881`–`54882` | Read-Only | **OTRT Price (Low/High)**: Returns the price of the queried item. |
| `$D663`–`$D66B` | `54883`–`54891` | Read-Only | **Stock Quantity Buffer**: Returns the current on-chain stock. |
| `$D66C` | `54892` | Write-Only | **Order Placement Trigger**: Strobe with `1` to execute order. |

---

## 3. Emulated Catalog Index

The on-chain emulator supports the following liquidation hardware and Epyx software catalog:

| Item ID | Product Name | Original MSRP | Protecto Price (OTRT) | On-Chain Register Value |
| :--- | :--- | :--- | :--- | :--- |
| `0x01` | **Commodore B128 Computer** | $995.00 | **12 OTRT** | `12` |
| `0x02` | **SFD-1001 Floppy Drive** (1MB) | $895.00 | **8 OTRT** | `8` |
| `0x03` | **Commodore 1902 Monitor** | $499.00 | **6 OTRT** | `6` |
| `0x04` | **Centronics Parallel Cable** | $49.00 | **1 OTRT** | `1` |
| `0x05` | **Crush, Crumble & Chomp!** | $29.95 | **2 OTRT** | `2` |
| `0x06` | **The Temple of Apshai** | $39.95 | **3 OTRT** | `3` |

---

## 4. On-Chain Order Strobe Execution Flow

When a user triggers an order by poking `$D66C` with `1`, the following operations are executed atomically in Yul:

1. **Item Validation**: Fetches the selected item ID from `$D660` and resolves its price.
2. **Balance Verification**: Queries the user's OTRT token balance (mapped to C64 memory-mapped RAM address `$0350` / slot `848`).
3. **Billing**: Deducts `price + 1 OTRT` (for shipping and ledger fees).
4. **Treasury Deposit**: Credits the platform treasury address (`0x1111111111111111111111111111111111111111`).
5. **Telemetry Event Log**: Dispatches an EVM event containing the buyer, item ID, and price paid using the signature:
   `log3(0, 0, 0x9bcbf7ea2838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b999, getContextUser(), itemId)`

---

## 5. CRT Simulation and Phosphor Visual Demos

In [protecto.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/protecto.html), the mail-order console features a vintage green/amber phosphor CRT screen layout matching the PET/CBM-II style:

- **Scanline & Flicker Filters**: Canvas shaders render simulated 60Hz flicker, scanlines, and a subtle glowing vignetting effect.
- **Epyx Game Previews**: Emulates real-time gameplay loops of Epyx titles inside the CRT window, displaying smooth-moving sprites and verifying instruction states on the EVM interface.
- **Direct Wallet Integration**: Connects with Web3 providers to fetch and render user balances, pending order history, and on-chain invoice receipts.
