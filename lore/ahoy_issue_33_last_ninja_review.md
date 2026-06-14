# Cleveland M. Blakemore's *The Last Ninja* (Ahoy! Issue 33) — Code & Historical Review

This document reviews the historical context and technical architecture of **Cleveland M. Blakemore's *The Last Ninja***, published in **Issue 33 of *Ahoy!* Magazine (September 1986)**, and addresses labeling conflicts in our emulation assets.

---

## 1. Historical Context: The Two Last Ninjas

In the retro-computing ecosystem, there are two distinct C64 releases named *The Last Ninja*:
1.  **Cleveland M. Blakemore's Version (September 1986)**: Published as a type-in code listing in **Issue 33 of *Ahoy!* Magazine**. It is an action game featuring a side-scrolling or single-screen combat design where players control a ninja traversing levels.
2.  **System 3's Version (1987)**: A commercially released, critically acclaimed action-adventure game featuring an advanced **3D Isometric perspective** and score/music by Ben Daglish and Anthony Crowther.

> [!IMPORTANT]
> The code listing published in *Ahoy!* Issue 33 is the work of **Cleveland M. Blakemore** and predates System 3's commercial release.

---

## 2. Technical Analysis of Blakemore's Code Listing

Blakemore's C64 code listing utilizes standard custom sprite interrupts and SID chiptune registers to handle timing and inputs:
*   **Sprite Control**: Enables VIC-II registers at `$D015` to toggle hardware sprites for the player, enemies, and obstacles.
*   **Custom Coordinates**: Maps hardware coordinates to memory locations:
    - Player X coordinate: `$D000` (Sprite 0 X)
    - Player Y coordinate: `$D001` (Sprite 0 Y)
*   **Collision Detection**: Relies on Sprite-to-Sprite and Sprite-to-Background collision registers (`$D01E` and `$D01F`) to resolve damage and items collection.

---

## 3. Codebase Discrepancies & Recommendations

In [datamost.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/datamost.html), the game metadata is currently mislabeled or conflated:
*   **Label Conflict**: The HUD lists `"SYSTEM 3 1987"` (lines 6158 and 6163) and refers to System 3 character/sprite names (such as `"Armakuni"` and `"Shogun Guard"`).
*   **Correction**: The HUD and dropdown options should be corrected to properly reference **Cleveland M. Blakemore** and **Ahoy! Issue 33 (1986)**, resolving the historical inaccuracies.

### Proposed VRAM HUD Correction:
```javascript
// HUD
ctx.fillStyle = "#39ff14";
ctx.font = "bold 8px monospace";
ctx.textAlign = "left";
ctx.fillText("AHOY! ISSUE 33", 10, 15);
ctx.textAlign = "right";
ctx.fillText("LIVES: 3 | SWORD", canvas.width - 10, 15);
ctx.textAlign = "center";
ctx.fillStyle = "#ffffff";
ctx.fillText("THE LAST NINJA (C. BLAKEMORE 1986)", canvas.width / 2, 15);
```
