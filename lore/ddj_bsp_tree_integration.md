# BSP Tree Traversal from Dr. Dobb's Journal & TSFi2 Integration

This document details the mathematical model and Yul integration of **Binary Space Partitioning (BSP) Tree Traversal**, popularized by Michael Abrash's columns on the *Quake* engine in *Dr. Dobb's Journal*, to resolve polygon rendering order inside the **TSFi2 graphicsSystem.yul** contract.

---

## 1. The Mathematical Model of BSP Traversal

A BSP tree splits 3D space using planes. A plane is defined by a normal vector $\vec{N} = (A, B, C)$ and a distance $D$:
$$A \cdot x + B \cdot y + C \cdot z + D = 0$$

To determine the rendering order of polygons relative to the camera position $\vec{P}_{\text{cam}} = (x_c, y_c, z_c)$:
1. Calculate the dot product:
   $$S = A \cdot x_c + B \cdot y_c + C \cdot z_c + D$$
2. **If $S > 0$** (camera is in front of the splitting plane):
   * Traverse the **back** subtree first.
   * Render the partition polygon.
   * Traverse the **front** subtree.
3. **If $S < 0$** (camera is behind the splitting plane):
   * Traverse the **front** subtree first.
   * Render the partition polygon.
   * Traverse the **back** subtree.

This recursive traversal yields a correct back-to-front (Painter's Algorithm) or front-to-back rendering order, resolving depth sorting without sorting polygon vertices dynamically.

---

## 2. BSP Node Traversal in Yul

We can represent the BSP tree in storage. Each node is packed in a 256-bit word at `storage[nodeId]`:
*   `normalA`, `normalB`, `normalC` (signed 16-bit plane coefficients).
*   `planeDistanceD` (signed 32-bit distance).
*   `frontChildId`, `backChildId` (16-bit indexes).
*   `polygonId` (16-bit index).

Below is the Yul traversal routine:

```yul
// Traverses the BSP tree to output the correct rendering sequence of polygons
// nodeId represents the starting root node index
// cx, cy, cz represent the camera coordinates (fixed-point 1e18)
// outputPtr represents the memory address to write the sorted polygon IDs
function traverseBspTree(nodeId, cx, cy, cz, outputPtr) -> nextPtr {
    nextPtr := outputPtr
    
    // Leaf node check: if nodeId is 0xffff, we stop
    if eq(nodeId, 0xffff) { leave }
    
    // Load node layout from storage
    let nodeData := sload(nodeId)
    
    // Unpack plane normal coefficients and distance
    let normalA := and(nodeData, 0xffff)
    let normalB := and(shr(16, nodeData), 0xffff)
    let normalC := and(shr(32, nodeData), 0xffff)
    let distD   := and(shr(48, nodeData), 0xffffffff)
    
    let frontChild := and(shr(80, nodeData), 0xffff)
    let backChild  := and(shr(96, nodeData), 0xffff)
    let polygonId  := and(shr(112, nodeData), 0xffff)
    
    // Calculate dot product: normalA * cx + normalB * cy + normalC * cz + distD
    let dot := add(add(add(mul(normalA, cx), mul(normalB, cy)), mul(normalC, cz)), distD)
    
    // Front-to-back traversal based on dot product sign
    if sgt(dot, 0) {
        // Camera is in front: traverse back child first, then self, then front
        nextPtr := traverseBspTree(backChild, cx, cy, cz, nextPtr)
        
        mstore(nextPtr, polygonId)
        nextPtr := add(nextPtr, 32)
        
        nextPtr := traverseBspTree(frontChild, cx, cy, cz, nextPtr)
    }
    if sle(dot, 0) {
        // Camera is behind: traverse front child first, then self, then back
        nextPtr := traverseBspTree(frontChild, cx, cy, cz, nextPtr)
        
        mstore(nextPtr, polygonId)
        nextPtr := add(nextPtr, 32)
        
        nextPtr := traverseBspTree(backChild, cx, cy, cz, nextPtr)
    }
}

// sle helper: signed less-than-or-equal
function sle(x, y) -> r {
    r := iszero(sgt(x, y))
}
```

---

## 3. Benefits of Integration
*   **Constant-Time Sorting**: Resolves depth-ordering statically relative to the camera position, avoiding the $O(N \log N)$ cost of dynamic polygon sorting algorithms like Quicksort.
*   **Planar Occlusion Culling**: Enables front-to-back rendering with screen coverage tracking (1D scanline spans) to discard hidden polygons before rasterizing them, matching the performance tuning of early software renderers.

---

## 4. Conclusion

By integrating BSP Tree Traversal into our graphics contract pipeline, we apply the 3D occlusion optimizations discussed in *Dr. Dobb's Journal* to the TSFi2 platform, allowing the Synthesis Studio to display dense wireframe environments like Targ with perfect depth sorting and zero rendering anomalies.
