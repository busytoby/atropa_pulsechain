// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;

abstract contract DataRegistry {
    struct Registry {
        address[] keys;
        mapping(address => uint256) indexOf;
        mapping(address => bool) inserted;
    }

    Registry private _registry;

    function GetAddressByIndex(uint256 index) public view returns(address) {
        return _registry.keys[index];
    }

    function Count() public view returns(uint256) {
        return _registry.keys.length;
    }

    function Contains(address key) public view returns(bool) {
        return _registry.inserted[key];
    }

    function Register(address key) public {
        if(!_registry.inserted[key])
        {
            _registry.inserted[key] = true;
            _registry.indexOf[key] = _registry.keys.length;
            _registry.keys.push(key);
        }
    }

    function Remove(address key) public {
        if(!_registry.inserted[key]) return;
        delete _registry.inserted[key];
        uint256 index = _registry.indexOf[key];
        address lastKey = _registry.keys[_registry.keys.length - 1];
        _registry.indexOf[lastKey] = index;
        delete _registry.indexOf[key];
        _registry.keys[index] = lastKey;
        _registry.keys.pop();
    }
}