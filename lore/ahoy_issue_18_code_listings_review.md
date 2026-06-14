# Ahoy! Issue 18 Program Listings & Code Review

Here is the technical code analysis of the primary listings featured in **Ahoy! Magazine Issue 18 (June 1985)**:

---

## 1. Towers of Hanoi (Daniel Miller)
A BASIC implementation of the classic recursive peg-and-disk puzzle.

### **Core Logic**
* **Memory & Storage**: Tracks disk sizes on each peg using a simple array structure.
* **Input Hook**: Uses `GET K$` in a loop to capture non-blocking inputs for selecting source and target peg moves.
* **Verification**: Checks that the disk to be placed is smaller than the destination peg's topmost disk.

---

## 2. Duck Shoot (Bob Blackmer)
An action game where ducks fly across the screen, and the player shoots them using a crosshair.

### **Core Logic**
* **Sprite Graphics**: Leverages the C64's VIC-II sprite capabilities, setting custom sprite pointer shapes in memory addresses `2040-2047`.
* **Sound Generation**: Uses the SID chip (registers at `54272` and above) to play high-frequency noise sounds when ducks are shot.

---

## 3. 6510 Simulator
A machine-language instructional tool that visualizes register states of the 6510 CPU.

### **Core Logic**
* **Register Tracking**: Models accumulator (`A`), indexes (`X`, `Y`), program counter (`PC`), stack pointer (`SP`), and status flags.
* **Visualization**: Creates a visual memory map and stepping interface, showing memory locations and updating states in real time.

---

## 4. Quad-Print (Michael Beutjer)
A print utility linking up to four Doodle! format high-resolution graphics files.

### **Core Logic**
* **Device Control**: Sends special control sequences directly to printer devices (device #4, #5) for custom density dot-matrix printing.
