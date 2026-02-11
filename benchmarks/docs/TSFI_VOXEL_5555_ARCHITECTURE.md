# TSFi ARGB 5555: 5-Bit Voxel Basis

**Status:** ACTIVE / VERIFIED (2026-02-10)
**Format:** 32-bit Atom (20 bits Data, 12 bits Secret)
**Output:** 64-bit High-Precision Voxel (RGBA16)

## 1. Architectural Overview
The **ARGB 5555** basis is a high-efficiency categorical encoding for voxel state. It utilizes 5-bit quantization for each color channel and a dedicated 5-bit smoothing channel to define the physical "intensity" or "density" of the voxel.

## 2. Bit Layout (32-bit Word)
| Bits | Field | Description |
| :--- | :--- | :--- |
| 0-4 | **Red** | 5-bit Red Channel (0-31) |
| 5-9 | **Green** | 5-bit Green Channel (0-31) |
| 10-14 | **Blue** | 5-bit Blue Channel (0-31) |
| 15-19 | **Smoothing** | 5-bit Alpha/Smoothing Factor (0-31) |
| 20-31 | **Secret** | 12-bit Physical Secret (Axiom 5 Handshake) |

## 3. Smoothing Logic (Physical Realization)
The "Smoothing" channel (S5) is not a simple transparency mask. It acts as a **Categorical Multiplier** during expansion:
1.  **Normalization:** $C_{norm} = C_5 / 31.0$ and $S_{norm} = S_5 / 31.0$.
2.  **Modulation:** $C_{final} = C_{norm} 	imes S_{norm} 	imes 	ext{Global\_Intensity}$.
3.  **Expansion:** The resulting float32 values are packed into **RGBA16** (Half-Float) to produce a 64-bit color space capable of representing HDR gradients and sub-categorical transitions.

## 4. Symmetry (CPU / GPU)
-   **GPU:** Handled by `shaders/tsfi_voxel_5555.comp` using `packHalf2x16`.
-   **CPU:** Handled by `thunks/tsfi_voxel_5555.c` using **AVX-512** intrinsics (`vcvtps2ph`).
-   **Integrity:** Both paths are numerically bijective, ensuring the simulation's visual state remains consistent regardless of the execution unit.

## 5. Vision Integration
The `tsfi_vision` subsystem can analyze the **Smoothing** channel directly to detect surface boundaries or high-density voxel clusters without expanding to the full 64-bit space, minimizing analysis latency.
