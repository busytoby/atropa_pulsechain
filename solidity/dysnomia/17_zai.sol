// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./14_qi.sol";
import "./interfaces/15b_qininterface.sol";
import "./16_zao.sol";

contract ZAI is DYSNOMIA {
    string public constant Type = "ZAI";

    ZAO public Zao;
    mapping(address => uint64) private _qi;

    constructor(address ZaoAddress) DYSNOMIA("Dysnomia Zai", "ZAI", address(DYSNOMIA(ZaoAddress).Xiao())) {
        Zao = ZAO(ZaoAddress);
        Zao.addOwner(address(this));
        addOwner(tx.origin);
    }

    error UnknownQin(address token);
    function Conjure() public onlyOwners returns (QI Conjuring, bytes memory Geng, uint64 Entropy) {
        if(Zao.GetQinEntropy(msg.sender) == 0) revert UnknownQin(msg.sender);
        QININTERFACE Conjuror = QININTERFACE(msg.sender);
        (Entropy, Geng) = Conjuror.SUN().Encapsulate(Conjuror.On(), Zao.GetQinEntropy(msg.sender), Conjuror.Alt().Saat(0), Conjuror.Alt().Saat(1), Conjuror.Alt().Saat(2));
        Conjuring = new QI(address(this), Geng, address(Conjuror.Location()));
        Zao.SetQinEntropy(msg.sender, Entropy);
    }

}