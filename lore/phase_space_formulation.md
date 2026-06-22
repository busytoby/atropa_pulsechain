# 🌀 Auncient Phase Space & Hamiltonian Mechanics Blueprint

This document details the transition from velocity-rate configurations (tangent space $TQ$) to canonical Hamiltonian mechanics in **Phase Space** (cotangent bundle $T^*Q$) for the 6-DOF spacecraft simulation.

---

## 1. The Configuration Manifold & Cotangent Bundle

To describe the state of the spacecraft in phase space, we map its position and orientation to a configuration manifold $Q$:

$$Q = \mathbb{R}^3 \times S^3$$

Where:
*   $\mathbf{r} = (x, y, z)^T \in \mathbb{R}^3$ represents translation.
*   $\mathbf{q} = (w, x, y, z)^T \in S^3 \subset \mathbb{R}^4$ represents rotation (unit quaternion double cover of $\text{SO}(3)$), satisfying the constraint $\|\mathbf{q}\|^2 = 1$.

The **Phase Space** is defined as the cotangent bundle $T^*Q$, where every point is a pair of configuration coordinates and their corresponding canonical conjugate momenta:

$$\mathbf{x} = (\mathbf{r}, \mathbf{q}, \mathbf{p}_r, \mathbf{p}_q) \in T^*Q$$

---

## 2. Translation Phase Space

For translation, the canonical coordinates are:
*   **Position:** $\mathbf{r} = (x, y, z)^T$
*   **Conjugate Linear Momentum:** $\mathbf{p}_r = (p_x, p_y, p_z)^T = m \dot{\mathbf{r}}$

The translation Hamiltonian $H_{\text{trans}}$ is:

$$H_{\text{trans}}(\mathbf{r}, \mathbf{p}_r) = \frac{\|\mathbf{p}_r\|^2}{2m} + V(\mathbf{r})$$

Where $V(\mathbf{r})$ is the external potential field (e.g., gravitational pull of targets or planetary bodies).

---

## 3. Rotational Phase Space on the $S^3$ Hypersphere

For rotation, we express dynamics directly on the unit quaternion hypersphere $S^3$ rather than mapping to coordinates that introduce gimbal locking.

### Conjugate Rotational Momentum
Let $\mathbb{I}$ be the $3 \times 3$ rigid-body inertia tensor in the local body-fixed frame.
The angular velocity in the body frame is $\boldsymbol{\omega} \in \mathbb{R}^3$.
The quaternion derivative $\dot{\mathbf{q}}$ relates to $\boldsymbol{\omega}$ via:

$$\dot{\mathbf{q}} = \frac{1}{2} E(\mathbf{q}) \boldsymbol{\omega}$$

Where the $4 \times 3$ matrix $E(\mathbf{q})$ is defined as:

$$E(\mathbf{q}) = \begin{pmatrix} 
-x & -y & -z \\
w  & -z &  y \\
z  &  w & -x \\
-y &  x &  w 
\end{pmatrix}$$

The kinetic energy of rotation is:

$$T_{\text{rot}} = \frac{1}{2} \boldsymbol{\omega}^T \mathbb{I} \boldsymbol{\omega}$$

Defining the canonical conjugate momentum $\mathbf{p}_q \in T^*_{\mathbf{q}} S^3$ as:

$$\mathbf{p}_q = \frac{\partial T_{\text{rot}}}{\partial \dot{\mathbf{q}}} = 4 E(\mathbf{q}) \mathbb{I} E(\mathbf{q})^T \dot{\mathbf{q}}$$

Using the relation $E(\mathbf{q})^T E(\mathbf{q}) = \|\mathbf{q}\|^2 \mathbb{E}_{3\times3} = \mathbb{E}_{3\times3}$ (on the unit sphere), the rotational Hamiltonian $H_{\text{rot}}$ is:

$$H_{\text{rot}}(\mathbf{q}, \mathbf{p}_q) = \frac{1}{8} \mathbf{p}_q^T E(\mathbf{q}) \mathbb{I}^{-1} E(\mathbf{q})^T \mathbf{p}_q$$

---

## 4. Hamilton's Canonical Equations of Motion

The total Hamiltonian of the spacecraft system is $H = H_{\text{trans}} + H_{\text{rot}}$. 
Hamilton's equations dictate the phase flow:

$$\dot{\mathbf{r}} = \frac{\partial H}{\partial \mathbf{p}_r} = \frac{\mathbf{p}_r}{m}$$

$$\dot{\mathbf{p}}_r = -\frac{\partial H}{\partial \mathbf{r}} = -\nabla V(\mathbf{r}) + \mathbf{F}_{\text{ext}}$$

$$\dot{\mathbf{q}} = \frac{\partial H}{\partial \mathbf{p}_q} = \frac{1}{4} E(\mathbf{q}) \mathbb{I}^{-1} E(\mathbf{q})^T \mathbf{p}_q$$

$$\dot{\mathbf{p}}_q = -\frac{\partial H}{\partial \mathbf{q}} = -\frac{1}{8} \frac{\partial}{\partial \mathbf{q}} \left( \mathbf{p}_q^T E(\mathbf{q}) \mathbb{I}^{-1} E(\mathbf{q})^T \mathbf{p}_q \right) + \boldsymbol{\tau}_q$$

Where $\mathbf{F}_{\text{ext}}$ and $\boldsymbol{\tau}_q$ are external non-conservative forces and thruster torques mapped into conjugate coordinates.

---

## 5. Symplectic Integration (Phase-Volume Preservation)

A standard Euler integrator violates Liouville's theorem, causing artificial energy drift and orbital decay. To maintain the **Auncient** geometric properties of the Phase Space, we must implement a **Symplectic Integrator** (such as a Semi-Implicit Euler or Stormer-Verlet scheme):

### Symplectic Euler Step ($\Delta t$):
1. Update momentum using current configuration:
   $$\mathbf{p}_r(t + \Delta t) = \mathbf{p}_r(t) - \nabla V(\mathbf{r}(t)) \Delta t + \mathbf{F}_{\text{thruster}} \Delta t$$
   $$\mathbf{p}_q(t + \Delta t) = \mathbf{p}_q(t) - \frac{\partial H_{\text{rot}}}{\partial \mathbf{q}} \Delta t + \boldsymbol{\tau}_{\text{thruster}} \Delta t$$

2. Update configuration using new momentum:
   $$\mathbf{r}(t + \Delta t) = \mathbf{r}(t) + \frac{\mathbf{p}_r(t + \Delta t)}{m} \Delta t$$
   $$\mathbf{q}(t + \Delta t) = \text{Normalize}\left( \mathbf{q}(t) + \frac{1}{4} E(\mathbf{q}(t)) \mathbb{I}^{-1} E(\mathbf{q}(t))^T \mathbf{p}_q(t + \Delta t) \Delta t \right)$$

---

## Next Steps
We can map these equations into a symplectic update loop inside [quaternion_fighter.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/quaternion_fighter.html) to transition the simulator from simple angular rate inputs to a complete phase space dynamic model tracking coordinate-momentum trajectories.
