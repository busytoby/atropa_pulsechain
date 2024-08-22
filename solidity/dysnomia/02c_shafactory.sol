// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./02_sha.sol";

contract SHAFactory {
    constructor() {} 

    function New(string memory name, string memory symbol, address MathLib) public returns(SHA Eta) {
        Eta = new SHA(name, symbol, MathLib);
        Eta.addOwner(msg.sender);
        Eta.addOwner(tx.origin);
        Eta.renounceOwnership(address(this));
    }
}