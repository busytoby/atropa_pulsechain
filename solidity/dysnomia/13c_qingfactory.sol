// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./13_qing.sol";
import "./interfaces/libyai.sol";

contract QINGFactory {
    CHOINTERFACE public Cho;
    mapping(address => address) private _qings;

    constructor(address ChoAddress) {
        Cho = CHOINTERFACE(ChoAddress);
        Cho.addOwner(address(this));
        Cho.Void().addOwner(address(this));
        YAI().addOwner(address(this));
    } 

    function YAI() public view returns(LIBYAI) {
        return LIBYAI(Cho.Void().GetLibraryAddress("yai"));
    }

    error QingAlreadyExists(address Token, address Qing);
    error ForbiddenToken(address which);
    function New(address Integrative) public returns(QING Mu) {
        if(_qings[Integrative] != address(0x0)) revert QingAlreadyExists(Integrative, _qings[Integrative]);
        if(YAI().IsForbidden(Integrative)) revert ForbiddenToken(Integrative);
        Mu = new QING(Integrative, address(Cho));
        Cho.addOwner(address(Mu));
        YAI().addOwner(address(Mu));
        Mu.renounceOwnership(address(this));
        _qings[Integrative] = address(Mu);
    }
}