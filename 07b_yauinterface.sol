// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "06b_zhouinterface.sol";

interface YAU {
    function addOwner(address newOwner) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Tau() external returns(ZHOU);
    function Monopole() external returns(uint64);
    function Shio() external returns(SHIO);
    function Coulomb() external returns(uint64);
    function Phi() external returns(Bao memory, uint64);
}