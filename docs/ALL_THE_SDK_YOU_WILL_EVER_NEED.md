# All The SDK You Will Ever Need

Welcome to the definitive guide for the **Auncient VPP Software Development Kit (SDK)**. This documentation outlines the interfaces, APIs, and design paradigms required to build, compile, and run high-efficiency, PLL-compliant vector packet routing containers.

---

## 1. Core Architecture Overview

The Auncient VPP SDK operates on a **decoupled graph node topology** designed for sub-microsecond vector transaction switching.

```
       +---------------------------------------------+
       |           Physical Coaxial Cable            |
       +---------------------------------------------+
                              |
                              v
       +---------------------------------------------+
       |   First Input Node (PLL Compliance Gate)    |
       +---------------------------------------------+
                              | (Vector Ingestion)
                              v
       +---------------------------------------------+
       |    SIMD Verification & Security Checker     |
       +---------------------------------------------+
          | (Cache Hit)                       | (Cache Miss)
          | Bypass Check                      v
          |                          TWS Compiler Gate
          |                                   |
          v                                   v
       +---------------------------------------------+
       |   Transpiler Node (Yul / BTC Script Gen)    |
       +---------------------------------------------+
                              |
                              v
       +---------------------------------------------+
       |      Forwarding Node (Multi-Port Route)     |
       +---------------------------------------------+
```

---

## 2. Ingestion & The First Node

Every compiled `.bin` executable runs its own independent **First Node (Input Node)**. This node directly polls the physical coaxial loopback interfaces, enforces security clearances, and performs real-time telemetry logging.

### API Reference: First Node Ingestion

```c
// Polls the socket and ingests incoming packets into a vector batch
int vpp_node_ingest(vpp_context_t *vpp, aligned_packet_t *vector_batch);
```

### Ingestion Telemetry Events
At the moment of ingestion, the first node records events directly to an immutable `.dat.bin` binary file:
1. **INGEST**: Records sequence number, arrival cycle, and security metadata.
2. **ESCALATION**: Triggered if a packet matches `PREC_FLASH` precedence.
3. **SECURITY_ALERT**: Triggered if a packet violates Bell-LaPadula clearance parameters (e.g. `TOP_SECRET` payload matching a `SECRET` reader).

---

## 3. SIMD Checksums & Data Alignment

To maximize CPU cache locality, VPP processes transactions in 32-byte memory aligned blocks.

```c
// 32-byte aligned block to match AVX register width
typedef union {
    uint8_t  u8[32];
    uint64_t u64[4];
} __attribute__((aligned(32))) avx_block_t;
```

### SIMD Checksum Node
The verification node performs parallel XOR calculations across the packet payload blocks:
```c
uint64_t compute_simd_checksum(const aligned_packet_t *pkt) {
    uint64_t accumulator = 0;
    for (int b = 0; b < PAYLOAD_BLOCKS; b++) {
        accumulator ^= pkt->payload[b].u64[0];
        accumulator ^= pkt->payload[b].u64[1];
        accumulator ^= pkt->payload[b].u64[2];
        accumulator ^= pkt->payload[b].u64[3];
    }
    return accumulator;
}
```

---

## 4. Static Verification Cache

To achieve sub-nanosecond processing times, VPP implements a verification cache shortcut. If a transaction payload hash is already present in the cache, the slow TWS compiler clearance checks are bypassed entirely.

```c
// Performs lookup and returns true on cache hit
bool lookup_cache(const vpp_verify_cache_t *cache, const uint8_t *hash, bool *out_pure);

// Inserts a newly verified transaction signature
void insert_cache(vpp_verify_cache_t *cache, const uint8_t *hash, bool is_pure);
```

---

## 5. VPPD Self-Contained Daemon

The **`vppd.bin`** daemon is self-contained with its own execution stack, ABI, and CICS terminal. It runs in the background to orchestrate ports, manage references, and monitor latency.

### The CICS Diagnostic Terminal
When launched in interactive mode via `./vppd.bin --interactive`, you gain direct access to the CICS Terminal:
```
========================================
--- CICS TERMINAL DIAGNOSTIC SCREEN ---
  VPPD STATUS    : ACTIVE (Self-Contained)
  PACKETS SWITCHED: 154092
  PLL PHASE ERROR: 0.0014
  SYSTEM CYCLES  : 84091244
  STACK DEPTH    : 3
========================================
vppd> _
```

### Supported Interactive Commands
* `ping`: Queries daemon responsiveness.
* `status`: Retrieves active system cycles and PLL phase errors.
* `route <N>`: Simulates routing transaction sequence `N` on the virtual stack.
* `exit`: Safely terminates the diagnostic session.
