// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/fa.sol";

interface SHA {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns (string memory);
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