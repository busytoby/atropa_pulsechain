# Unified Architectural Specification: The Travellers Paradigm

This specification consolidates the concept of **"Travellers"** across three disparate layers of the **Auncient** system architecture: 3D spatial flight kinematics, Yul message queue routing, and real-time audio synthesis. Under this model, **travellers are functionally identical to containers (bags/collections)**, serving as self-routing packages that hold nested data elements or assets.

```mermaid
flowchart TD
    Spatial[3D Spaceflight Kinematics] -->|Orientation Quaternion q| Gen[Routing Slip Generator]
    Gen -->|Traveller Envelope| WMQ[WinchesterMQ Message Bus]
    WMQ -->|Step-by-Step Dispatch| Synth[Auncient Audio Synth]
    Synth -->|Modulation Feedback| Spatial

    subgraph Object Tree (Bags & Rooms)
        PrimaryContainer[Primary Container Element / Card] -->|Child| TravellerContainer[Traveller Container / Bag]
        TravellerContainer -->|Child| NestedAsset[Nested Reagent / Asset / State Data]
    end
    
    WMQ -.->|Updates Parent/Sibling/Child Pointers| PrimaryContainer
```

---

## 1. The Unified Core Concepts

| Domain | How is the Traveller a "Container"? | Mathematical / Data Structure Basis |
| :--- | :--- | :--- |
| **Spatial Kinematics** | Orbiting orientation vectors moving on a 3-Sphere manifold to avoid gimbal lock. | Unit Quaternions: $q = (w, x, y, z)$ |
| **WinchesterMQ** | Dynamic messages containing routing envelopes (routing slips) that hop sequentially across LUNs. | State checklists & transaction indices: `[LUN_A, LUN_B, ...]` |
| **Audio Synthesis** | Wave packets propagating along virtual string nodes, modulated by spatial panning orbits. | Panned delay lines & Formant wave guides: $f(t) \cdot \sin(\theta)$ |
| **Inventory Ledger** | Dynamic objects nested within parent nodes ("Bags" or "Rooms") representing active cards. | Z-Machine Hierarchical Object Tree Pointers: `parent(obj)`, `child(obj)`, `sibling(obj)` |

---

## 2. The Unified Lifecycle Flow

1. **Generation (Spatial Vector)**: 
   * A spaceship's quaternion orientation $q = (w, x, y, z)$ is updated in real-time.
   * A mapping function maps the unit sphere coordinates onto a discrete set of destination slots (LUN target IDs).

2. **Transmission (WinchesterMQ Slip)**:
   * The message is wrapped in a **Traveller Envelope** carrying the mapped LUN checklist:
     $$\text{Slip} = f(q) \rightarrow [\text{LUN}_A, \text{LUN}_B, \text{LUN}_C]$$
   * The message hops through the Yul virtual hardware registers. At each stop, the receiver updates the payload.

3. **Collection Containment (Bags & Rooms)**:
   * As the traveller arrives at a destination node, `move(object, new_parent)` updates the Z-Machine object tree.
   * **The Traveller acts as a Container itself**: It has its own `child` pointer, allowing it to carry other assets or payload elements as it travels through the hierarchy.
   * Unlinking and reparenting are performed dynamically as the traveller hops between locations.

4. **Realization (Synth Modulation)**:
   * The destination processes receive the data thunk and extract the spatial offsets.
   * These offsets modulate the traveling wave guide delay parameters and formant filters in the audio engine:
     $$\text{Freq}_{\text{mod}} = \text{Freq}_0 \times (1 + x_{\text{quaternion}})$$
   * Panning coordinates match the exact spatial location of the traveller.

---

## 3. Storage Register Mappings (`WinchesterMQ.yul`)

To implement this pattern at the assembly level, the following memory offsets are reserved:

* `0x2100`: Traveller active route checklist pointer.
* `0x2120`: Traveller current hop index.
* `0x2140`: Quaternion telemetry slot $w$.
* `0x2160`: Quaternion telemetry slot $x$.
* `0x2180`: Quaternion telemetry slot $y$.
* `0x21A0`: Quaternion telemetry slot $z$.
* `0x2200`: `parent(obj)` storage key offset.
* `0x2220`: `child(obj)` storage key offset.
* `0x2240`: `sibling(obj)` storage key offset.

---

> [!NOTE]
> This unified design binds spatial intent, transactional message propagation, hierarchical container storage, and physical sound output into a single, closed-loop state machine.

