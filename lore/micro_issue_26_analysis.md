# MICRO: The 6502 Journal - Issue 26 (July 1980) & The Hidden Math Engine

In July 1980, **Issue 26** of *MICRO: The 6502 Journal* was published, delivering some of the most influential technical teardowns for Apple II, Commodore PET, and SYM/KIM/AIM microcomputers. 

Among these, the standout feature was **"Apple's Hidden Floating-Point Routines" by John Martellaro** (pp. 132–135).

---

## 1. John Martellaro’s Discovery: The Applesoft ROM Math Engine

In 1980, RAM was scarce (typically 16KB to 48KB), and writing custom floating-point routines in assembly language was notoriously difficult and memory-intensive. Martellaro revealed that assembly programmers didn't need to write their own math libraries: **Applesoft BASIC (written by Microsoft) already had a complete, high-performance math coprocessor built directly into the Apple II's ROM ($E000-$FFFF).**

By calling these hidden entry points directly from 6502 Assembly, developers could perform 5-byte floating-point calculations with extreme precision, leaving almost all RAM free for the program itself.

```
       +---------------------------------------------+
       |             6502 Assembly Code              |
       |       Loads FAC ($9D) and ARG ($A1)         |
       +---------------------------------------------+
                              |
                              v (JSR $E7BE)
       +---------------------------------------------+
       |             Applesoft BASIC ROM             |
       |         Executes hardware-speed FADD        |
       +---------------------------------------------+
                              |
                              v
       +---------------------------------------------+
       |           Result returned in FAC            |
       +---------------------------------------------+
```

---

## 2. Key Applesoft Floating-Point Entry Points

Applesoft stores floating-point numbers in a **5-byte format** (1-byte exponent, 4-byte mantissa). The two main storage registers are the **Primary accumulator (FAC)** at `$9D-$A2` and the **Secondary accumulator (ARG)** at `$A1-$A6`.

Martellaro documented the primary entry points:

| Routine | Address | Description |
| :--- | :--- | :--- |
| **FADD** | `$E7BE` | Add FAC to ARG (Result in FAC) |
| **FSUB** | `$E7A7` | Subtract FAC from ARG (Result in FAC) |
| **FMULT** | `$E97F` | Multiply ARG by FAC (Result in FAC) |
| **FDIV** | `$EA66` | Divide ARG by FAC (Result in FAC) |
| **FOUT** | `$ED34` | Convert FAC to an ASCII string (for printing) |

---

## 3. Recreating the "Hidden ROM" Pattern in our Virtual VM

This design pattern—where a low-level environment utilizes pre-baked ROM routines to save RAM—is exactly how we architected our **ZMM VM transient memory engine**.

Instead of writing complex mathematical functions inside the Z-Machine bytecode or Forth scripts, we map standard helper functions to **fixed, virtual ROM addresses** or trap events:

1. **Host-Side Operations**: The VM interceptor handles complex calculations or on-chain queries when the program jumps to specific virtual vectors (e.g. calling out to PulseChain for dynamic token data).
2. **Space Efficiency**: The game logic stays small, fast, and light, while the host VM does the heavy lifting.
