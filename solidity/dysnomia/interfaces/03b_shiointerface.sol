// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./02b_shainterface.sol";

struct Shao {
    SHA Rod;
    SHA Cone;

    uint64 Barn;
}

interface SHIO {
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
    function Type() external view returns(string memory);
    function Rho() external view returns(Shao memory);
    function Manifold() external returns(uint64);
    function Monopole() external returns(uint64);
    function Rod() external view returns(SHA);
    function Cone() external view returns(SHA);
    function Generate(uint64 Xi, uint64 Alpha, uint64 Beta) external;
    function Isomerize() external;
    function Isolate() external;
    function Magnetize() external returns(uint64);
    function React(uint64 Pi) external returns(uint64, uint64);
    function Log(uint64 Soul, uint64 Aura, string memory LogLine) external;
}