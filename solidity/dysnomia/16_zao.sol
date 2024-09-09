// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./15_qin.sol";
//import "./interfaces/15b_qininterface.sol";

contract ZAO is DYSNOMIA {
    string public constant Type = "ZAO";

    QING public VoidQing;
    mapping(address => uint64) private _qin;
    mapping(address => address) private _qinpresences;
    mapping(address => address) private _players;

    constructor(address VoidQingAddress) DYSNOMIA("Dysnomia Zao", "ZAO", address(DYSNOMIA(VoidQingAddress).Xiao())) {
        VoidQing = QING(VoidQingAddress);
        addOwner(tx.origin);
    }

    function SetQinEntropy(address who, uint64 value) public onlyOwners {
        _qin[_players[who]] = value;
    }

    function GetQinEntropy(address who) public view returns (uint64) {
        return _qin[_players[who]];
    }

    function GetPlayerQin(address who) public view onlyOwners returns (address) {
        return _players[who];
    }

    error AlreadyRegistered(address who);
    function Register(address UserToken) public returns (QIN Beta) {
        if(_players[tx.origin] != address(0x0)) revert AlreadyRegistered(tx.origin);
        Beta = new QIN(address(this), UserToken);
        Beta.addOwner(address(this));
        addOwner(address(Beta));
        _qin[address(Beta)] = Beta.Entropy();
        _qinpresences[address(VoidQing)] = address(Beta);
        _players[tx.origin] = address(Beta);
    }

    error Unregistered(address who);
    function Move(string memory toQing) public onlyOwners {
        if(_players[tx.origin] == address(0x0)) revert Unregistered(tx.origin);
        QIN Player = QIN(_players[tx.origin]);
        uint64 From = Player.Location().Saat(1);
        address To = VoidQing.VAT().Alias(From, toQing);
        _qinpresences[To] = tx.origin; 
        Player.Move(To);
    }
}