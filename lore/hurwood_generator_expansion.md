# Technical Proposal: Hurwood Code Generator Optimization & Integration

This document reviews advanced upgrades for the **Hurwood Code Generator** suite (`HURWOOD [MAZE | SOUND | SPRITE | CHARSET | RASTER | JOYSTICK]`), focusing on dynamic compilation, execution bridges, and integration with `YULBUILD`.

---

## 1. Dynamic Frequency Calculations (`SOUND` Parameterization)

Currently, `HURWOOD SOUND` outputs a static envelope for C-4. We can calculate the exact register values dynamically based on target frequencies:

*   **SID Frequency formula**:
    $$\text{Register Value} = \text{Frequency (Hz)} \times \frac{16777216}{f_{\text{clk}}}$$
    Where $f_{\text{clk}} = 1022727\text{ Hz}$ (NTSC clock rate).
*   **Implementation**: A CLI input like `HURWOOD SOUND --freq 440` will compute:
    $$\text{Value} = 440 \times 16.405 = 7218 \implies \text{High Byte} = 28, \text{Low Byte} = 50$$
    Generating the customized line:
    `30 POKE 54273, 28: POKE 54272, 50`

---

## 2. Compaction & Optimization of BASIC Listings

To fit resource-constrained emulators and save cycles, the generator should pack statements using standard Commodore 64 BASIC shortcuts:

| Original Template | Optimized Structure | Memory Saved |
| :--- | :--- | :--- |
| `10 POKE 53280,0`<br>`20 POKE 53281,0` | `10 POKE 53280,0:POKE 53281,0` | 5 bytes (combines lines) |
| `10 FOR I=832 TO 894`<br>`20 POKE I,255`<br>`30 NEXT I` | `10 FORI=832TO894:POKEI,255:NEXT` | 9 bytes (removes spaces, optional variable in `NEXT`) |

---

## 3. Direct Memory Injector Bridge

Rather than just displaying the text, a `--stage` flag can write the generated code directly to RAM starting at the BASIC limit pointer (`$0801`):

```mermaid
graph LR
    Command["HURWOOD SOUND --stage"] --> Tokenizer["BASIC Tokenizer"]
    Tokenizer --> RAM["VM Memory Space ($0801)"]
    RAM --> VM["Run / List instantly"]
```

*   **Poking Tokenized Lines**: The C code parses the text representation (e.g. `10 PRINT "HI"`) and converts keywords to Commodore 64 tokens (`PRINT` = `$99`, `POKE` = `$97`) and writes them directly into the emulated RAM.

---

## 4. Cross-Tool Integration with `YULBUILD`

We can add a bridge so that generating code outputs assembly routines readable by `YULBUILD`:

*   **Syntax**: `HURWOOD [SCHEMA] --asm`
*   **Result**: Outputs standard 6502 assembly formatting instead of Commodore BASIC:
    ```assembly
    LDA #$0F
    STA $D418 ; Max volume
    LDA #$0F
    STA $D405 ; ADSR
    ```
*   **Piping to Editor**: `HURWOOD SOUND --asm --edit` copies these assembly statements directly into the `g_yulbuild_asm` editor buffer, allowing the user to immediately debug or step through the generated audio routine inside `YULBUILD`!
