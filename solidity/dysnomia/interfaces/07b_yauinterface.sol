// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./06b_zhouinterface.sol";

interface YAU {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external returns(string memory);
    function Tau() external returns(ZHOU);
    function Theta() external returns(Bao memory);
    function Monopole() external returns(uint64);
    function Shio() external returns(SHIO);
    function React() external returns(Bao memory);
}