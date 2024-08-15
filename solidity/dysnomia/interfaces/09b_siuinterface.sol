// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./08b_yanginterface.sol";

interface SIU {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Psi() external returns(YANG);
    function Miu(string memory name, string memory symbol) external returns(uint64[3] memory Saat, Bao memory On);
}