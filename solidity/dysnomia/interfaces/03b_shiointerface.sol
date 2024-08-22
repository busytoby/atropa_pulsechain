// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./02b_shainterface.sol";

struct Shao {
    SHA Rod;
    SHA Cone;

    uint64 Barn;
}

interface SHIO {
    function name() external returns (string memory);
    function symbol() external returns (string memory);
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external returns(string memory);
    function Rho() external returns(Shao memory);
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