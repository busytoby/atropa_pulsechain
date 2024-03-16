// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "asset.sol";
import "whitelist.sol";

abstract contract Incorporation is ERC20, ERC20Burnable, Ownable, Asset, Whitelist {
    enum Type {
        COMMODITY,
        HEDGE,
        SUBSIDY
    }

    struct Article {
        uint256 Divisor;
        address Adder;
        uint256 Expiration;
    }

    struct Map {
        address[] keys;
        mapping(address => Article) values;
        mapping(address => uint256) indexOf;
        mapping(address => bool) inserted;
    }
    Map private _registry;
    uint256 public minDivisor = 1110;
    Type public Class;


    function getbyaddress(address key) public view returns (Article memory) {
        return _registry.values[key];
    }

    function getbyindex(uint256 index) public view returns(address) {
        return _registry.keys[index];
    }

    function count() public view returns(uint256) {
        return _registry.keys.length;
    }

    function registered(address key) public view returns(bool) {
        return _registry.inserted[key];
    }

    function expired(address key) public view returns(bool) {
        return (block.timestamp > _registry.values[key].Expiration);
    }

    function set(address key, uint256 Divisor, address Adder, uint256 Length) private {
        if(_registry.inserted[key]) _registry.values[key].Divisor = Divisor;
        else {
            _registry.inserted[key] = true;
            _registry.values[key].Divisor = Divisor;
            _registry.values[key].Adder = Adder;
            _registry.values[key].Expiration = block.timestamp + Length;
            _registry.indexOf[key] = _registry.keys.length;
            _registry.keys.push(key);
        }
    }

    function remove(address key) public {
        Incorporation.Article memory A = getbyaddress(key);
        if(A.Adder != msg.sender) 
            Whitelist.Assert(msg.sender);
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

    function register(address pool, uint256 divisor, address registree, uint256 length) public {
        Whitelist.Assert(msg.sender);
        assert(divisor > minDivisor);
        assert(Asset.Sync(pool) == true);
        set(pool, divisor, registree, length * 1 weeks);
    }

    function(uint256) internal returns (bool) Mint;

    function transfer(address to, uint256 amount) public override returns (bool) {
        address owner = _msgSender();
        if(!(Class == Type.SUBSIDY))
            if(Incorporation.registered(to) || Incorporation.registered(owner))
                Mint(amount);
        _transfer(owner, to, amount);
        return true;
    }

    function transferFrom(address from, address to, uint256 amount) public override returns (bool) {
        address spender = _msgSender();
        if(!(Class == Type.HEDGE))
            if(Incorporation.registered(from) || Incorporation.registered(to))
                Mint(amount);
        _spendAllowance(from, spender, amount);
        _transfer(from, to, amount);
        return true;
    }
}
