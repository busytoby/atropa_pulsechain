// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

library LibRegistry {
    struct Registry {
        uint64[] keys;
        mapping(uint64 => uint64) indexOf;
        mapping(uint64 => bool) inserted;
    }

    function GetHashByIndex(Registry storage _registry, uint64 index) public view returns(uint64) {
        return _registry.keys[index];
    }

    function Count(Registry  storage _registry) public view returns(uint64) {
        return uint64(_registry.keys.length);
    }

    function Contains(Registry storage _registry, uint64 key) public view returns(bool) {
        return _registry.inserted[key];
    }

    function Register(Registry storage _registry, uint64 key) public {
        if(!_registry.inserted[key])
        {
            _registry.inserted[key] = true;
            _registry.indexOf[key] = uint64(_registry.keys.length);
            _registry.keys.push(key);
        }
    }

    function Remove(Registry storage _registry, uint64 key) public {
        if(!_registry.inserted[key]) return;
        delete _registry.inserted[key];
        uint64 index = _registry.indexOf[key];
        uint64 lastKey = _registry.keys[_registry.keys.length - 1];
        _registry.indexOf[lastKey] = index;
        delete _registry.indexOf[key];
        _registry.keys[index] = lastKey;
        _registry.keys.pop();
    }
}