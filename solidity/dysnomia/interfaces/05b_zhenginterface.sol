// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./04b_yiinterface.sol";

interface ZHENG {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function AssertAccess(Bao memory Beta) external;
    function Eta() external returns(YI);
    function GetRodByIdx(uint64 _theta) external returns(Bao memory);
    function InstallRod(uint64 Theta, Bao memory Beta, uint64 DIRAC) external returns(Bao memory);
    function InstallCone(uint64 Theta, Bao memory Beta, uint64 DIRAC) external returns(Bao memory);
    function Iodize(SHIO Shio) external;
}