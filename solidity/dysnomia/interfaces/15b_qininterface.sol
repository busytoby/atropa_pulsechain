// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./14b_qiinterface.sol";

interface QININTERFACE  {
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
    function Location() external view returns (QING);
    function Alt() external view returns (LAU);
    function On() external view returns (Bao memory);
    function qp() external view returns (uint256);
    function Entropy() external view returns (uint64);
    function AddMarketRate(address _a, uint256 _r) external;
    function SelectAlt(address UserToken) external;
    function SUN() external view returns (LIBEncrypt);
    function Conjure(uint64 Gamma) external returns (QIINTERFACE Conjuring, bytes memory Geng);
    function GetInventoryCount(uint256 class) external view returns (uint256);
    function GetDefense(uint256 n) external view returns (address);
    function GetDecoration(uint256 n) external view returns (address);
    function GetOffense(uint256 n) external view returns (address);
    function Throw() external view returns (uint64);
    function Save(uint64 pQi) external view returns (uint64);
    function Equip(address qi) external;
    function Remove(address qi) external;
}