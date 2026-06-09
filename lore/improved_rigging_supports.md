# Upgrading Phase 2.5: Advanced Kinematic Rigging Supports

This document specifies advanced improvements to our **Phase 2.5 (Kinematic Rigging & Hull Generation)** pipeline, introducing Inverse Kinematics (IK) target solvers, hull elasticity deformation, and dynamic center-of-mass balance vectors.

---

## 1. Inverse Kinematics (IK) Solvers
To allow entity limbs to reach for specific coordinates (e.g. the Teddy Bear reaching for an object, or feet conforming exactly to uneven planetary terrain in the *Mercenary* engine), we implement a fixed-point **Analytical IK Solver** in Yul:

```
    [Shoulder Joint (0,0)]
            \  L1 (Upper Arm)
             \
          [Elbow Joint]
             /
            /  L2 (Forearm)
           v
    [Hand Target (Tx, Ty)]
```

### The Trigonometric Solver
Using the Cosine Rule, the solver calculates the target interior elbow angle $\theta_2$ and shoulder angle $\theta_1$:
$$\cos(\theta_2) = \frac{T_x^2 + T_y^2 - L_1^2 - L_2^2}{2 L_1 L_2}$$
1. **Scaled Integer Math**: Standard float trig is converted to 1000-scaled fixed-point integer lookup tables.
2. **Angle Clamping**: If the hand target exceeds the boundary distance ($T_x^2 + T_y^2 > (L_1 + L_2)^2$), the target is clamped to the maximum extension line, preventing joint dislocation.

---

## 2. Hull Elasticity & Collision Deformation
Rather than treating collision hulls as perfectly rigid shapes, we introduce **Soft Hull Elasticity**:
* **Deformation Vectors**: When a collision hull (e.g., the Teddy Bear's torso cylinder) collides with a boundary wall, the engine computes a deformation offset based on overlapping depth:
  $$\vec{D} = \text{penetration\_depth} \times k_{\text{elastic}}$$
* **Visual Representation**: The Phase 4 shader applies this deformation offset directly to the wireframe vertices falling within the collision quadrant. This creates a visible "squish" or impact recoil when hitting surfaces, simulating soft matted stuffing under pressure.

---

## 3. Center of Mass (CoM) & Dynamic Balance Vectors
To prevent unnatural walking cycles and tipping over, the rigging compiler tracks weight distribution:
1. **CoM Coordinates**: Derived by multiplying the vertex coordinates of each hull by its mass density, then taking the weighted average.
2. **Base of Support (BoS)**: The polygonal area formed by the entity's feet (leg end vertices) touching the ground.
3. **The Balance Rule**:
   * If the projection of the CoM falls inside the BoS, the entity is stable.
   * If it falls outside the BoS, the rigging engine triggers a fall state or offsets the torso coordinates in the opposite direction to restore balance.

```
          (CoM) * (Stable: Projection falls inside feet)
                |
             [Torso]
             /     \
            /       \
         [Foot]===|[Foot] (Base of Support)
```
This guarantees physically stable bipedal stand and walk-off cycles.
