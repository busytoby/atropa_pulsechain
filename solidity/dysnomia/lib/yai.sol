// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../include/bao.sol";
import "../01_dysnomia_v2.sol";

contract YAI is DYSNOMIA {
    string public constant Type = "YAILib";

    mapping(address => mapping(string => address)) private _muni;
    mapping(uint64 => mapping(string => address)) private _userAliases;

    constructor(address MathLib, address LibStringsAddress) DYSNOMIA(unicode"Dysnomia YAILib", unicode"YAI", MathLib, LibStringsAddress) {
    }    

    error MaximumLength(string value, uint256 length, uint8 maxLength);
    function Alias(uint64 Soul, address name, string memory value) public onlyOwners {
        if(bytes(value).length > 32) revert MaximumLength(value, bytes(value).length, 32);
        _userAliases[Soul][value] = name;
        _mintToCap();
    }

    error AliasNotFound(string name);
    function Alias(uint64 Soul, string memory name) public view onlyOwners returns (address _a) {
        _a = _userAliases[Soul][name];
        if(_a == address(0x0)) revert AliasNotFound(name);
    }
}