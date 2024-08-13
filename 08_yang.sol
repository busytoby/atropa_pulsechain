// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "01_dysnomia.sol";
import "07b_yauinterface.sol";

struct Tai {
    Bao Bang;
    uint64 Lai;
    Bao Le;
}

contract YANG is DYSNOMIA {
    string public constant Type = "YANG";

    YAU public Mu;
    Tai public Rho;

    uint64 private Context;
    uint64 private Operator;
    address private Value;

    constructor(address YauAddress) DYSNOMIA(unicode"VM Yang", unicode"YANG", DYSNOMIA(YauAddress).Xiao.address, 8) MultiOwnable(msg.sender) {
        Mu = YAU(YauAddress);
        (Rho.Bang, Rho.Lai) = Mu.Phi();
        Rho.Le = Mu.Tau().Pi();
        mintToCap();
    }
}