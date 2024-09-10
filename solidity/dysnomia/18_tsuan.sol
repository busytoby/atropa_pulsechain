// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/17b_zaiinterface.sol";
import "./include/enchantment.sol";

contract TSUAN is DYSNOMIA {
    string public constant Type = "TSUAN";

    VOID public Void;
    mapping(string => address) internal _libraries;
    mapping(address => Enchantment) internal _enchantments;

    constructor(address VoidAddress) DYSNOMIA("Dysnomia Tsuan", "TSUAN", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        acceptLibraryUpdate("conjure");
        DYSNOMIA(_libraries["conjure"]).addOwner(address(this));
    }

    function acceptLibraryUpdate(string memory lib) public onlyOwners {
        if(_libraries[lib] != address(0x0)) renounceOwnership(_libraries[lib]);
        _libraries[lib] = Void.GetLibraryAddress(lib);
        if(_libraries[lib] != address(0x0)) addOwner(_libraries[lib]);
    }

    function Level(address QiAddress) public view returns(uint8) {
        uint256 QiSaat1 = QIINTERFACE(QiAddress).Saat(1);
        for(uint8 i = 0; i < 10; i++)
            if(QiSaat1 < VAI().Levels(i)) return i;
        return 10;
    }

    function SUN() public view returns (LIBEncrypt) {
        return LIBEncrypt(_libraries["encrypt"]);
    }

    function VAI() public view returns (LIBCONJURE) {
        return LIBCONJURE(_libraries["conjure"]);
    }
}