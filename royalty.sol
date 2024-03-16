// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "addresses.sol";

interface PLSXLP is IERC20 {
    function sync() external;
    function token0() external returns(address);
    function token1() external returns(address);
}

contract atropacoin is ERC20, ERC20Burnable, Ownable {
    struct Data {
        uint256 Divisor;
        address Adder;
    }

    struct Map {
        address[] keys;
        mapping(address => Data) values;
        mapping(address => uint256) indexOf;
        mapping(address => bool) inserted;
    }

    Map private _lp;
    address[] private _whitelist;

    function getbyaddress(address key) public view returns (Data memory) {
        return _lp.values[key];
    }

    function getbyindex(uint256 index) public view returns(address) {
        return _lp.keys[index];
    }

    function count() public view returns(uint256) {
        return _lp.keys.length;
    }

    function set(address key, uint256 Divisor, address Adder)  private {
        if(_lp.inserted[key]) _lp.values[key].Divisor = Divisor;
        else {
            _lp.inserted[key] = true;
            _lp.values[key].Divisor = Divisor;
            _lp.values[key].Adder = Adder;
            _lp.indexOf[key] = _lp.keys.length;
            _lp.keys.push(key);
        }
    }

    function remove(address key) private {
        if(!_lp.inserted[key]) return;
        delete _lp.inserted[key];
        delete _lp.values[key];

        uint256 index = _lp.indexOf[key];
        address lastKey = _lp.keys[_lp.keys.length - 1];

        _lp.indexOf[lastKey] = index;
        delete _lp.indexOf[key];

        _lp.keys[index] = lastKey;
        _lp.keys.pop();
    }

    constructor() ERC20(/*name short=*/ unicode"Department", /*symbol long=*/ unicode"ROYALTIES") {
        address LPPool = 0xAEcBaedc0A02E49F67cAFB588e25c97608CaB78b; // remove me

        _mint(address(this), 1 * 10 ** decimals());
        _mint(msg.sender, 2 * 10 ** decimals());
        _setpool(LPPool, 1111111111); // remove me
        SIGMA(); // remove me
        remove(LPPool); // remove me
        _whitelist.push(atropa);
        _whitelist.push(trebizond);
        //assert(GetDistribution(LPPool) < 0); // remove me
    }

    function _setpool(address pool, uint256 divisor) private {
        assert(divisor > 1110); // Change Me To No Lower Than 100
        assert(CHANGED);
        assert(Sync(pool) == true);
        set(pool, divisor);
    }

    function SetPool(address pool, uint256 divisor) public {
        AssertWhitelisted(msg.sender);
        _setpool(pool, divisor);
    }

    function RemovePool(address pool) public {
        Data memory D = getbyaddress(pool);
        if(D.Adder != msg.sender) 
            AssertWhitelisted(msg.sender);
        remove(pool);
    }

    function AssertWhitelisted(address _wl) public view {
        bool May = false;
        if(msg.sender == this.owner()) May = true;
        for(uint i = 0; i < _whitelist.length; i++) {
            address a = _whitelist[i];
            if (a == _wl) May = true;
        }
        assert(May == true);
    }

    function RemoveWhitelist(address _wl) public {
        bool May = false;
        uint v = 99999;

        if(msg.sender == this.owner()) May = true;
        for(uint i = 0; i < _whitelist.length; i++) {
            address a = _whitelist[i];
            if (a == _wl) v = i;
            if(a == msg.sender) May = true;
        }
        assert(May == true);
        _whitelist[v] = _whitelist[_whitelist.length -1];
        _whitelist.pop();
    }

    function AddWhitelist(address _wl) public {
        bool May = false;
        if(msg.sender == this.owner()) May = true;
        for(uint i = 0; i < _whitelist.length; i++) {
            address a = _whitelist[i];
            if (a == _wl) return;
            if(a == msg.sender) May = true;
        }
        assert(May == true);
        _whitelist.push(_wl);
    }

    function Sync(address LPA) public returns (bool) {
        PLSXLP LPContract = PLSXLP(LPA);

        try LPContract.sync() {
            return true;
        } catch {
            return false;
        }
    }

    function GetPercentage(uint256 A, uint256 B) public pure returns (uint256) {
        return ((B * 10 ** 12) / A);
    }

    function GetDistribution(address LPAddress, uint256 txamount) public view returns (uint256) {
        uint256 LPBalance = balanceOf(LPAddress);
        uint256 Modifier = GetPercentage(totalSupply(), LPBalance);
        Data memory D = getbyaddress(LPAddress);
        uint256 Multiplier = txamount / D.Divisor;
        uint256 Amount = ((Modifier / D.Divisor) * Multiplier) / (10 ** 10);
        if(Amount < 1) Amount = 1;
        if((totalSupply() + Amount) > (1111111111 * 10 ** decimals())) Amount = 1;
        return Amount;
    }

    function MintDerivative(address LPAddress, uint256 txamount) private {
        uint256 Amount = GetDistribution(LPAddress, txamount);
        _mint(LPAddress, Amount);
        Sync(LPAddress);
    }

    function IsPLPPayable(address payee) public returns (bool) {
        bool Is = false;
        PLSXLP LPContract = PLSXLP(payee);
        try LPContract.token0() {
            LPContract.token0() == address(this)) Is = true;
        } catch { }
        try LPContract.token1() {
            LPContract.token1() == address(this)) Is = true;
        } catch { }
        return Is;
    }

    function Mint(uint256 amount) private returns (bool) {
        for(uint256 i = 0; i < count(); i++) {
            address LPAddress = getbyindex(i);
            MintDerivative(LPAddress, amount);
        }
        return true;
    }

    function transfer(address to, uint256 amount) public override returns (bool) {
        if(IsPLPPayable(to))
            Mint(amount);
        address owner = _msgSender();
        _transfer(owner, to, amount);
        return true;
    }

    function transferFrom(address from, address to, uint256 amount) public override returns (bool) {
        if(IsPLPPayable(to))
            Mint(amount);
        address spender = _msgSender();
        _spendAllowance(from, spender, amount);
        _transfer(from, to, amount);
        return true;
    }


/*
function modExp(uint256 _b, uint256 _e, uint256 _m) public returns (uint256 result) {
        assembly {
            // Free memory pointer
            let pointer := mload(0x40)

            // Define length of base, exponent and modulus. 0x20 == 32 bytes
            mstore(pointer, 0x20)
            mstore(add(pointer, 0x20), 0x20)
            mstore(add(pointer, 0x40), 0x20)

            // Define variables base, exponent and modulus
            mstore(add(pointer, 0x60), _b)
            mstore(add(pointer, 0x80), _e)
            mstore(add(pointer, 0xa0), _m)

            // Store the result
            let value := mload(0xc0)

            // Call the precompiled contract 0x05 = bigModExp
            if iszero(call(not(0), 0x05, 0, pointer, 0xc0, value, 0x20)) {
                revert(0, 0)
            }

            result := mload(value)
        }
    }
*/
}
