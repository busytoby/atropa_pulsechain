// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/17b_zaiinterface.sol";

contract TSUAN is DYSNOMIA {
    string public constant Type = "TSUAN";

    VOID public Void;

    mapping(string => address) internal _libraries;

    constructor(address VoidAddress) DYSNOMIA("Dysnomia Tsuan", "TSUAN", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
    }

    function acceptLibraryUpdate(string calldata lib) public onlyOwners {
        if(_libraries[lib] != address(0x0)) renounceOwnership(_libraries[lib]);
        _libraries[lib] = Void.GetLibraryAddress(lib);
        if(_libraries[lib] != address(0x0)) addOwner(_libraries[lib]);
    }

    function SUN() public view returns (LIBEncrypt) {
        return LIBEncrypt(Void.GetLibraryAddress("encrypt"));
    }
}