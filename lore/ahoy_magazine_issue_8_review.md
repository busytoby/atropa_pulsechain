# Ahoy! Magazine Issue 8 (August 1984) — Technical & Historical Review

Published in August 1984, **Issue 8 of *Ahoy!* Magazine** highlights floppy disk directory structures, word processor buyer's guides, and practical system utilities for the Commodore 64 and VIC-20.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section:

| Program Title | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Micro-Minder** | C64 | Bob Lloret | A daily scheduler, calendar organizer, and memo database system. |
| **Directory Assistance** | VIC-20 & C64 | Joyce Woods | A disk utility to read, display, sort, and manage floppy directory structures. |
| **The Terrible Twins** | VIC-20 & C64 | George Trepal | A logic puzzle game testing deduction skills using coordinate grids. |

---

## 2. Deep Dive: Directory Assistance (Commodore 1541 BAM Layout)

*Directory Assistance* reads the directory sectors directly from the 1541 disk drive on Track 18:
*   **Track 18, Sector 0**: Holds the Block Availability Map (BAM) and pointer to first directory sector.
*   **Track 18, Sector 1**: The start of the file directory. Each directory sector contains up to 8 file entries (32 bytes each):
    *   `Offset 0`: File type (PRG, SEQ, USR, REL, DEL).
    *   `Offset 1-2`: Track/Sector link to next directory block.
    *   `Offset 3-18`: 16-byte filename (padded with shifted spaces `$A0`).
    *   `Offset 19-20`: Starting track and sector of the file.
    *   `Offset 21-22`: File size in blocks (LSB/MSB).

---

## 3. Emulation Integration Path

To integrate these utilities into our emulator workspace:
1.  **Directory Assistance**: Can be linked with our `diskSystem.yul` contract by exposing raw sector reading capabilities (Track 18 parsing) through the disk controller.
2.  **Micro-Minder**: Could serve as a command line scheduler utility (`MINDER`) inside our shell.
