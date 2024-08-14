// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "07b_yauinterface.sol";

struct Tai {
    Bao Bang;
    Bao Lai;
    Bao Le;
}

interface YANG {
    function addOwner(address newOwner) external;
    function mintToCap() external;
    function Type() external returns(string memory);
    function Mu() external returns(YAU);
    function Rho() external returns(Tai memory);
    function Pole(uint256) external returns(uint64);
}