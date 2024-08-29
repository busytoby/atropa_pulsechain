// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/fa.sol";

interface SHA {
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
    function Dynamo() external returns (uint64);
    function View() external returns (Fa memory);
    function Fuse(uint64 _rho, uint64 Upsilon, uint64 Ohm) external;
    function Avail(uint64 Xi) external;
    function Form(uint64 Chi) external;
    function Polarize() external;
    function Conjugate(uint64 Chi) external;
    function Conify(uint64 _Beta) external;
    function Saturate(uint64 _Beta, uint64 Epsilon, uint64 Theta) external;
    function Bond() external;
    function Adduct(uint64 _Phi) external returns(uint64);
    function React(uint64 Pi, uint64 Theta) external returns(uint64, uint64);
}