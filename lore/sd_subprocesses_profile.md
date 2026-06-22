# Stable Diffusion Sub-Processes Profile Report

We profiled the three core internal stages of the Stable Diffusion inference pipeline: **CLIP Text Encoding**, **UNet Denoising**, and **VAE Decoding**.

The profiling script is located at [profile_sd_subprocesses.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/profile_sd_subprocesses.py).

---

## 1. Latency Breakdown by Sub-Process

### Stage 1: CLIP Text Encoding vs. Prompt Length
We tested how prompt length (in characters) affects the CLIP Transformer translation speed:
- **Short Prompt (15 chars):** **19.0 ms**
- **Medium Prompt (74 chars):** **19.0 ms**
- **Long Prompt (240 chars):** **20.0 ms**

*CLIP text encoding is extremely fast and remains constant (~19-20ms) because it always pads prompts to the standard CLIP budget of 77 tokens and executes in a single forward pass.*

### Stage 2: UNet Sampling Denoising vs. Step Count
We tested how step count affects the main sampling loops:
- **4 steps:** **0.680s** (0.170s per step)
- **8 steps:** **1.370s** (0.171s per step)
- **15 steps:** **2.570s** (0.171s per step)

*UNet sampling scales strictly linearly on the Vulkan compute pipeline at exactly **0.171 seconds per step**.*

### Stage 3: VAE Decoding (Latent to Pixel)
We measured the final decoding step translating the latent space grid to the output RGB image:
- **VAE Decode Duration:** **0.250 seconds** (constant across all step counts)

*VAE decoding is a one-time single-pass compute shader invocation executed after denoising finishes, meaning its latency is independent of prompt length or step count.*

---

## 2. Dynamic Scaling Rules
Based on these baseline metrics, the generation time (excluding weight loading) can be modeled as:
$$\text{Generation Time (sec)} = 0.020s \text{ (CLIP)} + (0.171s \times \text{Steps}) + 0.250s \text{ (VAE)}$$

| Steps | Expected Generation Time | Quality Tier | Target Output |
| :--- | :--- | :--- | :--- |
| **4 Steps** | **0.954s** | Draft | Fast Render UI preview |
| **8 Steps** | **1.638s** | Medium | Balanced preview |
| **15 Steps** | **2.835s** | High | Standard Storybook template |
| **20 Steps** | **3.690s** | Masterpiece | Background deep render |
