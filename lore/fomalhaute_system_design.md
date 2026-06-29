# System Design: Fomalhaute Visual & compute Architecture

In the Dysnomia VM, `Fomalhaute` is a physical-engine target resolved via the `XIA` engine contract, representing the core dielectric rod alignments defined by the triple-star coordinates of the **Fomalhaut** system.

---

## 🌌 The Triple Star Mapping Schema

```mermaid
graph TD
    subgraph Fomalhaut System Architecture
        FA["Fomalhaut A (Primary Star)"] -->|Defines| Xi["Dielectric Rod (Xi Base Coordinates)"]
        FB["Fomalhaut B (TW Piscis Austrini)"] -->|Dictates| DV["Drift Velocity & Temporal Logic Mutations"]
        FC["Fomalhaut C (LP 876-10)"] -->|Constrains| Dai["Diejective Cone (Daiichi Endpoint)"]
    end

    subgraph Dysnomia Compute Engine (XIA)
        Xi & DV & Dai -->|Inputs| XIA_E["XIA Engine Module"]
        XIA_E -->|Resolves Rod| SHIO["SHIO Contract (Fomalhaute)"]
    end

    subgraph Reaction Evaluation
        SHIO -->|Passed to| RF["ReactFomalhaute(Mu)"]
        RF -->|Evaluates| RSC["ReactShioCone(Fomalhaute, Mu)"]
    end
```

---

## 📐 Shape & Structural Relationships

### 1. Fomalhaut A: The Dielectric Rod Core (`Xi`)
*   **Visual Geometry**: Renders as a massive, central spherical node surrounded by a dense, rotating debris disk of Verlet dust particles.
*   **Behavior**: Serves as the origin coordinate (`0, 0, 0`) for the trilateral Banach-Hilbert grid, acting as the primary anchor pulling the particle waves.

### 2. Fomalhaut B: The Flare Drift Vector
*   **Visual Geometry**: Renders as a secondary, highly volatile flare star that periodically ejects high-energy plasma filaments.
*   **Behavior**: Directly scales the horizontal drift velocity of the background grid mesh and accelerates the mutation rates of the logic gates (shifting their shapes at twice the standard cycle frequency during flares).

### 3. Fomalhaut C: The Diejective Cone Constraint (`Daiichi`)
*   **Visual Geometry**: Renders as a distant, dim red dwarf node at the outer boundaries of the coordinate space.
*   **Behavior**: Establishes the terminal boundary line constraint for the centripetal funnel. All convergent flow lines in the Conify phase (Epoch 5) align along the vector connecting Fomalhaut A to Fomalhaut C.

---

## 💎 Cryptographic Mass vs. Topological Connectivity

This system adheres to a strict physical mapping law:

1.  **Cryptographic Mass (SHA Nodes)**: 
    *   *Rule*: Wherever a component is bound to one or more SHA hashes (like the Rod, Cone, and resolved Fomalhaute contract addresses), it represents physical **mass and matter**.
    *   *Visual Style*: Rendered as solid, detailed, textured 3D surfaces with high light-reflection, depth shadow, and carbon casing.
2.  **Topological Connectivity (The Grid)**: 
    *   *Rule*: The remaining components possess no SHA hashes. They represent pure **connections and functioning order**.
    *   *Visual Style*: Rendered strictly as flat mathematical lines, glowing translucent plasma vectors, and fluid particle tracks showing energy flow direction.
