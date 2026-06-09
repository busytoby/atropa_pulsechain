# Graphics Display Technologies Review

We have several graphics display technologies integrated across the workspace:

---

## 1. Virtual VIC-II Sprite Collision Engine
Located inside [graphicsSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/graphicsSystem.yul), this system emulates the classic Commodore 64 **MOS 6567/6569 VIC-II** sprite control registers:
* **Register Mappings**:
  * `53248` to `53263` (`$D000` to `$D00F`): Sprite X/Y coordinate storage for Sprites 0–7.
  * `53269` (`$D015`): Sprite Enable Mask (active sprite states).
  * `53278` (`$D01E`): Sprite-to-Sprite Collision register.
* **Collision Logic**: A smart contract method `checkCollisions()` dynamically compares active sprite coordinate bounds using C64 24x21 bounding box rules:
  $$\text{Collision iff } |\Delta X| < 24 \text{ and } |\Delta Y| < 21$$

---

## 2. KoalaPainter (.koa) Multicolored Bitmap Decoder
Also located inside [graphicsSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/graphicsSystem.yul), this module decodes raw Commodore KoalaPainter format files (`.koa`, 10003 bytes):
* **Memory Map Parsing**:
  * **Bitmap Area** (8000 bytes): 40x25 character grid cells, 8 bytes per cell.
  * **Screen RAM** (1000 bytes): Holds primary and secondary cell colors (upper/lower nibbles).
  * **Color RAM** (1000 bytes): Holds tertiary cell colors.
  * **Background Color** (1 byte): The default fallback color.
* **Pixel Resolver**: The contract extracts color indexes dynamically based on target $(X, Y)$ screen coords.

---

## 3. KoalaPad Analog Touch Input Emulation
* Implemented in [graphicsSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/graphicsSystem.yul) and tested in [test_koala.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_koala.js).
* Provides an interface `updateKoalaPad(uint8 x, uint8 y, uint8 buttonState)` to simulate analog touch pad input, mapping coordinates into retro pointer subsystems.

---

## 4. B-Graph (bGraph) Plotting & Computational Geometry
Located inside [bGraph.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/bGraph.yul), this module offers statistical graphics processing:
* **Linear Regression**: Calculates slopes, intercepts, and $R^2$ values for sets of coordinates.
* **Bar Chart Coordinator**: Takes array inputs and computes scaled coordinate bounds (`X1, Y1, X2, Y2`) for plotting bars on a $320 \times 180$ display layout.
* **Decision Tree Classifier**: Computes options for classification tasks (such as the *Ahoy!* Issue 3 "What's My Job?" tree).
