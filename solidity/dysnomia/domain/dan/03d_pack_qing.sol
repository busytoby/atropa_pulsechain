// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./03_qing.sol";

contract PackQing is QING {
    string public constant PackSubType = "PackQing";

    // Dynamic array of packaged QING coordinate words
    uint256[] public PackedWords;

    constructor(uint256 Luo, address Integrative, address ChoAddress) QING(Luo, Integrative, ChoAddress) {
        /* Constructor inherits and runs initializations */
    }

    // Packages many QING coordinate words up together at this pack qing
    function PackQingWords(address[] memory qingAddresses) public onlyOwners {
        for (uint256 i = 0; i < qingAddresses.length; i++) {
            uint256 targetWaat = QING(qingAddresses[i]).Waat();
            PackedWords.push(targetWaat);
        }
    }

    // Returns the total number of packed coordinate words
    function GetPackedCount() public view returns (uint256) {
        return PackedWords.length;
    }
}
