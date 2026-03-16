# TSFi A/B Test Manifest
**Objective:** Correlate specific code configurations with User Feedback % to isolate the "Focus Fix" from the "Visibility Break".

| Epoch | Configuration | Changes vs Control | Expected | Actual (User) | Verdict |
|-------|---------------|--------------------|----------|---------------|---------|
| **A-001** | **Control** | Standard XDG Toplevel, No Seat Trap | Visible, Focus Theft | TBD | **Baseline** |
| **B-001** | **AppID + Floating** | Added `app_id="tsfi_test_no_focus"` | Visible, No Theft? | 35% | **Partial** |
| **C-001** | **Layer Shell** | Replaced XDG with Layer Shell | Visible, No Focus | 0% (Hidden) | **Break** |
| **D-001** | **Seat Trap** | Unbound `wl_seat` | Visible, No Input | 46% (Stolen) | **Partial** |

## Current Active Epoch: A-001 (Restoration)
*   **Protocol:** XDG Shell
*   **Surface:** Toplevel
*   **Input:** Standard Binding
*   **Trap:** Disabled (Clean Slate)
*   **Goal:** Restore 25% Visibility.
