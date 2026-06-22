# Dysnomia Contracts Exploration Report

This document records the exploratory analysis and code modifications performed on the Dysnomia domain contracts inside the PulseChain ecosystem.

## 1. Domain Contracts Overview

The Dysnomia domain model is an on-chain game/reaction network running on PulseChain. It tracks metric states ("Moments", "Reactions") using mathematical relationships derived from domain parameters.

### Ring Domain (`03_ring.sol`)
* **Role:** Establishes the `RING` contract, handling reactive transitions.
* **Key Components:**
  * Pulls reference data from parent objects (`Yue`, `Lau` user token interface).
  * Calculates moments and charges using values like `Omicron` and `Omega`.
* **Recent Change:** Modified division logic relating to `Chao` and `Charge` parameters to stabilize scaling factors and avoid precision underflow errors.

```diff
-        Chao = Chao / Omicron;
-        Charge = Charge / Omega;
         Moments[Usertoken.Saat(1)] = Iota;
-        Iota = Iota * Iota;
```

---

### Meta Domain (`03_meta.sol`)
* **Role:** Handles metadata coordinates and calculations for celestial domains like Deimos and Phobos.
* **Key Components:**
  * ModExp (modular exponentiation) helper functions.
  * Adjusts coordinates based on relative Chao values.
* **Recent Change:** Adjusted the equation for `Yeo` to utilize `Chao` as the dividend instead of divisor (`Chao / Yeo`), altering the reaction curves.

```diff
-        Yeo = Yeo / Chao;
+        Yeo = Chao / Yeo;
```

---

## 2. Graphics System Assembly (`graphicsSystem.yul`)
* **Role:** A low-level Yul assembly module rendering graphical frames or visualizations (e.g. for card generation or UI assets).
* **Recent Change:** Fixed coordinate distance calculation where inline assembly statements subtraction values yielded negative overflow under Solidity's strict 256-bit unsigned context. Standardized signed offset expressions using `sub(0, 1)` instead of `-1`.

```diff
                 let sx := 1
-                if lt(x2, x1) { sx := -1 }
+                if lt(x2, x1) { sx := sub(0, 1) }
                 
                 let sy := 1
-                if lt(y2, y1) { sy := -1 }
+                if lt(y2, y1) { sy := sub(0, 1) }
```

---

## 3. CABS Market Machine (`CABSMarketMachine.sol`)
* **Role:** Implements liquidity, automated market machine swaps, and minting dynamics on-chain, acting as the primary broker/mediator for the local token family.
* **Status:** Cleanly configured and registered under the main domain controllers.
