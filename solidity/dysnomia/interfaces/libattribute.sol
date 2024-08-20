// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

interface LIBATTRIBUTE {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function addAttribute(string memory name) external;       
    function removeAttribute(string memory name) external;
    function Set(uint64 Soul, string memory name, string memory value) external;
    function Get(uint64 Soul, string memory name) external view returns (string memory);
    function Alias(uint64 Soul, address name, string memory value) external;
    function Alias(uint64 Soul, address name) external view returns (string memory _a);
}