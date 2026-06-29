# FET Discharge Physics Simulation (Refined)

This report presents the soft body mass-spring relaxation dynamics coupled with temperature-dependent elasticity and electrical channel properties.

## Simulation Setup
- Nodes: 12
- Spring Constant (Base k): 100.0
- Damping coefficient (Base beta): 0.5

## Dynamic Parameters
- **Channel Resistance ($R_{ds}$)**: Modulated by node spacing / channel contraction.
- **Temperature ($T$)**: Elevated by resistive power loss ($I^2 R$) and dissipated to ambient.

### Telemetry Output
| Step | Node 1 (X) | Node 10 (X) | Resistance (Ω) | Temperature (K) |
|------|------------|-------------|----------------|-----------------|
| 0 | 1.175000 | 11.750000 | 1.5000 | 293.26 |
| 199 | 1.175000 | 11.750000 | 1.5000 | 314.69 |
| 200 | 1.175000 | 11.750000 | 8.5106 | 314.71 |
| 205 | 1.175000 | 11.750000 | 8.5107 | 314.78 |
| 210 | 1.175000 | 11.750000 | 8.5109 | 314.86 |
| 250 | 1.175000 | 11.749904 | 8.5165 | 315.47 |
| 500 | 1.175000 | 11.721597 | 8.5899 | 319.14 |
| 999 | 1.175000 | 11.616378 | 8.6812 | 325.87 |
