// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./03_qing.sol";

contract PackQing is QING {
    string public constant PackSubType = "PackQing";

    // Parent QING contract reference
    address public ParentQing;

    // Dynamic array of packaged QING coordinate words
    uint256[] public PackedWords;

    constructor(uint256 Luo, address Integrative, address ChoAddress, address ParentQingAddress) QING(Luo, Integrative, ChoAddress) {
        ParentQing = ParentQingAddress;
    }

    // Packages many QING coordinate words up together at this pack qing relative to the parent qing
    function PackQingWords(address[] memory qingAddresses) public onlyOwners {
        uint256 parentWaat = QING(ParentQing).Waat();
        for (uint256 i = 0; i < qingAddresses.length; i++) {
            uint256 targetWaat = QING(qingAddresses[i]).Waat();
            // Establish the word relative to the parent qing using modular difference
            uint256 relativeWaat = (targetWaat + MotzkinPrime - (parentWaat % MotzkinPrime)) % MotzkinPrime;
            PackedWords.push(relativeWaat);
        }
    }

    // Returns the total number of packed coordinate words
    function GetPackedCount() public view returns (uint256) {
        return PackedWords.length;
    }
}
