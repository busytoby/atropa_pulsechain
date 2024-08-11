// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "07_yau.sol";

contract YANG is DYSNOMIA {
    string public constant Type = "YANG";

    YAU private Upsilon;

    uint64 private Context;
    uint64 private Operator;
    address private Value;

    constructor(address YauAddress) DYSNOMIA(unicode"VM Yang", unicode"YANG", DYSNOMIA(YauAddress).Xiao.address, 8) MultiOwnable(msg.sender) {
        Upsilon = YAU(YauAddress);
        mintToCap();
    }
}