# Pixar USD Custom Schemas & Code Generation Tutorial

This document summarizes the complete implementation of custom USD schemas, code generation pipelines, Hydra delegates, and their integration with the Memory Augmented Neural Network (MANN) memory controller.

## 1. Custom Schemas & Code Generation (`usd_gen_schema`)

The schema compiler parses custom Pixar USD schema definitions (`.usda`) and generates high-performance Flat C structures and memory-mapped initializers.

### Schema Blueprint (`tests/schema.usda`)
* **AuncientCactusSchema**: Governs spatial density parameters.
* **AuncientTextureAPI / AuncientPhysicsAPI**: Manages physical softness, dampening, and texture names.
* **AuncientMANNAPI**: Declares parameters for differentiable neural memory routing (write, read, and allocation gates).

### Generated Output (`tests/auncient_cactus_schema.h`)
* Defines `usd_auncient_mann_api_t` and `usd_init_auncient_mann_api`.
* Generates thread-safe inline getters and setters using the `usd_mann_api` namespace prefix.

---

## 2. Hydra Render Delegate Simulator

The delegate simulates GPU resource allocation under the USD Hydra composition engine:
* Responds to scene index synchronizations (adding and removing prims).
* Directly allocates texture and vertex buffers on the emulated virtual GPU memory space.
* Tested under Test 58 in the unified test suite.

---

## 3. MANN Differentiable Memory Controller

The memory controller maps WinchesterMQ virtual hardware registers to a Memory Augmented Neural Network (MANN) memory matrix.

### Mathematical Equations

#### Content Addressing (NTM Similarity)
Weights are computed using cosine similarity and normalized via softmax:
$$w_t^c(i) = \frac{\exp(\beta_t \cdot \cos(k_t, M_t[i]))}{\sum_{j} \exp(\beta_t \cdot \cos(k_t, M_t[j]))}$$

#### Allocation Addressing (DNC Least-Used Vector)
Allocation weights route information to the least active memory slots:
$$a_t(i) = \frac{\exp(-u_t[i])}{\sum_{j} \exp(-u_t[j])}$$

#### Blended Gating Updates
Write weights blend allocation and content vectors, modulated by the allocation gate:
$$w_t^w = g_t^a a_t + (1 - g_t^a) w_t^c$$
$$M_t[i] \leftarrow M_t[i] \cdot (1 - g_t^w w_t^w(i) e_t) + g_t^w w_t^w(i) a_t$$
