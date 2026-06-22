# Z-Machine Integration: Locus & Diyat Tax Design

This document details the virtual hardware, registers, and Yul execution designs to integrate the core concepts of **Internal Locus of Control**, **PEEK/POKE Diyat taxation**, and **Complex Dysnomia Reactions** directly into the on-chain Z-Machine platform.

---

## 1. Direct CPU & Register Coupling

To establish a complete internal locus of control, the Z-Machine's virtual execution loop is coupled directly with the Yul virtual CPU registers, eliminating intermediate translation layers.

```
                  [ Z-MACHINE CPU COUPLING ]
                  
     [Z-Machine Console] <══(Text command translation)══> [Yul execution loop]
             │                                                  │
    (PEEK read telemetry)                              (POKE register write)
             ▼                                                  ▼
      [Free Inspection]                                  [Compounded Diyat]
```

---

## 2. Hardware Register Access (Ammeter & Voltmeter)

The Z-Machine uses dedicated virtual registers mapped to CPU memory to pace opcodes and regulate resource consumption:

*   **`PEEK Ammeter` (`$D6E0`)**: Allows the console execution thread to query real-time load levels, tracking active transaction velocities and operational current without tax overhead.
*   **`POKE Voltmeter` (`$D6E1`)**: Allows the engine to adjust clock execution rates, logic gate configurations, and execution priority limits dynamically.

---

## 3. Diyat Game Tax (PEEK vs. POKE)

The Z-Machine strictly enforces Diyat taxes on mutations while keeping informational lookups free:

*   **Read-Only (`PEEK`):** Commands such as `LOOK`, `INVENTORY`, or `EXAMINE` only inspect state variables and are **free** of Diyat taxation.
*   **State-Mutating (`POKE`):** Commands that alter the game board state (such as `TAKE`, `DROP`, `OPEN`, `UNLOCK`) call the external `diyat.exciseTax` contract (selector `0x904a4bc3`), deducting the OTRT fee directly from the caller's balance.

---

## 4. Simplifying Complex Dysnomia Reactions via Z-Machine Command Parsing

Instead of constructing multi-step raw Ethereum transactions to interact with the underlying Dysnomia core, the Z-Machine parses simple text-based command verbs and translates them directly into Yul contract calls:

| Z-Machine Text Command | Target Selector | Decoupled Dysnomia Interaction |
| :--- | :--- | :--- |
| **`> REACT [MINTER]`** | `0x6bc32fe2` | Resolves the partner (Qing) address and calls `YUE.React(address)`. |
| **`> START [NAME] [SYM]`** | `0x78519019` | Automatically fetches connected LAU and calls `SEI.Start(Lau, Name, Sym)`. |
| **`> PLAY`** | `0x74ff4718` | Verifies identity card status and triggers `CHOA.Play(LauToken)`. |
| **`> PEEK AMMETER`** | *Direct Read* | Queries register `$D6E0` directly to check active VM current. |

This integration bridges retro text interface design with the most complex mathematical coordinates of the on-chain reaction layers, making the entire ecosystem simple to play.

---

> [!NOTE]
> These Z-Machine contract configurations can be verified against the test suites located inside `tests/` and deployed to the local PulseChain development environment.
