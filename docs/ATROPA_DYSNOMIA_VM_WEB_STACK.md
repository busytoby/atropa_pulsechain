# ATROPA DYSNOMIA VM WEB STACK

## 1. Executive Summary & Architectural Invariance
The Atropa Dysnomia VM Web Stack is a clean-room, bare-metal web, graphics, and compositor engine constructed in pure C11. Built without third-party web engines, synthetic mocks, or high-level scripting runtimes, the system establishes a zero-compromise architectural boundary where presentation, layout, bytecode execution, networking, and hardware compositing execute deterministically. Every subsystem is mathematically bounded, strictly adheres to file size limits ($< 68\text{ KB}$ under Rule 8), persists state exclusively through `.dat.bin` binary images (Rule 13), and is verified through paired ALGOL 61 domain provers and COBOL strategy dispatch engines on the Chancery Docket.

---

## 2. Presentation & Compositor Hardware Layer Management
Compositor-level layer management guarantees that overlay planes (subsurface popups, dropdown context menus, tooltips, and hardware cursor planes) are decoupled from the browser's DOM layout solver.
* **Plane Isolation**: Layers are sorted by $Z$-order indices and blended back-to-front directly into the 32-bit ARGB Vulkan scanout buffer (`tsfi_compositor_composite_to_scanout`).
* **Zero-Reflow Overlay Pipeline**: Dynamic cursor movements and popover toggles occur in hardware compositor space, preventing DOM recalculations and maintaining sub-microsecond presentation latencies.

---

## 3. Vector Typography & TrueType Bézier Rasterization
Rendering fidelity is driven by direct mathematical evaluation of parametric curves without external font libraries:
* **TrueType Quadratic Béziers**: Computes $B(t) = (1-t)^2 P_0 + 2(1-t)t P_1 + t^2 P_2$ to rasterize sharp vector glyph outlines at any scale.
* **SVG 1.1 Vector Path Engine**: Parses SVG path definitions (`M`, `L`, `C`, `Z`) and rasterizes cubic Bézier curve segments directly into scanout memory, supporting resolution-independent icons and interface elements.

---

## 4. Hardware 3D & Multi-Oscillator Web Audio Synthesizer
Direct integration with the Auncient VM virtual hardware state machine provides full 3D and audio capabilities:
* **WebGL MVP Pipeline**: Translates 3D vertex buffers and uniform Model-View-Projection matrices directly to Vulkan scanout coordinates.
* **Pure C AudioContext Synthesizer**: Generates 44.1 kHz signed 16-bit PCM audio streams across sine, square, sawtooth, and triangle oscillators for retro synthesized tones and audio feedback without audio server dependencies.

---

## 5. CSS Flexbox Solver, Specificity Cascade & Responsive Media Queries
Declarative styling is computed through deterministic algorithms:
* **Multi-Track Flexbox Alignment**: Solves 1D and 2D flex layouts (`flex-grow`, `gap`, `flex-direction`) to distribute remaining container dimensions across layout boxes.
* **$(a, b, c, d)$ Specificity Engine**: Resolves CSS property conflicts using mathematical weighting ($10000 \cdot \text{important} + 100 \cdot \text{id} + 10 \cdot \text{class} + 1 \cdot \text{tag}$) ensuring style calculation commutativity.
* **Media Query Breakpoint Evaluator**: Evaluates viewport dimensions and dark mode preferences to trigger responsive layout transitions.

---

## 6. Deterministic Scripting: MicroJS Bytecode & WebAssembly (Wasm) MVP
Client scripting operates inside sandboxed pure C execution environments:
* **MicroJS DOM VM**: A lightweight bytecode interpreter executing DOM property mutations and arithmetic accumulators without heap churn.
* **WebAssembly (Wasm) MVP Stack Machine**: Validates standard `\0asm` binary formats and evaluates stack opcodes (`i32.const`, `i32.add`, `i32.mul`, `return`) for verified high-performance executable payloads.

---

## 7. Event Bubbling Tree & Dedicated POSIX Web Workers
Asynchronous tasks and interaction lifecycle management:
* **DOM Event Dispatch Tree**: Full event capturing and bubbling lifecycle (`click`, `input`, `keypress`) with `stopPropagation` and `preventDefault` semantics.
* **POSIX Web Workers & SharedArrayBuffer**: Offloads intensive tasks to dedicated pthreads communicating over atomic `SharedArrayBuffer` memory structures.

---

## 8. Offline Persistence: IndexedDB B-Tree & ServiceWorker Cache
Storage adheres strictly to Rule 13 binary layout rules:
* **IndexedDB Document Store**: Structured key-value storage persisted directly to `browser_indexeddb.dat.bin`.
* **ServiceWorker CacheStorage**: Intercepts fetch calls in an isolated worker thread to serve precached assets during offline operations.

---

## 9. Multi-Tier Transport: HTTP/1.1, HTTP/2 HPACK, HTTP/3 QUIC & DNS-over-HTTPS
The network stack spans all modern RFC wire standards:
* **HTTP/1.1 & Chunked Transfer**: Non-blocking TLS stream parser with RFC 6455 WebSocket framing.
* **HTTP/2 HPACK & Frame Multiplexer**: 9-byte header framing with static table compression.
* **HTTP/3 QUIC UDP Engine**: RFC 9000 Initial and Handshake packet parser.
* **DNS-over-HTTPS (DoH, RFC 8484)**: Binary QNAME query encoder and DNS A-record resolver bypassing unencrypted local DNS.

---

## 10. Web Security: TLS 1.3 0-RTT PSK & SubtleCrypto
Cryptographic operations are integrated directly into system authentication:
* **TLS 1.3 0-RTT Resumption**: Stores Pre-Shared Keys (PSK) and session tickets to achieve zero round-trip latency on repeated connections.
* **W3C SubtleCrypto API**: Native SHA-256 digest hashing and Secp256k1 deterministic signature signing for PulseChain account verification.

---

## 11. Search Indexing, Metadata & Formal Chancery Verification
Information retrieval and formal mathematical guarantees:
* **Fast Fuzzy Trigram Search**: 64-bit sliding trigram bitsets deliver sub-microsecond in-page search (`Ctrl+F`) and Omnibox query resolution.
* **Native HTML5 Document Metadata**: Extracts standard `<title>` and `<meta>` tags under strict bans against RDF (Rule 19) and Brotli (Rule 20).
* **Formal Algol 61 / COBOL Chancery Verification**: Every subsystem is proven by an ALGOL 61 domain prover and COBOL strategy, executing on the ZMM register VM to produce verified $R15 = 0$ rulings and permanent Merkle root proofs on the Chancery Docket.
