# MTAC Issue #4: William Prager and Structural Dynamics in Synthesis

This document provides a detailed technical analysis of William Prager’s paper *"Tables of Certain Functions Occurring in Dynamics of Structures"* (MTAC Volume 1, Number 4, October 1943) and its direct application to physical modeling synthesis and resonant waveguide design.

---

## 1. William Prager and Structural Dynamics

William Prager (1903–1980) was a leading researcher in applied mechanics and elasticity. His October 1943 paper presented tables for solving the differential equations that govern the vibration of structural beams, plates, and elastic frames under dynamic loads.
* **Mass, Stiffness, and Damping**: The dynamics of structures are modeled by second-order differential equations:
  $$M \frac{d^2 x}{dt^2} + C \frac{dx}{dt} + K x = F(t)$$
  where $M$ is the mass matrix, $C$ is the damping matrix, $K$ is the stiffness matrix, and $F(t)$ is the external forcing function.

---

## 2. DSP Application: Physical Modeling and Waveguide Synthesis

In digital musical instrument design, **Physical Modeling Synthesis** replaces traditional wave generators with digital solvers that simulate these exact structural dynamics equations in real time:

### A. Digital Waveguides
A vibrating string or bar is modeled as a pair of delay lines propagating wave variables in opposite directions (representing traveling displacement waves), terminated by reflection and damping filters:
$$y(t, x) = y^+(t - x/v) + y^-(t + x/v)$$
* The reflection filter represents the boundary impedance (stiffness $K$ and damping $C$), which we implement as a low-level digital filter.

### B. Resonant Mass-Spring-Damper Solvers
To model cymbals, wooden soundboards, or structural plates, synthesizers discretize Prager's structural dynamics equations using finite-difference time-domain (FDTD) schemes:
$$x_{t+1} = \frac{2M \cdot x_t - (M - C \cdot dt) \cdot x_{t-1} - K \cdot dt^2 \cdot x_t + F_t \cdot dt^2}{M + C \cdot dt}$$
* Solving this in low-level bytecode (like the ZMM VM) allows the synthesizer to model authentic wood/metal strike dynamics, complete with organic decay and resonant modal densities, without relying on static pre-recorded samples.
