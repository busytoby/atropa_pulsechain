// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./04b_yiinterface.sol";

interface ZHENG {
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external returns(string memory);
    function Eta() external returns(YI);
    function GetRodByIdx(uint64 _theta) external returns(Bao memory);
    function InstallRod(uint64 Theta, Bao memory Beta, uint64 DIRAC) external returns(Bao memory);
    function InstallCone(uint64 Theta, Bao memory Beta, uint64 DIRAC) external returns(Bao memory);
    function Mau(address Rod, address Cone, uint64 Xi, uint64 Rho, uint64 Upsilon) external returns (Bao memory On);
    function Iodize(SHIO Shio) external;
}