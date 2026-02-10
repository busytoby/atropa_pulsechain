# TSFi Benchmarks

## 1. Math Library (`bench_tsfi_math`)
Benchmark of `tsfi_math` (AVX512 BigInt) and `lau_memory`.
- **Allocation:** ~247k cycles (alloc+free). High overhead due to wiring/audit.
- **Multiplication:** ~162k cycles (32k bits). Extremely fast AVX512 IFMA.
- **ModPow:** ~1.5M cycles (256 bits). Reasonable.

## 2. Thunk Latency (`bench_thunk_latency`)
Benchmark of "Zero Copy Switch" via `ThunkProxy`.
- **Direct Call:** 3 cycles.
- **Thunk Call:** 4 cycles.
- **Overhead:** 1 cycle.
- **Conclusion:** Context switching via R15-thunks is practically free.

## 3. Neurology System (`bench_neurology`)
Benchmark of "Synaptic Plasticity" (Dynamic ZMM Kernel Swapping).
- **Avg Latency:** 37 cycles per iteration (Locked dispatch).
- **Behavior:** Successfully demonstrated dynamic upgrading (Low -> High) and downgrading (High -> Low) based on `ZmmSynapse` feedback.
- **Switches:** 158 switches in 10k iterations.
- **Conclusion:** The feedback loop is extremely low-latency, enabling fine-grained adaptive compute.

## 4. TSFi ZMM VM MCP Server (`tsfi_mcp_server`)
Benchmark of the Wave512 Assembly Interpreter (MCP JSON-RPC) with Telemetry.
- **Latency:** ~20ms for 1,000,000 instructions.
- **Throughput:** ~50 Million Ops/s.
- **Features:** 
    -   `wave512.run`: Execute ASM.
    -   `wave512.inspect`: Read neural state.
    -   `wave512.scramble`: Trigger lattice scrambling.
    -   `wave512.attach`: Connect to `tsfi_cockpit` telemetry.
    -   ASM `WTELEM_MSG`/`WTELEM_VAL`/`WCHECK_COCKPIT`.

### 4b. ZMM VM Engine Direct (`tsfi_zmm_vm`)
Benchmark of the raw C API (Zero-Copy Lexer), bypassing JSON-RPC overhead.
- **Throughput:** ~124 Million Ops/s.
- **Latency:** ~8ns per instruction.

## 5. Wave512 Transcendence (`bench_svdag_transcendent`)
Benchmark of Quetta-scale SVDAG traversal simulation.
- **Throughput:** ~75 M-Traversals/s (Actual). ~380 M-Traversals/s (Logical).
- **Transcendence:** Verified (Processing sparse data at dense vector speeds).

## 6. Activation Transcendence (`bench_activation`)
Benchmark of Quetta-scale Activation Function (Softsign/Sigmoid approx).
- **Throughput:** ~23 Billion Quetta-Neurons/s.
- **Transcendence:** Verified (High parallelism via AVX-512).

## 7. Genetic Crossover (`tests/run_genetic_bench.sh`)
Benchmark of the Dysnomia evolution engine (Mu -> Thetan -> Theta-Eta).

### 7a. Basic Evolution (`bench_mu_thetan_theta`)
- **Throughput:** ~112k Mu->Theta Evolutions/s.
- **DNA Length:** 15-32 bytes.
- **ASan Overhead:** Significant.

### 7b. Deep Recursive Mixing (`bench_genetic_deep`)
- **Depth:** 5 levels (Fa -> SHA -> SHAO -> SHIO -> YI).
- **Throughput:** ~11.2k Deep-XO/s.
- **Complexity:** Measures full allocation/free cycle of the Dysnomia tree.

### 7c. In-Place Convergence (`bench_genetic_convergence_inplace`)
- **Optimization:** AVX-512 `vpavgb`, 4-way Unroll, Aligned Streaming Stores.
- **DNA Length:** 4096 bytes (Quetta-Scale).
- **Throughput:** **~1.41 Million InPlace-XO/s**.
- **Conclusion:** Achievement of high-frequency neurological evolution.

### 7d. Mutation Entropy & Cascades (`bench_genetic_entropy`, `bench_genetic_cascade`)
- **Entropy Rate:** ~105k XO/s with kernel resets.
- **Cascade Rate:** ~211k Stages/s (Recursive Thunking).
- **Conclusion:** Progeny waveforms successfully trigger secondary mutations via internal thunks with minimal latency.

## 8. Alpha Science Support (`tests/bench_alpha_science.c`)
Benchmarks for Google DeepMind-inspired scientific workloads.

### 8a. AlphaFold Prototype (Folding Latency)
- **Target:** Banach (Sequence) -> Hilbert (Structure).
- **Metric:** Folds/second.

### 8b. AlphaEvolve Prototype (Algorithmic Evolution)
- **Target:** ZMM Instruction Crossover -> VM Execution.
- **Metric:** Generations/second.

### 8c. AlphaProteo Prototype (De Novo Design)
- **Target:** Functional Requirement -> SVDAG Geometry.
- **Metric:** Designs/second.

## Build System
Benchmarks are compiled using `xgcc` (custom GCC toolchain) to verify compiler integration and intrinsic support.
Scripts: `tests/run_bench.sh`, `tests/run_thunk_bench.sh`, `tests/run_neuro_bench.sh`, `tests/run_bench_transcendent.sh`, `tests/run_bench_activation.sh`.
