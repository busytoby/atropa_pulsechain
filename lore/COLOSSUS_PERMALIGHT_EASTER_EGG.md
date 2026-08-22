# Colossus PermaLight Dynamic Illumination & Rasterization Engine

## 1. Overview and Historical Heritage
In Tommy Flowers' historic **Colossus** Mark 1 and Mark 2 valve-based optical teleprinter computers, vacuum tube thyratron registers and photocell optical tape readers maintained persistent state through thermal equilibrium and continuous gas-discharge luminescent glow. 

In our **CP/M-Tomie Enlightenment Desktop** and **Pixar RenderMan Utah Teapot** compute architecture on **VSEn** and **CDC 6600**, the **"PermaLight"** easter egg is implemented as an un-occluded, invariant dynamic point/directional light source that illuminates the bicubic Bézier patches with a non-decaying photon flux field.

## 2. Mathematical Formalism
The PermaLight shader calculates vertex and micropolygon shading using a non-preferential 3-term recurrence polynomial modulated by the Motzkin modular field:

$$I_{\text{PermaLight}}(u, v) = I_{\text{ambient}} + I_{\text{colossus}} \cdot \max\left(0, \mathbf{N}(u,v) \cdot \mathbf{L}_{\text{colossus}}\right)$$

Where:
* $\mathbf{L}_{\text{colossus}} = (\sin(\theta_{\text{colossus}}), \cos(\theta_{\text{colossus}}), 1.0)$ represents the orbiting Colossus valve photon vector.
* $\mathbf{N}(u,v)$ is the surface normal evaluated on the `DisplacementShader` cubic wavelet displacement field.
* $I_{\text{colossus}} = 1.000$ (permanent invariant flux).

## 3. EFL & VSEn Integration
* **Evas Canvas**: Renders the persistent golden-amber glow halo of the Colossus vacuum tubes around the Utah Teapot wireframe and shaded surface.
* **CDC 6600 PPUs**: Stream 10-channel parallel shadow maps illuminated by the PermaLight source.
* **EDI / Terminology**: Exposes the `PERMALIGHT ON/OFF` toggle command in CP/M-Tomie desktop binaries.
