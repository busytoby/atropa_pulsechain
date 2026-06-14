# Bessel 'Riders': Higher-Order Modes and Harmonic Overtones

This document provides a mathematical and acoustic analysis of Bessel "riders"—the higher-frequency harmonic modes and modulating wave ripples that ride on top of the primary Bessel envelope in physical membrane models.

---

## 1. The Boundary Conditions of a Circular Membrane

For a circular membrane of radius $R$ clamped at the boundary, the wave displacement must go to zero at the edge:
$$\psi(R, \theta, t) = 0$$

This boundary condition restricts the allowed radial wave numbers $k$ to the discrete roots of the Bessel function:
$$J_m(k_n R) = 0$$

Let $\alpha_{m,n}$ be the $n$-th positive root of the $m$-th order Bessel function $J_m(x)$. The allowed frequencies are:
$$f_{m,n} = \frac{\alpha_{m,n}}{2\pi R} \sqrt{\frac{T}{\sigma}}$$
where $T$ is membrane tension and $\sigma$ is area density.

---

## 2. Bessel Overtones as 'Riders'

In a real physical strike (such as a kettledrum or gong), the resulting sound is not a pure tone. Instead, the total wave displacement is a superposition of the fundamental mode and multiple higher-frequency **riders** (overtones):
$$\Psi(r, \theta, t) = \sum_{m=0}^{\infty} \sum_{n=1}^{\infty} A_{m,n} \cdot J_m\left(\frac{\alpha_{m,n} r}{R}\right) \cos(m\theta) \cos(\omega_{m,n} t)$$

### A. Harmonic Ratios of the Riders
Unlike a vibrating string (where the overtones are integer multiples of the fundamental: $2f_0, 3f_0, 4f_0$), the roots of Bessel functions are **non-integer** (anharmonic):
*   **Fundamental ($J_0$, root 1)**: $\alpha_{0,1} \approx 2.4048$ (Frequency ratio: $1.0$)
*   **Rider 1 ($J_1$, root 1)**: $\alpha_{1,1} \approx 3.8317$ (Frequency ratio: $1.59$)
*   **Rider 2 ($J_2$, root 1)**: $\alpha_{2,1} \approx 5.1356$ (Frequency ratio: $2.14$)
*   **Rider 3 ($J_0$, root 2)**: $\alpha_{0,2} \approx 5.5201$ (Frequency ratio: $2.30$)

These high-frequency Bessel riders ride on top of the fundamental envelope, creating the complex, metallic, and dispersive timbre characteristic of bells and drums.

### B. Modulator Amplitude Riders (FM)
In FM synthesis, the term "rider" also describes the high-frequency modulator wave nested inside the carrier phase:
$$\sin(\omega_c t + \beta \sin(\omega_m t))$$
The Bessel coefficient $J_n(\beta)$ directly acts as the scaling envelope for each sideband "rider," governing how much energy ripples out to the higher frequencies.
