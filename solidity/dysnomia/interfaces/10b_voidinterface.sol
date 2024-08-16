// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./09b_siuinterface.sol";

interface VOID {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Nu() external returns(SIU);
    function AddLibrary(string memory name, address _a) external;
    function Log(string memory LogLine) external;
    function Log(uint64 Sigma, string memory LogLine) external;
    function Log(address Sigma, string memory LogLine) external;
    function Log(string memory Xi, string memory LogLine) external;
    function SetAttribute(string memory name, string memory value) external;
    function GetAttribute(string memory name) external view returns (string memory);
    function Enter() external returns(uint64[3] memory Saat, Bao memory On);
    function Enter(string memory name, string memory symbol) external returns(uint64[3] memory Saat, Bao memory On);
}