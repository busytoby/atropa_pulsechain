# Guide: How to Examine and Use the Evolutionary Consensus System

This workspace provides an interactive and visual way to study how decentralized agents (modeled as the 5 Teddy Bears) coordinate and reach consensus in a discrete, non-linear mathematical state space.

---

### Step 1: Open the Interactive Web Client
1. Locate the frontend file in your local environment: [reaction_synthesizer.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/reaction_synthesizer.html).
2. Open this file in your standard web browser (Chrome, Firefox, or Safari).
3. Click the red banner at the top: **"TAP TO ACTIVATE AUDIOPATHWAY"** to initialize the Web Audio engine.

---

### Step 2: Run Experiment 1 — The State of Discordance (Growling)
1. **Disable Twinning**: Uncheck the **"Evolutionary Twinning"** checkbox at the top.
2. **Program Discord**: Enter non-prime, highly composite values (e.g., `1000`, `2000`, `3000`) into the numeric input fields below the step sequencer buttons.
3. **Play**: Click the green **"Play"** button.
4. **What to Observe**:
   *   **The Sound**: You will hear a harsh, discordant, low-frequency soundscape modulated by a $33\text{ Hz}$ ring-oscillator (projecting the sub-vocal "growl").
   *   **The Trust Matrix**: Look at the 5x5 grid in the sidebar. The cells will turn deep crimson, indicating that the bears cannot verify the parameters and trust levels are collapsing.
   *   **The Constraint Circle**: On the oscilloscope canvas, the boundary circle around the rotating rosette will shrink to a tiny radius, representing the collapse of the eigenvalues ($\lambda$).

---

### Step 3: Run Experiment 2 — Emergent Consensus (Twinning)
1. **Enable Twinning**: Check the **"Evolutionary Twinning"** checkbox.
2. **Watch the Mutation**: As the playhead loops, watch the numeric input text fields. The sequencer will begin mutating the values automatically on the screen, searching for prime keys.
3. **Consensus Snap**: Once the input values snap to prime/harmonious numbers:
   *   **The Sound**: The $33\text{ Hz}$ growling instantly ceases, resolving into clean, resonant, harmonious binaural frequencies.
   *   **The Trust Matrix**: The cells in the 5x5 grid will drift dynamically from red to emerald green, indicating trust has stabilized.
   *   **The Constraint Circle**: The boundary circle on the canvas expands back to its full size as the eigenvalues recover.

---

### How this helps us examine our task
*   **Acoustic Debugging**: Rather than parsing complex hexadecimal EVM state logs, you can *hear* immediately when a node's parameter mapping is misaligned (growling vs. harmony).
*   **Visualizing Trust**: The 5x5 matrix acts as a live network topology map, showing which nodes are currently aligned and which are experiencing translation mismatch.
*   **State-Space Constraints**: The collapsing circle shows how quickly a node's allowable search space shrinks when it is out of sync with its peers, forcing it to focus its search parameters.
