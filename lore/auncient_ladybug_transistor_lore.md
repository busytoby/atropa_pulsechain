# The Lore of the Ladybug Transistor

## 1. Physical Layout and Historical Origins
During the layout optimization of early WinchesterMQ SCSI interfaces, standard rectangular FET geometries suffered from severe parasitics due to high capacitive coupling between adjacent trace paths. To mitigate this signal leakage, engineers developed a circular, multi-gate layout known as the **Ladybug Transistor**.

```
              Ladybug Transistor Topology
                      [ Antenna ]
                       /       \
                     (   o   o   )  <-- Active Ring
                    (  o   o   o  )
                     \   o   o   /
                      [  Legs  ]
```

The device is structured with a circular drain active region surrounded by split gate legs that resemble the legs and antennae of a ladybug. The circular gate segments (spots) act as localized control nodes. This radial topology distributes the electrostatic discharge field evenly, eliminating corner-concentration capacitance and reducing inter-gate coupling spikes under Moon Ki Kim's Elastic Network Model (ENM) parameters.

---

## 2. Glossary Modulation Specifications

### LadybugTransistor
* **VM Register Context**: Multiplexer select register `LADYBUG_SEL` within the WinchesterMQ state machine. It maps three distinct guest VDM lines to a single coaxial write gate output.
* **Mathematical Function**: Resolves the radial electrostatic potential $V_r$ at spot coordinates ($r_i, \theta_i$) using the coupled spring forces:
  $$V_r = \sum_{i=1}^{Spots} \left( k_i \cdot (r_i - R_0)^2 - c_i \cdot \frac{dr_i}{dt} \right)$$
* **Visual / Geometric Modulation**: Modulates the starting phase angle shift ($\phi_w$) of the $q_w$ coordinate. This projects a pulsing, circular array of visual "spots" on the wireframe envelope that expand or contract according to the gate's charge conduction levels.
