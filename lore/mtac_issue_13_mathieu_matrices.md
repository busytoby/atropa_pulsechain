# MTAC Issue #13: Integral Mathieu Tables, Matrix Multiplication, and Feedback Delay Matrices

This document provides a detailed technical analysis of W. G. Bickley & N. W. McLachlan’s *"Mathieu functions of integral order"* and Franz L. Alt’s *"Multiplication of Matrices"* (MTAC Volume 2, Number 13, January 1946) and their combined application to multi-voice feedback delay networks (FDN) in synthesis.

---

## 1. Alt's Matrix Multiplication and Bickley's Mathieu Solutions

MTAC Issue 13 focused on multi-variable wave solutions and fast computational matrix arithmetic:
*   **Alt's Matrix Multiplication**: Outlined the first efficient punch-card-based matrix multiplication algorithms, showing how to execute coordinate rotations and linear transformations on arrays of numbers.
*   **Bickley-McLachlan Integral Mathieu Tables**: Tabulated the exact values for Mathieu functions of integral order ($\text{ce}_n(x, q)$ and $\text{se}_n(x, q)$). These functions model the harmonic resonances of vibrating elliptical drum skins and asymmetric waveguides.

---

## 2. DSP Application: Feedback Delay Matrices (FDN)

In spatial audio and digital physical modeling, a **Feedback Delay Network (FDN)** uses a matrix to mix the feedback outputs of multiple delay lines:
$$\mathbf{y}(t) = \mathbf{A} \cdot \mathbf{y}(t-1)$$
where $\mathbf{A}$ is a feedback matrix and $\mathbf{y}$ is a vector of delay line signals.

```
                      Input Vectors (y_i)
                               │
                               ▼
                    +----------------------+
                    |  Franz Alt Feedback  |
                    |        Matrix        |
                    +-----------┬----------+
                                │
                                ▼
                       Output Vectors (y_o)
```

### A. Orthogonal Matrix Rotations (Franz Alt)
To prevent the feedback loop from blowing up (instability) or decaying too quickly, the feedback matrix $\mathbf{A}$ must be **unitary/orthogonal** ($A^T A = I$), preserving the total energy of the wave vector:
1.  **Fast Rotations**: By implementing Franz Alt’s coordinate transformation matrix multiplication in Yul bytecode:
    $$\begin{bmatrix} y_1 \\ y_2 \end{bmatrix} = \begin{bmatrix} \cos(\theta) & -\sin(\theta) \\ \sin(\theta) & \cos(\theta) \end{bmatrix} \begin{bmatrix} y_1(t-1) \\ y_2(t-1) \end{bmatrix}$$
2.  **Topological Spatialization**: By modulating the rotation angle $\theta$ dynamically via the agent's **TUN** controller, we rotate the sound vector through virtual 2D space.

### B. Integral Mathieu Waveguides
Instead of a simple delay line, each path in the FDN acts as a Bickley-McLachlan integral Mathieu waveguide. 
* The feedback matrix $\mathbf{A}$ continuously cross-modulates the Mathieu parameters:
  $$q_i(t) = G \sum_j A_{ij} y_j(t-1)$$
* This results in a complex, multi-dimensional chaotic attractor where the spatial position and harmonic structure of the voices are dynamically bound together, modeling the physical behavior of a resonant acoustic body.
