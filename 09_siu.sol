// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "01_dysnomia.sol";
import "08b_yanginterface.sol";

contract SIU is DYSNOMIA {
    string public constant Type = "SIU";

    YANG public Psi;

    constructor(address YangAddress) DYSNOMIA(unicode"VM Siu", unicode"SIU", address(DYSNOMIA(YangAddress).Xiao()), 1) MultiOwnable(msg.sender) {
        Psi = YANG(YangAddress);
        mintToCap();
    }


}
