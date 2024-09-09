// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./14_qi.sol";
import "./15_qin.sol";
import "./interfaces/15b_qininterface.sol";

contract ZAO is DYSNOMIA {
    string public constant Type = "ZAO";

    QING public VoidQing;
    mapping(address => uint64) private _qi;
    mapping(address => uint64) private _qin;

    constructor(address VoidQingAddress) DYSNOMIA("Dysnomia Zao", "ZAO", address(DYSNOMIA(VoidQingAddress).Xiao())) {
        VoidQing = QING(VoidQingAddress);
        addOwner(tx.origin);
    }

    error UnknownQin(address token);
    function Conjure(uint64 Gamma) public onlyOwners returns (QI Conjuring, bytes memory Geng) {
        if(_qin[msg.sender] == 0) revert UnknownQin(msg.sender);
        QININTERFACE Conjuror = QININTERFACE(msg.sender);
        (_qin[msg.sender], Geng) = Conjuror.SUN().Encapsulate(Conjuror.On(), _qin[msg.sender] ^ Gamma, Conjuror.Alt().Saat(0), Conjuror.Alt().Saat(1), Conjuror.Alt().Saat(2));
        Conjuring = new QI(address(this), Geng, address(Conjuror.Location()));
    }

}