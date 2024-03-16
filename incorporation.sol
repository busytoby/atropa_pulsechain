// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "addresses.sol";
import "asset.sol";
import "whitelist.sol";

abstract contract Incorporation is ERC20, ERC20Burnable, Ownable, Asset {
    enum Type {
        COMMODITY,
        HEDGE,
        SUBSIDY,
        OPTION,
        EXCHANGE
    }

    struct Article {
        uint256 Divisor;
        address Adder;
        uint256 Expiration;
        Type Class;
    }

    struct Map {
        address[] keys;
        mapping(address => Article) values;
        mapping(address => uint256) indexOf;
        mapping(address => bool) inserted;
    }
    Map internal _registry;
    uint256 internal minDivisor = 1110;
    Type immutable internal AssetClass;

    function(address) internal AssertAccess;
    function(uint256, Type) internal returns (bool) Disbersement;

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

    function IsClass(address key, Type class) public view returns(bool) {
        return _registry.values[key].Class == class;
    }

    function set(address key, uint256 Divisor, address Adder, uint256 Length, Type Class) private {
        assert(Class == Type.COMMODITY || Class == Type.OPTION || Class == Type.EXCHANGE);
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
        Incorporation.Article memory A = GetArticleByAddress(key);
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

    function Register(address pool, uint256 divisor, address registree, uint256 length, Type class) public {
        assert(length < 367);
        AssertAccess(msg.sender);
        assert(divisor > minDivisor);
        if(class != Type.EXCHANGE)
            assert(Asset.Sync(pool) == true);
        set(pool, divisor, registree, length * 1 days, class);
    }

    function transfer(address to, uint256 amount) public override returns (bool) {
        address owner = _msgSender();
        if(!(AssetClass == Type.SUBSIDY))
            if(Incorporation.Registered(to) || Incorporation.Registered(owner))
                Disbersement(amount, Type.COMMODITY);
        if(Incorporation.Registered(to) && Incorporation.Registered(owner) && (IsClass(to, Type.EXCHANGE)))
                Disbersement(amount, Type.OPTION);
        _transfer(owner, to, amount);
        return true;
    }

    function transferFrom(address from, address to, uint256 amount) public override returns (bool) {
        address spender = _msgSender();
        if(!(AssetClass == Type.HEDGE))
            if(Incorporation.Registered(from) || Incorporation.Registered(to))
                Disbersement(amount, Type.COMMODITY);
        if(Incorporation.Registered(from) && Incorporation.Registered(to) && (IsClass(from, Type.EXCHANGE)))
            Disbersement(amount, Type.OPTION);
        _spendAllowance(from, spender, amount);
        _transfer(from, to, amount);
        return true;
    }
}
