// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/17b_zaiinterface.sol";
import "./include/enchantment.sol";

contract TSUAN is DYSNOMIA {
    string public constant Type = "TSUAN";

    ZAOINTERFACE public Zao;
    mapping(string => address) internal _libraries;
    mapping(address => string[]) internal _knownEnchantments;
    mapping(address => mapping(string => Enchantment)) _enchantments;

    constructor(address ZaoAddress) DYSNOMIA("Dysnomia Tsuan", "TSUAN", address(DYSNOMIA(ZaoAddress).Xiao())) {
        Zao = ZAOINTERFACE(ZaoAddress);
        Zao.addOwner(address(this));
        acceptLibraryUpdate("conjure");
        DYSNOMIA(_libraries["conjure"]).addOwner(address(this));
    }

    function acceptLibraryUpdate(string memory lib) public onlyOwners {
        if(_libraries[lib] != address(0x0)) renounceOwnership(_libraries[lib]);
        _libraries[lib] = Zao.VoidQing().Void().GetLibraryAddress(lib);
        if(_libraries[lib] != address(0x0)) addOwner(_libraries[lib]);
    }

    function KnownEnchantmentCount(address QiAddress) public view returns (uint256) {
        return _knownEnchantments[QiAddress].length;
    }

    function GetEnchantment(address QiAddress, string memory Noun) public view returns (Enchantment memory) {
        return _enchantments[QiAddress][Noun];
    }

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
        Saat = SUN().Saat(Geng);
        Upsilon = VAI().TrainFromOrigin(QiAddress, Saat[0], Saat[1], Saat[2], IntendedNoun);
        if(_enchantments[QiAddress][IntendedNoun].Saat[1] == 0) _knownEnchantments[QiAddress].push(IntendedNoun);
        _enchantments[QiAddress][IntendedNoun].Saat[0] += Saat[0];
        _enchantments[QiAddress][IntendedNoun].Saat[1] += Upsilon;
        _enchantments[QiAddress][IntendedNoun].Saat[2] += Saat[2];
    }

    function SUN() public view returns (LIBEncrypt) {
        return LIBEncrypt(_libraries["encrypt"]);
    }

    function VAI() public view returns (LIBCONJURE) {
        return LIBCONJURE(_libraries["conjure"]);
    }
}