# Unified Specification: The CABS-Folklore Secure Escrow & Resonance Engine

This specification maps the **Cybernetic Address Bus System (CABS)** double-entry registers, **Folklore CPU** memory boundary, and **Z-Machine** text-adventure opcodes to form a unified combat, market, and escrow system.

---

## 1. Architectural Layout

```
     [ Z-Machine Interpreter ] <--- Reads Game ROM Story Space
                |
     (Triggers Trade or Combat)
                |
                v
       [ Folklore CPU ] <--- Dynamic State / Memory Bridge (peek/poke)
                |
                v
    [ CABS 6502 Register Map ] <--- Cryptographic Double-Entry Vault
     - Acc 1100: User Assets (YUE/CHO)
     - Acc 1200: Escrow Chamber (Reflected Power)
     - Acc 2200: Accrued Diyat / Settlement Fees
                |
        +-------+-------+
        |               |
  (Match / Win)   (Mismatch / Timeout)
        v               v
  Settle Trade    Refund to Sender
 (Infinite RL)    (Reflected Wave)
```

---

## 2. CABS 6502 Register Mapping on Folklore RAM

The Folklore CPU reserves memory slots starting at register `$E000` (57344) to expose the 6502 CABS double-entry channels to the Z-Machine interpreter:

| Register Address | Name | Description / RF Equivalent |
|:---|:---|:---|
| `57344` (`$E000`) | `CABS_CTRL` | Control Register (0: IDLE, 1: DEPOSIT, 2: COMMIT, 3: REFUND) |
| `57345` (`$E001`) | `CABS_SNDR` | Sender Identity Soul Token ID (`LAU.Saat(1)`) |
| `57346` (`$E002`) | `CABS_RCVR` | Recipient Identity Soul Token ID |
| `57347` (`$E003`) | `CABS_VAL`  | Transaction Value / Attenuation Scale |
| `57348` (`$E004`) | `CABS_COND` | Unlock Condition Key (Z-Machine Object Property Offset) |
| `57349` (`$E005`) | `CABS_SW_R` | Standing Wave Ratio metric (determines transaction tax / loss) |

---

## 3. Secure Escrow Logic Flow

### Step 1: Isolation (The Forward Wave)
When a transaction is staged (e.g., buying a rare shield from an NPC or challenging a boss for a prize pool):
1. The Z-Machine writes the trade variables into CABS memory via the Folklore CPU:
   * `poke(57345, senderSoul)`
   * `poke(57347, assetValue)`
   * `poke(57348, unlockProperty)`
2. The Folklore CPU executes an on-chain transfer moving the target ERC-20 (`YUE` or `CHO`) from **Account 1100** (User) to **Account 1200** (Escrow).
3. The assets are now locked within the address bus boundary, isolated from both players' active wallets.

### Step 2: Resonance Evaluation (The Junction)
During execution, the Folklore CPU monitors the Z-Machine property registers.
* **Lossless Match (Quest Complete / Target Acquired)**:
  * When the condition resolves (e.g., the Troll is defeated, changing the state of Object `99` to `1`), the SWR collapses to $1.0$.
  * The Folklore CPU executes `transfer` from **Account 1200** (Escrow) to **Account 2200** (Recipient/Seller), completing the circuit.
* **Controlled Mismatch (The Tariffs)**:
  * If the transaction executes under poor RF tuning conditions, the system computes the Return Loss ($RL$).
  * The difference is debited as a settlement fee (accrued Diyat fees) to the treasury pool, while the remainder is transferred to the seller.

### Step 3: Reflection (Refund)
* If the quest fails or a party cancels, the circuit experiences a total wave reflection.
* The directional coupler logic of the Folklore CPU routes the locked energy in **Account 1200** back to **Account 1100** (Sender), restoring their starting balance.
