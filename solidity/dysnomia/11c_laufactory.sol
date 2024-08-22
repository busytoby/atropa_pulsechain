// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./11_lau.sol";

contract LAUFactory {
    VOID public Void;

    constructor(address VOIDAddress) {
        Void = VOID(VOIDAddress);
        Void.addOwner(address(this));
    } 

    function New(string memory name, string memory symbol) public returns(LAU Mu) {
        Mu = new LAU(name, symbol, address(Void));
        Mu.addOwner(msg.sender);
        Mu.addOwner(tx.origin);
        Mu.renounceOwnership(address(this));
    }
}