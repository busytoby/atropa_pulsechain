# QST Issue #14 Genetic Algorithm Node Selection: "One Thousand Listening Ears"

This document details the mathematical model and optimization strategy for selecting an optimal subset of $K$ receiver nodes from a large array to minimize Geometric Dilution of Precision (GDOP) using a Genetic Algorithm.

---

## 1. Geometric Dilution of Precision (GDOP) Mathematics

Given a localized transmitter estimate at $(\hat{x}_{\text{tx}}, \hat{y}_{\text{tx}})$, we choose receiver $0$ as the TDOA reference. For each selected receiver $i \in \{1, \dots, K-1\}$, the TDOA equation is:
$$f_i(x, y) = \frac{\sqrt{(x_i - x)^2 + (y_i - y)^2} - \sqrt{(x_0 - x)^2 + (y_0 - y)^2}}{c}$$

The partial derivatives forming the TDOA Jacobian matrix $\mathbf{H} \in \mathbb{R}^{(K-1) \times 2}$ are:
$$\mathbf{H}_{i, 0} = \frac{\partial f_i}{\partial x} = \frac{1}{c} \left( \frac{x - x_i}{d_i} - \frac{x - x_0}{d_0} \right)$$
$$\mathbf{H}_{i, 1} = \frac{\partial f_i}{\partial y} = \frac{1}{c} \left( \frac{y - y_i}{d_i} - \frac{y - y_0}{d_0} \right)$$
where $d_i = \sqrt{(x_i - x)^2 + (y_i - y)^2}$ and $d_0 = \sqrt{(x_0 - x)^2 + (y_0 - y)^2}$.

The Fisher Information Matrix (FIM) of the spatial coordinate estimates is:
$$\mathbf{G} = \mathbf{H}^T \mathbf{H} = \begin{bmatrix} g_{00} & g_{01} \\ g_{10} & g_{11} \end{bmatrix}$$
where:
$$g_{00} = \sum_{i=1}^{K-1} H_{i,0}^2, \quad g_{11} = \sum_{i=1}^{K-1} H_{i,1}^2, \quad g_{01} = g_{10} = \sum_{i=1}^{K-1} H_{i,0} H_{i,1}$$

The inverse matrix covariance is:
$$\mathbf{G}^{-1} = \frac{1}{g_{00}g_{11} - g_{01}^2} \begin{bmatrix} g_{11} & -g_{01} \\ -g_{01} & g_{00} \end{bmatrix}$$

The **Geometric Dilution of Precision (GDOP)** is:
$$\text{GDOP} = \sqrt{\text{trace}(\mathbf{G}^{-1})} = \sqrt{\frac{g_{00} + g_{11}}{g_{00}g_{11} - g_{01}^2}}$$

If the determinant $\text{det}(\mathbf{G}) = g_{00}g_{11} - g_{01}^2 \le 10^{-15}$, the configuration is singular (e.g. collinear stations) and $\text{GDOP} = \infty$.

---

## 2. Genetic Algorithm (GA) Optimization Loop

We optimize the selection of $K$ stations using a GA:

1.  **Chromosome**: A binary array $\mathbf{b} \in \{0, 1\}^N$ where $\sum_{i=0}^{N-1} b_i = K$ (exactly $K$ stations selected).
2.  **Population**: $M = 30$ individuals initialized randomly.
3.  **Fitness Function**:
    $$\text{Fitness}(\mathbf{b}) = \begin{cases} \frac{1}{\text{GDOP}(\mathbf{b}) + 1e-9} & \text{if } \text{det}(\mathbf{G}) > 10^{-15} \\ 0.0 & \text{otherwise} \end{cases}$$
4.  **Selection**: Tournament selection of size 3.
5.  **Crossover**: Single-point crossover on the active indices, followed by random repair (addition/deletion) to preserve the constraint of exactly $K$ active bits.
6.  **Mutation**: Swap active and inactive nodes with mutation probability $p_m = 0.15$.
