// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../interfaces/16b_zaointerface.sol";

contract CONJURELIB is DYSNOMIA {
    string public constant Type = "ConjureLib";

    ZAO public Zao;
    mapping(uint64 => string[2]) private _qi;

    constructor(address ZaoAddress) DYSNOMIA(unicode"CHATLOG LibCrypt", unicode"LibCrypt", address(DYSNOMIA(ZaoAddress).Xiao())) {
        Zao = ZAO(ZaoAddress);
        addOwner(ZaoAddress);
        Zao.VoidQing().Void().AddLibrary("conjure", address(this));
        _mintToCap();
    }

    error NotTokenOwner(address Token, address originRequestor);
    error NotAQiToken(address Token);
    function Rename(uint64 _c, string memory _name, string memory _symbol) public onlyOwners {
        if(!DYSNOMIA(msg.sender).owner(tx.origin)) revert NotTokenOwner(msg.sender, tx.origin);
        _qi[_c][0] = _name;
        _qi[_c][1] = _symbol;
    }

    function name(uint64 _c) public view returns (string memory) {
        return _qi[_c][0];
    }

    function symbol(uint64 _c) public view returns (string memory) {
        return _qi[_c][1];
    }
}