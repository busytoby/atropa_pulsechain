# 🤝 Consensus Task & Priority Voting Board

This board tracks active issues, proposed enhancements, and priorities for the retro emulator diagnostic suite. You can vote on these priorities to signal which subsystems should be expanded next.

---

## 📊 Active Priorities & Voting Status

| ID | Priority / Feature Name | Subsystem | Description | Node A (User) | Node B (Copilot) | Node C (Validator) | Consensus Status |
| :--- | :--- | :--- | :--- | :---: | :---: | :---: | :---: |
| **P-01** | VIC-II Raster Interrupt Timing Accuracy | VIC-II Renderer | Align scanline-level timing models to match exact PAL/NTSC cycles. | 🟢 Yes | 🟢 Yes | 🔴 No | **AGREED (2/3)** |
| **P-02** | Multi-channel SID Audio Envelope Controls | SID Synth | Expose ADSR sliders directly on the diagnostic control deck. | 🟢 Yes | 🟢 Yes | 🟢 Yes | **IMPLEMENTED (3/3)** |
| **P-03** | Local DFS Directory Mounting & Drag-Drop | Disk Filers | Drag `.d64` or `.t64` files directly onto the browser window to mount. | 🟢 Yes | 🔴 No | 🔴 No | **FAILED (1/3)** |
| **P-04** | Byzantine Protection & Slashing Dashboard | Consensus Engine | Expose node weights and automatic slashing parameters in the UI. | 🟢 Yes | 🟢 Yes | 🟢 Yes | **UNANIMOUS (3/3)** |
| **P-05** | Retro Rom Palette Swap & Filter Pack | Renderer | Add scanline overlays, CRT curve, and customizable index palettes. | 🔴 No | 🟢 Yes | 🟢 Yes | **FAILED (2/3 required)** |

---

## 🗳️ Voting & Resolution Guidelines

### How to Vote
1. **Interactive Voting**: Use the **DeepSeek Consensus & Input Voting** panel on the diagnostic dashboard to cast real-time simulated votes.
2. **Priority Adjustment**: If a feature is marked **FAILED**, you can lower the consensus threshold (e.g. from Unanimous to Simple Majority) or increase the Node B/C accuracy sliders to pass the consensus.
3. **Byzantine Intervention**: Turn on *Byzantine Mode* on Node C to test if the consensus pool can tolerate malicious voting patterns without dropping feature flags.

> [!TIP]
> **Reputation Slashing**: If Node B or Node C repeatedly vote against the user's priority selections, their reputation will decay by $10\%$ per round. If they drop below $50\%$, they will be suspended from voting until you hit the **🔧 Reset** button.
