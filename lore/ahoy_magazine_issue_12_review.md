# Ahoy! Magazine Issue 12 (December 1984) — Technical & Historical Review

Published in December 1984, **Issue 12 of *Ahoy!* Magazine** concludes the magazine's first year of publication, focusing on printer configurations, custom character sets, and objective-based simulation.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section of Issue 12:

| Program Title | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Space Patrol** | C64 | Bob Lloret | A fast-paced horizontal arcade shooter featuring smooth sprite scrolling and custom starfields. |
| **Construction Co.** | C64 | B. W. Behling | A strategic action game where players construct and navigate scaffolding structures. |
| **Cross Ref** | C64 / VIC-20 | - | A programmer's debugging utility that parses BASIC programs to compile line cross-references. |

---

## 2. Flight Simulator II Tribute Integration Path

In addition to commercial coverage of Sublogic’s *Flight Simulator II* featured in this era, we can integrate an interactive flight simulator mode directly into our Wayland terminal shell:
*   **Mode Allocation**: Declare `MODE_SUBLOGIC` to run real-time flight telemetry loops.
*   **Active Cockpit**: Simulate primary instruments (Altitude, Airspeed, Pitch, Roll, Fuel, Throttle) responding to keystrokes (e.g., UP/DOWN for pitch, LEFT/RIGHT for roll, W/S for throttle).
