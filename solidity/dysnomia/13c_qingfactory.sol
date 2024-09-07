// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./13_qing.sol";

contract QINGFactory {
    VOID public Void;

    constructor(address VOIDAddress) {
        Void = VOID(VOIDAddress);
        Void.addOwner(address(this));
    } 

    function New(address Integrative) public returns(QING Mu) {
        Mu = new QING(Integrative, address(Void));
        addOwner(address(Mu));
        Mu.renounceOwnership(address(this));
    }
}