// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;
import "atropamath.sol";

library LibRegistry {
    struct Registry {
        uint256[] keys;
        mapping(uint256 => uint256) indexOf;
        mapping(uint256 => bool) inserted;
    }

    function GetHashByIndex(Registry storage _registry, uint256 index) public view returns(uint256) {
        return _registry.keys[index];
    }

    function Count(Registry  storage _registry) public view returns(uint256) {
        return _registry.keys.length;
    }

    function Contains(Registry storage _registry, uint256 key) public view returns(bool) {
        return _registry.inserted[key];
    }

    function Register(Registry storage _registry, uint256 key) public {
        if(!_registry.inserted[key])
        {
            _registry.inserted[key] = true;
            _registry.indexOf[key] = _registry.keys.length;
            _registry.keys.push(key);
        }
    }

    function Remove(Registry storage _registry, uint256 key) public {
        if(!_registry.inserted[key]) return;
        delete _registry.inserted[key];
        uint256 index = _registry.indexOf[key];
        uint256 lastKey = _registry.keys[_registry.keys.length - 1];
        _registry.indexOf[lastKey] = index;
        delete _registry.indexOf[key];
        _registry.keys[index] = lastKey;
        _registry.keys.pop();
    }
}