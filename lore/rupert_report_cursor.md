# Dale Rupert's "Don't Curse That Cursor!" (Ahoy! Issue 1, Page 44)

Dale Rupert's inaugural column in *Ahoy!* magazine details how to control cursor placement on Commodore systems. 

---

## 1. Cursor Location System Registers

To position the cursor, Commodore systems maintain internal pointers tracking the current grid coordinate (row and column):

| Register (Hex) | Register (Dec) | Function | Range |
| :---: | :---: | :--- | :---: |
| `$D3` | `211` | Current Cursor Column | `0` – `39` |
| `$D6` | `214` | Current Cursor Row | `0` – `24` |

*   **BASIC Implementation:** A developer can manipulate these pointers in BASIC:
    ```basic
    10 POKE 214, ROW : PRINT : POKE 211, COL
    ```
    *Note: The intermediate `PRINT` triggers the Screen Editor to recalibrate to the target line.*

---

## 2. KERNAL PLOT Routine (`$FFF0`)

The standard, hardware-independent method for reading and writing the cursor coordinate is calling the KERNAL `PLOT` subroutine at address `$FFF0` (65520).

### Setting the Cursor Position:
1. Clear the Carry flag (`CLC` - opcode `$18`).
2. Load target column (X) in the **X register**.
3. Load target row (Y) in the **Y register**.
4. Jump to Subroutine: `JSR $FFF0` (opcode `$20 F0 FF`).

### Reading the Cursor Position:
1. Set the Carry flag (`SEC` - opcode `$38`).
2. Call `JSR $FFF0`.
3. Returns current column in the **X register** and row in the **Y register**.

---

## 3. EVM CPU Emulator Integration

Our CPU emulator (`cpu6502.yul`) maps addresses `211` and `214` into standard zero-page space. When custom programs execute `JSR $FFF0`, the emulator intercepts the execution at PC `$FFF0` to execute the native cursor PLOT update.
