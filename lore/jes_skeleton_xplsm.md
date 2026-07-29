# Lore: JES as the Relocatable SKELETON Template Format in the XPL Compiler Generator

In the virtual mainframe architecture, the Job Entry Subsystem (**JES**) is not merely a job queue queue manager; it functions as the relocatable **SKELETON** format specification. 

This template format is compiled by the **XCOM** backend and executed dynamically by the **XPLSM** state machine.

---

## 1. JES as the SKELETON Layout Template
The JES card deck structures system actions (steps, classes, allocations, and conditions). Rather than operating as static, hardcoded instructions, the JES layout defines the target template layout (the **SKELETON** dictionary format). It specifies symbol variables and execution offsets without binding them to absolute physical addresses.

---

## 2. Compilation via the XCOM Backend
The **XCOM** compiler backend reads the JES SKELETON template.
* It parses the 6-bit Fieldata characters.
* It resolves variables and options against its active symbol tables.
* It outputs a continuous stream of relocatable bytecode instructions directly to memory-mapped registers.

---

## 3. Dynamic Execution on the XPLSM Engine
The compiled bytecode is loaded into the **XPLSM** (the active-high execution monitor).
* **State Verification**: The XPLSM state machine processes the compiled instruction cards in sequence.
* **Gate Control**: Any out-of-bounds register references or priority violations automatically trigger the static RED rail gate (`ANALYZER`), halting system conduction instantly to protect the CPU registers.

This tripartite compiler generator design ensures that scheduling templates are compiled and verified dynamically, bypassing legacy terminal bottlenecks.
