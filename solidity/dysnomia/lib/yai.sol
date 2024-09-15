// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../interfaces/13b_qinginterface.sol";

contract YAI is DYSNOMIA {
    string public constant Type = "YAILib";

    VOID public Void;
    mapping(uint256 => mapping(string => address)) private _muni;
    mapping(uint256 => string[]) private _aliases;
    mapping(address => bool) private _forbidden;

    constructor(address VoidAddress) DYSNOMIA(unicode"Dysnomia YAILib", unicode"YAI", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        addOwner(tx.origin);
        Void.AddLibrary("yai", address(this));
        _mintToCap();
    }

    function has(address _contract, string memory what) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes(what)));
        bytes memory data = abi.encodeWithSelector(selector);
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
    }

    error NotOwner(address what, address who);
    function Forbid(address what) public {
        if(has(what, "owner()") && DYSNOMIA(what).owner() == tx.origin) _forbidden[what] = true;
        else if(DYSNOMIA(what).owner(tx.origin)) _forbidden[what] = true;
        else revert NotOwner(what, tx.origin);
        _mintToCap();
    }

    function Unforbid(address what) public {
        if(has(what, "owner()") && DYSNOMIA(what).owner() == tx.origin) _forbidden[what] = false;
        else if(DYSNOMIA(what).owner(tx.origin)) _forbidden[what] = false;
        else revert NotOwner(what, tx.origin);
        _mintToCap();
    }

    function IsForbidden(address Token) public view returns (bool) {
        return _forbidden[Token];
    }

    function GetAliasCount(uint256 Waat) public view returns (uint256) {
        return _aliases[Waat].length;
    }

    function GetAlias(uint256 Waat, uint256 idx) public view returns (string memory) {
        return _aliases[Waat][idx];
    }

    function Remove(uint256 Waat, string memory name) public onlyOwners {
        uint256 idx = getaliasidx(Waat, name);
        delete _muni[Waat][name];
        _aliases[Waat][idx] = _aliases[Waat][_aliases[Waat].length - 1];
        _aliases[Waat].pop();
        _mintToCap();
    }

    function getaliasidx(uint256 Waat, string memory name) internal view returns (uint64 idx) {
        bytes memory vB = bytes(name);
        for(uint64 i=0; i<_aliases[Waat].length; i++) {
            bytes memory aB = bytes(_aliases[Waat][i]);
            if(aB.length != vB.length) continue;
            for(uint64 j=0; j < vB.length; j++)
                if(aB[j] != vB[j]) break;
                else if (j == vB.length - 1) return i;
        }
        assert(false);
    }

    error MaximumLength(string name, uint256 length, uint8 maxLength);
    error AlreadyConnected(address what);
    function Alias(uint256 Waat, string memory name, address token) public onlyOwners {
        if(IsForbidden(token)) revert NotOwner(token, tx.origin);
        if(bytes(name).length > 16) revert MaximumLength(name, bytes(name).length, 16);
        if(_muni[Waat][name] != address(0x0)) revert AlreadyConnected(_muni[Waat][name]);
        _muni[Waat][name] = token;
        _aliases[Waat].push(name);
        _mintToCap();
    }

    error AliasNotFound(string name);
    function Alias(uint256 Waat, string memory name) public view returns (address _a) {
        _a = _muni[Waat][name];
        if(IsForbidden(_a)) revert NotOwner(_a, tx.origin);
        if(_a == address(0x0)) revert AliasNotFound(name);
    }
}