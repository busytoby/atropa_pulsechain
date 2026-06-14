# Ahoy! Magazine Issue 19 (July 1985) — Technical & Historical Review

Published in July 1985, **Issue 19 of *Ahoy!* Magazine** focused on graphic optimizations, game code structure, and basic program merge utilities.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section of Issue 19:

| Program Title | Target System | Purpose / Architecture |
| :--- | :--- | :--- |
| **Wizard of Im** | C64 | A machine language action game by Bob Spirko. Players move a wizard on screen using joystick commands and cast fireballs to vaporize objects. |
| **Auto-Append** | C64 / VIC-20 | A utility that adjusts pointers inside the Commodore memory space to dynamically merge two separate BASIC programs. |
| **Script Analysis** | C64 | An interactive profile manager by Bob Spirko that collects user inputs and calculates profiles based on handwriting traits. |
| **Speeding Pixels** | C64 | A collection of fast assembly routines by Dale W. Rupert and Mark Andrews designed to accelerate high-resolution bitmap screens. |

---

## 2. Technical Code Analysis

### **Auto-Append (William V. Braun)**
The tool works by re-mapping the pointers inside C64 zero page (`$2B–$2C` for start of BASIC, `$2D–$2E` for start of variables). When merging programs:
1. Loads the second program higher in memory.
2. Locates the link pointers at the beginning of each BASIC line.
3. Chains the second program's line links onto the end of the first program's final line link.
4. Updates the end-of-basic pointer (`$2D–$2E`) so Commodore's interpreter recognizes the unified program correctly.
