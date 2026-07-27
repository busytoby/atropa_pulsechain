# XplOS Binary Executable Format Specification (.dat.bin)

This document defines the custom binary executable file format layout for compiling and running XPL program binaries directly on the ZMM virtual machine under **Auncient** and **Rule 13** (.dat.bin extension only) constraints.

---

## 1. Binary Layout Layout

An XplOS executable is structured into five sequential sections:

```
+---------------------------------------------------+
| Magic & Version (8 Bytes)                         |
+---------------------------------------------------+
| Section Segment Header Table                      |
+---------------------------------------------------+
| VM Register Initializer Block (Base, Signal, etc) |
+---------------------------------------------------+
| Relocation & Dynamic Contract Linkage Table       |
+---------------------------------------------------+
| Raw Code / Instruction Stream (Yul / EVM)         |
+---------------------------------------------------+
```

---

## 2. File Header Fields

| Byte Offset | Size (Bytes) | Field Name | Description |
|---|---|---|---|
| `0x00 - 0x03` | 4 | Magic Bytes | Constant: `0x58504C30` (`XPL0`) |
| `0x04 - 0x05` | 2 | Target VM Version | Target VM execution model (e.g., `0x0002` for ZMM v2). |
| `0x06 - 0x07` | 2 | Segment Count | Number of descriptors in the segment table. |

---

## 3. Section Segment Table

Following the file header, each segment descriptor maps a target storage namespace on ZMM:

```c
typedef struct {
    uint8_t segment_type;     // 1: Code, 2: Data, 3: ACL Header, 4: Stack Config
    uint32_t stack_offset;    // Starting pointer offset on ZMM stack
    uint32_t virtual_size;    // Allocated size in memory
    uint32_t raw_data_offset; // File offset containing source data bytes
} XplosSegmentDescriptor;
```

---

## 4. VM Register Initializer Block

Sets initial virtual hardware state properties mapped to Lissajous parameters before jumping to the code entry point:

| Field Name | Size (Bytes) | Role | Math function / Manifestation |
|---|---|---|---|
| `Base` | 32 | Reference register | Defines initial temporal phase twist ($\phi_w$). |
| `Signal` | 32 | Carrier register | Governs orbital velocity scaling and line bloom. |
| `Secret` | 32 | Transform register | Modulates the coordinate shear distortion matrix. |
| `MotzkinPrime` | 32 | Modular Divisor | Standard prime field value `953467954114363`. |

---

## 5. Relocation & Dynamic Contract Linkage

Instead of physical memory addresses, the relocation table registers dynamic contract slot bindings.
* **Format:** Records contain dynamic address signatures (`dynamic_<address>`).
* **Mounting Loop:** During load-time, the ZMM dynamic linker walks the relocation table, queries the `Delegate` registry mapping, recovers the Soul/wallet authorizations, and patches the instruction stream jumps to bind target dependencies dynamically.
