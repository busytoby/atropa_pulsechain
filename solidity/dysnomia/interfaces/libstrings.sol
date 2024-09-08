// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

interface LIBSTRINGS {
    function Rename(string memory newName, string memory newSymbol) external;
    function GetMarketRate(address _a) external view returns(uint256);
    function Purchase(address _t, uint256 _a) external;
    function Redeem(address _t, uint256 _a) external;
    function name() external view returns (string memory);
    function symbol() external view returns (string memory);
    function decimals() external view returns (uint8);
    function totalSupply() external view returns (uint256);
    function balanceOf(address account) external view returns (uint256);
    function transfer(address to, uint256 value) external returns (bool);
    function allowance(address owner, address spender) external view returns (uint256);
    function approve(address spender, uint256 value) external returns (bool);
    function transferFrom(address from, address to, uint256 value) external returns (bool);
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external view returns(string memory);
    function has(address _contract, string memory what) external view returns (bool does);
    function CheckPalindrome(string memory S) external pure returns(bool);
    function CheckPalindrome(bytes memory S) external pure returns(bool);
    function Reverse(string memory S) external pure returns(string memory Reversed);
    function Reverse(bytes memory S) external pure returns(bytes memory Reversed);
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