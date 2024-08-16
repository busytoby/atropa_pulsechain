// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia.sol";
import "../interfaces/10b_voidinterface.sol";

contract ATTRIBUTE is DYSNOMIA {
    string public constant Type = "LibAttribute";

    VOID public Void;
    mapping(string => uint8) private _attributes;
    mapping(uint64 => mapping(string => string)) private _userAttributes;


    constructor(address VoidAddress) DYSNOMIA(unicode"VM LibAttribute", unicode"LibAttribute", address(DYSNOMIA(VoidAddress).Xiao()), 666) MultiOwnable(msg.sender) {
        Void = VOID(VoidAddress);
        addOwner(VoidAddress);
        Void.AddLibrary("libattribute", address(this));
        addAttribute("Username", 12);
        addAttribute("TestAttribute", 140);
        mintToCap();
    }

    function addAttribute(string memory name, uint8 maxLength) public onlyOwners {
        _attributes[name] = maxLength;
        mintToCap();
    }

    function removeAttribute(string memory name) public onlyOwners {
        _attributes[name] = 0;
        mintToCap();
    }

    function Set(uint64 Soul, string memory name, string memory value) public onlyOwners {
        assert(bytes(value).length < _attributes[name]);
        _userAttributes[Soul][name] = value;
        mintToCap();
    }

    function Get(uint64 Soul, string memory name) public view onlyOwners returns (string memory _a) {
        _a = _userAttributes[Soul][name];
        assert(bytes(_a).length > 0);
    }
}
