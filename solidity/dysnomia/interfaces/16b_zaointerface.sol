// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./15b_qininterface.sol";

interface ZAOINTERFACE {
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
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external view returns (string memory);
    function VoidQing() external view returns (QING);
    function AddLibraryOwner(string memory what) external;
    function SetQinEntropy(address who, uint64 value) external;
    function GetQinEntropy(address who) external view returns (uint64);
    function GetPlayerQin(address who) external view returns (address);
    function Register(address UserToken) external returns (QININTERFACE Beta);
    function Move(string memory toQing) external;
}