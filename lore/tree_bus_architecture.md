# Technical Specification: 2-3 Tree Virtual Bus Architecture

This document defines the architecture of the **2-3 Tree Bus**, a logical routing framework designed for the Dysnomia VM and the Folklore CPU. This architecture replaces the legacy concept of parallel physical backplanes (such as the ISA BUS) with a balanced, self-balancing search tree structure that handles addressing, I/O routing, and device interrupt handshakes.

---

## 1. Architectural Overview

The legacy Industry Standard Architecture (ISA) bus relies on shared physical traces, address latches, and interrupt request lines (IRQs). This model introduces hardware bottlenecks, resource mapping conflicts, and fixed scale limits.

The **2-3 Tree Bus** replaces physical address space decoding with a balanced ternary tree ($O(\log N)$ depth). Every peripheral, memory segment, and hardware register is mapped as a node within a global system tree. Address decoding is executed as a recursive traversal of key boundaries.

```
                     [ System Root Node ]
                              |
         +--------------------+--------------------+
         |                    |                    |
  [ Low-Latency ]      [ Middle Stack ]     [ High-Latency ]
  RAM / GPU Registers    Unification Bus      SCSI / WinchesterMQ
```

---

## 2. Bus Node Topologies

The bus uses two structural node types to organize and route system calls:

### 2.1 The 2-Node (Single boundary Router)
*   **Structure**: 1 Key (Address Boundary), 2 Child pointers (Left, Right).
*   **Routing**: Addresses lower than the Key route to the Left branch; addresses equal to or higher route to the Right branch.

### 2.2 The 3-Node (Dual Boundary Router)
*   **Structure**: 2 Keys (Boundary Low, Boundary High), 3 Child pointers (Left, Mid, Right).
*   **Routing**: 
    *   $\text{Address} < \text{Boundary Low} \implies$ Route Left.
    *   $\text{Boundary Low} \le \text{Address} < \text{Boundary High} \implies$ Route Middle (Unification path).
    *   $\text{Address} \ge \text{Boundary High} \implies$ Route Right.

The **Middle branch** of a 3-node serves as the active unification stack. Transactions traversing this branch must unify their state properties (e.g. gas constraints, transaction signatures) before data propagation is allowed.

---

## 3. Unification-Based Address Resolution

Accessing a device or memory address is treated as a logic unification query:

$$\text{unify}( \text{TargetAddress}, \text{NodeKey} )$$

1.  **Deref**: The CPU dereferences the target address cell.
2.  **Traverse**: The execution loop branches down the tree matching the address key boundaries.
3.  **Unify**: When a leaf node (representing a device) is reached, the write payload is unified with the device's internal input registers.
4.  **Trail**: All state mutations during the traversal are recorded on the Trail stack. If the device rejects the transaction (e.g., buffer full or invalid parity), the unification fails, triggering backtracking to restore the bus state.

---

## 4. Hardware Interrupt Handshakes (Replacing IRQs)

Instead of dedicated interrupt lines toggling voltages, interrupts on the 2-3 Tree Bus are processed as asynchronous state unifications:

1.  **Event Node Creation**: When a device (such as the WinchesterMQ SCSI interface) requires CPU attention, it inserts an event node into the shared middle stack of the root 3-node.
2.  **Resolution Cycle**: On the next CPU cycle, the unification engine resolves the root node's middle branch. 
3.  **Execution**: If the event node unifies successfully, the CPU executes the device handler. If it fails, the choice point reverts the transition, deferring the interrupt safely until system resources are cleared.
