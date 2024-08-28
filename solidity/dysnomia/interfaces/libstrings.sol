// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

interface LIBSTRINGS {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function RandomAcronym(uint8 MaxLength) external returns(bytes memory Acronym);
    function CaseInsensitiveCompare(bytes1 A, bytes1 B) external pure returns (bool);
    function CheckAcronym(string memory _A, string memory _B) external pure returns(bool);
    function CheckAcronym(bytes memory _acronym, string memory _Beta) external pure returns(bool);
    function log10(uint256 value) external pure returns (uint256);
    function String(uint256 value) external pure returns (string memory buffer);
    function Hex(address account) external pure returns(string memory);
    function Hex(uint256 value) external pure returns(string memory);
    function Hex(bytes32 value) external pure returns(string memory);
    function Hex(bytes memory data) external pure returns(string memory);
}