// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./14_qi.sol";
import "./interfaces/15b_qininterface.sol";
import "./16_zao.sol";

contract ZAI is DYSNOMIA {
    string public constant Type = "ZAI";

    ZAO public Zao;

    constructor(address ZaoAddress) DYSNOMIA("Dysnomia Zai", "ZAI", address(DYSNOMIA(ZaoAddress).Xiao())) {
        Zao = ZAO(ZaoAddress);
        Zao.addOwner(address(this));
        VAI().addOwner(address(this));
        addOwner(tx.origin);
    }

    function AddLibraryOwner(string memory what) public onlyOwners {
        _addLibraryOwner(Zao.VoidQing().Void(), what);
    }

    function VAI() public view returns (LIBCONJURE) {
        return LIBCONJURE(Zao.VoidQing().Void().GetLibraryAddress("conjure"));
    }

    error UnknownQin(address token);
    function Conjure() public onlyOwners returns (QI Conjuring, bytes memory Geng, uint64 Entropy) {
        if(Zao.GetQinEntropy(tx.origin) == 0) revert UnknownQin(msg.sender);
        QININTERFACE Conjuror = QININTERFACE(Zao.GetPlayerQin(tx.origin));
        (Entropy, Geng) = Conjuror.SUN().Encapsulate(Conjuror.On(), Zao.GetQinEntropy(tx.origin), Conjuror.Alt().Saat(0), Conjuror.Alt().Saat(1), Conjuror.Alt().Saat(2));
        Zao.SetQinEntropy(tx.origin, Entropy);
        Conjuring = new QI(Geng, address(Conjuror.Location()));
        Conjuring.addOwner(tx.origin);
        Conjuring.addOwner(address(VAI()));
        VAI().addOwner(address(Conjuring));
        Conjuring.Rename("Mysterious Qi", "QI");
    }
}