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
        _qin[who] = value;
    }

    function GetQinEntropy(address who) public view returns (uint64) {
        return _qin[who];
    }

    error AlreadyRegistered(address who);
    function Register(address UserToken) public returns (QIN Beta) {
        if(_players[tx.origin] != address(0x0)) revert AlreadyRegistered(tx.origin);
        Beta = new QIN(address(VoidQing), UserToken);
        Beta.addOwner(address(this));
        addOwner(address(Beta));
        _qin[address(Beta)] = Beta.Entropy();
        _qinpresences[address(VoidQing)] = address(Beta);
        _players[tx.origin] = address(Beta);
    }

    error Unregistered(address who);
    function Move(address toQing) public onlyOwners {
        if(_players[tx.origin] == address(0x0)) revert Unregistered(tx.origin);
        _qinpresences[toQing] = tx.origin; 
    }
}