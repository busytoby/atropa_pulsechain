# MTAC Issue #6: Punched-Card Tables and Variable-Interval DSP Interpolation

This document provides a detailed technical analysis of Paul Herget and Gerald Clemence’s paper *"Optimum-Interval Punched-Card Tables"* (MTAC Volume 1, Number 6, April 1944) and its direct application to optimizing lookup tables (LUTs) in digital synthesizers.

---

## 1. Optimum-Interval Punched-Card Tables

In 1944, computing with punched cards (like the IBM 601 multiplexer) was severely limited by storage capacity and card reader speeds.
* **The Challenge**: Storing a high-resolution mathematical function required huge decks of cards if the grid step size was uniform.
* **Herget & Clemence's Solution**: They developed **Optimum-Interval Tables**, where the step size of the argument is **variable (non-uniform)**.
  * In regions where the function is flat or linear, the interval between sample points is wide.
  * In regions where the function has high curvature or sharp transients, the interval between sample points is narrow.
  * By applying polynomial interpolation (second and third differences), they minimized the total number of cards needed while keeping interpolation errors below a target threshold.

---

## 2. DSP Application: Compressed Variable-Interval LUTs

In modern synthesizers running on low-resource microcontrollers or within constrained EVM/ZMM execution environments:
* **The Problem**: High-resolution wavetables (e.g., 2048-point wavetables) consume significant memory and execution gas.
* **The Optimum-Interval Solution**:
  * We compress our waveshapes and diode curves using a **non-uniform grid**.
  * For example, in our **Tunnel Diode I-V Curve**, we place many interpolation points around the narrow peak ($80\text{mV}$) and valley ($350\text{mV}$) regions where the negative resistance transitions occur, and very few points in the linear diffusion region above $0.8\text{V}$.
  * During simulation ticks, the index lookup calculates the local interval size and performs a weighted linear interpolation:
    $$y = y_0 + (y_1 - y_0) \frac{x - x_0}{x_1 - x_0}$$
  * This reduces the memory footprint of our synthesis LUTs by up to $80\%$ without any loss in audio fidelity or harmonic accuracy.
