// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./05b_zhenginterface.sol";

interface ZHOU {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Upsilon() external returns(ZHENG);
    function Monopole() external returns(uint64);
    function Xi() external returns(uint64);
    function Alpha(string memory Name, string memory Symbol) external returns (SHA);
    function Qin(uint64 Iota) external returns (uint64);
    function Pi() external returns(Bao memory);
}