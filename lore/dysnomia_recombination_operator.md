# The Erisian Splitting-Stabilization Recombination Operator

This document provides a formal number-theoretic definition of the algebraic function used to split a composite boundary, locate its nearest prime limits, and reconstruct the values to find new prime vertices.

---

## 1. Mathematical Definition of the Operator

Let $M \in \mathbb{N}$ be an $N$-digit integer. We define the **Splitting-Stabilization Recombination Operator** $\mathcal{H}_k(M)$ with respect to a split boundary index $k$ ($1 \le k < N$) as follows:

### Step 1: Digit-Based Partition (Decomposition)
We partition $M$ into its upper section (Apogee) $U$ and lower section (Apex) $L$ using base-10 divisor division:
$$U(M, k) = \lfloor M \cdot 10^{-k} \rfloor$$
$$L(M, k) = M \pmod{10^k}$$

---

### Step 2: Boundary Prime Stabilization
We define two prime-locating operators over the set of prime numbers $\mathbb{P}$:
1.  **Next Prime Operator** (Apogee stabilization):
    $$\pi^+(U) = \min \{ p \in \mathbb{P} \mid p > U \}$$
2.  **Previous Prime Operator** (Apex stabilization):
    $$\pi^-(L) = \max \{ p \in \mathbb{P} \mid p < L \}$$

---

### Step 3: Concatenative Recombination
Let $d$ be the number of decimal digits of the stabilized lower component $\pi^-(L)$, given by:
$$d = \lfloor \log_{10}(\pi^-(L)) \rfloor + 1$$

The recombination function $\mathcal{H}_k(M)$ is defined by shifting the stabilized upper component by $d$ digits and adding the lower component:
$$\mathcal{H}_k(M) = \pi^+(U) \cdot 10^d + \pi^-(L)$$

---

## 2. Application to the 36th Motzkin Prime

Using $M = M_{36} = 953467954114363$ and a split index of $k = 9$:

1.  **Partition**:
    $$U = \lfloor 953467954114363 \cdot 10^{-9} \rfloor = 953467$$
    $$L = 953467954114363 \pmod{10^9} = 954114363$$
2.  **Stabilization**:
    $$\pi^+(953467) = 953473$$
    $$\pi^-(954114363) = 954114361$$
3.  **Concatenation**: Since $\pi^-(L) = 954114361$ has $d = 9$ digits:
    $$\mathcal{H}_9(M_{36}) = 953473 \cdot 10^9 + 954114361 = 953473954114361$$

Because the output is prime, the operator has successfully mapped the chaotic boundary of $M_{36}$ into the stable prime point: the **VERTEX of DYSNOMIA**.
