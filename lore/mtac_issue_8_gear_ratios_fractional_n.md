# MTAC Issue #8: 14,000 Gear Ratios and Fractional-N Digital Tuning

This document provides a detailed technical analysis of L. J. Comrie’s review of R. M. Page’s *"14,000 Gear Ratios"* (MTAC Volume 1, Number 8, October 1944) and its direct application to fractional-N frequency division and microtonal digital tuning.

---

## 1. Mechanical Gear Ratios and Decimal Approximation

In mechanical engineering and early computing machinery (such as differential analyzers or gear-driven calculators), approximating an arbitrary decimal ratio using physical gears was a core challenge:
* **The Problem**: A designer needs a shaft to turn at a speed ratio of exactly $0.785398$ (representing $\pi/4$) relative to a drive shaft. Gears must have an integer number of teeth ($N_1, N_2, \dots$).
* **Page's 14,000 Gear Ratios**: Page compiled tables of integer pairs whose quotients matched decimal fractions up to six decimal places, enabling engineers to select the optimal tooth combination (e.g., $N_1/N_2 \approx \text{target}$).

---

## 2. DSP Application: Fractional-N Frequency Synthesis

In digital synthesizers, the equivalent problem is **Fractional-N Frequency Synthesis**—generating an arbitrary output pitch $f_{\text{out}}$ from a fixed system master clock $f_{\text{clk}}$ using integer counters:
$$f_{\text{out}} = f_{\text{clk}} \times \frac{M}{N}$$

### A. Rational Fraction Tuning
Instead of using floating-point operations (which are slow and introduce rounding drift in Yul bytecode), the synthesizer's phase accumulator uses integer gear ratios:
1. **The Clock Gear**: Let the master clock tick at a fixed rate (e.g., $10^9\text{ Hz}$).
2. **The Output Gear**: To generate a specific note (e.g., Middle C, $261.6256\text{ Hz}$), the phase increment is represented by the fraction $M/N$. 
3. **Approximation via Continued Fractions**: We calculate the optimum "gear teeth" $M$ and $N$ using continued fractions to minimize the frequency error:
   $$\frac{261.6256}{1000} \approx \frac{327}{1250}$$
   The VM updates the phase accumulator by adding $327$ every tick and wrapping around when it exceeds $1250$, guaranteeing zero long-term frequency drift and maximum timing accuracy.
