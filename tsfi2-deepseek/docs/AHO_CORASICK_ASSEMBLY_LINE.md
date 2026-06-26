# Auncient Compositor Caching Assembly Line: Aho-Corasick Guidelines and Guards
Date: June 26, 2026
Status: [STEADY / CODE-VERIFIED]

The Auncient compositor caching system uses a thread-safe Aho-Corasick pattern matching trie to index and match pre-render/post-render pipeline signatures, replacing linear search loops and avoiding heavy state recompilation overhead.

## 1. Architectural Design (The Assembly Line)
During compositor rendering, shader configurations and pipeline parameters are serialized into formatted pipeline signatures. The caching assembly line intercepts these signatures:
1. **Registration**: Newly compiled shader stages and resource bindings are registered via `tsfi_compositor_cache_register` which updates the Aho-Corasick pattern trie.
2. **Lookup**: Active render threads invoke `tsfi_compositor_cache_lookup` before dispatching draw calls. If a matching signature exists, the cached pre-render metadata is re-bound instantly, bypassing setup passes.

## 2. Implementation Guidelines
To preserve sub-microsecond latency, all developers and automated tools must adhere to the following guidelines:

* **Strict Thread Isolation**: Trie reads and updates must be synchronized via the interop cache mutex (`cache_mutex`). Mutex lock duration must be minimized.
* **Zero Console Prints in Hot Path**: Logging or printing (e.g. `printf`) inside `tsfi_compositor_cache_lookup` is strictly banned. Any I/O operations create thread contention, which degrades average latency from ~250 ns to over 20,000 ns.
* **Buffer Safety and Hardening**: Format strings generating pipeline keys must use hardened buffers of at least 256 bytes to prevent compiler `-Wformat-overflow` warnings under strict `-Werror` flags.
* **No Mocking**: Follow the Interop design pattern (`libmozilla_interop.c`). Stub/mock layers are banned; all caching logic must operate on the actual multithreaded trie structure.
* **Auncient Orthography**: All code comments, error messages, and documentation must preserve the **Auncient** spelling constraint (e.g., Auncient Compositor).

## 3. Performance Guards
System performance is protected by automated gates in the profiler suite:

1. **Latency Threshold**: Caching lookups must execute in under **1000.0 nanoseconds** per lookup.
2. **Regression Check**: The `aho_corasick_compositor` check in `benchmarks/profiler_suite/performance_guard.py` enforces:
   ```python
   "aho_corasick_compositor": {
       "lookup_latency_ns": {"max": 1000.0, "desc": "Aho-Corasick Compositor Latency"}
   }
   ```
3. **Execution Protocol**: Run `make profile` to verify that recent caching logic meets latency and throughput metrics. Any check failure will reject the build pipeline.
