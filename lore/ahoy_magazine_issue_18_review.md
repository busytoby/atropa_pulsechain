# Ahoy! Magazine Issue 18 (June 1985) — Technical & Historical Review

Published in June 1985, **Issue 18 of *Ahoy!* Magazine** focused on graphics utilities, logic puzzles, and processor simulators.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section of Issue 18:

| Program Title | Target System | Purpose / Architecture |
| :--- | :--- | :--- |
| **Quad-Print** | C64 | A graphics utility for dumping high-resolution screen images to compatible dot-matrix printers. |
| **Towers of Hanoi** | C64 / VIC-20 | A classic mathematical/logic puzzle game. |
| **Duck Shoot** | C64 | An interactive arcade shooting game. |
| **6510 Simulator** | C64 | A simulator showing register states, flag status, and instruction stepping for the C64's main CPU. |
| **Sing a Song of Anything** | C64 | An automatic musical score and note entry system. |

---

## 2. Implemented Features: Towers of Hanoi

To continue our modern integrations from *Ahoy!*, we have implemented a high-performance interactive **Towers of Hanoi** puzzle directly inside the terminal:

### **Gameplay & Mechanics**
* **Command**: Type `TOWERS` in the terminal, or select it from the `BOOTER` menu.
* **Controls**:
  * Press **`A`**, **`B`**, or **`C`** to select the source peg.
  * Press **`A`**, **`B`**, or **`C`** again to select the destination peg.
  * Press **`ESC`** to return to the shell.
* **Visualization**: Rendered with dynamic height block representation (`*` characters indicating disk sizes) on retro-styled pegs.
* **Rules**: Standard Hanoi rules (larger disks cannot be placed on smaller ones). Solved when all 5 disks reach Peg C.
