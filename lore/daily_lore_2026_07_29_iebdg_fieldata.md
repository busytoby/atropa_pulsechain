# The Codex of 6-Bit Initiation: IEBDG, FIELDATA, and Initial Orders 1

## 1. Initial Orders 1 & the Primacy of FIELDATA
* **VM Register Context:** Under the compiler firewall and instruction loader of the Dysnomia VM, the Initial Orders 1 phase acts as the speculative prefetch gate. Incoming instructions must be audited by the `AuncientAnalyzer` and checked against the prohibited opcode mask. The primary opcode register `op` is masked directly to a 6-bit FIELDATA boundary (`op & 0x3F`) before being promoted to Initial Orders 2 coordinates.
* **Mathematical Function:** The verification equation for speculative batch validation evaluates the congruence:
  $$\text{Valid} = \prod_{i=1}^{count} (1 - (\text{prohibited} \gg (\text{opcode}_i - \text{'A'}) \ \& \ 1)) \pmod 2$$
  where opcodes are mapped to 6-bit index offsets.
* **Visual / Geometric Manifestation:** Governs the camera's translation vector offset and orbital velocity scaling ($\phi_w$). When Initial Orders 1 validates instruction tracks, the projected coordinate grid shifts along the spatial axis to draw green vertex tracking lines; a validation reject freezes the wireframe envelope in cyan solid lines.

## 2. FIELDATA 6-Bit Format & Base64 Alignment
* **VM Register Context:** Storing virtual tape data streams in native 6-bit FIELDATA structures rather than standard 8-bit bytes matches the 64-character Base64 character register layout.
* **Mathematical Function:** Byte packing mapping is defined by a 1-to-1 conversion ratio:
  $$Char\_Index = Word\_Val \pmod{64}$$
  This completely removes bit-shifting instructions and padding characters (`=`) from the storage pipeline.
* **Visual / Geometric Manifestation:** Modulates the vector lines and dash patterns of the projected Lissajous orbits. The 6-bit value is used as an index to lookup the pixel line bloom radius and line opacity parameter ($\alpha$), enabling instantaneous trail pattern changes without parsing byte boundaries.

## 3. 4-4-4 AB4H Color Palette Indexing
* **VM Register Context:** The 6-bit FIELDATA structure indexes a 64-entry color lookup table where each entry contains a 12-bit 4-4-4 AB4H (Alpha-Red-Green-Blue) palette vector.
* **Mathematical Function:** The color coordinate mapping maps two 6-bit indices to a single 12-bit pixel:
  $$Pixel = (Index_1 \ll 6) \mid Index_2$$
* **Visual / Geometric Manifestation:** Modulates the red, green, blue, and alpha color blending ratios on the VTAM USS and 3270 virtual screen buffers, producing distinct visual color regions and high-speed palette swaps.

## 4. The IEBDG Generator Pipeline
* **VM Register Context:** `IEBDG` acts as the low-level data generator, populating sequential test structures inside the virtual file system.
* **Mathematical Function:** Sequential output records are generated using modular congruence loops to write EBCDIC-to-ASCII datasets.
* **Visual / Geometric Manifestation:** Modulates the grid coordinate density and layout scaling of the simulated VTAM terminal display options.
