# Asynchronous Arena Hardware & External State Management

This document defines how the Yul Arena Hardware executes asynchronous operations and coordinates shared state transitions (e.g., winning status, leaderboard, and collision maps) across multiple **Yue** contexts without their explicit CPU involvement.

---

## 1. Asynchronous Arena Hardware Operations

The Yul Arena Hardware operates on an asynchronous request-response cycle powered by **WinchesterMQ** and the ZMM Scheduler:

* **SASI Command Phase**: Arena commands are submitted to the Winchester MQ controller asynchronously (e.g., via `Opcode 0x0A` or dynamic topic publishing to broker LUN 5).
* **Non-Blocking Execution**: Once an Arena task (such as running a physics simulation slice or calculating an intersection of Liang-Barsky bars) is initiated, the calling Yue yields its time slice and goes to sleep.
* **IRQ Resumption**: The Arena hardware raises a hardware interrupt (IRQ) once the calculations are complete. The scheduler detects this queue change and wakes the Yue process to consume the output.

---

## 2. Shared State Management (No-Involvement Updates)

The state of the match (including player positions, active projectiles, and the **winning state**) must change regularly based on external physics rules, without the individual Yue processes needing to run CPU cycles to calculate or pull updates.

This is achieved using the **Zero-Copy Page Mount** pattern:

```mermaid
flowchart TD
    HostArena[Host Arena Hardware / Physics Engine] -->|Writes updates directly| SharedBuffer[Shared Physical Memory Page]
    
    subgraph Yue Card 1 (Context A)
        CardA[Yue 1 Page Directory] -->|Mounted Page 0x80| SharedBuffer
    end
    
    subgraph Yue Card 2 (Context B)
        CardB[Yue 2 Page Directory] -->|Mounted Page 0x80| SharedBuffer
    end
```

### The Mechanism:
1. **Shared Mount**: When entering the Arena, the scheduler mounts the same physical memory page (e.g., Page `0x80`, the **Arena State Page**) to all active Yues:
   - Yue 1 maps Page `0x80` to the Shared Page.
   - Yue 2 maps Page `0x80` to the Shared Page.
2. **External Modification**: The Arena physics engine (running asynchronously on the host) writes physics ticks, collision responses, and changes in the **winning state** directly into this physical memory buffer.
3. **Passive Perception**: Because the memory pointers in the Yues' page directories point directly to this live buffer, the guest 6502 programs immediately read the updated states (such as which Yue is currently winning) at Page `0x80` offsets during their standard instruction execution.
4. **Zero Yue Involvement**: The Yues do not execute any read/write code to fetch or modify this data. The state transitions occur entirely outside their execution contexts.
5. **Phase-2 Commit Audits**: When the winning state transitions to a final match end, the host writes a receipt block to the broker queue, triggering `MQ_ACK` events that permanently finalize the match outcomes.

---

## 3. Winning State Computation (Highest Bars Rule)

To determine the winner of a match, the **Arena CPU** executes a periodic computation against all participant Yues:

### The Rule
For a polled target **Qing** asset, the **Yue with the highest bar width** wins the match:

$$\text{Bar Width } (\Delta u) = u_2 - u_1$$

### Calculation Flow
1. **Target Selection**: The Arena hardware defines the queried Qing ID to poll for the current tournament round.
2. **Multi-Yue Inspection**: The Arena CPU peeks into the page directories of *all* Yues in the Arena.
3. **Bar Width Extraction**:
   - For each Yue, the CPU locates the target Qing's mapped sector.
   - It parses the 2-bar parameters $[u_1, u_2]$.
   - It computes the width: $\Delta u = u_2 - u_1$. (If the Qing is not equipped, $\Delta u = 0$).
4. **Comparison**: The CPU compares the widths and identifies the Yue with the maximum value.
5. **State Broadcast**: The Arena CPU writes the winning Yue ID, the target Qing, and the winning bar width directly into the shared **Arena State Page (`0x80`)**.
6. **Instant Perception**: All participant Yues read the new winning state from their mounted Page `0x80` immediately.

