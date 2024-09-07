// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./13_qing.sol";
import "./interfaces/libyai.sol";

contract QINGFactory {
    LIBYAI public Yai;

    constructor(address YAIAddress) {
        Yai = LIBYAI(YAIAddress);
        Yai.addOwner(address(this));
    } 

    error ForbiddenToken(address which);
    function New(address Integrative) public returns(QING Mu) {
        if(Yai.IsForbidden(Integrative)) revert ForbiddenToken(Integrative);
        Mu = new QING(Integrative, address(Yai.Void()));
        Yai.addOwner(address(Mu));
        Mu.renounceOwnership(address(this));
    }
}