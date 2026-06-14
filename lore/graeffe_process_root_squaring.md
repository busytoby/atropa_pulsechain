# The Graeffe Root-Squaring Process: Step-by-Step Algorithm

This document provides a step-by-step mathematical explanation of the Graeffe root-squaring process and its implementation inside our synthesis control systems.

---

## 1. The Mathematical Objective

Given a polynomial of degree $n$:
$$P_0(x) = a_n x^n + a_{n-1} x^{n-1} + \dots + a_1 x + a_0 = 0$$
with roots $r_1, r_2, \dots, r_n$ ordered by descending magnitude:
$$|r_1| > |r_2| > \dots > |r_n|$$
our goal is to find these roots using root-squaring.

---

## 2. The Root-Squaring Formula

In each iteration $k$ (where $k = 1, 2, 3, \dots$), we construct a new polynomial $P_k(x)$ whose roots are the squares of the roots of $P_{k-1}(x)$:

### Step 1: Algebraic Squaring
We multiply $P_k(x)$ by its conjugate $P_k(-x)$ and substitute $y = x^2$:
$$Q(y) = (-1)^n P_k(\sqrt{y}) P_k(-\sqrt{y}) = A_n y^n + A_{n-1} y^{n-1} + \dots + A_0 = 0$$

### Step 2: Coefficient Update Rule
For each new coefficient $A_m$ at iteration $k+1$:
$$A_m = a_m^2 + 2 \sum_{j=1}^{\min(m, n-m)} (-1)^j a_{m-j} a_{m+j}$$

For a quadratic polynomial $a_2 x^2 + a_1 x + a_0 = 0$ (such as our 2-pole filters):
*   $A_2 = a_2^2$
*   $A_1 = a_1^2 - 2 a_2 a_0$
*   $A_0 = a_0^2$

---

## 3. Root Extraction

After $m$ iterations, the roots of the original polynomial are raised to the power of $2^m$. Due to the exponential separation:
$$|r_1|^{2^m} \gg |r_2|^{2^m} \gg \dots \gg |r_n|^{2^m}$$
the coefficients approximate the simple products of the separated roots:
$$|r_1|^{2^m} \approx \left| \frac{A_{n-1}}{A_n} \right| \implies |r_1| \approx \left( \left| \frac{A_{n-1}}{A_n} \right| \right)^{1/2^m}$$
$$|r_2|^{2^m} \approx \left| \frac{A_{n-2}}{A_{n-1}} \right| \implies |r_2| \approx \left( \left| \frac{A_{n-2}}{A_{n-1}} \right| \right)^{1/2^m}$$

This allows the virtual machine to solve the absolute magnitudes of the poles using only simple additions, multiplications, and a final bit-shift approximation of the root extraction, guaranteeing instant, division-free execution.
