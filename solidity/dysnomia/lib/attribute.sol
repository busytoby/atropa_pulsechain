// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia.sol";
import "../interfaces/10b_voidinterface.sol";

contract ATTRIBUTE is DYSNOMIA {
    string public constant Type = "LibAttribute";

    VOID public Void;
    mapping(string => uint8) private _attributes;
    mapping(uint64 => mapping(string => string)) private _userAttributes;
    mapping(uint64 => mapping(address => string)) private _userGrades;

    constructor(address VoidAddress) DYSNOMIA(unicode"CHATLOG LibAttribute", unicode"LibAttribute", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        addOwner(VoidAddress);
        Void.AddLibrary("libattribute", address(this));
        addAttribute("Username", 12);
        addAttribute("TestAttribute", 140);
        _mintToCap();
        _mint(tx.origin, 1 * 10 ** decimals());
    }

    function addAttribute(string memory name, uint8 maxLength) public onlyOwners {
        _attributes[name] = maxLength;
        _mintToCap();
    }

    function removeAttribute(string memory name) public onlyOwners {
        _attributes[name] = 0;
        _mintToCap();
    }

    error MaximumLength(string value, uint256 length, uint8 maxLength);
    function Set(uint64 Soul, string memory name, string memory value) public onlyOwners {
        if(bytes(value).length > _attributes[name]) revert MaximumLength(value, bytes(value).length, _attributes[name]);
        _userAttributes[Soul][name] = value;
        _mintToCap();
    }

    error AttributeNotFound(string name);
    function Get(uint64 Soul, string memory name) public view onlyOwners returns (string memory _a) {
        _a = _userAttributes[Soul][name];
        if(bytes(_a).length < 1) revert AttributeNotFound(name);
    }

    function Alias(uint64 Soul, address name, string memory value) public onlyOwners {
        if(bytes(value).length > 32) revert MaximumLength(value, bytes(value).length, 32);
        _userGrades[Soul][name] = value;
        _mintToCap();
    }

    error AliasNotFound(address name);
    function Alias(uint64 Soul, address name) public view onlyOwners returns (string memory _a) {
        _a = _userGrades[Soul][name];
        if(bytes(_a).length < 1) revert AliasNotFound(name);
    }

    function Alias(uint64 Soul, Bao memory entity, string memory value) public onlyOwners {
        if(bytes(value).length > 32) revert MaximumLength(value, bytes(value).length, 32);
        _userGrades[Soul][entity.Phi] = value;
        _mintToCap();
    }

    function Alias(uint64 Soul, Bao memory entity) public view onlyOwners returns (string memory _a) {
        _a = _userGrades[Soul][entity.Phi];
        if(bytes(_a).length < 1) revert AliasNotFound(entity.Phi);
    }
}
