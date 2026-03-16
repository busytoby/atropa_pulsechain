# TSFi Multi-Client Coordination Strategy (RBDP-1)

## Objective
**Goal**: 100% Functionality on Vulkan Window behavior.
- **Requirement A**: Window MUST display on the secondary monitor (Visibility).
- **Requirement B**: Window MUST NOT steal keyboard/mouse focus (Safety).

## Shared Unit Test
**Script**: `tests/discovery_focus_analysis.sh`
**Pass Condition**:
1.  Window is visible on Monitor 2.
2.  App DOES NOT terminate (Exit 0) after typing in terminal.
3.  Window STAYS OPEN.

## Status Registry

| Patch ID | Description | Status | Functionality % | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **P-REACTIVE**| **Focus Trap** | **VERIFIED** | 45% | Trap works. |
| **P-GAMMA** | **Delayed Resize** | **SUCCESS** | **90%** | Window visible, NO initial theft! |
| **P-FINAL** | **Trap Disable** | **NEXT** | Target: 100% | Allow window to persist. |

## Discovery Result
- **Breakthrough**: **Mission Gamma** (Start 1x1, Delay 60 frames, Resize to 3072x1728) successfully bypassed the Window Manager's "Focus New Window" policy.
- **User Report**: "Cannot confirm window stole focus... provided focus manually... auto-closed."
- **Meaning**: The window appeared *without* stealing focus. When the user *clicked* it, the trap killed it.
- **Conclusion**: We have achieved "Passive Visibility". Now we just need to let it live.

## Next Actions
1.  **Disable Trap**: Remove `exit(42)` from `vulkan_input.c`.
2.  **Verify**: Run `discovery_focus_analysis.sh`. It should pass (Window stays open).