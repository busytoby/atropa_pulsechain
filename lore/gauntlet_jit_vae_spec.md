# Specification: Gauntlet JIT & Vulkan VAE Graphics Model

This specification details the implementation of a native AVX-512 JIT compiler for the Gauntlet game logic, alongside GPU-accelerated Vulkan compute shaders for VAE-driven volumetric graphics.

---

## 1. Yul AVX-512 JIT Compiler Architecture (A)

To bypass the interpreter overhead of the `PmgSystem` and `cpu6502` Yul thunks during Gauntlet gameplay, we compile Yul AST blocks directly to native x86_64 machine code utilising AVX-512 vector registers.

### Register Mapping Spec
The 16 virtual registers of the ZMM VM are mapped directly to physical AVX-512 CPU registers:

| ZMM Virtual Register | x86_64 AVX-512 Hardware Register | Usage in Gauntlet Physics |
| :--- | :--- | :--- |
| `REG_0` (Accumulator) | `zmm0` | Player Position vector $[X, Y, Z, 1]$ |
| `REG_1` | `zmm1` | Velocity vector $[dX, dY, dZ, 0]$ |
| `REG_2` | `zmm2` | Active Projectile vector $[PX, PY, P_t, \text{Active}]$ |
| `REG_3` | `zmm3` | Threat Vector Array (Ghosts) |
| `REG_4` | `zmm4` | Spawner coordinates and spawn timers |
| `REG_5` .. `REG_15` | `zmm5` .. `zmm15` | Scratchpads / Temporary calculation steps |

### JIT Compilation Machine Code Generation
During initialization, Yul block structures are compiled to native machine instructions. The memory is allocated using `mmap` with `PROT_READ | PROT_WRITE` permissions, populated, and then marked as executable:

```c
// Mark JIT buffer executable
mprotect(jit_code_buffer, jit_size, PROT_READ | PROT_EXEC);
```

#### Fused Multiplying-Add (FMA) Loop JIT Spec
For calculating multi-ghost distance vectors simultaneously in Gauntlet:
```assembly
; Load player coordinates to zmm0
vmovups zmm0, [rdi + PlayerOffset]

; Load threat coordinate array to zmm3
vmovups zmm3, [rdi + GhostArrayOffset]

; Subtract coordinates
vsubps zmm5, zmm3, zmm0

; Square distances and sum (FMA)
vmulps zmm6, zmm5, zmm5
vfmadd213ps zmm6, zmm5, [rip + ScaleFactor]
```

---

## 2. Vulkan VAE Compute Shader Model (B)

By moving the VAE decoder pipeline to Vulkan Compute Shaders, the GPU processes convolutions in parallel, allowing us to model volumetric moss rendering and wilderness whispering fog directly in the Gauntlet arena.

```
+---------------------------------------------------------------+
|                       VULKAN GPU PIPELINE                     |
|                                                               |
|   [1024 NAND Fibers]                                          |
|           |                                                   |
|           v                                                   |
|   [Vulkan Compute Shader] (tsfi_vae_convolution_kernel.comp)  |
|           |                                                   |
|           +---> [Volumetric Moss SDF 3D Texture]              |
|           |                                                   |
|           +---> [Dynamic Wilderness Whispering Fog Density]   |
|                                                               |
+---------------------------------------------------------------+
```

### Volumetric Moss SDF Shader Spec
The VAE outputs a 3D density volume representing the wilderness environment. The fragment shader raymarches this density field to render moss growing procedurally over the game grid boundaries:

```glsl
#version 450
layout(binding = 0) uniform sampler3D vaeDensityVolume;
layout(location = 0) out vec4 outColor;

struct Ray {
    vec3 origin;
    vec3 direction;
};

float evaluateSDF(vec3 p) {
    // Sample VAE density field for organic moss shapes
    float vaeDensity = texture(vaeDensityVolume, p).r;
    float baseSphere = length(p) - 0.45;
    return mix(baseSphere, vaeDensity, 0.35); // Blend structural geometry with VAE noise
}

void main() {
    Ray ray = getCameraRay();
    float t = 0.0;
    bool hit = false;
    for (int i = 0; i < 64; i++) {
        vec3 p = ray.origin + ray.direction * t;
        float d = evaluateSDF(p);
        if (d < 0.001) {
            hit = true;
            break;
        }
        t += d;
    }
    
    if (hit) {
        // Organic moss coloring modulated by shading intensity
        vec3 normal = calculateNormal(ray.origin + ray.direction * t);
        float diff = max(dot(normal, vec3(0.577, 0.577, -0.577)), 0.0);
        vec3 mossColor = vec3(0.15, 0.45, 0.22) * (diff * 0.7 + 0.3);
        outColor = vec4(mossColor, 1.0);
    } else {
        outColor = vec4(0.01, 0.0, 0.02, 1.0); // Cyber-Black void
    }
}
```

### Whispering Wilderness Fog Particle Shader Spec
Using VAE `spectral_feedback` values to drive a GPU particle simulation:
*   Particles are spawned at the edges of the Gauntlet arena.
*   The velocity of each fog particle is modulated in the compute shader by a vector field derived from the Fresnel Cornu spiral:
    $$\vec{V} = \text{lti\_factor} \times \left[ \cos(ph + \text{Cornu}_c), \sin(ph + \text{Cornu}_s) \right]$$
*   As the player shoots projectiles or ghosts spawn, the VAE feedback register spikes, causing the whispering fog to swirl and disperse organically in high-intensity areas.

---

## 3. Dynamic Volumetric Wind Modulation (W-Component)
To simulate dynamic forest weather elements blowing across the organic structures, the VAE pipeline integrates wind velocity drift variables:
*   **Source**: Evaluates `external_params[2]` from the parent `NandTrapState`.
*   **Vector Channels**: Modulates **Channel 0** (Structural Mass) and **Channel 3** (Depth) of the generated latent space.
*   **Masking**: Uses AVX-512 vector mask operations (`_mm512_mask_add_ps` with mask `0x9999`) to dynamically offset target lanes relative to the wind drift factor.
