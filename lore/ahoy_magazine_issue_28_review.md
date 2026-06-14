# Ahoy! Magazine Issue 28 (April 1986) — Technical & Historical Review

Published in April 1986, **Issue 28 of *Ahoy!* Magazine** featured articles on strategic planning games, comet watching, and several high-performance arcade and utility listings for the Commodore 64.

This review details the technical architecture and features of the ready-to-enter programs in Issue 28.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section starting on Page 47:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Air Rescue** | C64 | Side-Scrolling Arcade | A helicopter/zeppelin rescue game developed by Cleveland M. Blakemore, featuring smooth horizontal screen scrolling and particle velocity tracking. |
| **JCALC** | C64 | Spreadsheet | An early interactive spreadsheet editor by Jeff Balvanz, using cell coordinate indexing and mathematical parser algorithms. |
| **Flankspeed** | C64 | Utility Creator | A system listing editor designed by Gordon F. Wheat to accelerate keyboard inputs and format text layouts. |
| **Mr. Mysto** | C64 / C128 | Card Simulator | A card trick and logic game developed by Frank DiNunzio. |
| **Chrono-Wedge** | C64 | Adventure | A time-travel puzzle text adventure by Dennis Muscatelli. |
| **Swoop** | C64 | Space Shooter | A fast-paced vertical alien intercept shooter by Tony Brantner. |

---

## 2. Technical Implementation details

*   **Smooth Horizontal Scrolling**: *Air Rescue* demonstrates double-buffered character shifting on the VIC-II (using fine scroll registers `$D016`), moving column matrices off-screen to prevent jitter.
*   **Dynamic Matrix Parsing**: *JCALC* implements custom coordinate hashing functions that map cells (e.g. `A1` to `E5`) into float arrays and runs relative formula evaluation passes.
