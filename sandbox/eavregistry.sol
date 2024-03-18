// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;
import "sandbox/eav.sol";

abstract contract EAVRegistry {
    using EAV for EAV.Data;
    
    struct Registry {
        uint256[] keys;
        mapping(uint256 => EAV.Data) values;
        mapping(uint256 => uint256) indexOf;
        mapping(uint256 => bool) inserted;
    }

    Registry internal _registry;

    function GetKeyByIndex(uint256 index) public view returns(uint256) {
        return _registry.keys[index];
    }

    function Count() public view returns(uint256) {
        return _registry.keys.length;
    }

    function Contains(uint256 key) public view returns(bool) {
        return _registry.inserted[key];
    }

    function Add(address host, string memory key, EAV.Data memory value) public {
        uint256 addKey = uint256(keccak256(abi.encodePacked(host, key)));
        Add(addKey, value);
    }

    function Add(uint256 key, EAV.Data memory value) public {
        if(!_registry.inserted[key])
        {
            _registry.inserted[key] = true;
            _registry.values[key] = value;
            _registry.indexOf[key] = _registry.keys.length;
            _registry.keys.push(key);
        }
    }

    function Remove(address host, string memory key) public {
        uint256 addKey = uint256(keccak256(abi.encodePacked(host, key)));
        Remove(addKey);
    }

    function Remove(uint256 key) public {
        if(!_registry.inserted[key]) return;
        delete _registry.inserted[key];
        uint256 index = _registry.indexOf[key];
        uint256 lastKey = _registry.keys[_registry.keys.length - 1];
        _registry.indexOf[lastKey] = index;
        delete _registry.indexOf[key];
        delete _registry.values[key];
        _registry.keys[index] = lastKey;
        _registry.keys.pop();
    }
}