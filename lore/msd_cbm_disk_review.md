# Micro Systems Development (MSD) vs. Commodore Business Machines (CBM)

This review covers the historical context of **Micro Systems Development (MSD)**, its relationship to **Commodore Business Machines (CBM)**, and how their legacy floppy disk drive commands are emulated inside our virtual [DiskSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/diskSystem.yul) contract.

---

## 1. Historical Background

* **Commodore Business Machines (CBM)**: Dominant maker of the Commodore 64, VIC-20, and PET. Their standard disk drives (like the CBM 1541) were notoriously slow due to serial bus protocols and a custom MOS 6502 processor executing CBM DOS on-board the drive itself.
* **Micro Systems Development (MSD)**: Founded in 1982 in Dallas by former CBM engineers. MSD designed high-performance third-party alternatives.
* **The MSD SD-1 and SD-2**: Released in 1983, these were the first major third-party drives for the C-64. 
  * The **SD-1** (single drive) and **SD-2** (dual drive) featured both the Commodore serial (IEC) bus and the faster IEEE-488 parallel bus.
  * The dual-drive **SD-2** became legendary because it could perform ultra-fast sector-to-sector disk copies directly from Drive 0 to Drive 1 without sending data back and forth to the computer.

---

## 2. Command Channel Protocols

Both CBM and MSD drives communicated using a dedicated **Command Channel** (typically channel 15). Programmers opened channel 15 to send instructions as string command payloads:

| Command | Name | CBM/MSD Behavior | `DiskSystem.yul` Emulation |
| :--- | :--- | :--- | :--- |
| `R0` | Read | Read file contents sequentially. | Reads private sandboxed or global BBS files via Keccak256 hash routing. |
| `W0` | Write | Write file contents sequentially. | Appends (for global BBS) or overwrites (for private sandboxed partitions) via EVM storage. |
| `D0` | Duplicate | Copy files/disks (`D0:dest=src`). | Direct copy of storage slots from source base slots to destination base slots. |
| `U1` | Block Read | Read raw sector data (`U1 track sector`). | Extracts 256 bytes from a mapped track/sector storage key. |
| `U2` | Block Write | Write raw sector data (`U2 track sector data`). | Overwrites 256 bytes directly into a mapped track/sector storage key. |

---

## 3. Implementation in Yul Assembly

Our virtual [DiskSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/diskSystem.yul) contract replicates these CBM and MSD commands using EVM storage:

### A. Utility Block Operations (`U1` and `U2`)
The contract parses tracks and sectors using `parseTrackSector()`:
```yul
// Block Read Command (U1)
if eq(cmdType, 0x5531) { // ASCII "U1"
    let track, sector := parseTrackSector(68, cmdLength)
    ...
    let sectorKey := keccak256(0x00, 64)
    let sectorSlot := keccak256(0x00, 64)
    ...
    return(0x00, 320) // Returns the 256-byte sector data
}
```

### B. High-Speed Duplication (`D0`)
Mimicking the MSD SD-2's internal copying function, the `D0` parser copies file contents slot-by-slot on-chain:
```yul
// Duplicate Command (D0:destfile=srcfile)
if eq(cmdType, 0x4430) { // ASCII "D0"
    ...
    let srcLen := sload(srcLengthSlot)
    sstore(destLengthSlot, srcLen)
    let numSlots := div(add(srcLen, 31), 32)
    for { let i := 0 } lt(i, numSlots) { i := add(i, 1) } {
        sstore(add(destBaseSlot, i), sload(add(srcBaseSlot, i)))
    }
}
```

---

## 4. Modern Takeaways

By emulating CBM DOS and MSD commands in Yul:
1. **Developer Nostalgia**: Legacy 6502 codebases running in the virtual CPU (such as `cpu6502_v18` or `FolkloreCPU`) can use standard BASIC/Assembly disk command syntax.
2. **On-Chain Sandbox**: Users get an isolated partition to store and copy virtual disk blocks (`U1`/`U2`) and files (`W0`/`R0`/`D0`) securely using their address namespace.
