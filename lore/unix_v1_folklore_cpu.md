# Implementing Unix Version 1 on the Folklore CPU

This document analyzes the architectural feasibility of running **Unix Version 1** (originally written for the 16-bit PDP-11/20 in 1971) on the EVM-native **Folklore CPU** written in Yul.

---

## 1. The Challenge: Hardware Emulation vs. Native Porting

There are two primary approaches to running Unix v1 on the Folklore CPU:

### Approach A: PDP-11 Emulation in Yul (Virtualization)
In this model, the Folklore CPU is programmed to interpret the PDP-11 instruction set (opcodes like `MOV`, `ADD`, `BR`, `JSR`).
*   **Feasibility**: Possible, but highly inefficient. Emulating a 16-bit CPU on top of the 256-bit EVM introduces massive gas overhead due to opcode decoding loops and simulated register state storage loading/storing.
*   **Memory constraints**: Simulating the PDP-11's 64KB RAM space would require heavy usage of EVM memory or storage mappings, making execution slow and expensive.

### Approach B: Native Yul Unix v1 Port (Recommended)
Rather than emulating PDP-11 hardware, we port the core **architectural concepts** of Unix v1 directly into native Yul functions. This leverages the Folklore CPU's native 256-bit capabilities.

| Unix v1 Concept | PDP-11/20 Implementation (1971) | Folklore CPU Yul Implementation |
| :--- | :--- | :--- |
| **Word Size** | 16-bit | Native 256-bit (`u256`) |
| **Memory** | 24KB Core RAM | Dynamic EVM Memory (`mload`/`mstore`) |
| **Storage (Disk)** | RK03 Decpack disk drives | EVM Storage Slots (`sload`/`sstore`) |
| **System Calls** | trap instructions (`sys open`, `sys read`) | Yul `switch` selector dispatcher |
| **Processes** | Process Table in Core Memory | Process array mapped to storage slots |

---

## 2. Implementing Unix v1 System Calls in Yul

We can map Unix v1's core system calls to native Yul functions. Below is a conceptual prototype of the file descriptor and I/O pipeline:

```solidity
switch selector()

// sys_open(pathHash, mode) -> fd
case 0x15a0c0f4 {
    let pathHash := calldataload(4)
    let mode := calldataload(36)
    
    // 1. Locate i-node in storage
    let inodeSlot := findInode(pathHash)
    if iszero(inodeSlot) {
        // Create new inode if write mode
        inodeSlot := createInode(pathHash)
    }
    
    // 2. Allocate File Descriptor in process state
    let fd := allocateFD(inodeSlot, mode)
    mstore(0x00, fd)
    return(0x00, 0x20)
}

// sys_write(fd, bufferOffset, byteCount) -> writtenCount
case 0xb826f0aa {
    let fd := calldataload(4)
    let memOffset := calldataload(36)
    let count := calldataload(68)
    
    let inodeSlot := getInodeFromFD(fd)
    if iszero(inodeSlot) { revert(0, 0) }
    
    // Write data bytes directly to EVM storage slots assigned to this inode
    for { let i := 0 } lt(i, count) { i := add(i, 32) } {
        let val := mload(add(memOffset, i))
        sstore(add(inodeSlot, div(i, 32)), val)
    }
    
    mstore(0x00, count)
    return(0x00, 0x20)
}
```

---

## 3. Advantages of the Native Folklore Unix v1 Port

1.  **Unlimited File System Storage**: Instead of being constrained by the PDP-11's 2MB RK03 disk limits, the Unix v1 filesystem maps directly to the EVM's $2^{256}$ storage space, enabling deep, persistent file hierachies.
2.  **Native 256-bit Security**: System-level file permissions and user authentication can be linked directly to **ECDSA signatures** and Ethereum addresses, making a cryptographically secure multi-user operating system.
3.  **High-Performance Pipes**: Inter-process pipes (`|`) can be implemented as ultra-fast transient memory buffers in EVM RAM rather than slow physical disk swaps.
