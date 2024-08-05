// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "05b_zhenginterface.sol";

interface ZHOU {
    function addOwner(address newOwner) external;
    function RegisterAddress(address _a) external;
    function KnownAddresses() external view returns(address[] memory);
    function mintToCap() external;
    function Type() external returns(string memory);
    function Tau() external returns(ZHENG);
    function Alpha(string memory Name, string memory Symbol) external returns (SHA);
    function Beta(uint64 _theta, uint64 Iota) external returns (Bao memory);
    function Pi(uint64 _theta, uint64 _a) external returns(Bao memory);
}