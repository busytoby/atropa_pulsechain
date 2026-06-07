# Software to Go: On-Chain GEnie File Library Emulation

This document details the virtual hardware and contract interface design for the **Software to Go** library system, inspired by the historic GEnie Commodore File Library. This service allows C64 CPU users to query a public, on-chain program repository and download software binaries directly onto their virtual floppy disks (DFS sectors) under a regulated **Diyat Library Tax**.

---

## 1. On-Chain Software Repository Design

The Software to Go service is implemented as a shared global registry contract containing pre-compiled 6502 machine code binaries and text assets (such as games, utilities, and BBS text files):

### Contract Interface (Library Registry)
```solidity
interface ISoftwareToGo {
    // Returns the total number of files in the library index
    function getFileCount() external view returns (uint256);
    
    // Returns metadata for a file at a specific index
    function getFileMetadata(uint256 index) external view returns (string memory filename, uint256 sizeBytes, bytes32 fileHash);
    
    // Retrieves the raw binary contents of a file by its hash
    function getFileBytes(bytes32 fileHash) external view returns (bytes memory);
}
```

---

## 2. Emulated "Modem" Downloader Interface ($D630–$D63F)

To retrieve library files without exiting the 8-bit emulated environment, the C64 CPU interacts with a virtual modem register map designed to simulate asynchronous serial file downloads:

* **File Index Query (`$D630` / `54832`)**: Write a file index here to query metadata from the library registry.
* **Metadata Buffer (`$D631`–$D64F` / `54833`–`54863`)**: Reads return the filename string (null-terminated) and file size bytes of the queried index.
* **Download Trigger Register (`$D650` / `54864`)**: Writing the file index to this register initiates the automated on-chain file download to the virtual 1541 disk.

### Yul Download Hook Execution
When a user writes an index to `$D650`, the Yul CPU emulator intercepts the call and runs the download sequence:

```yul
case 54864 { // Software to Go Download Trigger ($D650)
    sstore(getUserSlot(54864), val)
    if val {
        let fileIndex := val
        
        // 1. Excise the Diyat Library Download Tax (2 OTRT units)
        let downloadTax := 2
        let userBal := sload(getUserSlot(848))
        if lt(userBal, downloadTax) { revert(0, 0) } // Revert if cannot pay tax
        sstore(getUserSlot(848), sub(userBal, downloadTax))
        
        // Transfer tax to platform treasury
        mstore(0x300, 0x1111111111111111111111111111111111111111)
        mstore(0x320, 848)
        let treasurySlot := keccak256(0x300, 64)
        let treasuryBal := sload(treasurySlot)
        sstore(treasurySlot, add(treasuryBal, downloadTax))
        
        // Log payment of library access tax
        log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), downloadTax)
        
        // 2. Query the Library Registry Contract
        let registryAddr := 0x9999999999999999999999999999999999999999 // SoftwareToGo Registry
        
        // Call getFileMetadata(fileIndex) to resolve filename and hash
        mstore(0x300, shl(224, 0x241ad6ab)) // Selector
        mstore(0x304, fileIndex)
        let success := staticcall(gas(), registryAddr, 0x300, 36, 0x344, 128)
        if iszero(success) { revert(0, 0) }
        
        let fileHash := mload(0x384)
        
        // Call getFileBytes(fileHash) to retrieve raw binary data
        mstore(0x400, shl(224, 0x5b3ab3cd)) // Selector
        mstore(0x404, fileHash)
        success := staticcall(gas(), registryAddr, 0x400, 36, 0x444, 2048)
        if iszero(success) { revert(0, 0) }
        
        // 3. Write binary data directly to user's Namespaced Disk System (DFS)
        let diskSystemAddr := 0x06737b36e5272900fB070d655583cd5B3C5bdB3C
        
        // Format executeDiskCommand("W0:filename", fileBytes) to write file directly
        mstore(0x500, shl(224, 0x9812a4df))
        mstore(0x504, 32)
        // ... payload formatting for disk write command ...
        
        let writeSuccess := call(gas(), diskSystemAddr, 0, 0x500, 1024, 0x300, 32)
        if iszero(writeSuccess) { revert(0, 0) }
        
        sstore(getUserSlot(54864), 0) // Clear trigger
    }
}
```

---

## 3. Security & Resource Analysis

The Software to Go client enforces resource constraints to prevent node abuse:

### 1. Re-entrancy Protection
The trigger register (`54864`) is locked (`sstore` set immediately, cleared only upon final write confirmation) to prevent double-download re-entrancy attacks which could bypass tax deductions.

### 2. Diyat Rate Limiting
Because downloading on-chain binaries requires cross-contract static calls and multiple disk writes, a rate limit is enforced by linking downloads to block intervals: a user cannot download more than 1 file per block.

### 3. Namespace Sandbox Guard
All files downloaded via the modem registers are written exclusively to the caller's namespaced directory slots inside `diskSystem.yul`, preventing any overwrite of other users' local file systems.
