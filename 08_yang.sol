// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "07_yau.sol";

contract YANG is SH {
    string public constant Type = "YANG";
    address public constant MathLib = 0x48087234ed7bc38e04347176b08B860E643806e2;

    YAU private Upsilon;

    uint64 private Context;
    uint64 private Operator;
    address private Value;

    constructor(address YauAddress) ERC20(unicode"VM Yang", unicode"YANG") SH(MathLib, 8) MultiOwnable(msg.sender) {
        Upsilon = YAU(YauAddress);
        address[] memory YauAddresses = Upsilon.KnownAddresses();
        for(uint256 i = 0; i < YauAddresses.length; i++) {
            RegisterAddress(YauAddresses[i]);
        }
        Upsilon.RegisterAddress(address(this));
        RegisterAddress(address(Upsilon));
    }
}