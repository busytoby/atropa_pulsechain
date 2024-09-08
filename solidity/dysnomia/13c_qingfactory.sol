// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./13_qing.sol";
import "./interfaces/libyai.sol";

contract QINGFactory {
    LIBYAI public Yai;
    mapping(address => address) private _qings;

    constructor(address YAIAddress) {
        Yai = LIBYAI(YAIAddress);
        Yai.addOwner(address(this));
        Yai.Void().addOwner(address(this));
    } 

    error QingAlreadyExists(address Token, address Qing);
    error ForbiddenToken(address which);
    function New(address Integrative) public returns(QING Mu) {
        if(_qings[Integrative] != address(0x0)) revert QingAlreadyExists(Integrative, _qings[Integrative]);
        if(Yai.IsForbidden(Integrative)) revert ForbiddenToken(Integrative);
        Mu = new QING(Integrative, address(Yai.Void()));
        Yai.addOwner(address(Mu));
        Mu.renounceOwnership(address(this));
        _qings[Integrative] = address(Mu);

    }
}