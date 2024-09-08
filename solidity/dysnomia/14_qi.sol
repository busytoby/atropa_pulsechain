// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";
import "./interfaces/13d_qingfactoryinterface.sol";

contract QI is DYSNOMIA {
    Bao public On;
    uint256 public q;

    constructor(bytes memory Geng, address VoidQingAddress) DYSNOMIA("Qi", "QI", address(DYSNOMIA(VoidQingAddress).Xiao())) {
        addOwner(tx.origin);


    }
}