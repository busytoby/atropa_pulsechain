// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/18b_tsuaninterface.sol";

contract TSO is DYSNOMIA {
    string public constant Type = "TSO";

    TSUAN public Tsuan;
    mapping(string => address) private _libraries;
    mapping(address => string[]) private _knownImbuements;
    mapping(address => mapping(string => uint64[3])) private _imbuemets;

    constructor(address TsuanAddress) DYSNOMIA("Dysnomia Tsuan", "TSUAN", address(DYSNOMIA(TsuanAddress).Xiao())) {
        Tsuan = TSUAN(TsuanAddress);
        Tsuan.addOwner(address(this));
        acceptLibraryUpdate("conjure");
        DYSNOMIA(_libraries["conjure"]).addOwner(address(this));
    }

    function acceptLibraryUpdate(string memory lib) public onlyOwners {
        if(_libraries[lib] != address(0x0)) renounceOwnership(_libraries[lib]);
        _libraries[lib] = Tsuan.Zao().VoidQing().Void().GetLibraryAddress(lib);
        if(_libraries[lib] != address(0x0)) addOwner(_libraries[lib]);
    }

    function KnownImbuementCount(address QiAddress) public view returns (uint256) {
        return _knownImbuements[QiAddress].length;
    }

    function GetImbuement(address QiAddress, string memory Adjective) public view returns (uint64[3] memory) {
        return _imbuemets[QiAddress][Adjective];
    }

/*
    error NotTokenOwner(address Token, address originRequestor);
    function Train(address QiAddress, string memory IntendedNoun) public {
        uint64[3] memory Saat;
        uint64 Upsilon;
        bytes memory Geng;
        QIINTERFACE Qi = QIINTERFACE(QiAddress);
        if(!Qi.owner(tx.origin)) revert NotTokenOwner(QiAddress, tx.origin);

        QININTERFACE Conjuror = QININTERFACE(Zao.GetPlayerQin(tx.origin));
        (Upsilon, Geng) = Conjuror.SUN().Encapsulate(Conjuror.Alt().On(), Conjuror.Entropy(), Conjuror.Alt().Saat(0), Conjuror.Alt().Saat(1), Conjuror.Alt().Saat(2));
        Zao.SetQinEntropy(tx.origin, Upsilon);
        Saat = Conjuror.SUN().Saat(Geng);
        Upsilon = VAI().Enchant(QiAddress, Saat[0], Saat[1], Saat[2], IntendedNoun);
        if(_enchantments[QiAddress][IntendedNoun][1] == 0) _knownEnchantments[QiAddress].push(IntendedNoun);
        _enchantments[QiAddress][IntendedNoun][0] += Saat[0];
        _enchantments[QiAddress][IntendedNoun][1] += Upsilon;
        _enchantments[QiAddress][IntendedNoun][2] += Saat[2];
    }
*/
}