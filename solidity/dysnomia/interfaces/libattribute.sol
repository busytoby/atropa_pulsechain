// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/bao.sol";

interface LIBATTRIBUTE {
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
    function addAttribute(string memory name) external;       
    function removeAttribute(string memory name) external;
    function Set(uint64 Soul, string memory name, string memory value) external;
    function Get(uint64 Soul, string memory name) external view returns (string memory);
    function Alias(uint64 Soul, address name, string memory value) external;
    function Alias(uint64 Soul, address name) external view returns (string memory _a);
    function Alias(uint64 Soul, Bao memory entity, string memory value) external;
    function Alias(uint64 Soul, Bao memory entity) external view returns (string memory _a);
}