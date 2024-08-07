// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./04b_yiinterface.sol";

interface ZHENG {
    function addOwner(address newOwner) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Eta() external returns(YI);
    function Alpha(Bao memory Beta, uint64 Iota) external returns(Bao memory);
    function GetRodByIdx(uint64 _theta) external view returns(Bao memory);
    function GetConeByIdx(uint64 _theta) external view returns(Bao memory);
    function InstallRod(uint64 Theta, Bao memory Beta) external returns(Bao memory);
    function Ionize(SHIO Shio) external;
}