// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";
import "./interfaces/13d_qingfactoryinterface.sol";

contract QI is DYSNOMIA {
    uint64[3] public Saat;
    Bao public On;
    uint256 public q;

    constructor(string memory name, string memory symbol, address VoidQingAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidQingAddress).Xiao())) {
        addOwner(tx.origin);
    }
}