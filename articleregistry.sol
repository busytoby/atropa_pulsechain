// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;

abstract contract ArticleRegistry {
    enum IncorporationType {
        COMMODITY,
        HEDGE,
        SUBSIDY,
        OPTION,
        EXCHANGE,
        FUTURE,
        CAP,
        CLIMA,
        OFFSET
    }

    struct Article {
        uint256 Divisor;
        address Adder;
        uint256 Expiration;
        IncorporationType Class;
    }

    struct Registry {
        address[] keys;
        mapping(address => Article) values;
        mapping(address => uint256) indexOf;
        mapping(address => bool) inserted;
    }

    Registry internal _registry;
    function Register(address pool, uint256 divisor, address registree, uint256 length, IncorporationType class) public virtual;
    function(address) internal AssertAccess;

    function GetArticleByAddress(address key) public view returns (Article memory) {
        return _registry.values[key];
    }

    function GetAddressByIndex(uint256 index) public view returns(address) {
        return _registry.keys[index];
    }

    function RegistryCount() public view returns(uint256) {
        return _registry.keys.length;
    }

    function Registered(address key) public view returns(bool) {
        return _registry.inserted[key];
    }

    function Expired(address key) public view returns(bool) {
        return (block.timestamp > _registry.values[key].Expiration);
    }

    function IsClass(address key, IncorporationType class) public view returns(bool) {
        return _registry.values[key].Class == class;
    }

    function set(address key, uint256 Divisor, address Adder, uint256 Length, IncorporationType Class) internal {
        assert(Class == IncorporationType.COMMODITY || Class == IncorporationType.OPTION || Class == IncorporationType.EXCHANGE);
        if(_registry.inserted[key]) _registry.values[key].Divisor = Divisor;
        else {
            _registry.inserted[key] = true;
            _registry.values[key].Divisor = Divisor;
            _registry.values[key].Adder = Adder;
            _registry.values[key].Expiration = block.timestamp + Length;
            _registry.values[key].Class = Class;
            _registry.indexOf[key] = _registry.keys.length;
            _registry.keys.push(key);
        }
    }

    function Deregister(address key) public {
        Article memory A = GetArticleByAddress(key);
        if(A.Adder != msg.sender) 
            AssertAccess(msg.sender);
        if(!_registry.inserted[key]) return;
        delete _registry.inserted[key];
        delete _registry.values[key];

        uint256 index = _registry.indexOf[key];
        address lastKey = _registry.keys[_registry.keys.length - 1];

        _registry.indexOf[lastKey] = index;
        delete _registry.indexOf[key];

        _registry.keys[index] = lastKey;
        _registry.keys.pop();
    }
}