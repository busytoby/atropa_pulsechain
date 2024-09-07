// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/bao.sol";
import "../13_qing.sol"; // CHANGEME To Interface

contract YAI is DYSNOMIA {
    string public constant Type = "YAILib";

    VOID public Void;
    mapping(uint64 => mapping(string => address)) private _muni;
    mapping(uint64 => string[]) private _aliases;
    mapping(address => bool) private _forbidden;

    constructor(address VoidAddress) DYSNOMIA(unicode"Dysnomia YAILib", unicode"YAI", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        Void.AddLibrary("yai", address(this));
    }

    error NotOwner(address what, address who);
    function Forbid(address what) public {
        if(has(what, "owner()") && DYSNOMIA(what).owner() == tx.origin) _forbidden[what] = true;
        else if(DYSNOMIA(what).owner(tx.origin)) _forbidden[what] = true;
        else revert NotOwner(what, tx.origin);
    }

    function Unforbid(address what) public {
        if(has(what, "owner()") && DYSNOMIA(what).owner() == tx.origin) _forbidden[what] = false;
        else if(DYSNOMIA(what).owner(tx.origin)) _forbidden[what] = false;
        else revert NotOwner(what, tx.origin);
    }

    function IsForbidden(address Token) public view returns (bool) {
        return _forbidden[Token];
    }

    function GetAliasCount(uint64 Soul) public view returns (uint256) {
        return _aliases[Soul].length;
    }

    function GetAlias(uint64 Soul, uint256 idx) public view returns (string memory) {
        return _aliases[Soul][idx];
    }

    function Remove(uint64 Soul, string memory name) public onlyOwners {
        uint64 idx = getaliasidx(Soul, name);
        delete _muni[Soul][name];
        _aliases[Soul][idx] = _aliases[Soul][_aliases[Soul].length - 1];
        _aliases[Soul].pop();
    }

    function getaliasidx(uint64 Soul, string memory name) internal view returns (uint64 idx) {
        bytes memory vB = bytes(name);
        for(uint64 i=0; i<_aliases[Soul].length; i++) {
            bytes memory aB = bytes(_aliases[Soul][i]);
            if(aB.length != vB.length) continue;
            for(uint64 j=0; j < vB.length; j++)
                if(aB[j] != vB[j]) break;
                else if (j == vB.length - 1) return i;
        }
        assert(false);
    }

    error MaximumLength(string name, uint256 length, uint8 maxLength);
    error AlreadyConnected(address what);
    function Alias(uint64 Soul, string memory name, address token) public onlyOwners {
        if(IsForbidden(token)) revert NotOwner(token, tx.origin);
        if(bytes(name).length > 16) revert MaximumLength(name, bytes(name).length, 16);
        if(_muni[Soul][name] != address(0x0)) revert AlreadyConnected(_muni[Soul][name]);
        _muni[Soul][name] = token;
        _aliases[Soul].push(name);
        _mintToCap();
    }

    error AliasNotFound(string name);
    function Alias(uint64 Soul, string memory name) public view returns (address _a) {
        _a = _muni[Soul][name];
        if(IsForbidden(_a)) revert NotOwner(_a, tx.origin);
        if(_a == address(0x0)) revert AliasNotFound(name);
    }
}