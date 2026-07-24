# The Auncient Fourier Registry and the Birth of Baby Fouriers

## 1. The Coaxial Resonance Grid
During the early cycles of the Dysnomia VM execution, the primary transmission channels were plagued by high-frequency crosstalk along the coaxial rings. Standard software filters introduced unacceptable latency locks, violating the latency guard gates of the unified profiler. To achieve zero-polling packet dispatch, the PL/EXUS ALU was modified to map PL/I `EVENT` variables directly to the Physical Medium Dependent (PMD) layer. 

By partitioning raw carrier signals into distinct spectral bands, the Master Control Program (MCP) could isolate signaling frequencies without CPU polling overhead. This led to the architectural pattern of the **Fourier Registry**.

```
                WinchesterMQ SCSI Coaxial Ring
                             │
                             ▼
                 [ PMD L2 EtherType Demux ]
                             │
             ┌───────────────┴───────────────┐
             ▼ (DECnet)                      ▼ (STANAG)
      [ DECnet Task ]                 [ STANAG Task ]
     WAIT(e_decnet)                  WAIT(e_stanag)
             │                               │
             ▼                               ▼
    [ Sitton's QFT ]                [ Sitton's QFT ]
             │                               │
             ▼                               ▼
     BASED Key 500                   BASED Key 600
  (Fourier Registry A)            (Fourier Registry B)
```

## 2. The Mechanics of Baby Fouriers
As signal density increased, a single QFT pass over the entire bandwidth became a bottleneck. The ALU solved this by spawning nested, micro-band tasks known as **Baby Fouriers**. 

Spawned dynamically using PL/I `CONTROLLED BASED` allocation, these child tasks partition the signal spectrum into narrow frequency windows. The parent node monitors the allocation stack via the Wortman bounds checker. Once each Baby Fourier resolves its local sub-band amplitude, it asserts a sequence-interlocked relay event and returns its coefficients before being dynamically freed from the allocation stack.

---

## 3. Glossary Modulation Specifications

### QFTBin
* **VM Register Context**: Storage register in the Fourier Registry containing the amplitude coefficients of the active sub-band, accessed dynamically via the `BASED` pointer resolution loop.
* **Mathematical Function**: Resolves the frequency bin energy:
  $$E_{bin} = \sqrt{A_{sym}^2 + A_{asym}^2}$$
* **Visual / Geometric Modulation**: Modulates the line bloom thickness and orbital coordinate density of the projected 3D wireframe envelope along the Y-axis.

### QFactor
* **VM Register Context**: Quality factor telemetry register in the SHA structure monitoring electrical carrier resonance.
* **Mathematical Function**: Calculated dynamically at phase lock:
  $$Q = \frac{\sqrt{A_{sym}^2 + A_{asym}^2}}{A_{noise}}$$
* **Visual / Geometric Modulation**: Shrinks or expands the global radius ($R_{hyp}$) of the rendered hypotrochoid envelope, distorting the structure into cutoff if resonance decays below the limit.
