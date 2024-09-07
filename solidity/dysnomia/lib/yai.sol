// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/bao.sol";
import "../13_qing.sol"; // CHANGEME To Interface

contract YAI is DYSNOMIA {
    string public constant Type = "YAILib";

    mapping(uint64 => mapping(string => address)) private _muni;
    mapping(uint64 => string[]) private _aliases;

    constructor(address QingAddress) DYSNOMIA(unicode"Dysnomia YAILib", unicode"YAI", address(DYSNOMIA(QingAddress).Xiao())) {
        addOwner(QingAddress);
        VOID Void = QING(QingAddress).Void();
        Void.AddLibrary("yai", address(this));
    }

    function GetAliases(uint64 Soul) public view returns (string[] memory) {
        return _aliases[Soul]; // might need start & offset parameters
    }

    function Remove(uint64 Soul, string memory value) public onlyOwners {
        uint64 idx = getaliasidx(Soul, value);
        delete _muni[Soul][value];
        _aliases[Soul][idx] = _aliases[Soul][_aliases[Soul].length - 1];
        _aliases[Soul].pop();
    }

    function getaliasidx(uint64 Soul, string memory value) internal view returns (uint64 idx) {
        bytes memory vB = bytes(value);
        for(uint64 i=0; i<_aliases[Soul].length; i++) {
            bytes memory aB = bytes(_aliases[Soul][i]);
            if(aB.length != vB.length) continue;
            for(uint64 j=0; j < vB.length; j++)
                if(aB[j] != vB[j]) break;
                else if (j == vB.length - 1) return i;
        }
        assert(false);
    }

    error MaximumLength(string value, uint256 length, uint8 maxLength);
    function Alias(uint64 Soul, address name, string memory value) public onlyOwners {
        if(bytes(value).length > 32) revert MaximumLength(value, bytes(value).length, 32);
        _muni[Soul][value] = name;
        _mintToCap();
    }

    error AliasNotFound(string name);
    function Alias(uint64 Soul, string memory name) public view onlyOwners returns (address _a) {
        _a = _muni[Soul][name];
        if(_a == address(0x0)) revert AliasNotFound(name);
    }
}