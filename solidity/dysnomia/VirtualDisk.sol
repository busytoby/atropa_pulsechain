// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

import "./lib/multiownable.sol";

contract VirtualDisk is MultiOwnable {
    // Sparse mapping of Sector LBA => 256-byte data payload
    mapping(uint256 lba => bytes32[8] data) private _sectors;

    // Track the total number of blocks allocated
    uint256 public blockCount;

    // Events to track sector writes for indexers / telemetry
    event SectorUpdated(uint256 indexed lba, bytes32[8] data);

    constructor() MultiOwnable(msg.sender) {}

    /**
     * @notice Read a 256-byte sector payload by its LBA.
     */
    function readSector(uint256 lba) external view returns (bytes32[8] memory sectorData) {
        return _sectors[lba];
    }

    /**
     * @notice Write a 256-byte sector payload. Only callable by the controller or disk owner.
     */
    function writeSector(uint256 lba, bytes32[8] calldata sectorData) external onlyOwners {
        _sectors[lba] = sectorData;
        
        if (lba >= blockCount) {
            blockCount = lba + 1;
        }

        emit SectorUpdated(lba, sectorData);
    }
}
