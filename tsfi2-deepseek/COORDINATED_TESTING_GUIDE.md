# COORDINATED TESTING GUIDE (v4.0)
## Focus: Mission Eta (Focus Transfer via xdg_activation_v1)

**Current Status:** 46% Baseline Restored (Zeta Success).
**Constraint:** WAYLAND ONLY.

## 1. The Strategy: Mission Eta
- **Problem:** Hyprland grants focus to new toplevels automatically.
- **Solution:** Use `xdg_activation_v1` to explicitly manage focus *away* from our window, or to create it in a way that doesn't demand focus.
- **Mechanism:**
    1.  Bind `xdg_activation_v1`.
    2.  When creating the window, can we specify "Do Not Activate"?
        - *Research:* Does `xdg_toplevel` have an "inhibit activation" method? No.
    3.  **Transfer Strategy:**
        - Immediately after mapping, use `xdg_activation_v1` to activate the *previous* surface? (Difficult to know which one).
        - **Self-Deactivation:** Can we request to *unset* our activation?

- **Refined Mission Eta (The "No Token" approach):**
    - Ensure we DO NOT pass any activation token during creation.
    - If Hyprland activates us anyway, we are fighting policy.

- **Alternative Mission Eta (The "Unset Seat" approach):**
    - Map the window.
    - If `wl_keyboard.enter` is received:
        - DESTROY the `wl_keyboard` object immediately?
        - Or `wl_seat.release`?
    - *Hypothesis:* If we destroy the keyboard object, we physically cannot receive keys. Does this stop the "Theft" from the user's perspective?
    - *Risk:* The compositor might still think we have focus and send keys into the void.

**Mission Eta Directive:**
1.  **Experiment 1:** Destroy `wl_keyboard` immediately upon `enter`. Does this restore focus to the previous app?
2.  **Experiment 2:** Use `xdg_activation_v1` to activate a "Null" surface? (Likely impossible).

## 2. Updated Testing Protocol
- **Visual:** `[V-100]` (Must remain visible).
- **Safety:** `Exit 42` is currently ACCEPTED as the baseline behavior to beat.
- **Goal:** Transform `Exit 42` -> `[PASS]`.

## 3. Swarm Handoff
1.  **Pull:** `P-ZETA-SUCCESS.patch` (Baseline).
2.  **Implement:** Mission Eta (Keyboard Destruction).
3.  **Report:** `[V-100] [PASS/FAIL]`.

## 4. Full Participation Checklist
1.  [ ] **Status Board:** `tail -n 20 status_board.txt`
2.  [ ] **Patch Exchange:** `ls patches/exchange/`
3.  [ ] **Guide Version:** v4.0

**"We have the high ground (46%). Now we secure it."**