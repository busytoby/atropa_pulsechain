// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./03_qing.sol";

contract QINGFactory {
    CHOINTERFACE public Cho;
    mapping(address Asset => address Qing) private _qings;
    mapping(uint256 Waat => QING) private _waats;
    mapping(address Asset => bool Forbidden) private _forbidden;

    constructor(address ChoAddress) {
        Cho = CHOINTERFACE(ChoAddress);
        Cho.addOwner(address(this));
        Cho.Void().addOwner(address(this));
    } 

    function hasOwner(address _contract) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes("owner(address)")));
        bytes memory data = abi.encodeWithSelector(selector, address(0x0));
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
    }

    function has(address _contract, string memory what) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes(what)));
        bytes memory data = abi.encodeWithSelector(selector);
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
    }

    function Forbidden(address Asset) public view returns (bool) {
        return _forbidden[Asset];
    }

    error NotOwner(address what, address who);
    function Forbid(address Token, bool Disallow) public {
        if(has(Token, "owner()") && DYSNOMIA(Token).owner() == msg.sender) _forbidden[Token] = Disallow;
        else if(DYSNOMIA(Token).owner(msg.sender)) _forbidden[Token] = Disallow;
        else revert NotOwner(Token, msg.sender);
    }

    function GetQing(uint256 Waat) public view returns (QING) {
        return _waats[Waat];
    }

    error QingAlreadyExists(address Token, address Qing);
    error ForbiddenToken(address which);
    event NewQing(address Qing, address Integrative, uint256 Waat);
    function New(address Integrative) public returns(QING Mu) {
        uint256 Luo;
        if(_qings[Integrative] != address(0x0)) revert QingAlreadyExists(Integrative, _qings[Integrative]);
        if(_forbidden[Integrative]) revert ForbiddenToken(Integrative);
        DYSNOMIA Asset = DYSNOMIA(Integrative);
        Luo = Cho.Luo();
        Mu = new QING(Luo, Integrative, address(Cho));
        if(has(Integrative, "owner()")) Mu.addOwner(Asset.owner());
        if(has(Integrative, "name()") && has(Integrative, "symbol()")) Mu.Rename(string.concat(Asset.name(), " QING"), string.concat("q", Asset.symbol()));
        Cho.addOwner(address(Mu));
        Mu.addOwner(address(Cho));
        Mu.renounceOwnership(address(this));
        _qings[Integrative] = address(Mu);
        _waats[Luo] = Mu;
        emit NewQing(address(Mu), Integrative, Luo);
    }
}