// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/17b_zaiinterface.sol";

contract TSUAN is DYSNOMIA {
    string public constant Type = "TSUAN";

    constructor(address LibConjureAddress) DYSNOMIA("Dysnomia Tsuan", "TSUAN", address(DYSNOMIA(LibConjureAddress).Xiao())) {
    }

}