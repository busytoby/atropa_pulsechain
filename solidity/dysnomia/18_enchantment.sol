// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/17b_zaiinterface.sol";

contract ENCHANTMENT is DYSNOMIA {
    string public constant Type = "ENCHANTMENT";

    constructor(address LibConjureAddress) DYSNOMIA("Dysnomia Enchantment", "ENCHANTMENT", address(DYSNOMIA(LibConjureAddress).Xiao())) {
    }

}