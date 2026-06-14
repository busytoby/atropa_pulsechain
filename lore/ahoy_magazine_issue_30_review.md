# Ahoy! Magazine Issue 30 (June 1986) — Technical & Historical Review

Published in June 1986, **Issue 30 of *Ahoy!* Magazine** focused on desktop accessory utilities and multi-genre gaming.

This review details the technical architecture and features of the ready-to-enter programs in Issue 30.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Hidden Assets** | C64 | "Sidekick" style desktop accessory | Memory-resident utility hosting a phone directory, calendar, and screen-dump function via custom interrupts. |
| **LazyBASIC** | C64 | BASIC Extension | Extends Commodore BASIC V2 with convenience macros and shorthand command keys. |
| **Star Strike** | C64 | Action Game | Fast-paced space shooter utilizing sprite multiplexing and custom starfield scrolling. |
| **Shaker** | C64 | Arcade Game | Action game centering on player reflexes and sprite collisions. |
| **Queen's & Bishop's Tours** | C64 | Chess Simulator | An algorithmic search program that walks through Chess board trajectories and solutions. |
| **Got A Match?** | C64 | Game | Reflex and matching logic game. |
| **Trackdown** | C64 | Game | Maze / navigation puzzle game. |
| **Hidden Cavern** | C64 | Game | Exploration adventure game utilizing room transition vectors. |
| **Air Rescue** | C64 | Action Game | Helicopter rescue arcade game. |

---

## 2. Technical Features & Implementation Details

*   **Interrupt-Driven Utilities**: *Hidden Assets* is designed as a memory-resident TSR (Terminate and Stay Resident) tool for the C64. It hooks into the hardware vector interrupt address (usually `$0314/$0315`) to intercept keyboard inputs and render pop-up overlay menus without losing active memory state.
*   **Recursive Board Solvers**: *Queen's & Bishop's Tours* features backtracking search algorithms to calculate chessboard paths visiting every space without repeating.
*   **Sprite Multiplexing**: *Star Strike* utilizes custom raster interrupt splits to reuse hardware sprites, allowing more than 8 active alien objects to be displayed on a single screen refresh line.
