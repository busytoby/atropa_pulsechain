// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "04b_yiinterface.sol";

interface ZHENG {
    function addOwner(address newOwner) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Eta() external returns(YI);
    function Clear(Bao memory Beta, uint64 Iota) external returns(uint64);
    function Sign(Bao memory Beta, uint64 DIRAC1) external returns(Bao memory);
    function GetRodByIdx(uint64 _theta) external returns(Bao memory);
    function InstallRod(uint64 Theta, Bao memory Beta, uint64 DIRAC) external returns(Bao memory);
    function Iodize(SHIO Shio) external;
}