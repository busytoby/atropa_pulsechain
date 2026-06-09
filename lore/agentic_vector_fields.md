# Agentic Vector Fields — Integrating Potential Fields with Autonomous Planners

To support autonomous decision-making in agentic AI frameworks, environmental representation must shift from static mapping to **dynamic potential fields**. By modeling goals as attractors and hazards as repellers, an agent can compute its next optimal action vector natively at runtime.

This document outlines the integration of vector fields with agentic AI planners, detailing path utility, resolution of local minima (traps) via decaying history buffers, and on-chain implementations.

---

## 1. Vector Fields as Agentic Utility Functions

In agentic planning, actions are selected by maximizing a utility function. We map the physical vector field directly to the agent's decision tree:

```
                  [Agent Position (Xe, Ye)]
                             |
         +-------------------+-------------------+
         |                                       |
  [Attractor Nodes]                      [Repeller Nodes]
  - Positive Utility                     - Negative Cost
  - Exert Pull Forces                    - Exert Push Forces
         |                                       |
         +-------------------+-------------------+
                             |
               [Accumulate Net Force Vector]
                             |
            [Direction Angle & Speed Decided]
```

### 1.1 Action Mapping
The resolved net force vector $\vec{F} = (F_x, F_y)$ represents the agent's **intended velocity**. The agent translates this into discrete actions:
* **Move Direction:** $\theta = \arctan2(F_y, F_x)$
* **State Shift:** If $|\vec{F}|$ drops below an activation threshold, the agent transitions from "Travel" to "Exploitation" (e.g., loading cargo, engaging combat, or searching locally).

### 1.2 Overcoming Local Minima (The Stuck State)
A common failure in potential field pathfinding is getting stuck in a local minimum (e.g., an attractor sits directly behind a wall repeller, resulting in a net force of zero).
To resolve this autonomously:
1. **The "Stuck" Detector:** If velocity falls below a threshold while distance to the primary goal is high, the agent registers a stuck state.
2. **Self-Avoiding Walk (Tabu Search):** The agent drops a temporary **repeller node** at its current coordinate. This pushes the agent away from its recent path, forcing it to navigate around the obstacle.

---

## 2. Decaying Memory Buffers (Tabu Repellers)

To implement self-avoiding exploration, the agent maintains a **history ring buffer** of visited coordinates. Each historical entry acts as a temporary repeller that decays in weight over time:
$$W_{\text{history}}(t) = W_{\text{max}} - \lambda \cdot (t - t_{\text{visited}})$$
Where $\lambda$ is the decay rate. As the agent moves away, the repulsion of previously visited spots fades, allowing it to return later if necessary.

---

## 3. On-Chain Yul Implementation: Decaying History Field Resolver

The following Yul function implements **Method 18: `resolveAgenticDecayField`** (Selector: `0x5d9b3fa1`). It parses static environmental nodes *and* a list of decaying history coordinates to output the optimal pathing velocity.

### 3.1 Memory Layout
* **Bits 0 - 7:** Node Weight ($W_i$)
* **Bit 8:** Node Type (`0` = Attractor, `1` = Repeller)
* **Bits 9 - 72:** Coordinate $X_i$
* **Bits 73 - 136:** Coordinate $Y_i$
* **Bits 137 - 144:** Sensory Radius ($R_i$)
* **Bits 145 - 208:** Timestamp/Age ($t_{\text{visited}}$) — Used for history nodes to compute decay.

### 3.2 Yul Method 18: `resolveAgenticDecayField`
```yul
// Method 18: resolveAgenticDecayField(entityX, entityY, numNodes, currentEpoch, nodes...)
// Selector: 0x5d9b3fa1
if eq(selector, 0x5d9b3fa1) {
    let entityX := calldataload(4)
    let entityY := calldataload(36)
    let numNodes := calldataload(68)
    let currentEpoch := calldataload(100)

    let netFx := 0
    let netFy := 0
    let signBit := 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

    for { let i := 0 } lt(i, numNodes) { i := add(i, 1) } {
        let nodeWord := calldataload(add(132, mul(i, 32)))

        let baseWeight := and(nodeWord, 0xff)
        let nodeType := and(shr(8, nodeWord), 0x01)
        let nodeX := and(shr(9, nodeWord), 0xfffffffffffffff)
        let nodeY := and(shr(73, nodeWord), 0xfffffffffffffff)
        let radius := and(shr(137, nodeWord), 0xff)
        let age := and(shr(145, nodeWord), 0xfffffffffffffff)

        // 1. Calculate Time-Based Weight Decay for history nodes (Type = 1 with non-zero age)
        let weight := baseWeight
        if and(eq(nodeType, 1), gt(age, 0)) {
            let elapsed := sub(currentEpoch, age)
            // Decay formula: weight = baseWeight - (elapsed * 5)
            let decayAmt := mul(elapsed, 5)
            if gt(weight, decayAmt) {
                weight := sub(weight, decayAmt)
            }
            if iszero(gt(weight, decayAmt)) {
                weight := 0
            }
        }

        if gt(weight, 0) {
            // 2. Manhattan Distance calculation
            let dx := 0
            if gt(nodeX, entityX) { dx := sub(nodeX, entityX) }
            if iszero(gt(nodeX, entityX)) { dx := sub(entityX, nodeX) }

            let dy := 0
            if gt(nodeY, entityY) { dy := sub(nodeY, entityY) }
            if iszero(gt(nodeY, entityY)) { dy := sub(entityY, nodeY) }

            let dist := add(dx, dy)
            if iszero(dist) { dist := 1 }

            // 3. Radius Cutoff Check
            let withinRadius := 1
            if gt(radius, 0) {
                if gt(dist, radius) {
                    withinRadius := 0
                }
            }

            if withinRadius {
                let force := div(mul(weight, 100), dist)

                let dirX := 0
                if gt(nodeX, entityX) { dirX := 1 }
                if lt(nodeX, entityX) { dirX := sub(0, 1) }

                let dirY := 0
                if gt(nodeY, entityY) { dirY := 1 }
                if lt(nodeY, entityY) { dirY := sub(0, 1) }

                if eq(nodeType, 1) {
                    dirX := sub(0, dirX)
                    dirY := sub(0, dirY)
                }

                netFx := add(netFx, mul(force, dirX))
                netFy := add(netFy, mul(force, dirY))
            }
        }
    }

    mstore(0x00, netFx)
    mstore(0x20, netFy)
    return(0x00, 64)
}
```

---

## 4. Subagent Integration Loop

High-level subagents (such as the `research` or `self` planners) use this solver to evaluate navigation states inside the ZMM VM without triggering expensive EVM writes:

```
[Subagent Planner] -> Writes candidate positions & history nodes to ZMM VM memory
       |
[ZMM YULEXEC (0x5d9b3fa1)] -> Runs vector field simulation loop in milliseconds
       |
[Action Decided] -> Subagent moves entity coordinates or updates plan variables
```

This ensures the agentic AI makes rapid, coordinate-accurate decisions based on decaying potential fields.
