# Ahoy! Magazine Issue 7 (July 1984) Code Listings Review

This document reviews the architecture, memory layouts, and algorithmic design of the program listings published in *Ahoy!* Issue 7 (July 1984) for the Commodore 64 and VIC-20.

---

## 1. Math Defender (VIC-20 & C64) — Andy Van Duyne
An educational arcade action game where mathematical equations fall down columns, and the player must input the correct answer before the equations reach the bottom of the screen.

### 1.1 Architectural Flow
*   **Game Loop**: Operates on a vertical gravity tick. Every $N$ frames, the active equation moves down by one row.
*   **Input Handling**: Maps keys **`F`** and **`J`** to navigate left/right columns, and the number row to input numerical answers.
*   **Equation Generator**: Dynamically constructs random math pairs:
    $$\text{Value}_1 \times \text{Value}_2 = \text{Target}$$
    Difficulty registers scale the maximum operand size from single-digit integers up to double-digit products.

### 1.2 Sound & VIC Registers (VIC-20 version)
*   **Voice Registry**: Writes to the middle voice generator at address `36876`.
*   **Siren Effect**: Decrements voice frequency value from 240 down to 130 inside the game-over loop to simulate a failing defense warning.
*   **Border Color**: Set via `POKE 36879, 238` for high contrast (Green background, black border).

---

## 2. Renumbering Utility (C64) — Anthony Wood
A system utility written by a teenage Anthony Wood (later founder of Roku) designed to re-index BASIC line numbers.

### 2.1 BASIC RAM Structure Manipulation
In C64 BASIC V2, programs are stored as a singly linked list starting at `$0801` (2049):
*   **Link Pointer** (2 bytes): Points to the memory address of the next line.
*   **Line Number** (2 bytes): Big-endian line index.
*   **BASIC Tokens**: Null-terminated stream of tokens and values.

```
+------------------+------------------+------------------+------------------+
| Link Pointer LSB | Link Pointer MSB | Line Number LSB  | Line Number MSB  |
+------------------+------------------+------------------+------------------+
| <---------------- 2 Bytes ---------> | <---------------- 2 Bytes ---------> |
```

### 2.2 Algorithm Design
1.  **Pass 1 (Address Collection)**: Traverses the link pointers starting at `2049`, mapping the old line numbers to their memory addresses and new target line numbers.
2.  **Pass 2 (Line Renumbering)**: Overwrites the 2-byte line number fields with the new linear progression sequence.
3.  **Pass 3 (Link Resolution & GOTO Update)**: Decodes tokenized statements looking for `GOTO` (`$89`), `GOSUB` (`$8D`), and `ON GOSUB` token sequences, modifying the target line numbers in-place based on the Pass 1 mapping.

---

## 3. What's My Job? (C64) — B.W. Behling
A dynamic database game demonstrating a **Binary Decision Tree** for classification.

```
                    [ Has fur? ]
                     /        \\
                  (Yes)       (No)
                   /            \\
             [ Bark? ]        [ Slithers? ]
              /     \\          /         \\
          (Dog)    (Cat)   (Snake)      (Fish)
```

### 3.1 Memory & Data Serialization
*   **Nodes Array**: Stored in a flat table representing parent-child linkages:
    $$\text{Node} = \{ \text{Text}[64], \text{LeftChildIndex}, \text{RightChildIndex} \}$$
*   **Dynamic Learning**: When a guess is incorrect, the program prompts the user:
    1.  *What is the correct job?*
    2.  *Provide a distinguishing yes/no question.*
    3.  Splits the current leaf node into a question node with two new leaves (the old guess and the new job).
*   **Disk/Tape Storage**: Writes sequentially to disk via device 8:
    `OPEN 1, 8, 2, "0:JOB DATA,S,W"`

---

## 4. Bug Repellent V2 (C64 & VIC-20)
The magazine's proofreader tool to prevent typos when entering code.

*   **Logic**: Hooked into the C64's **`CHRGET`** routine at memory address `$0073`.
*   **Checksum Verification**: Sums the character values of the currently typed line, performs an XOR check, and outputs a 2-letter alphabetic proof code. The reader compares this code against the published tables to check for spelling mistakes.
