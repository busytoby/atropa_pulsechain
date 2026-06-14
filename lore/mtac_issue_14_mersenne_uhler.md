# MTAC Volume 2, Number 14: Horace Uhler and Mersenne Numbers

This document provides a detailed technical analysis of Horace S. Uhler’s April 1946 paper *"A new result concerning a Mersenne number"* (MTAC Volume 2, Number 14) and its relation to recursive modular arithmetic optimizations.

---

## 1. Horace Uhler's 1946 Mersenne Discovery

In the mid-1940s, searching for large primes was limited by the speed of manual arithmetic and electromechanical calculators.
* **Mersenne Numbers**: Numbers of the form $M_p = 2^p - 1$, where $p$ is prime.
* **Uhler's Contribution**: Uhler utilized the **Lucas-Lehmer Primality Test** to analyze Mersenne candidates, performing calculations by hand and with desktop calculators. In April 1946, he published verification that $M_{199} = 2^{199} - 1$ is composite, demonstrating the power of recursive modular testing to rule out candidate primes.

---

## 2. The Lucas-Lehmer Primality Test

The Lucas-Lehmer test is a highly optimized primality test for Mersenne numbers.

### A. The Recurrence Relation
Define the sequence $S_i$ for $i \ge 0$ by:
$$S_0 = 4$$
$$S_i = \text{mod}(S_{i-1}^2 - 2, M_p)$$

The Mersenne number $M_p$ is prime if and only if:
$$S_{p-2} \equiv 0 \pmod{M_p}$$

### B. DSP and VM Optimization: Shift-and-Add Modulo
In a low-level binary virtual machine (such as the ZMM VM), performing a modulo operation by a general number is expensive. However, division by a Mersenne number $2^p - 1$ can be optimized using bitwise shifts:
$$X \equiv (X \pmod{2^p}) + (X \gg p) \pmod{2^p - 1}$$
This allows the VM to compute the entire Lucas-Lehmer loop using only fast additions, subtractions, and bit-shifts, completely bypassing division.
