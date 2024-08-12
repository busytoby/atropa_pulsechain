// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "01_dysnomia.sol";
import "07b_yauinterface.sol";

contract YANG is DYSNOMIA {
    string public constant Type = "YANG";

    YAU private Upsilon;

    Bao public X;
    Bao public Y;

    uint64 public Eta;

    uint64 private Context;
    uint64 private Operator;
    address private Value;

    constructor(address YauAddress) DYSNOMIA(unicode"VM Yang", unicode"YANG", DYSNOMIA(YauAddress).Xiao.address, 8) MultiOwnable(msg.sender) {
        Upsilon = YAU(YauAddress);
        (X, Eta) = Upsilon.Phi();
        Y = Upsilon.Tau().Pi();
        mintToCap();
    }
}