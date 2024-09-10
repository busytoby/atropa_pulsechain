// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../interfaces/16b_zaointerface.sol";

contract CONJURELIB is DYSNOMIA {
    string public constant Type = "ConjureLib";

    ZAO public Zao;
    mapping(uint64 => string[2]) private _qi;
    uint64[10] public Levels = [5040, 40320, 362880, 3628800, 39916800, 479001600, 6227020800, 87178291200, 1307674368000, 20922789888000];

    constructor(address ZaoAddress) DYSNOMIA(unicode"DYSNOMIA ConjureLib", unicode"ConjureLib", address(DYSNOMIA(ZaoAddress).Xiao())) {
        Zao = ZAO(ZaoAddress);
        addOwner(ZaoAddress);
        addOwner(tx.origin);
        Zao.VoidQing().Void().AddLibrary("conjure", address(this));


        _mintToCap();
    }

    error NotTokenOwner(address Token, address originRequestor);
    error NotAQiToken(address Token);
    function RenameQi(string memory _name, string memory _symbol) public onlyOwners {
        if(!DYSNOMIA(msg.sender).owner(tx.origin)) revert NotTokenOwner(msg.sender, tx.origin);
        uint64 Soul = QIINTERFACE(msg.sender).Saat(1);
        _qi[Soul][0] = _name;
        _qi[Soul][1] = _symbol;
        _mintToCap();
    }

    function qName() public view returns (string memory) {
        return _qi[QIINTERFACE(msg.sender).Saat(1)][0];
    }

    function qSymbol() public view returns (string memory) {
        return _qi[QIINTERFACE(msg.sender).Saat(1)][1];
    }
}