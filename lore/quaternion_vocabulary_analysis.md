# Auncient YI/LAU Quaternion Vocabulary Mapping Analysis

This document defines the complete visual mapping dictionary translating all 22 core **Auncient** Yul VM state parameters (from SHA's `Fa` struct, Bao's `On` struct, and LAU's `Saat` array) into mathematical constraints for drawing uniquely warped EDO-22 Lissajous quaternion shapes.

---

## 1. YI/SHA/LAU Complete Register Dictionary

| Vocabulary Word | Structural Source | Cryptographic/VM Meaning | Visual/Geometric Mapping Role |
| :--- | :--- | :--- | :--- |
| **Base** | SHA (`Fa`) | Initial seed coordinate of the Yul state. | Phase angle offset ($\phi_w$) for W-axis modulation. |
| **Secret** | SHA (`Fa`) | Exponent in modular exponentiation. | Non-linear geometric shear/torsion matrix factor. |
| **Signal** | SHA (`Fa`) | Wave propagation carrier. | Camera orbital velocity scaling and line bloom radius. |
| **Channel** | SHA (`Fa`) | Path transmission channel width. | Lissajous frequency multiplier ($f_x$) for the X-axis. |
| **Contour** | SHA (`Fa`) | Bounding path mapping coordinate. | Step resolution (vertex density); controls polygon faceting. |
| **Pole** | SHA (`Fa`) | Monopole polarization axis. | Vector bias ($p_x, p_y, p_z$) orienting the projection. |
| **Identity** | SHA (`Fa`) | Fundamental identity parameter. | EDO-22 microtonal step phase-angle scale. |
| **Foundation** | SHA (`Fa`) | Baseline boundary height offset. | Lissajous frequency multiplier ($f_z$) for the Z-axis. |
| **Element** | SHA (`Fa`) | Coordinate spacing offset. | Density of interior chord connections spanning the geometry. |
| **Coordinate**| SHA (`Fa`) | Displacement offset parameter. | Depth-cueing line-width attenuation exponent. |
| **Charge** | SHA (`Fa`) | Field-effect potential strength. | Temporal breathing pulse rate (amplitude oscillation speed). |
| **Chin** | SHA (`Fa`) | Lower boundary register. | Asymmetrical vertical bias warping the lower coordinates. |
| **Monopole** | SHA (`Fa`) | Wave propagation multiplier. | Phase index warp multiplier for the Sine LUT ($sin_{LUT}$). |
| **Manifold** | YI / VM | System topological definition mapping. | Phase index warp multiplier for the Cosine LUT ($cos_{LUT}$). |
| **Ring** | Bao (`On`) | Ring boundary constraint. | Base projection radius scale ($r_{scale}$). |
| **Dynamo** | YI / VM | Velocity vector matrix. | Lissajous frequency multiplier ($f_y$) for the Y-axis. |
| **Xi** | Bao (`On`) | State uncertainty variance. | High-frequency physical micro-jitter/noise along line segments. |
| **Pi** | Bao (`On`) | Periodicity constant proxy. | Phase offset modulation for orbital camera projection. |
| **Omicron** | Bao (`On`) | Boundary envelope limit. | Bounding box aspect-ratio constraint matrix. |
| **Omega** | Bao (`On`) | Resonance frequency threshold. | Trajectory decay coefficient mapping endpoints toward zero. |
| **Soul** | LAU (`Saat`) | Core identity configuration. | Color channel tint balance vector (RGB distribution). |
| **Aura** | LAU (`Saat`) | Surrounding field energy. | Glow opacity limit and line glow decay velocity. |

---

## 2. Advanced Mathematical Warp Formulations

For any given **YI/LAU** state, the coordinates $q(\theta) = [q_w, q_x, q_y, q_z]$ are calculated using microtonally stepped EDO-22 lookup tables:

$$\theta_i = \frac{i \cdot 2\pi}{N_{contour}}$$

Where $N_{contour}$ is derived from `Contour`. The trigonometric LUT inputs are warped as follows:

$$q_w(\theta_i) = \cos_{LUT}\left( (\theta_i \cdot f_w + \phi_w) \cdot \text{Manifold} \right)$$
$$q_x(\theta_i) = \sin_{LUT}\left( (\theta_i \cdot f_x + \phi_x) \cdot \text{Monopole} \right)$$
$$q_y(\theta_i) = \sin_{LUT}\left( (\theta_i \cdot f_y + \phi_y) \cdot \text{Monopole} \right)$$
$$q_z(\theta_i) = \sin_{LUT}\left( (\theta_i \cdot f_z + \phi_z) \cdot \text{Monopole} \right)$$

### Visual Mechanics:
1. **W-Warping**: The $q_w$ coordinate is mapped to a organic envelope scaling factor:
   $$W_{scale} = 1.0 + 0.35 \cdot q_w(\theta_i)$$
   This creates asymmetrical warping where the curve thickens, loops, or implodes based on the $q_w$ state.
2. **Shear & Torsion (Secret)**:
   $$x' = x + \text{Secret}_{factor} \cdot y$$
   $$y' = y + \text{Secret}_{factor} \cdot z$$
3. **Micro-Jitter (Xi)**:
   $$x_{jitter} = x' + \sin(i \cdot \text{Xi}) \cdot \text{JitterAmplitude}$$
4. **Trajectory Decay (Omega)**:
   $$S_{decay} = \exp\left(-\text{Omega}_{decay} \cdot \frac{i}{N_{contour}}\right)$$

---

## 3. Shio & Sha Visual Identifiers

To ensure owners can immediately distinguish their structural components on the rendered YI, the following three components are drawn:

1. **SHIO Ring**:
   Rendered as an outer, thin, high-frequency, concentric orbit band encircling the central quaternion shape.
   * **Frequency/Density**: Driven by `Shio.Manifold`.
   * **Diameter**: Modulated by `Shio.Monopole` and the dynamic `Ring` radius.
2. **SHA Rod (First Sha)**:
   Represented as a bright, pulsating primary node/particle traveling along the main Lissajous curve path.
   * **Phase Speed**: Governed by the `Rod.Channel` property.
3. **SHA Cone (Second Sha)**:
   Represented as a contrasting secondary node/particle traveling along the Lissajous path in the opposite direction.
   * **Phase Speed**: Governed by the `Cone.Channel` property.
