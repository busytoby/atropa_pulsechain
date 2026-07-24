# Auncient CICS-ALU Coaxial SDK: Architectural & API Reference

This document provides comprehensive documentation for every data structure defined in the [auncient_sdk.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/inc/auncient_sdk.h) header. The SDK maps low-level ALU register execution to CICS transaction layers over Coaxial network loops.

---

## 1. `auncient_abi_packet_t`

### Struct Definition
```c
typedef struct {
    uint8_t alu_opcode;
    uint8_t status_flag;
    uint16_t payload_length;
    uint32_t payload_value;
    uint64_t timestamp_counter;
    uint32_t writer_id;
} auncient_abi_packet_t;
```

### Description
The formal binary Application Binary Interface (ABI) packet layout transmitted over Coaxial Unix sockets. It encapsulates register instructions and consensus payloads.

### Fields
* `alu_opcode`: The target operation code (e.g. `ALU_OP_WRITE_ABD` or `ALU_OP_READ_KERMIT`).
* `status_flag`: Flags execution status (e.g., success, rollback, or error conditions).
* `payload_length`: The size of the active payload in bytes.
* `payload_value`: The register payload (e.g. state value, frequency, or routing flags).
* `timestamp_counter`: Monotonically increasing counter for ABD sequence checks.
* `writer_id`: Unique identifier of the writing node used for tie-breaker resolution.

---

## 2. `sdk_timestamp_t`

### Struct Definition
```c
typedef struct {
    uint64_t counter;
    uint32_t writer_id;
} sdk_timestamp_t;
```

### Description
Represents a logical timestamp tuple used by the multi-writer ABD protocol. Provides total ordering of write operations across asynchronous networks.

### Fields
* `counter`: Monotonically increasing operation count.
* `writer_id`: The ID of the node that initiated the write, serving as a lexicographical tie-breaker.

---

## 3. `sdk_register_t`

### Struct Definition
```c
typedef struct {
    uint32_t value;
    sdk_timestamp_t ts;
} sdk_register_t;
```

### Description
A single register entry in the distributed database. Associates a stored data value with its logical ABD consensus timestamp.

### Fields
* `value`: The stored register configuration or state value.
* `ts`: The logical [sdk_timestamp_t](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/inc/auncient_sdk.h#L22-L25) associated with this update.

---

## 4. `sdk_coaxial_env_t`

### Struct Definition
```c
typedef struct {
    sdk_register_t registers[SDK_NUM_NODES];
    uint32_t weights[SDK_NUM_NODES];
    int socket_fds[2];
} sdk_coaxial_env_t;
```

### Description
Defines the physical layout of the coaxial network loop. Manages node weights and the Unix socket descriptors emulating the coaxial bus.

### Fields
* `registers`: An array of local [sdk_register_t](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/inc/auncient_sdk.h#L27-L30) states representing the distributed database.
* `weights`: An array of node weights used for Ackerman Weighted Quorum validations.
* `socket_fds`: Bidirectional Unix domain socket pair used to transmit and receive ABI frames.

---

## 5. `sdk_kermit_cache_t`

### Struct Definition
```c
typedef struct {
    uint32_t cached_value;
    sdk_timestamp_t cached_ts;
    bool is_warm;
} sdk_kermit_cache_t;
```

### Description
The local high-speed cache layer. Bypasses multi-phase network lookups to provide sub-microsecond register read latencies.

### Fields
* `cached_value`: The locally stored register value.
* `cached_ts`: The [sdk_timestamp_t](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/inc/auncient_sdk.h#L22-L25) associated with the cached entry.
* `is_warm`: Boolean flag indicating if the cache is active and valid.

---

## 6. `sdk_cics_context_t`

### Struct Definition
```c
typedef struct {
    sdk_coaxial_env_t *env;
    sdk_kermit_cache_t *cache;
    sdk_quorum_type_t quorum_type;
    uint32_t writer_id;
} sdk_cics_context_t;
```

### Description
The CICS Transaction Engine execution context. Orchestrates state lookup, cache status, and selected consensus parameters.

### Fields
* `env`: Pointer to the active [sdk_coaxial_env_t](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/inc/auncient_sdk.h#L32-L36) representing the coaxial network environment.
* `cache`: Pointer to the local [sdk_kermit_cache_t](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/inc/auncient_sdk.h#L38-L42).
* `quorum_type`: The active quorum system policy (Majority, Grid, or Weighted) enforced during execution.
* `writer_id`: The unique writer identifier assigned to this CICS transaction task.
