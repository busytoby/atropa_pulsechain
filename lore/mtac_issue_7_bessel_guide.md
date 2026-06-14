# MTAC Issue #7: The Bateman-Archibald Bessel Map and Multi-Family Synthesis

This document provides a detailed technical analysis of Harry Bateman and Raymond Clare Archibald’s monumental paper *"A Guide to Tables of Bessel Functions"* (MTAC Volume 1, Number 7, July 1944) and its application to designing a multi-family Bessel oscillator routing matrix.

---

## 1. The Bateman-Archibald Bessel Classification (Part I)

Bateman and Archibald organized the entire landscape of Bessel function tables into four primary sections under Part I, which we map directly to our synthesis modules:

### Section I: Standard Bessel Functions of the First Kind ($J_n(x)$)
* **Mathematical Focus**: Integral orders $J_0(x), J_1(x)$, their zeros $\alpha_{m,n}$, and derivatives.
* **Acoustic Equivalent**: Governs standard circular membrane resonances and standing waves. This is the core engine for metallic, bell-like, and percussive physical modeling timbres.

### Section II: Standard Bessel Functions of the Second Kind ($Y_n(x)$)
* **Mathematical Focus**: Weber/Neumann functions and their singular behavior near the origin.
* **Acoustic Equivalent**: Models pluck transients. Because $Y_n(x) \to -\infty$ as $x \to 0$, routing this function during the initial attack phase simulates the high-velocity, chaotic contact noise of a physical plectrum or hammer strike.

### Section III: Modified Bessel Functions ($I_n(x), K_n(x)$) and Third Kind ($H_n^{(1)}, H_n^{(2)}$)
* **Mathematical Focus**: Exponential growth ($I_n$) and decay ($K_n$) inside potential barriers, plus Hankel functions representing incoming/outgoing cylindrical waves.
* **Acoustic Equivalent**: Governs quantum tunneling saturation. By mixing $K_n(x)$ as a dynamic waveshaper, it compresses hot transients with soft, warm exponential curves mimicking a reverse-biased semiconductor junction.

### Section IV: Spherical Bessel Functions ($j_n(x), y_n(x)$)
* **Mathematical Focus**: Functions of fractional order $\pm(n + 1/2)$, also known as Riccati-Bessel functions.
* **Acoustic Equivalent**: Solves acoustic wave equations in 3D spherical chambers (e.g. the inside of a hollow wooden box or vocal tract resonant sphere), providing more organic, three-dimensional acoustic density than 2D cylindrical models.

---

## 2. DSP Application: The Multi-Family Bessel Synthesizer

By utilizing the four Bateman-Archibald classifications (Sections I–IV), we construct a **Multi-Family Bessel Oscillator**:

```
                         Bessel Input (x)
                                |
        +---------------+-------+-------+---------------+
        |               |               |               |
     [ Sec I ]       [ Sec II ]      [ Sec III ]     [ Sec IV ]
      Standard         Weber          Modified       Spherical
       Jn(x)           Yn(x)         In/Kn(x)        jn/yn(x)
        |               |               |               |
     Percussion       Pluck          Tunneling       3D Chamber
     Resonance      Transient       Saturation       Resonator
```

The user can morph between these sections using the **TUN** slider, shifting the oscillator's mathematical profile dynamically:
$$\text{Output}(x) = (1 - \text{TUN}) \cdot J_0(x) + \text{TUN} \cdot K_0(x) + \text{Transient}(\text{TUN}) \cdot Y_0(x) + \text{Chamber}(\text{TUN}) \cdot j_0(x)$$
